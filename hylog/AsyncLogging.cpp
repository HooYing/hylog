#include "AsyncLogging.h"
#include "TimeStamp.h"

using namespace hying;

AsyncLogging::AsyncLogging(const std::string& basename,
    off_t rollSize,
    int flushInterval)
    : m_flushInterval(flushInterval),
    m_basename(basename),
    m_rollSize(rollSize),
    m_thread(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    m_latch(1),
    m_mutex(),
    m_cond(),
    m_currentBuffer(new Buffer),
    m_nextBuffer(new Buffer),
    m_buffers()
{
    m_currentBuffer->bzero();
    m_nextBuffer->bzero();
    // ����16���洢�ռ�
    m_buffers.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // ��ǰ����ʣ��Ŀռ��㹻��
    if (m_currentBuffer->avail() > len)
    {
        m_currentBuffer->append(logline, len);
    }
    else
    {
        // ��ǰ�������ˣ�����buffer
        m_buffers.push_back(std::move(m_currentBuffer));

        // ��Ԥ��������Ϊ��ǰ����
        if (m_nextBuffer)
        {
            m_currentBuffer = std::move(m_nextBuffer);
        }
        // ���黺�嶼�����ˣ�������һ���µ�
        else
        {
            m_currentBuffer.reset(new Buffer); // Rarely happens
        }
        m_currentBuffer->append(logline, len);

        // ֪ͨ�߳�д���ļ�
        m_cond.notify_one();
    }
}

void AsyncLogging::threadFunc()
{
    // ������־�ļ�
    LogFile output(m_basename, m_rollSize, false);

    // ׼������buffer���Ա����ٽ����ڵ���(m_currentBuffer, m_nextBuffer)
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    // ������ǰ̨��m_buffers���н���
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    // Ԥ�������������߳���������
    m_latch.countDown();

    while (m_thread.isRunning())
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_buffers.empty())  // û����Ҫд��ģ��ȴ�ָ����ʱ��
            {
                m_cond.wait_for(lock, std::chrono::seconds(m_flushInterval));
            }
            // �ٽ���ǰ����ŵ�buffers���������벻��
            m_buffers.push_back(std::move(m_currentBuffer));
            // �黹һ��buffer
            m_currentBuffer = std::move(newBuffer1);
            // ʹ�õ��µĽ���m_buffers,д���ļ�
            buffersToWrite.swap(m_buffers);
            // ��֤ǰ��ʼ����һ��Ԥ��buffer�ɵ���
            if (!m_nextBuffer)
            {
                m_nextBuffer = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        // �����Ҫд���ļ���buffer��������25����ô����������ɾ��
        // ��Ϣ�ѻ���ǰ��������ѭ����ƴ��������־��Ϣ��������˵Ĵ�������
        // ���͵������ٶȳ��������ٶȣ�������������ڴ��еĶѻ�
        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                Timestamp::now().toFormattedString().c_str(),
                buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            // ����������־�����ڳ��ڴ棬���������黺����
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        // ��buffersToWrite������д�뵽��־��
        for (const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        // ���µ���buffersToWrite�Ĵ�С
        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        // ������鱸��buffer��Ϊ�գ�����ǰ�˵�����buffer���õ���
        // ���buffers�϶�����2
        // �պóɻ��ˣ�m_currentBuffer��m_nextBuffer -> m_buffers -> buffersToWrite -> newBuffer1��newBuffer2 -> (m_currentBuffer��m_nextBuffer)
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset(); //����
        }
        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}

