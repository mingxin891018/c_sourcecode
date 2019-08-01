#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/ip_icmp.h>
#include "swapi.h"
#include "swnetconnect.h"
#include "swnetsocket.h"
#include "swnetwork.h"
#include "swthrd.h"
#include "swmutex.h"
#include "swparameter.h"
#include  <sys/un.h>
#include "swwifi.h"
#include "swether.h"
#include "swnetutil.h"
#include "swnetlog.h"
#include "swnetevent.h"

/* 分析是不是一个有效的点分十进制IP地址 */
static bool IsAddress(char* buf)
{
	int i = 0, len = 0;
	char *p = NULL, *pSep = NULL;
	int iCount = 0;
	char szTemp[8];
	int nSect = 0;

	if( buf == NULL )
		return false;
		
	len = strlen(buf);
	if( len == 0 || len > 15 )
		return false;

	for( i = 0; i < len; i++)
	{
		if( ( buf[i] < '0' || buf[i] >'9' ) && ( buf[i] != '.' ) )
			return false;
	}

	p = buf;
	while( ( pSep = strchr(p,'.') ) != NULL )
	{
		++iCount;

		if( pSep - p > 3)
			return false;

		memset( (void*) szTemp, 0, sizeof(szTemp) );
		strlcpy( szTemp, p, sizeof(szTemp) );

		nSect = atoi( szTemp );
		if( nSect > 255 )
			return false;

		pSep++;
		p = pSep;
	}

	if( iCount != 3 )
	{
		return false;
	}
	else
	{
		memset( (void*) szTemp, 0, sizeof(szTemp) );
		strlcpy( szTemp, p, sizeof(szTemp));

		nSect = atoi( szTemp );
		if( nSect > 255 )
			return false;
	}
	return true;	
}

/**
 *  组织请求报文
 *  报文格式:  Name(乿结尾) + Type(2字节) + Class(2字节)
 *  @return: 报文长度
 */
static int make_name_query( char* buf, int buflen, char* name )
{
	int i, j, len;
	unsigned short sTmp;

	len = 0;
	//将name编码
	for( i=0;; i=j+1 )
	{
		for( j=i; name[j] && name[j]!='.'; j++ );
		if( len + j-i >= buflen-6 )
			return -1;
		buf[len] = j-i;
		memcpy( buf+len+1, name+i, j-i );
		len += j-i+1;
		if( name[j]==0 )
			break;
	}
	if( buf[len-1] )
		buf[len++] = 0;
	//Type = A ( 0x1 = Host Address )
	sTmp = htons(0x1);
	memcpy( buf+len, &sTmp, 2 );
	len += 2;

	//Class = IN ( 0x1 = IN )
	sTmp = htons(0x1);
	memcpy( buf+len, &sTmp, 2 );
	len += 2;
	return len;
}


/**
 *    组织查询报文
 *    报文结构: transaction_id(2字节) + Flags(2字节) + Questions(2字节) + Answer RRS(2字节)
 *                   + Authority RRS(2字节) + Additional RRS(2字节)
 *                   + Queries
 */
static int make_dns_query( char* buf, int buflen, unsigned short transaction_id, char* dns_name )
{
	int len, n;
	unsigned short sTmp;

	if( buflen<18 )
		return -1;

	len = 0;
	//transaction id
	sTmp = htons(transaction_id);
	memcpy(buf+len, &sTmp, 2);
	len += 2;
	//Flags
	sTmp = htons(0x100);	//standard query, 只将递归标志置位
	memcpy(buf+len, &sTmp, 2);
	len += 2;
	//Questions
	sTmp = htons(1);	//1个请毿
	memcpy(buf+len, &sTmp, 2);
	len += 2;
	//跳过 Answer RRS(2字节), Authority RRS(2字节), Additional RRS(2字节)
	memset( buf+len, 0, 6 );
	len += 6;
	//Queries
	n = make_name_query( buf+len, buflen-len, dns_name );
	len += n;
	if( n<0 )
		return -1;
	return len;
}


