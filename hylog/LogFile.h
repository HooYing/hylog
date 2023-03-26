#ifndef __FILEUTIL_H__
#define __FILEUTIL_H__

#include "pch.h"
#include <mutex>

namespace hying
{
	// 适配const char*和std::string
	class StringArg
	{
	public:
		StringArg(const char* str)
			:m_str(str)
		{}
		StringArg(const std::string& str)
			:m_str(str.c_str())
		{}

		const char* c_str() const {
			return m_str;
		}
	private:
		const char* m_str;
	};

	class AppendFile
	{
	public:
		explicit AppendFile(StringArg filename);

		~AppendFile();

		void append(const char* logline, const size_t len);

		void flush();

		// off_t 类型用于指示文件的编译量
		off_t writtenBytes() const { return m_writtenBytes; }

	private:
		size_t write(const char* logline, size_t len);

		FILE* m_fp;
		char m_buffer[64 * 1024];
		off_t m_writtenBytes;
	};

	class LogFile
	{
	public:
		LogFile(const std::string& basename,
			off_t rollSize,
			bool threadSafe = true,
			int flushInterval = 3,
			int checkEveryN = 1024);
		~LogFile();

		void append(const char* logline, int len);
		void flush();
		bool rollFile();

	private:

		void append_unlocked(const char* logline, int len);
		static std::string getLogFileName(const std::string& basename, time_t* now);

	private:
		const std::string m_basename;
		const off_t m_rollSize;
		const int m_flushInterval;
		const int m_checkEveryN;

		int m_count;

		std::unique_ptr<std::mutex> m_mutex;
		time_t m_startOfPeriod;
		time_t m_lastRoll;
		time_t m_lastFlush;
		std::unique_ptr<AppendFile> m_file;

		const static int kRollPerSeconds = 60 * 60 * 24;

	};
}

#endif