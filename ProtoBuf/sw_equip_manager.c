#include <arpa/inet.h>
#include "swapi.h"
#include "swlog.h"
#include "swparameter.h"
#include "swgethostbyname.h"
#include "swtxtparser.h"
#include "cJSON.h"
#include "swurl.h"
#include "sw_http.h"
#include "mojing_control.pb-c.h"
#include "swthrd.h"
#include "ssl_mbedtls.h"
#include "swhashmap.h"
#include "swmutex.h"
#include "MOJING_IPCManager.h"

#define SW_EQUIP_MANAGER_DEBUG( format, ... ) 	    sw_log( LOG_LEVEL_INFO, "SW_EQUIP_MANAGER_DEBUG",__func__,__LINE__, format, ##__VA_ARGS__  )

#define DEFAULT_DGW_URL "https://open.mj.sctel.com.cn:2995"
#define DEFAULT_FIRMWARE_VERSION "V1.11"
#define DEFAULT_FIRMWARE_MD5 ""
#define DEFAULT_SOFTWARE_VERSION "V1.11"
#define DEFAULT_SOFTWARE_MD5 ""
#define DEFAULT_KEEPALIVE_INTERVAL 30
#define KEEPALIVE_SEND_MESSAGE_CODE 1006
#define HASHMAP_NUM  16
#define NULL 0
static swhashmap_t * m_hashmap = NULL;
static int m_keepalive_interval = DEFAULT_KEEPALIVE_INTERVAL;
static void *m_tls_client = NULL;

//是否有数据要发送
static bool m_need_send = false;

#define MAX_PROTOBUF_LENGTH 1 * 1024
#define TCP_HEADER_LEN 9
#define MV 0x4D56
#define V 0x01

typedef struct _mojing_tcp_info{
    uint16_t check_digit;
    uint8_t version_num;
    uint16_t seq_id;
    uint16_t message_type;
    uint16_t protobuf_len;
    unsigned char protobuf[MAX_PROTOBUF_LENGTH];
}sw_mojing_tcp_info_t;

static int m_timeout = 12000;
static char m_dsgw_url[256] = {0};
static char m_dsgw_ip[16] = {0};
static int m_dsgw_port = 0;
static HANDLE m_equip_control_communication_thrd = NULL;
static HANDLE m_equip_control_active_thrd = NULL;
static tls_client *m_tls_dsgw_client = NULL;

/*
 * @ingroup cwmp_method
 * @brief 功能函数指针
 *
 * @param message_type - 消息类型
 * @param protobuf - 消息结构体或者 sw_mojing_tcp_info_t
 *
 * @return 0 - success, other - errorcode
 */
typedef int (* sw_equip_manager_request_func_handle)(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer);

/*
 * @ingroup sw_equip_method
 * @brief initialize  "SetParameterValues"、"GetParameterValues" method hash
 *
 * @return 0 - success, other - errorcode
 */
int sw_equip_pack_and_send_or_unpack_and_callback_method_func_init()
{
	if (NULL != m_hashmap)
		return 0;

	/** create hashmap */
	m_hashmap =  sw_hashmap_create(HASHMAP_NUM, KEY_STRING);
	if (NULL != m_hashmap)
	{
		SW_EQUIP_MANAGER_DEBUG("---------Creat equipment function hashmap %d members Success!\n", HASHMAP_NUM);
		return 0;
	}
	else
	{
		SW_EQUIP_MANAGER_DEBUG("---------Creat equipment function hashmap Failed!--------\n");
		return -1;
	}
}

/*
 * @ingroup sw_equip_method
 * @brief destory  "SetParameterValues"、"GetParameterValues" method hash
 *
 * @return 0 - success, other - errorcode
 */
void sw_equip_pack_and_send_or_unpack_and_callback_method_func_exit()
{
	if (NULL != m_hashmap)
	{
		sw_hashmap_destroy(m_hashmap);
		m_hashmap = NULL;
	}
}

