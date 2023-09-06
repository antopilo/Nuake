#include "SkeletalAnimation.h"

namespace Nuake
{
	BoneTransformTrack::BoneTransformTrack()
	{
		m_Positions = std::vector<Vector3>();
		m_Rotations = std::vector<Quat>();
		m_Scales = std::vector<Vector3>();

		m_PositionTimestamps = std::vector<float>();
		m_RotationTimestamps = std::vector<float>();
		m_ScaleTimestamps = std::vector<float>();
	}

	float BoneTransformTrack::GetScaleFactor(float lastTime, float nextTime, float animationTime)
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTime;
		float framesDiff = nextTime - lastTime;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	Nuake::Matrix4 BoneTransformTrack::InterpolatePosition(float time)
	{
		if (m_Positions.size() == 0)
		{
			return Matrix4(1.0f);
		}

		if (m_Positions.size() == 1)
		{
			return glm::translate(Matrix4(1.0f), m_Positions[0]);
		}

		int p0Index = GetPositionIndex(time);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_PositionTimestamps[p0Index], m_PositionTimestamps[p1Index], time);
		glm::vec3 finalPosition = glm::mix(m_Positions[p0Index], m_Positions[p1Index], scaleFactor);
		return glm::translate(Matrix4(1.0f), finalPosition);
	}

	Nuake::Matrix4 BoneTransformTrack::InterpolateRotation(float time)
	{
		if (m_Rotations.size() == 0)
		{
			return Matrix4(1.0f);
		}

		if (m_Rotations.size() == 1)
		{
			auto rotation = glm::normalize(m_Rotations[0]);
			return glm::toMat4(rotation);
		}

		int p0Index = GetRotationIndex(time);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_RotationTimestamps[p0Index],
			m_RotationTimestamps[p1Index], time);
		glm::quat finalRotation = glm::slerp(m_Rotations[p0Index],
			m_Rotations[p1Index], scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return glm::toMat4(finalRotation);
	}

	Nuake::Matrix4 BoneTransformTrack::InterpolateScale(float time)
	{
		if (m_Scales.size() == 0)
		{
			return Matrix4(1.0f);
		}

		if (1 == m_Scales.size())
			return glm::scale(glm::mat4(1.0f), m_Scales[0]);

		int p0Index = GetScaleIndex(time);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_ScaleTimestamps[p0Index],
			m_ScaleTimestamps[p1Index], time);
		glm::vec3 finalScale = glm::mix(m_Scales[p0Index], m_Scales[p1Index], scaleFactor);
		return glm::scale(glm::mat4(1.0f), finalScale);
	}

	SkeletalAnimation::SkeletalAnimation(const std::string& name, float duration, float ticksPerSecond)
	{
		m_Name = name;
		m_Duration = duration;
		m_TicksPerSecond = ticksPerSecond;
		m_CurrentTime = 0.0f;
		m_Loop = true;
	}

	BoneTransformTrack& SkeletalAnimation::GetTrack(const std::string& name)
	{
		if (m_Tracks.find(name) != m_Tracks.end())
		{
			return m_Tracks[name];
		}

		m_Tracks[name] = BoneTransformTrack();
		return m_Tracks[name];
	}
}