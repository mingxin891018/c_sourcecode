#ifndef __GETTIMEOFDAY_H__
#define __GETTIMEOFDAY_H__

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef WIN32
int gettimeofday(struct timeval *tv, void *tz);
#endif

#ifdef __cplusplus
}
#endif
#endif
