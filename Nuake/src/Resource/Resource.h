#pragma once
#include "src/Resource/UUID.h"

#include <string>

namespace Nuake 
{
	class Resource
	{
	public:
		UUID ID;

		bool IsEmbedded = false;
		std::string Path; // Only if embedded

		void MakeExternal();
		void Duplicate();
		void MakeEmbedded();
	};
}