/*
 * @ingroup sw_equip_method
 * @brief initialize  "SetParameterValues"、"GetParameterValues" method hash
 *
 * @return 0 - success, other - errorcode
 */
void sw_equip_pack_and_send_or_unpack_and_callback_method_func_add(char *message_type_buf, sw_equip_manager_request_func_handle func)
{
	if (NULL != m_hashmap)
		sw_equip_pack_and_send_or_unpack_and_callback_method_func_init();

	sw_hashmap_put(m_hashmap, (void *)message_type_buf, (void *)(sw_equip_manager_request_func_handle)func);
}

/*
 * @ingroup sw_equip_method
 * @brief 功能函数
 *
 * @return  0 - success, other - errorcode
 */
int  sw_equip_pack_and_send_or_unpack_and_callback_method_func(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
	sw_equip_manager_request_func_handle func = NULL;
    int ret = -1;
    char message_type_buf[16] ={0};


	if (NULL == message_struct_buffer)
	{
		SW_EQUIP_MANAGER_DEBUG("message_struct_buffer is NULL!\n");
		return -1;
	}
    snprintf(message_type_buf, sizeof(message_type_buf) - 1, "%d", message_type);
	if (0 == sw_hashmap_get(m_hashmap, message_type_buf, (void **)&func))
	{
        ret = func(message_type, message_struct_buffer);
	}
	else
	{
		SW_EQUIP_MANAGER_DEBUG("can't find func!\n");
		ret = -1;
	}
	return ret;
}

/*
 * @ingroup 魔镜设备管理
 * @brief protobuf组包
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_tcp_pack(int message_type, uint8_t *protobuf, unsigned long protobuf_length, int seq_id)
{
    int send_len = 0;
    int tcp_buffer_len = 0;

    if(NULL == protobuf || protobuf_length > sizeof(sw_mojing_tcp_info_t) -1)
    {
       SW_EQUIP_MANAGER_DEBUG("protobuf is NULL:%p or tcp_buf_length is too larger:%lu\n", protobuf, protobuf_length);
       return -1;
    }

    sw_mojing_tcp_info_t tcp_buffer;

    memset(&tcp_buffer, 0, sizeof(sw_mojing_tcp_info_t));

    tcp_buffer.check_digit = MV;
    tcp_buffer.version_num = V;
    tcp_buffer.seq_id = seq_id;
    tcp_buffer.protobuf_len = protobuf_length;
    tcp_buffer.message_type = message_type;
    //二进制数据，不能使用字符类型函数
    memcpy(tcp_buffer.protobuf, protobuf, protobuf_length);
    tcp_buffer_len = protobuf_length + TCP_HEADER_LEN;


    if(0 != sw_tls_write(m_tls_client, (const unsigned char *)&tcp_buffer, tcp_buffer_len, &send_len))
        SW_EQUIP_MANAGER_DEBUG("[%s]send failed\n", __FUNCTION__);

    SW_EQUIP_MANAGER_DEBUG("send size:%d\n", send_len);


    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief protobuf组包单独列一个函数，防止后期增加更改协议增加验证等
 *
 * @param tcp_buf - sw_mojing_tcp_info_t数据
 * @param tcp_buf_length - tcp_buf 数据长度
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_tcp_unpack(sw_mojing_tcp_info_t * tcp_buf, int tcp_buf_length)
{
    int ret = 0;
    int message_type = tcp_buf->message_type;
    /* int protobuf_len = tcp_buf->protobuf_len; */
    /* int check_digit = tcp_buf->check_digit; */
    /* int seq_id = tcp_buf->seq_id; */
    /* int version_num = tcp_buf->version_num; */

    ret = sw_equip_pack_and_send_or_unpack_and_callback_method_func(message_type, (void *)tcp_buf);
    if(0 != ret)
    {
        SW_EQUIP_MANAGER_DEBUG("sw_equip_manager_tcp_unpack  error!\n");
        return -1;
    }

    return 0;
}


DSGW_response_callback sw_equip_manager_register_response = NULL;

