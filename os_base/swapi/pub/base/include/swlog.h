#ifndef __SWLOG_H__
#define __SWLOG_H__

/* 在android环境下编译的代码如果要使用ANDROID的log需要在Android.mk配置下 */
#if (defined(ANDROID) && defined(SUPPORT_SWAPI30))
#include "swlog_android.h"
#else
#include "swlog_linux.h"
#endif

#endif /* __SWLOG_H__ */