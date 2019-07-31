/** 
 * @file swevent.h
 * @brief  参考SDL event定义事件数据结构
 * @author chenkai
 * @date 2010-12-10 created
 * @update:yihong
 */
#ifndef __SWEVENT_H__
#define __SWEVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define HANDLE void*
/** Event enumerations */
typedef enum 
{
	SW_NOEVENT = 0,			/**< Unused (do not remove) */
	SW_FONTKEYDOWN,			/**< Keys pressed */
	SW_FONTKEYUP,		    /**< Keys released */

	SW_SYSTEM_EVENT,		/**< System event,SLEEP,WAKEUP,REBOOT,UPGRADE,QUIT*/
	SW_NETWORK_EVENT,		/**< Network event*/
	SW_HARDWARE_EVENT,		/**< Hardware event*/
	SW_MEDIA_EVENT,			/**多媒体事件*/
	SW_NTP_EVENT,			/**ntp 同步事件*/
	SW_HEARTBEAT_EVENT,		/**10ms的心跳事件*/
	SW_EVENT_RESERVED3,		/**< Reserved for future use.. */
	SW_EVENT_RESERVED4,		/**< Reserved for future use.. */
	SW_EVENT_RESERVED5,		/**< Reserved for future use.. */

    SW_VOIP_EVENT = 15,
    SW_A2DP_EVENT = 16,
    SW_SAI_EVENT,          /* 声智事件 */
	SW_USEREVENT = 24,		
	/** Events SW_USEREVENT through SW_MAXEVENTS-1 are for your use */
	/** This last event is only for bounding internal arrays
	 *  It is the number of bits in the event mask datatype -- Uint32
	 */
	SW_NUMEVENTS = 32,

} sw_eventtype_t;

/** @name Predefined event masks */
#define SW_EVENTMASK(X)	(1<<(X))
typedef enum 
{
	SW_FONTKEYDOWNMASK = SW_EVENTMASK(SW_FONTKEYDOWN),
	SW_FONTKEYUPMASK	 = SW_EVENTMASK(SW_FONTKEYUP),
	SW_KEYEVENTMASK	= SW_EVENTMASK(SW_FONTKEYDOWN)| SW_EVENTMASK(SW_FONTKEYUP),
    SW_INPUTEVENTMASK = SW_KEYEVENTMASK,
	SW_SYSTEMEVENTMASK = SW_EVENTMASK(SW_SYSTEM_EVENT),
	SW_NETWORKEVENTMASK = SW_EVENTMASK(SW_NETWORK_EVENT),
	SW_HARDWAREEVENTMASK = SW_EVENTMASK(SW_HARDWARE_EVENT),
	SW_MEDIAEVENTMASK = SW_EVENTMASK(SW_MEDIA_EVENT),
	SW_NTPEVENTMASK = SW_EVENTMASK(SW_NTP_EVENT),
	SW_HEARTBEATEVENTMASK = SW_EVENTMASK(SW_HEARTBEAT_EVENT),
	SW_VOIPEVENTMASK = SW_EVENTMASK(SW_VOIP_EVENT),
	SW_A2DPEVENTMASK = SW_EVENTMASK(SW_A2DP_EVENT),
    SW_SAIEVENTMASK = SW_EVENTMASK(SW_SAI_EVENT),
	SW_USEREVENTMASK = SW_EVENTMASK(SW_USEREVENT)
}sw_eventmask_t;

#define SW_ALLEVENTS		0xFFFFFFFF
typedef struct _sw_keyevent
{
    uint8_t type;
    uint8_t state;
    uint32_t code;
    uint32_t press_time;
}sw_keyevent_t;

typedef struct _sw_networkevent
{
	uint8_t type; /**SW_NETWORK_EVENT*/
	uint8_t which;/**NETWORK_CABLE,NETWORK_WIFI*/
	uint8_t	mode; /**NETWORK_DHCP,NETWORK_PPPOE,NETWORK_STATIC*/
	uint8_t state;/* CONNECT_SUCCESS,CONNECT_FAILED,TIMEOUT,CABLE_ON,CABLE_OFF,IP_CONFLICT*/
	int		reason;/*AUTHOR FAILED,DISCOVERY TIMEOUT,REQUEST_TIMEOUT,NAK*/
	int		ip_ver;
}sw_networkevent_t;

typedef struct _sw_hardwareevent
{
	uint8_t type;
	uint8_t which; /*HDD,USB Store,Smartcard*/
	uint8_t state; /*On,Off,Full,Error,Format*/
	uint8_t wparam;
	void 	*devnode;	/* for usb,smartcard */
	void	*mountdir;
	void	*data;		/* extern parameter */
}sw_hardwareevent_t;

