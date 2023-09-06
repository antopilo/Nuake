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

	struct BoneTransformTrack
	{
		std::vector<float> positionTimestamps = {};
		std::vector<float> rotationTimestamps = {};
		std::vector<float> scaleTimestamps = {};

		std::vector<Vector3> positions = {};
		std::vector<Quat> rotations = {};
		std::vector<Vector3> scales = {};
	};

	class SkeletalAnimation
	{
	private:
		float m_Duration;
		int m_TicksPerSecond;

		std::unordered_map<std::string, BoneTransformTrack> m_Tracks;
		std::vector<Bone> m_Bones;

	public:
		SkeletalAnimation() = default;
		~SkeletalAnimation() = default;

		Bone& FindBone(const std::string& boneName);

		float GetTicksPerSecond() const { return m_TicksPerSecond; }
	};
}