/*
 * @ingroup 魔镜设备管理
 * @brief 注册
 *
 * @param message_struct_buffer - 消息结构体类型
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_register(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    static unsigned long m_seq_id = 0;
    static char m_device_id[32] = {0};
    static int64_t m_register_time = 0;

    if(1000 == message_type)
    {//发送没带认证的register
        int protobuf_pack_length = 0;
        uint8_t *protobuf_buffer = NULL;
        //将参数中的指针转换成对应的结构体
        Register_Request_t *register_request = (Register_Request_t *)sw_mojing_tcp_info_t_or_message_struct_buffer;

        //读取传递过来的数据并赋值到对应的protobuf结构体中, protobuf中没有数组，只有指针
        SwMojingControl__RegisterRequest register_request_portobuf_struct = SW_MOJING_CONTROL__REGISTER_REQUEST__INIT; 
        register_request_portobuf_struct.deviceid = register_request->DeviceID;
        register_request_portobuf_struct.registertime = register_request->RegisterTime;

        //保存注册信息待认证注册使用
        strlcpy(device_id, register_request->DeviceID, sizeof(device_id));
        register_time = register_request->RegisterTime;
        //pack
        protobuf_pack_length = sw_mojing_control__register_request__get_packed_size(&register_request_portobuf_struct);
        SW_EQUIP_MANAGER_DEBUG("register_request protobuf pack length:%d\n", protobuf_pack_length);
        protobuf_buffer = (uint8_t *)malloc(protobuf_pack_length);
        memset(protobuf_buffer, 0, protobuf_pack_length);
        sw_mojing_control__register_request__pack(&register_request_portobuf_struct, protobuf_buffer);

        //组 tcp-ssl报文,并发送
        sw_equip_manager_tcp_pack(message_type, protobuf_buffer, protobuf_pack_length, m_seq_id);
        m_seq_id++;


        if(NULL != protobuf_buffer)
            free(protobuf_buffer);
    }
    else if (1001 == message_type)
    {
        //返回待nonce的字段
        //TODO:是否需要比较seq_id? 暂不比较
        
        Register_Response_With_Nonce_t register_response;
        SwMojingControl__RegisterResponseWithNonce *msg = NULL;
        sw_mojing_tcp_info_t *tcp_buffer = (sw_mojing_tcp_info_t *)sw_mojing_tcp_info_t_or_message_struct_buffer;
        msg = sw_mojing_control__register_response_with_nonce__unpack(NULL, tcp_buffer->protobuf_len, tcp_buffer->protobuf);
        if(NULL == msg)
        {
            SW_EQUIP_MANAGER_DEBUG("protobuf unpack error!\n");
            return -1;
        }
        memset(register_response, 0, sizeof(Register_Response_With_Nonce_t));
        SW_EQUIP_MANAGER_DEBUG("nonce:%s, Code:%d, message:%s\n", msg->nonce, msg->code, msg->message);

        //发送带认证的register
        Register_Request_With_Auth_t register_request_with_auth;
        memset(register_request_with_auth, 0, sizeof(Register_Request_With_Auth_t));
        //使用 MD5(MD5(用户名+":"+密码)+ ":"+nonce)生成校验
        
        //protobuf打包程序

        //组 tcp-ssl报文,并发送1002
        sw_equip_manager_tcp_pack();
    }
    else if (1003 == message_type)
    {
        Register_Response_t response_buf;
        //解包，并确认是否认证成功





        //调用回调
        sw_equip_manager_register_response(message_type, (void *)&response_buf);
        
    }

    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief 设备信息上报
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_device_info_upload(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief 心跳功能函数
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_keepalive(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief 升级
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_update(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief 下载
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_download_stat(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief 画面翻转
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_frame_flip(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief SD卡功能
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_sdcard(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief 云存储功能
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_cloudstorage(int message_type, void *sw_mojing_tcp_info_t_or_message_struct_buffer)
{
    return 0;
}

//组一个函数映射，将消息和功能函数进行对应
/*
 * @ingroup 魔镜设备管理
 * @brief protobuf解包
 *
 * @param message_type - 消息类型
 * @param protobuf - protobuf 类型数据，函数里根据message_type进行类型转换
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_protobuf_unpack(int message_type, unsigned char *buffer, int buffer_len)
{

    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief 主动信息处理线程
 *
 * @param wparam  - 第一个参数
 * @param lparam  - 第二个参数
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_active_proc()
{
    unsigned int sw_tick = sw_thrd_get_tick();
    int ret = 0;

    while(1)
    {
        /* 检测心跳是否到期 */
        if(m_keepalive_interval <= sw_thrd_get_tick() - sw_tick)
        {
            sw_mutex_lock(m_mojing_tcp_buffer.send_mutex);
            m_need_send = 1;
            ret = sw_equip_manager_keepalive(&m_mojing_tcp_buffer.send_buffer_info);
            if(0 > ret)
            {
                SW_EQUIP_MANAGER_DEBUG("keepalive send error!");
            }
            sw_mutex_unlock(m_mojing_tcp_buffer.send_mutex);
        }
        /* 检测下载状态 */

        sw_thrd_delay(100);
    }

    return 0;
}


