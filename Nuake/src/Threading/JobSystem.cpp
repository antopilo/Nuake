#include "JobSystem.h"

#include <Tracy.hpp>

namespace Nuake {

	void JobSystem::Update()
	{
		ZoneScoped;

		for(auto it = m_Jobs.begin(); it != m_Jobs.end();)
		{
			if(it->get()->IsDone())
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
}


