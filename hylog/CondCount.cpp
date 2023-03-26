#include "CondCount.h"
#include "Utils.h"
namespace hying
{
	CondCount::CondCount(int count)
		: m_count(count)
	{

	}

	void CondCount::wait()
	{
		std::unique_lock<std::mutex> lck(m_mutex);
		while (m_count > 0)
		{
			m_condition.wait(lck);
		}
	}

	void CondCount::countDown()
	{
		std::unique_lock<std::mutex> lck(m_mutex);
		--m_count;
		if (m_count == 0)
		{
			m_condition.notify_all();
		}
	}

	int CondCount::getCount() const
	{
		std::unique_lock<std::mutex> lck(m_mutex);
		return m_count;
	}
}