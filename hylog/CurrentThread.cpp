#include "CurrentThread.h"

namespace hying
{
	namespace CurrentThread
	{
		__thread int t_cachedTid = 0;
		__thread char t_tidString[32];
		__thread int t_tidStringLength = 6;
		__thread const char* t_threadName = "unknown";
		// static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

		void cacheTid()
		{
			if (t_cachedTid == 0)
			{
#ifdef WIN32
				t_cachedTid = static_cast<int>(GetCurrentThreadId());
#else
				t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
#endif
				t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
			}
		}
	}
}