#ifndef __HYCONDCOUNT_H__
#define __HYCONDCOUNT_H__

#include "pch.h"
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace hying
{
	class CondCount
	{
	public:
		explicit CondCount(int count = 1);

		void wait();

		void countDown();

		int getCount() const;

	private:
		mutable std::mutex m_mutex;
		std::condition_variable m_condition;
		std::atomic<int> m_count;
	};
}

#endif