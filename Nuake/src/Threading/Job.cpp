#include "Job.h"
#include <Tracy.hpp>

namespace Nuake {

	Job::Job(std::function<void()> job, std::function<void()> end)
		: m_Job(job)
		, m_End(end)
	{
		m_End = end;

		m_Thread = std::thread([this, job]()
		{
			ZoneScoped;
			job();
			m_IsDone = true;
		});
	}

	void Job::End()
	{
		if (m_End)
		{
			m_End();
		}
	}
}