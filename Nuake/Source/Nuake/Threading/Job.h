#pragma once

#include <atomic>
#include <functional>
#include <thread>

namespace Nuake {

	class Job
	{
	public:
		Job(std::function<void()> logic, std::function<void()> endLogic);
		Job(const Job&) = delete;
		Job& operator=(const Job&) = delete;
		~Job() { thread.join(); }

	public:
		bool IsDone() { return isDone; }

		void End();

	private:
		std::thread thread;
		std::atomic<bool> isDone;
		std::function<void()> job;
		std::function<void()> end;
	};
}