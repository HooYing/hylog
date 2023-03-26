#ifndef __UTILS_H__
#define __UTILS_H__

#include "pch.h"

namespace hying
{
	// 输出具体错误信息
	const char* strerror_tl(int savedErrno);

#ifdef WIN32
	inline int gettimeofday(struct timeval* tp, void* tzp)
	{
		time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;
		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm.tm_isdst = -1;
		clock = mktime(&tm);
		tp->tv_sec = clock;
		tp->tv_usec = wtm.wMilliseconds * 1000;
		return (0);
	}

	inline std::wstring gbk_to_utf16(const char* gbk_data)
	{
		DWORD dwlen = MultiByteToWideChar(CP_ACP, 0, gbk_data, -1, NULL, 0);
		wchar_t* utf16_data = new TCHAR[dwlen + 1];
		MultiByteToWideChar(CP_ACP, 0, gbk_data, -1, utf16_data, dwlen);
		std::wstring data(utf16_data);
		delete[] utf16_data;
		return data;
	}

#endif

	//
	//constexpr int32_t basename_index(
	//	const char* const path, const int32_t index = 0, const int32_t slash_index = -1
	//)
	//{
	//	return path[index]
	//		? ((path[index] == '/' || path[index] == '\\')  // (see below)
	//			? basename_index(path, index + 1, index)
	//			: basename_index(path, index + 1, slash_index)
	//			)
	//		: (slash_index + 1)
	//		;
	//}
	//
	//template <int32_t Value>
	//struct require_at_compile_time
	//{
	//	static constexpr const int32_t value = Value;
	//};
	//
	//#define JUST_FILENAME (__FILE__ + require_at_compile_time<basename_index(__FILE__)>::value)

}

#endif