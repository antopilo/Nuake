#pragma once

namespace Nuake
{
	class OS 
	{
	public:
		static int GetTime() 
		{
			return static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
		}
	};
}
