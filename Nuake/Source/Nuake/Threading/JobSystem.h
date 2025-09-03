#pragma once
#include "Job.h"

namespace Nuake {

	class JobSystem
	{
	public:
		JobSystem() = default;
		~JobSystem() = default;

		static JobSystem& Get()
		{
			static JobSystem instance;
			return instance;
		}

	public:
		void Dispatch(std::function<void()> job, std::function<void()> end)
		{
			jobs.push_back(std::make_unique<Job>(job, end));
		}

		void Update();

	private:
		std::vector<std::unique_ptr<Job>> jobs;
	};
}