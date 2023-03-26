#ifndef __HYLOG_H__
#define __HYLOG_H__

#include "pch.h"
#include "TimeStamp.h"
#include "LogStream.h"

namespace hying
{
	class Logger
	{
	public:
		enum LogLevel
		{
			TRACE,
			DEBUG,
			INFO,
			WARN,
			ERROR,
			FATAL,
			NUM_LOG_LEVELS,
		};

		class SourceFile
		{
		public:
			template<int N>
			SourceFile(const char(&arr)[N])
				: data_(arr), size_(N - 1)
			{
				const char* slash = strrchr(data_, '/');
				if (slash == nullptr)
					slash = strrchr(data_, '\\');
				if (slash)
				{
					data_ = slash + 1;
					size_ -= static_cast<int>(data_ - arr);
				}
			}

			explicit SourceFile(const char* filename)
				: data_(filename)
			{
				const char* slash = strrchr(filename, '/');
				if (slash == nullptr)
					slash = strrchr(data_, '\\');
				if (slash)
				{
					data_ = slash + 1;
				}
				size_ = static_cast<int>(strlen(data_));
			}

			const char* data_;
			int size_;
		};


		Logger(SourceFile file, int line);
		Logger(SourceFile file, int line, LogLevel level);
		Logger(SourceFile file, int line, LogLevel level, const char* func);
		Logger(SourceFile file, int line, bool toAbort);
		~Logger();

		LogStream& stream() { return impl_.stream_; }

		static LogLevel logLevel();
		static void setLogLevel(LogLevel level);

		typedef void (*OutputFunc)(const char* msg, int len);
		typedef void (*FlushFunc)();
		static void setOutput(OutputFunc);
		static void setFlush(FlushFunc);
		//static void setTimeZone(const TimeZone& tz);

	private:

		class Impl
		{
		public:
			typedef Logger::LogLevel LogLevel;
			Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
			void formatTime();
			void finish();

			Timestamp time_;
			LogStream stream_;
			LogLevel level_;
			int line_;
			SourceFile basename_;
		};

		Impl impl_;
	};


	extern Logger::LogLevel g_logLevel;

	inline Logger::LogLevel Logger::logLevel()
	{
		return g_logLevel;
	}


#define LOG_TRACE if (hying::Logger::logLevel() <= hying::Logger::TRACE) \
  hying::Logger(__FILE__, __LINE__, hying::Logger::TRACE, __FUNCTION__).stream()
#define LOG_DEBUG if (hying::Logger::logLevel() <= hying::Logger::DEBUG) \
  hying::Logger(__FILE__, __LINE__, hying::Logger::DEBUG, __FUNCTION__).stream()
#define LOG_INFO if (hying::Logger::logLevel() <= hying::Logger::INFO) \
  hying::Logger(__FILE__, __LINE__, hying::Logger::INFO, __FUNCTION__).stream()
#define LOG_WARN hying::Logger(__FILE__, __LINE__, hying::Logger::WARN, __FUNCTION__).stream()
#define LOG_ERROR hying::Logger(__FILE__, __LINE__, hying::Logger::ERROR, __FUNCTION__).stream()
#define LOG_FATAL hying::Logger(__FILE__, __LINE__, hying::Logger::FATAL, __FUNCTION__).stream()
#define LOG_SYSERR hying::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL hying::Logger(__FILE__, __LINE__, true).stream()


#define CHECK_NOTNULL(val) \
  ::hying::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
	template <typename T>
	T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr)
	{
		if (ptr == NULL)
		{
			Logger(file, line, Logger::FATAL).stream() << names;
		}
		return ptr;
	}

	const char* strerror_tl(int savedErrno);

#define log_trace(...) if(hying::Logger::logLevel() <= hying::Logger::TRACE) \
	log(__FILE__, __LINE__, hying::Logger::TRACE, __FUNCTION__, __VA_ARGS__)
#define log_debug(...) if(hying::Logger::logLevel() <= hying::Logger::DEBUG) \
	log(__FILE__, __LINE__, hying::Logger::DEBUG, __FUNCTION__, __VA_ARGS__)
#define log_info(...) if(hying::Logger::logLevel() <= hying::Logger::INFO) \
	log(__FILE__, __LINE__, hying::Logger::INFO, __FUNCTION__, __VA_ARGS__)
#define log_warn(...) log(__FILE__, __LINE__, hying::Logger::WARN, __FUNCTION__, __VA_ARGS__)
#define log_error(...) log(__FILE__, __LINE__, hying::Logger::ERROR, __FUNCTION__, __VA_ARGS__)
#define log_fatal(...) log(__FILE__, __LINE__, hying::Logger::FATAL, __FUNCTION__, __VA_ARGS__)


	void log(hying::Logger::SourceFile file, int line, hying::Logger::LogLevel level, const char* func, const char* fmt, ...);

}



#endif