#include "Job.h"
#include <Tracy.hpp>

using namespace Nuake;

Job::Job(std::function<void()> logic, std::function<void()> endLogic) : 
	job(logic),
	end(endLogic)
{
	this->thread = std::thread([this, logic]()
	{
		ZoneScoped;
		job();
		isDone = true;
	});
}

void Job::End()
{
	if (end)
	{
		end();
	}
}