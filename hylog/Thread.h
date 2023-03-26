#ifndef __HYTHREAD_H__
#define __HYTHREAD_H__

#include "pch.h"
#include <thread>
#include <functional>
#include "CondCount.h"

namespace hying
{
	class Thread
	{
	public:
		typedef std::function<void()> ThreadFunc;
		explicit Thread(ThreadFunc func, const std::string& name = std::string());
		~Thread();

		void start();
		void stop();

		bool isRunning();

		void join();

	private:

		void _run();

		CondCount m_cond;
		std::unique_ptr<std::thread> m_thread;
		int m_tid;
		std::string m_name;
		std::atomic<bool> m_running;
		std::atomic<bool> m_joined;
		ThreadFunc m_func;

		static std::atomic_int numCreated;
	};
}

#endif
