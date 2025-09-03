#include "JobSystem.h"

#include <Tracy.hpp>

using namespace Nuake;

void JobSystem::Update()
{
	ZoneScoped;

	for(auto it = jobs.begin(); it != jobs.end();)
	{
		if(it->get()->IsDone())
		{
			it->get()->End();
			it = jobs.erase(it);
		}
		else
		{
			++it;
		}
	}
}