/*
 * @ingroup 魔镜设备管理
 * @brief 与DSGW通信的线程
 *
 * @param wparam  - 第一个参数
 * @param lparam  - 第二个参数
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
static int sw_equip_manager_communication_proc(uint32_t wparam, uint32_t lparam)
{
    int ret = -1;

    while(NULL == m_tls_client)
    {
        SW_EQUIP_MANAGER_DEBUG("m_tls_client is NULL !\n");
        sw_thrd_delay(500);
    }

    while(1)
    {
        /* sw_mutex_lock(m_mojing_tcp_buffer.recv_mutex); */
        memset(&m_mojing_tcp_buffer.recv_buffer_info, 0, sizeof(sw_mojing_tcp_info_t));
        int recvsize = 0;

        ret = sw_tls_read(m_tls_client, (unsigned char *)&m_mojing_tcp_buffer.recv_buffer_info, sizeof(sw_mojing_tcp_info_t), &recvsize);
        if(0 >= ret)
        {
            SW_EQUIP_MANAGER_DEBUG("recv error!");
        }
        else
        {
            ret = sw_equip_manager_tcp_unpack(m_mojing_tcp_buffer.recv_buffer_info, recvsize);
            if(0 != ret)
            {
                SW_EQUIP_MANAGER_DEBUG("sw_equip_manager_tcp_unpack  error!\n");
            }
        }
    }
    sw_thrd_delay(200);
    return 0;
}

/*
 * @ingroup 对接魔镜平台
 * @brief  获取魔镜管理网关地址
 *
 *
 * @return 0 - 成功 -1 - 失败
 */
