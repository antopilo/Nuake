#pragma once
#include "Job.h"

namespace Nuake {

	class JobSystem
	{
	private:
		std::vector<std::unique_ptr<Job>> m_Jobs;

	public:

		JobSystem() = default;
		~JobSystem() = default;

		static JobSystem& Get()
		{
			static JobSystem instance;
			return instance;
		}

		void Dispatch(std::function<void()> job, std::function<void()> end)
		{
			m_Jobs.push_back(std::make_unique<Job>(job, end));
		}

		void Update()
		{
			for (auto it = m_Jobs.begin(); it != m_Jobs.end();)
			{
				if (it->get()->IsDone()) 
				{
					it->get()->End();
					it = m_Jobs.erase(it);
				}
				else 
				{
					++it;
				}
			}
		}
	};
}