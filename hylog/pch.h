#ifndef __FCH_H__
#define __FCH_H__

#include <iostream>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <memory>
#include <stdarg.h>

#ifdef WIN32
#define UNICODE
#include <Windows.h>
#include <shlwapi.h>

// ʹ��PRld64
#define __STDC_FORMAT_MACROS


#pragma comment(lib, "shlwapi.lib")

// ����־��ERRORö���ض���
#undef ERROR

#define __thread thread_local
#define strerror_r(err,buf,len) strerror_s(buf,len,err)
#define gmtime_r(sec, tmt) gmtime_s(tmt, sec) 

#else
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/prctl.h>
#endif


#endif
