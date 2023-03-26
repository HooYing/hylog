#include "Log.h"
#include "CurrentThread.h"

namespace hying 
{
	__thread char t_time[64];
	__thread time_t t_lastSecond;

	// 环境变量
	Logger::LogLevel initLogLevel()
	{
		if (::getenv("MUDUO_LOG_TRACE"))
			return Logger::TRACE;
		else if (::getenv("MUDUO_LOG_DEBUG"))
			return Logger::DEBUG;
		else
			return Logger::INFO;
	}

	Logger::LogLevel g_logLevel = initLogLevel();

	const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
	{
	  "TRACE ",
	  "DEBUG ",
	  "INFO  ",
	  "WARN  ",
	  "ERROR ",
	  "FATAL ",
	};

	// 帮助类：在编译时知道字符串长度
	class T
	{
	public:
		T(const char* str, unsigned len)
			:str_(str),
			len_(len)
		{
			assert(strlen(str) == len_);
		}

		const char* str_;
		const unsigned len_;
	};

	inline LogStream& operator<<(LogStream& s, T v)
	{
		s.append(v.str_, v.len_);
		return s;
	}

	inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
	{
		s.append(v.data_, v.size_);
		return s;
	}

	void defaultOutput(const char* msg, int len)
	{
		size_t n = fwrite(msg, 1, len, stdout);
		//FIXME check n
		(void)n;
	}

	void defaultFlush()
	{
		fflush(stdout);
	}

	Logger::OutputFunc g_output = defaultOutput;
	Logger::FlushFunc g_flush = defaultFlush;
	// TimeZone g_logTimeZone;

	Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)
		: time_(Timestamp::now()),
		stream_(),
		level_(level),
		line_(line),
		basename_(file)
	{
		formatTime();
		CurrentThread::tid();
		stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
		stream_ << T(LogLevelName[level], 6);
		if (savedErrno != 0)
		{
			stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
		}
	}

	void Logger::Impl::formatTime()
	{
		int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
		time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
		int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
		if (seconds != t_lastSecond)
		{
			t_lastSecond = seconds;
			struct tm tm_time;
			::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime

			int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
				tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
				tm_time.tm_hour + 8, tm_time.tm_min, tm_time.tm_sec);
			assert(len == 17); (void)len;
		}

		Fmt us(".%06dZ ", microseconds);
		assert(us.length() == 9);
		stream_ << T(t_time, 17) << T(us.data(), 9);
	}

	void Logger::Impl::finish()
	{
		stream_ << " - " << basename_ << ':' << line_ << '\n';
	}

	Logger::Logger(SourceFile file, int line)
		: impl_(INFO, 0, file, line)
	{
	}

	Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
		: impl_(level, 0, file, line)
	{
		impl_.stream_ << func << ' ';
	}

	Logger::Logger(SourceFile file, int line, LogLevel level)
		: impl_(level, 0, file, line)
	{
	}

	Logger::Logger(SourceFile file, int line, bool toAbort)
		: impl_(toAbort ? FATAL : ERROR, errno, file, line)
	{
	}

	Logger::~Logger()
	{
		impl_.finish();
		const LogStream::Buffer& buf(stream().buffer());
		g_output(buf.data(), buf.length());
		if (impl_.level_ == FATAL)
		{
			g_flush();
			abort();
		}
	}

	void Logger::setLogLevel(Logger::LogLevel level)
	{
		g_logLevel = level;
	}

	void Logger::setOutput(OutputFunc out)
	{
		g_output = out;
	}

	void Logger::setFlush(FlushFunc flush)
	{
		g_flush = flush;
	}

	//void Logger::setTimeZone(const TimeZone& tz)
	//{
	//	g_logTimeZone = tz;
	//}


	void log(Logger::SourceFile file, int line, Logger::LogLevel level, const char* func, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		int len = std::vsnprintf(NULL, 0, fmt, args) + 1;
		char* buf = new char[len];
		std::vsnprintf(buf, len, fmt, args);
		va_end(args);
		hying::Logger(file, line, level, func).stream() << buf;
		delete[] buf;
	}
}