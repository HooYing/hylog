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
    // 分配16个存储空间
    m_buffers.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // 当前缓冲剩余的空间足够大
    if (m_currentBuffer->avail() > len)
    {
        m_currentBuffer->append(logline, len);
    }
    else
    {
        // 当前缓冲满了，移入buffer
        m_buffers.push_back(std::move(m_currentBuffer));

        // 把预备缓冲移为当前缓冲
        if (m_nextBuffer)
        {
            m_currentBuffer = std::move(m_nextBuffer);
        }
        // 两块缓冲都用完了，再申请一块新的
        else
        {
            m_currentBuffer.reset(new Buffer); // Rarely happens
        }
        m_currentBuffer->append(logline, len);

        // 通知线程写入文件
        m_cond.notify_one();
    }
}

void AsyncLogging::threadFunc()
{
    // 创建日志文件
    LogFile output(m_basename, m_rollSize, false);

    // 准备两块buffer，以备在临界区内调用(m_currentBuffer, m_nextBuffer)
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    // 用来和前台的m_buffers进行交换
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    // 预备工作结束，线程真正启动
    m_latch.countDown();

    while (m_thread.isRunning())
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_buffers.empty())  // 没有需要写入的，等待指定的时间
            {
                m_cond.wait_for(lock, std::chrono::seconds(m_flushInterval));
            }
            // 再将当前缓冲放到buffers，无论满与不满
            m_buffers.push_back(std::move(m_currentBuffer));
            // 归还一个buffer
            m_currentBuffer = std::move(newBuffer1);
            // 使用的新的交换m_buffers,写入文件
            buffersToWrite.swap(m_buffers);
            // 保证前端始终有一个预备buffer可调配
            if (!m_nextBuffer)
            {
                m_nextBuffer = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        // 如果将要写入文件的buffer个数大于25，那么将多余数据删除
        // 消息堆积，前端陷入死循环，拼命发送日志消息，超过后端的处理能力
        // 典型的生产速度超过消费速度，会造成数据在内存中的堆积
        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                Timestamp::now().toFormattedString().c_str(),
                buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            // 丢掉多余日志，以腾出内存，仅保存两块缓冲区
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        // 将buffersToWrite的数据写入到日志中
        for (const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        // 重新调整buffersToWrite的大小
        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        // 如果两块备用buffer都为空，表明前端的两块buffer都用到了
        // 因此buffers肯定超过2
        // 刚好成环了：m_currentBuffer、m_nextBuffer -> m_buffers -> buffersToWrite -> newBuffer1、newBuffer2 -> (m_currentBuffer、m_nextBuffer)
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset(); //清理
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