int MOJING_IPCManager_Connect_DSGW(RGW_Connect_info_t *rgw_connect_info, RGW_response_t *rgw_response)
{
    int ret = -1;
    char url_path[1024] = {0};
    char *device_info_str = NULL;
    char response_message[16] = {0};
    int response_code = 0;
    sw_session_t sw_session;
    cJSON *device_info_json_array = NULL, *device_info_json = NULL, *json_root = NULL,
          *json_object = NULL, *dsgw_json_root = NULL;


    //组设备信息的json串
    device_info_json_array = cJSON_CreateArray();
    if(NULL == device_info_json_array)
    {
        SW_EQUIP_MANAGER_DEBUG("create json array failed!\n");
        return -1;
    }

    cJSON_AddItemToArray(device_info_json_array, device_info_json = cJSON_CreateObject());
    cJSON_AddStringToObject(device_info_json, "DeviceID", rgw_connect_info->DeviceID);
    cJSON_AddStringToObject(device_info_json, "FirmwareVersion", rgw_connect_info->FirmwareVersion);
    cJSON_AddStringToObject(device_info_json, "FirmwareMD5", rgw_connect_info->FirmwareMD5);
    cJSON_AddStringToObject(device_info_json, "SoftwareVersion", rgw_connect_info->SoftwareVersion);
    cJSON_AddStringToObject(device_info_json, "SoftwareMD5", rgw_connect_info->SoftwareMD5);


    device_info_str = cJSON_Print(device_info_json_array);
    if(NULL == device_info_str)
    {
        SW_EQUIP_MANAGER_DEBUG("cJSON_Print device_info_json_array error!\n");
        goto ERROR;
    }
    SW_EQUIP_MANAGER_DEBUG("device_info_json_array:\n\n%s\n\n", device_info_str);

    memset(&sw_session, 0, sizeof(sw_session_t));
    /* snprintf(url_path, sizeof(url_path) - 1, "%s%s", dgw_url, "/TerminalManager/DeviceServiceGateWay"); */
    snprintf(url_path, sizeof(url_path) - 1, "%s:%d%s", rgw_connect_info->RGW_ip, rgw_connect_info->RGW_port, rgw_connect_info->RGW_path);
    sw_url_parse(&sw_session.sw_url, url_path);

    //判断是否是https地址
    if(0 == strncmp(rgw_connect_info->RGW_ip, "https://", strlen("https://")))
        sw_session.is_https = true;
    else
        sw_session.is_https = false;
    strlcpy(sw_session.request_header.method, "POST", sizeof(sw_session.request_header.method));
    strlcpy(sw_session.request_header.content_type, "application/json; charset=utf-8", sizeof(sw_session.request_header.content_type));
    strlcpy(sw_session.send_recv_buffer, device_info_str, sizeof(sw_session.send_recv_buffer));
    sw_session.send_recv_buffer_len = strlen(device_info_str);
    sw_session.timeout = m_timeout;

    ret = sw_http_send(&sw_session);
    if(!ret)
    {
        SW_EQUIP_MANAGER_DEBUG("send error!");
        goto ERROR;
    }

    //释放CJSON
    if(NULL != device_info_json_array)
        cJSON_Delete(device_info_json_array);
    device_info_json_array = NULL;
    device_info_json = NULL;

    ret = sw_http_recv(&sw_session);
    if( 0 >= ret)
    {
        SW_EQUIP_MANAGER_DEBUG("recv error ret = %d!\n", ret);
        goto ERROR;
    }

    json_root = cJSON_Parse(sw_session.send_recv_buffer);
    if(json_root == NULL)
    {
        SW_EQUIP_MANAGER_DEBUG("json parse error\n");
        goto ERROR;
    }

    //code
    json_object = cJSON_GetObjectItem(json_root,"code");
    if(json_object && json_object->type == cJSON_Number)
    {
        SW_EQUIP_MANAGER_DEBUG("code = %d\n",json_object->valueint);
        response_code = json_object->valueint;
    }
    else
    {
        SW_EQUIP_MANAGER_DEBUG("get code error!\n");
        goto ERROR;
    }

    //message
    json_object = cJSON_GetObjectItem(json_root,"message");
    if(json_object && json_object->type == cJSON_String)
    {
        SW_EQUIP_MANAGER_DEBUG("message = %s\n",json_object->valuestring);
        strlcpy(response_message, json_object->valuestring, sizeof(response_message));
    }
    else
    {
        SW_EQUIP_MANAGER_DEBUG("get message error!\n");
        goto ERROR;
    }

    //DeviceServiceGateWayInfo
    dsgw_json_root = cJSON_GetObjectItem(json_root,"DeviceServiceGateWayInfo");
    if(!(dsgw_json_root && dsgw_json_root->type == cJSON_Object))
    {
        SW_EQUIP_MANAGER_DEBUG("get DeviceServiceGateWayInfo error!\n");
        goto ERROR;
    }
    else
    {
        //ServiceURI
        dsgw_json_root = cJSON_GetObjectItem(json_root,"DeviceServiceGateWayInfo");
        if(!(dsgw_json_root && dsgw_json_root->type == cJSON_Array))
        {
            SW_EQUIP_MANAGER_DEBUG("get DeviceServiceGateWayInfo error!\n");
            goto ERROR;
        }
        else
        {
            //ServiceURI
            json_object = cJSON_GetObjectItem(dsgw_json_root,"ServiceURI");
            if(json_object && json_object->type == cJSON_String)
            {
                SW_EQUIP_MANAGER_DEBUG("ServiceURI = %s\n",json_object->valuestring);
                strlcpy(m_dsgw_url, json_object->valuestring, sizeof(m_dsgw_url));
            }
            else
            {
                SW_EQUIP_MANAGER_DEBUG("get ServiceURI error!\n");
                goto ERROR;
            }

            //ServicePort
            json_object = cJSON_GetObjectItem(dsgw_json_root,"ServicePort");
            if(json_object && json_object->type == cJSON_Number)
            {
                SW_EQUIP_MANAGER_DEBUG("ServicePort = %d\n",json_object->valueint);
                m_dsgw_port = json_object->valueint;
            }
            else
            {
                SW_EQUIP_MANAGER_DEBUG("get ServicePort error!\n");
                goto ERROR;
            }
        }

    }
    SW_EQUIP_MANAGER_DEBUG("response json:\n%s\n!", cJSON_Print(json_root));
    SW_EQUIP_MANAGER_DEBUG("DeviceServiceGateWayInfo json:\n%s\n!", cJSON_Print(dsgw_json_root));
    cJSON_Delete(dsgw_json_root);
    cJSON_Delete(json_root);
    dsgw_json_root = NULL;
    json_root = NULL;

    //连接DSGW
    ret = sw_equip_manager_connect_dsgw();
    if(0 > ret)
    {
        SW_EQUIP_MANAGER_DEBUG("connect dsgw error!\n");
        return -1;
    }

    //调用回调函数
    memset(dsgw_info, 0, sizeof(rgw_connect_info_t));
    rgw_response.ServiceURI = m_dsgw_ip;
    rgw_response.ServicePort = m_dsgw_port;
    rgw_response.message = response_message;
    rgw_response.code = response_code;

    return 0;

ERROR:
    if(NULL != device_info_json_array)
        cJSON_Delete(device_info_json_array);
    device_info_json_array = NULL;
    device_info_json = NULL;
    if(NULL != sw_session.client)
        sw_http_disconnect(&sw_session);

    memset(&sw_session, 0, sizeof(sw_session_t));

    if(NULL != dsgw_json_root)
        cJSON_Delete(dsgw_json_root);
    if(NULL != json_root)
        cJSON_Delete(json_root);
    dsgw_json_root = NULL;
    json_root = NULL;

    return -1;

}

