#include "Thread.h"
#include "CurrentThread.h"
#include "Utils.h"

namespace hying
{
	std::atomic_int Thread::numCreated;
	Thread::Thread(ThreadFunc func, const std::string& name)
		: m_func(std::move(func)),
		  m_name(name),
		  m_tid(0),
		  m_running(false),
		  m_joined(false),
		  m_cond(1)
	{
		int num = ++numCreated;
		if (m_name.empty())
		{
			char buf[32];
			snprintf(buf, sizeof buf, "Thread%d", num);
			m_name = buf;
		}
	}

	Thread::~Thread()
	{
		std::cout << "~Thread()" << std::endl;
		if (m_thread && !m_joined)
		{
			m_thread->detach();
		}
	}

	void Thread::start()
	{
		assert(!m_running);
		m_running = true;
		m_thread = std::move(std::unique_ptr<std::thread>(new std::thread(&Thread::_run, this)));
		m_cond.wait();
		assert(m_tid > 0);
	}

	void Thread::stop()
	{
		m_running = false;
	}

	bool Thread::isRunning()
	{
		return m_running;
	}

	void Thread::join()
	{
		assert(m_thread);
		assert(!m_joined);
		m_thread->join();
		m_joined = true;
	}

	void Thread::_run()
	{
		m_tid = CurrentThread::tid();
		// 设置线程名
		CurrentThread::t_threadName = m_name.empty() ? "hyingThread" : m_name.c_str();
#ifndef WIN32
		::prctl(PR_SET_NAME, CurrentThread::t_threadName);
#else
		// windows10以上才能调用
		// SetThreadDescription(GetCurrentThread(), gbk_to_utf16(CurrentThread::t_threadName).c_str());
#endif

		m_cond.countDown();

		try {
			m_func();
			m_running = false;
			CurrentThread::t_threadName = "finished";
		}
		catch (const std::exception& ex) {
			CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "exception caught in Thread %s\n", m_name.c_str());
			fprintf(stderr, "reason: %s\n", ex.what());
			abort();
		}
		catch (...) {
			CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "unknown exception caught in Thread %s\n", m_name.c_str());
			throw; //重新丢出
		}
	}

}