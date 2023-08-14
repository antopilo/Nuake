#pragma once
#include <src/Rendering/Mesh/Bone.h>

namespace Nuake
{
	class Animation
	{
	private:
		float m_Duration;
		int m_TicksPerSecond;
		std::vector<Bone> m_Bones;
		//std::map<std::string, BoneInfo> m_BoneInfoMap;
	public:
		Animation() = default;
		Animation();

		Bone& FindBone(const std::string& boneName);

		float GetTicksPerSecond() const { return m_TicksPerSecond; }
	};
}
