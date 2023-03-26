#ifndef __ASYNCLOGGING_H__
#define __ASYNCLOGGING_H__

#include "pch.h"
#include "Thread.h"
#include "LogFile.h"
#include "LogStream.h"
#include <vector>
#include <condition_variable>

namespace hying
{
	class AsyncLogging
	{
	public:
        AsyncLogging(const std::string& basename, off_t rollSize, int flushInterval = 3);

        ~AsyncLogging()
        {
            printf("~AsyncLogging()\n");
            if (m_thread.isRunning())
            {
                stop();
            }
        }

        void append(const char* logline, int len);

        void start()
        {
            m_thread.start();
            m_latch.wait();
        }

        void stop()
        {
            m_thread.stop();
            m_cond.notify_one();
            // join等待线程把日志都写入到文件
            m_thread.join();
        }

    private:

        // 线程函数
        void threadFunc();

        using Buffer = FixedBuffer<kLargeBuffer>;
        using BufferVector = std::vector< std::unique_ptr<Buffer> >;
        using BufferPtr = std::unique_ptr<Buffer>;

        const int m_flushInterval;
        const std::string m_basename;
        const off_t m_rollSize;

        std::mutex m_mutex;
        std::condition_variable m_cond;

        Thread m_thread;
        // 用来确保线程函数启动了
        CondCount m_latch;

        BufferPtr m_currentBuffer;  // 当前缓冲
        BufferPtr m_nextBuffer;     // 预备缓冲
        BufferVector m_buffers;     // 待写入文件已填满的缓冲
	};
}

#endif