typedef struct _sw_systemevent
{
	uint8_t type;
	uint8_t action;/*REBOOT,SLEEP,WAKEUP,UPGRADE,LOADING*/
	uint8_t percent;
    int     upg_type;
    int     upg_status;
    int     upg_error;
}sw_systemevent_t;

typedef struct _sw_mediaevent
{
	uint8_t type;
	uint8_t action; /*BEGIN,FFWD,FBWD,PAUSE,PLAY,STOP*/
	int wparam;	
	int lparam;
	int lextend;
	void *handle;
}sw_mediaevent_t;

typedef struct _sw_ntpevent
{
	uint8_t type;
	uint8_t	state; /*NTP_SYNC_SUCCESS,NTP_SYNC_FAILED*/	
	uint32_t start_time;    //当年夏令时UTC开始时间
	uint32_t end_time;      //当年夏令时UTC结束时间
}sw_ntpevent_t;

/** A user-defined event type */
typedef struct _sw_cwmpevent
{
	uint8_t type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
	int code;		/**< User defined event code */
	void *data;	
}sw_cwmpevent_t;

/** A user-defined event type */
typedef struct _sw_userevent
{
	uint8_t type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
	int code;		/**< User defined event code */
	void *data;		/**< User defined data pointer */
} sw_userevent_t;

typedef struct _voip_event
{
    uint8_t type;
    int code;
    void* data;
}sw_voip_t;

typedef struct _a2dp_event
{
    uint8_t type;
    uint8_t action; /*PAUSE,PLAY,STOP*/
}sw_a2dp_t;

typedef struct _sai_event
{
    uint8_t type;
    uint8_t action; /*PAUSE,PLAY,STOP, TTS_PLAY, TTS_END*/
    uint8_t wparam;
    uint8_t lparam;
    void* data;
}sw_sai_t;

/** General event structure */
typedef union _sw_event 
{
	uint8_t type;
	sw_networkevent_t network;
	sw_hardwareevent_t hardware;
	sw_systemevent_t system;
	sw_mediaevent_t media;
	sw_ntpevent_t ntp;
	sw_userevent_t user;
	sw_keyevent_t key;
} sw_event_t;

typedef enum
{
    SW_EVT_REBOOT = 0,
    SW_EVT_UPGRADE,
    SW_EVT_SLEEP,
    SW_EVT_WAKEUP,
    SW_EVT_LOADING,
	SW_EVT_UPGRADE_DOWNLOAD,
	SW_EVT_RECOVERY,
    SW_EVT_MAX
}e_system_event_t;

typedef enum 
{
	SW_EVT_HARDWARE_NULL = 0,
	SW_EVT_HARDWARE_HDMI,
	SW_EVT_HARDWARE_HDD,
	SW_EVT_HARDWARE_USB,
	SW_EVT_HARDWARE_USB_KEYBOARD,
	SW_EVT_HARDWARE_USB_MOUSE,
	SW_EVT_HARDWARE_MAX
}e_hardware_event_t;

typedef enum
{
	SW_NTP_SYNC_SUCCESS = 0,
	SW_NTP_SYNC_FAILED,
	SW_NTP_TIMEZONE_CHANGE,
	SW_NTP_INTO_DST,
	SW_NTP_EXIT_DST,
	SW_NTP_DNS_OVERTIME,
	SW_NTP_DNS_FAILED
}sw_ntpstate_t;

typedef enum
{
    SW_VOIP_INCOMING_RING,
    SW_VOIP_OUTGOING_RING
}e_voip_event_t;

typedef enum
{
    SAI_MEDIA_EVENT_NULL,
    SAI_MEDIA_EVENT_PLAY,
    SAI_MEDIA_EVENT_PAUSE,
    SAI_MEDIA_EVENT_STOP,
    SAI_MEDIA_EVENT_END,
    SAI_MEDIA_EVENT_ERROR,
    SAI_TTS_EVENT_BEGIN,
    SAI_TTS_EVENT_END,
    SAI_TTS_EVENT_STOP,
	SAI_EVENT_WAKEUP,
	SAI_EVENT_VADEND,
    SAI_MEDIA_CMD_PREV = 20,
    SAI_MEDIA_CMD_NEXT,
    SAI_MEDIA_CMD_PLAY,
    SAI_MEDIA_CMD_PAUSE,
    SAI_NLP_CONNECT_FAILED
}e_sai_event_t;

typedef enum
{
	SW_CONNECT_WIFI = 0x1,
	SW_SCAN_QRCODE,
}sw_usr_envet_t;

typedef enum
{
	SW_SHORTKEY_STATE = 1,
	SW_LONGKEY_STATE
}sw_keystate_t;

/*定义事件提交的函数类型*/
typedef int (*event_post_func)(HANDLE handle,sw_event_t* event);


#ifdef __cplusplus
}
#endif

#endif //__SWEVENT_H__