/*
 * @ingroup 魔镜设备管理
 * @brief  连接DSGW
 *
 *
 * @return 0 - 连接成功 -1 - 连接失败
 */
static int sw_equip_manager_connect_dsgw()
{
    int ret = -1;
    struct in_addr in;
    char *p = NULL;

    memset(&in, 0, sizeof(struct in_addr));
    if(sw_txtparser_is_address(m_dsgw_url)){
        in.s_addr = sw_gethostbyname2(m_dsgw_url, m_timeout);
        p = inet_ntoa(in);
        if(NULL == p)
        {
            SW_EQUIP_MANAGER_DEBUG("inet_ntoa ip error!\n");
            return -1;
        }
        strlcpy(m_dsgw_ip, p, sizeof(m_dsgw_ip));
    }

    //连接DSGW
    m_tls_dsgw_client = sw_tls_init();
    if(NULL == m_tls_dsgw_client)
    {
        SW_EQUIP_MANAGER_DEBUG("connect DSGW init tls error!");
        goto ERROR;
    }

    ret = sw_tls_connect(m_tls_dsgw_client, m_dsgw_ip, m_dsgw_port);
    if(0 > ret)
    {
        SW_EQUIP_MANAGER_DEBUG("connect DSGW connect DSGW error!");
        goto ERROR;

    }

    return 0;
ERROR:
    if(NULL != m_tls_dsgw_client)
        sw_tls_release(m_tls_dsgw_client);
    m_tls_dsgw_client = NULL;

    return -1;
}