/**
 *  分析别名
 *  @response_buf: 响应报文起始地址
 *  @response_len: 响应报文长度
 *  @pos: 别名起始地址
 *  @ret_alias: 返回的别县
 *  @return: 别名嚿response_buf 中占的字节数(用于后续调用计算长度)
 */
static int parse_alias( char* response_buf, int response_len, int pos, char* ret_alias )
{
	unsigned char* buf = (unsigned char*)response_buf;
	int i, len = 0, ret_len = 0;
	int redirect_cnt = 0;

	for( i = pos; i<response_len; )
	{
		if( buf[i]==0 )
		{
			if( redirect_cnt==0 )
				len++;
			break;
		}
		if( (buf[i] & 0xc0) == 0xc0 )	//这是一个指鐿
		{
			if( redirect_cnt==0 )
				len += 2;
			redirect_cnt ++;
			i = (buf[i] << 8 | buf[i+1]) & 0x3fff;
		}
		else
		{
			if( ret_len>0 )
				ret_alias[ret_len++] = '.';
			memcpy(ret_alias+ret_len, buf+i+1, buf[i]);
			ret_len += buf[i];
			if( redirect_cnt==0 )
				len += 1 + buf[i];
			i += 1 + buf[i];
		}
	}
	ret_alias[ret_len++] = 0;
	return len;
}


/**
 *    分析响应的别名和ip地址
 *    报文结构：Name + Type(2字节) + Class(2字节) + Time to live(4字节) + 数据长度(2字节) + 数据
 *    将结果存放到alias中，ip放到ipaddr䶿
 *    @return: 分析报文消耗的长度
 */
static int parse_answer( char* response_buf, int response_len, int pos, char* alias, char* ipaddr, int *addrtype, int* addrlen )
{
	int n, len = 0, ret_len = 0;
	unsigned short nType, nClass, nDataLen;
	int nLiveSeconds;
	unsigned char* buf = (unsigned char*)response_buf;

	//Name
	n = parse_alias( response_buf, response_len, pos, alias );
	len += n;

	//Type
	nType = buf[pos+len] << 8 | buf[pos+len+1];
	len += 2;

	//Class
	nClass = buf[pos+len] << 8 | buf[pos+len+1];
	len += 2;

	//Time to live
	nLiveSeconds = buf[pos+len]<<24 | buf[pos+len+1]<<16 | buf[pos+len+2]<<8 | buf[pos+len+3];
	len += 4;

	//Data len
	nDataLen = buf[pos+len]<<8 | buf[pos+len+1];
	len += 2;

	//拷贝ip address
	if( nType==0x0001 )		//0x0001 = IN Addr
	{
		ret_len = (ret_len+3)&~3;
		memcpy(ipaddr, buf+pos+len, nDataLen);
		*addrlen = nDataLen;
	}
	else
	{
		*addrlen = 0;	//只拷贝ipv4的值，除此以外的地址都认为是无效
	}
	len += nDataLen;

	*addrtype = nType;
	return len;
}

/**
 *    分析响应报文
 *    报文结构: transaction_id(2字节) + Flags(2字节) + Questions(2字节) + Answer RRS(2字节)
 *                   + Authority RRS(2字节) + Additional RRS(2字节)
 *                   + Queries + Answers
 *    返回便 0表示无错误，hostent_buf中填充struct hostent结构的返回值⾿其它(1-15)：表示dns服务器返回的错误
 */
