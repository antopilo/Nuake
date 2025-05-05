#pragma once

#include <volk/volk.h>

namespace Nuake
{
	enum class SamplerType
	{
		Linear,
		Nearest
	};

	class Sampler
	{
	private:
		VkSampler vkSampler;
		float maxAnisotropy;

	public:
		Sampler(SamplerType samplerType);
		~Sampler();
	};
}