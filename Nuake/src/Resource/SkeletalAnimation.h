#pragma once
#include <src/Rendering/Mesh/Bone.h>

namespace Nuake
{
	struct SkeletonAnimationBoneAnimation
	{
		std::string Name;
		Vector3 Positions;
		Quat Rotations;
		Vector3 Scalings;
	};

	class SkeletalAnimation
	{
	private:
		float m_Duration;
		int m_TicksPerSecond;

		std::vector<Bone> m_Bones;

	public:
		SkeletalAnimation() = default;
		~SkeletalAnimation() = default;

		Bone& FindBone(const std::string& boneName);

		float GetTicksPerSecond() const { return m_TicksPerSecond; }
	};
}