/*
 * @ingroup 魔镜设备管理
 * @brief  初始化功能函数hash表
 *
 *
 * @return 0 - 成功 -1 - 失败
 */
static int sw_equip_function_hash_init()
{

    sw_equip_pack_and_send_or_unpack_and_callback_method_func_init();

    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1000", sw_equip_manager_register);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1001", sw_equip_manager_register);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1002", sw_equip_manager_register);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1003", sw_equip_manager_register);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1004", sw_equip_manager_device_info_upload);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1005", sw_equip_manager_device_info_upload);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1006", sw_equip_manager_keepalive);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("1007", sw_equip_manager_keepalive);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("2000", sw_equip_manager_update);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("2001", sw_equip_manager_update);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("2002", sw_equip_manager_update_notify);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("2003", sw_equip_manager_download_stat);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("2004", sw_equip_manager_download_stat);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("3004", sw_equip_manager_frame_flip);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("3005", sw_equip_manager_frame_flip);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("3006", sw_equip_manager_sdcard);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("3007", sw_equip_manager_sdcard);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("3008", sw_equip_manager_sdcard);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("3009", sw_equip_manager_sdcard);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("3010", sw_equip_manager_format_sdcard_stat_request);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4000", sw_equip_manager_cloudstorage);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4001", sw_equip_manager_cloudstorage);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4002", sw_equip_manager_cloudstorage);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4003", sw_equip_manager_cloudstorage);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4004", sw_equip_manager_cloudstorage);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4005", sw_equip_manager_cloudstorage);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4006", sw_equip_manager_cloudstorage);
    sw_equip_pack_and_send_or_unpack_and_callback_method_func_add("4007", sw_equip_manager_cloudstorage);


    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief  启动魔镜管理
 *
 *
 * @return 0 - 成功 -1 - 失败
 */
int MOJING_IPCManager_init()
{
    int ret = -1;

    m_equip_control_communication_thrd = sw_thrd_open("tSIPEquipControlReadProc", 10, SW_SCHED_NORMAL, 65536, (threadhandler_t)sw_equip_manager_communication_proc, 0, 0);
    if( NULL == m_equip_control_communication_thrd ){
        SW_EQUIP_MANAGER_DEBUG("%s Created read DSGW thread  failed\n", __FUNCTION__);
        return -1;
    }
    sw_thrd_resume( m_equip_control_communication_thrd );

    return 0;
}

/*
 * @ingroup 魔镜设备管理
 * @brief  退出魔镜管理
 *
 *
 * @return 0 - 成功 -1 - 失败
 */
void MOJING_IPCManager_Stop()
{
    if(NULL != m_equip_control_communication_thrd)
        sw_thrd_close(m_equip_control_communication_thrd, 100);
    m_equip_control_communication_thrd = NULL;
}

/*
 * @ingroup 对接魔镜平台
 * @brief 消息通知
 *
 * @param message_type - 消息类型
 * @param buffer - 消息体结构体指针
 *
 * @return 0 - 获取成功 -1 - 获取失败
 */
int MOJING_IPCManager_Inform(int message_type, void *message_struct_buffer, DSGW_response_callback response_func)
{
    int ret = -1
    //根据消息类型，找到并调用对应的功能函数，处理并发送消息给DSGW
    ret = sw_equip_pack_and_send_or_unpack_and_callback_method_func(message_type, message_struct_buffer);
    if(0 != ret)
    {
        SW_EQUIP_MANAGER_DEBUG("sw_equip_pack_and_send_or_unpack_and_callback_method_func failed!\n");
    }
    
    switch(message_type)
    {
        case 1000:
            //携带认证后，是否认证成功
            sw_equip_manager_register_response = response_func;
            break;
            //添加上个消息的回调函数
        default:
            break;
    }

    return 0;
}