static int parse_dns_response( char* response_buf, int response_len, char* hostent_buf, int hostent_len )
{
	unsigned char* buf = (unsigned char*)response_buf;
	int errcode, questions, answers;
	int i, j, n, len, host_len;
	char szName[512], szAddr[32];
	unsigned short sTmp;
	struct hostent *h = (struct hostent*)hostent_buf;

	host_len = sizeof(struct hostent);
	len = 0;
	//transaction_id
	//sTmp = buf[len]<<8 | buf[len+1];
	len += 2;

	//Flags

	sTmp = buf[len]<<8 | buf[len+1];
	errcode = sTmp & 0x000f;
	if( errcode )
		return errcode;
	len += 2;
	//Questions
	questions = buf[len]<<8 | buf[len+1];
	len += 2;
	//Answers
	answers = buf[len]<<8 | buf[len+1];
	len += 2;
	//跳过 Authority RRS(2字节) + Additional RRS(2字节)
	len += 4;

	//请求的域县
	for( i=0; i<questions; i++ )
	{
		n = parse_alias( response_buf, response_len, len, szName );
		len += n;
		//跳过Type 吿Class
		len += 4;
	}
	if( host_len + (answers+1)*2*sizeof(char*) + strlen(szName) >= (unsigned int)hostent_len )
		return -6;	//hostent_buf 长度不够

	//根据以上信息填充
	h->h_addr_list = (char **)(hostent_buf + host_len);
	memset( h->h_addr_list, 0, (answers+1)*sizeof(h->h_addr_list[0]) );
	host_len += (answers+1)*sizeof(h->h_addr_list[0]);

	h->h_aliases = (char **)(hostent_buf + host_len);
	memset( h->h_aliases, 0, (answers*+1)*sizeof(h->h_aliases[0]) );
	host_len += (answers+1)*sizeof(h->h_aliases[0]);

	h->h_name = hostent_buf + host_len;
	strncpy(h->h_name, szName, sizeof(szName));
	host_len += strlen(h->h_name)+1;

	//分析应答的域县
	for( i=0; i<answers; i++ )
	{
		int  nAddrLen, nAddrType;
		n = parse_answer( response_buf, response_len, len, szName, szAddr, &nAddrType, &nAddrLen );
		if( nAddrLen>0 )
		{
			if( host_len + nAddrLen + strlen(szName) >= (unsigned int)hostent_len )
				return -6;	//hosttent_buf 长度不够
			host_len = (host_len + 3) & ~3;
			for( j=0; j<answers && h->h_addr_list[j] && memcmp(h->h_addr_list[j], szAddr, nAddrLen)!=0; j++ );
			if( h->h_addr_list[j]==NULL )
			{
				h->h_addr_list[j] = hostent_buf + host_len;
				memcpy(hostent_buf + host_len, szAddr, nAddrLen);
				host_len += nAddrLen;
			}

			for( j=0; j<answers && h->h_aliases[j] && strcmp(h->h_aliases[j], szName)!=0; j++ );
			if( h->h_aliases[j]==NULL )
			{
				h->h_aliases[j] = hostent_buf + host_len;
				strncpy(hostent_buf+host_len, szName, sizeof(szName));
				host_len += strlen(szName)+1;
			}

			h->h_length = nAddrLen;
			h->h_addrtype = nAddrType;
		}
		len += n;
		if( len >= response_len )
			break;
	}

	if( h->h_addr_list[0] )
		return 0;
	return -5;		//域名存在，但没有找到ip地址
}




/**
 *    向dns服务器发送请求，解析 hostname。解析结果存放在 hostent_buf中，可以强制转换憿struct hostent 结构
 *    返回便  0: 成功
 *            >0: dns服务器返回的错误瞿 3: 无此域名 )
 *                1: Format error, 2: Server failure, 3: No such name, 5: Refused, 6: Name exists,
 *                7: RRset exists, 8: RRset does not exist, 9: Not Authoritative, 10: Name out of zone
 *            -1: 超时, -2: 创建socket失败, -3: 发送失貿 -4: 服务器无dns服务,
 *            -5: dns服务器存在，但没有解析出ip地址, -6: hostent_len太小, -7: 域名太长  -8: 参数错误
 */
