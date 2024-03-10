#pragma once

#include <atomic>
#include <functional>
#include <thread>

namespace Nuake {

	class Job
	{
	public:
		Job(std::function<void()> job, std::function<void()> end);
		Job(const Job&) = delete;
		Job& operator=(const Job&) = delete;
		~Job() { m_Thread.join(); }
		bool IsDone() { return m_IsDone; }

		void End();
	private:
		std::thread m_Thread;
		std::atomic<bool> m_IsDone;
		std::function<void()> m_Job;
		std::function<void()> m_End;
	};
}