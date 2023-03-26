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
            // join�ȴ��̰߳���־��д�뵽�ļ�
            m_thread.join();
        }

    private:

        // �̺߳���
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
        // ����ȷ���̺߳���������
        CondCount m_latch;

        BufferPtr m_currentBuffer;  // ��ǰ����
        BufferPtr m_nextBuffer;     // Ԥ������
        BufferVector m_buffers;     // ��д���ļ��������Ļ���
	};
}

#endif