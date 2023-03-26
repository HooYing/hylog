#include "ProcessInfo.h"

namespace hying
{
	int pid()
	{
#ifdef WIN32
		return static_cast<int>(GetCurrentProcessId());
#else
		return getpid();
#endif
	}

	std::string hostname()
	{
		char buf[256];
#ifdef WIN32
		DWORD nSize = 256;
		if (GetComputerNameA(buf, &nSize) == true)
#else
		if (::gethostname(buf, sizeof buf) == 0)
#endif
		{
			buf[sizeof(buf) - 1] = '\0';
			return buf;
		}
		else
		{
			return "unknownhost";
		}
	}
}