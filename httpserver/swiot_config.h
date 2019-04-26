#ifndef __SWIOT_CONFIG_H__
#define __SWIOT_CONFIG_H__

/**
 * SDK °æ±¾ºÅ
 **/
#define SWIOT_SDK_VERSION       "1.4.1"

/**
 *
 **/
#define SWIOT_SUCCESS           (-0)
#define SWIOT_EGENERIC          (-1)

/**
 * AP
 **/
#define SOFTAP_SSID             "softap_iot"
#define SOFTAP_PASSWORD         "88888888"

/**
 * platform
 **/
#define LINUX

//#define RTOS_8266

/**
 * log
 **/
#define LOG_DEBUG       1
#define LOG_INFO        2
#define LOG_WARN        3
#define LOG_ERROR       4
#define LOG_NONE        5

#define LOG_LEVEL 		LOG_DEBUG


#endif