int sw_dns_query(char* hostname, unsigned int dns_ip[], int dns_cnt, char* hostent_buf, int hostent_len, int timeout ,int sendtimeout,int sendtimes)
{
	int i, j,k,len, ret, iRet=-1;
	unsigned int now, starttime, lastsendtime=0;
	int *errs = NULL;		//错误数组, 用于保存 parse_dns_response 产生的错误
	int lefttime;
	struct timeval tv;
	unsigned short *transaction_id;
	char szBuf[768];
	int udpskt=-1;
	fd_set rset;
    int sendcount = 0;
	unsigned short localport;
	if(sendtimes==0)
		sendtimes=1;
	if(sendtimeout==0)
		sendtimeout=1000;

	if( hostname==NULL || *hostname==0 || dns_cnt<=0 )
		return -8;

	errs = (int*)malloc(dns_cnt*sizeof(int));
	transaction_id = (unsigned short *)malloc(dns_cnt*sizeof(unsigned short));
    if(!errs || !transaction_id)
    {
        iRet=-8;
        printf("%s,malloc error\n",__FUNCTION__);
        goto ERR_EXIT;
    }
	for( i=0; i<dns_cnt; i++ )
		errs[i] = -1;
	localport = 16800 + (rand() & 0x3fff);

	//创建一个udp socket, 用于绑定端口 localport, 防止出现端口不可达的问题
	struct sockaddr_in local,dst;
	udpskt = socket(AF_INET, SOCK_DGRAM, 0);
	if( udpskt<0 )
	{
        free(errs);
        free(transaction_id);
        return -8;
	}
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(localport);
	int bindret = bind(udpskt, (struct sockaddr *)&local, sizeof(local));
	sw_log_debug( "bindret=%d\n", bindret );

	for( i=0; i<dns_cnt; i++ )
		transaction_id[i] = rand();
	i = 0;
	starttime = sw_thrd_get_tick();

	while( iRet!=0 && sendcount < sendtimes)
	{
		now = sw_thrd_get_tick();
		if( (int)(now - starttime) >= timeout )
			break;	//超时
		//for( j=0; j<dns_cnt && errs[j]!=-1; j++ );

		//if( j>=dns_cnt )
			//break;	//所有的dns都有错误，返回
        
		//向dns server发送请求
		if( sendcount < sendtimes)
		{
			for(i=0; i < dns_cnt; i++)
			{
				/* construct dns data */
				memset( szBuf, 0, sizeof(szBuf) );
				len = make_dns_query( szBuf ,sizeof(szBuf) ,transaction_id[i%dns_cnt], hostname );
				if( len<0 )
				{
					iRet = -7;	//域名太长
                    continue;
				}

				memset(&dst, 0, sizeof(dst));
				dst.sin_family = AF_INET;

				dst.sin_addr.s_addr =dns_ip[i%dns_cnt] ;
				dst.sin_port = htons( 53 );
				//接收应答
				FD_ZERO( &rset );
				FD_SET( udpskt, &rset );

				ret = sendto(udpskt, szBuf, len,0, (struct sockaddr*)&dst, sizeof(dst));
				if( ret != len  )
				{
					sw_log_debug( "%s send error %d ret=%d\n", __FUNCTION__, errno, ret );
					iRet = -3;		//发送失败
                    continue;
				}
			}
            sendcount++;
            lastsendtime = sw_thrd_get_tick();
		}
 		
		if(sendcount >= sendtimes)
			lefttime = timeout - (now - starttime);
        else
            lefttime = sendtimeout;
        
        if(lefttime < 0)
			lefttime=0;

		
		tv.tv_sec = lefttime / 1000;
		tv.tv_usec = lefttime % 1000 * 1000;
		
		//while(1)
		{
			ret = select( udpskt+1, &rset, NULL, NULL, &tv );
			if( ret <= 0 )
			{
				break;
			}
			int fromlen = sizeof(dst);
			len = recvfrom(udpskt, szBuf, sizeof(szBuf), 0, (struct sockaddr*)&dst, (unsigned int*)&fromlen);
			unsigned short recv_id = ntohs(*(unsigned short*)(szBuf));
			sw_log_debug("recv_id=0x%x\n",recv_id);
			for(k=0;k<dns_cnt;k++)
			{
				sw_log_debug("recv_id=0x%x,transcation_id=%x error[%d]=%d\n",recv_id,transaction_id[k],k,errs[k]);
				if( recv_id == transaction_id[k] && (errs[k]==-1))
				{
					//接收到的报文是dns报文
					ret = parse_dns_response( szBuf ,len ,hostent_buf, hostent_len );
					sw_log_debug(" recv one packet ret=%d\n",ret);
					errs[k] = ret;
					if(ret==0)
					{
						iRet=ret;
						goto ERR_EXIT;
					}
					break;
			 
				}
			}
			 
		}
		sw_thrd_delay( sendtimeout-(sw_thrd_get_tick()-lastsendtime) );
	}

	//如果遇到网络错误，看看是否有 parse_dns_response 函数产生的错误
	for( j=0; j<dns_cnt ; j++ )
	{
		if(errs[j]==0)
		{
			iRet=0;
			break;
		}
		iRet = errs[j];
		 
	}
ERR_EXIT:
	if( errs )
		free( errs );
    if(transaction_id)
        free(transaction_id);
    if(udpskt >= 0)
	    close(udpskt);
	sw_log_debug( "%s %s err=%d\n", __FUNCTION__, hostname, iRet );
	return iRet;
}


