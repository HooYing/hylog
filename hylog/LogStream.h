#ifndef __HYLOGSTREAM_H__
#define __HYLOGSTREAM_H__

#include "pch.h"

namespace hying
{
	const int kSmallBuffer = 4000;
	const int kLargeBuffer = 4000 * 1000;

	template<int SIZE>
	class FixedBuffer
	{
	public:
		FixedBuffer() : cur_(data_)
		{
			setCookie(cookieStart);
		}

		~FixedBuffer()
		{
			setCookie(cookieEnd);
		}

		void append(const char* /*restrict*/ buf, size_t len)
		{
			// FIXME: append partially
			if (static_cast<size_t>(avail()) > len)
			{
				memcpy(cur_, buf, len);
				cur_ += len;
			}
		}

		const char* data() const { return data_; }
		int length() const { return static_cast<int>(cur_ - data_); }

		// 直接写入data_
		char* current() { return cur_; }
		int avail() const { return static_cast<int>(end() - cur_); }
		void add(size_t len) { cur_ += len; }

		void reset() { cur_ = data_; }
		void bzero() { memset(data_, 0, sizeof data_); }

		// gdb调试用
		const char* debugString();
		void setCookie(void (*cookie)()) { cookie_ = cookie; }
		// 单元测试用
		std::string toString() const { return std::string(data_, length()); }
		//StringPiece toStringPiece() const { return StringPiece(data_, length()); }
	private:
		const char* end() const { return data_ + sizeof data_; }
		// 必须在类外定义
		static void cookieStart();
		static void cookieEnd();
	private:
		void (*cookie_)();
		char data_[SIZE];
		char* cur_;
	};


	class LogStream
	{
	public:
		typedef FixedBuffer<kSmallBuffer> Buffer;
		LogStream& operator << (bool v)
		{
			buffer_.append(v ? "1" : "0", 1);
			return *this;
		}
		
		// 因为使用到了模板函数formatInteger，因此要定义在cpp上
		LogStream& operator << (int v);
		LogStream& operator << (unsigned int v);
		LogStream& operator << (long v);
		LogStream& operator << (unsigned long v);
		LogStream& operator << (long long v);
		LogStream& operator << (unsigned long long v);

		LogStream& operator << (short v)
		{
			*this << static_cast<int>(v);
			return *this;
		}

		LogStream& operator << (unsigned short v)
		{
			*this << static_cast<unsigned int>(v);
			return *this;
		}

		LogStream& operator << (const void*);

		LogStream& operator << (double v)
		{
			if (buffer_.avail() >= kMaxNumericSize)
			{
				int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
				buffer_.add(len);
			}
			return *this;
		}

		LogStream& operator << (float v)
		{
			*this << static_cast<double>(v);
			return *this;
		}

		LogStream& operator << (char v)
		{
			buffer_.append(&v, 1);
			return *this;
		}

		LogStream& operator << (const char* str)
		{
			if (str)
			{
				buffer_.append(str, strlen(str));
			}
			else
			{
				buffer_.append("(null)", 6);
			}
			return *this;
		}

		LogStream& operator << (const unsigned char* str)
		{
			*this << reinterpret_cast<const char*>(str);
			return *this;
		}

		LogStream& operator << (const std::string& v)
		{
			buffer_.append(v.c_str(), v.size());
			return *this;
		}

		/*
		LogStream& operator<<(const StringPiece& v)
		{
			buffer_.append(v.data(), v.size());
			return *this;
		}

		LogStream& operator<<(const Buffer& v)
		{
			*this << v.toStringPiece();
			return *this;
		}

		*/

		void append(const char* data, int len) { buffer_.append(data, len); }
		const Buffer& buffer() const { return buffer_; }
		void resetBuffer() { buffer_.reset(); }

	private:
		void staticCheck();

		template<typename T>
		void formatInteger(T);

	private:
		Buffer buffer_;
		static const int kMaxNumericSize = 48;
	};

	class Fmt // : noncopyable
	{
	public:
		template<typename T>
		Fmt(const char* fmt, T val);

		const char* data() const { return buf_; }
		int length() const { return length_; }

	private:
		char buf_[32];
		int length_;
	};

	inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
	{
		s.append(fmt.data(), fmt.length());
		return s;
	}

	std::string formatSI(int64_t n);
	std::string formatIEC(int64_t n);
}

#endif