/**
 *    县sw_dns_query, 只是不需要dns_ip 吿dns_cnt, 程序自动使用设定便
 *    增加返回便  -11: 网线没插好， -12: dhcp失败, -13: pppoe拔号失败, -14: 无dns地址
 */
int sw_network_gethostbyname( char* hostname, char* hostent_buf, int hostent_len, int timeout )
{
	int dns_cnt;
	unsigned int dns_ip[2];
	char *pszIP, szIP[32]={0};
	char* netmode = sw_network_get_defaultmode();

	if( sw_network_get_cable_connected()==0 )
	{
		return -11;		//网线没插壿
	}

	dns_cnt = 0;
	if( strcmp(netmode, "static")==0 )
	{
		if( sw_parameter_get( "lan_dns", szIP, sizeof(szIP) ) && IsAddress(szIP) )
		{
            dns_ip[dns_cnt] = inet_addr(szIP);
            dns_cnt++;
		}
		if( sw_parameter_get( "lan_dns2", szIP, sizeof(szIP) ) && IsAddress(szIP) )
		{
            dns_ip[dns_cnt] = inet_addr(szIP);
            dns_cnt++;
		}
            
	}
	else if( strcmp(netmode, "wifistatic")==0 )
	{   
		if( sw_parameter_get( "wifi_dns", szIP, sizeof(szIP) ) && IsAddress(szIP) )
		{
            dns_ip[dns_cnt] = inet_addr(szIP);
            dns_cnt++;
		}
		if( sw_parameter_get( "wifi_dns2", szIP, sizeof(szIP) ) && IsAddress(szIP) )
		{
            dns_ip[dns_cnt] = inet_addr(szIP);
            dns_cnt++;
		}
	}   
	else
	{
		if( sw_network_get_state() != NET_STATE_CONNECTED )
			return -12;	//dhcp失败
		pszIP = sw_network_get_currentdns();
		if( *pszIP )
		{
			dns_ip[dns_cnt] = inet_addr(pszIP);
            dns_cnt++;
			pszIP = sw_network_get_currentdns2();
			if( *pszIP )
    		{
                dns_ip[dns_cnt] = inet_addr(pszIP);
                dns_cnt++;
    		}
		}
	}

	
	if( dns_cnt==0 )
		return -14;
	return sw_dns_query( hostname, dns_ip, dns_cnt, hostent_buf, hostent_len, timeout ,1000,3);
}
