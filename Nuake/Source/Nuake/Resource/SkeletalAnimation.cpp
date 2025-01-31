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

		m_PositionTransform = Matrix4(1.0f);
		m_RotationTransform = Matrix4(1.0f);
		m_ScaleTransform = Matrix4(1.0f);
		m_FinalTransform = Matrix4(1.0f);
	}

	float BoneTransformTrack::GetScaleFactor(float lastTime, float nextTime, float animationTime)
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTime;
		float framesDiff = nextTime - lastTime;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	Matrix4 BoneTransformTrack::InterpolatePosition(float time)
	{
		if (m_Positions.size() == 0)
		{
			return Matrix4(1.0f);
		}

		if (m_Positions.size() == 1)
		{
			return glm::translate(Matrix4(1.0f), m_Positions[0]);
		}

		// This returns the last position when we are at the last keyframe
		int p0Index = GetPositionIndex(time);
		if (p0Index == m_Positions.size() - 1)
		{
			return glm::translate(Matrix4(1.0f), m_Positions[p0Index]);
		}

		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_PositionTimestamps[p0Index], m_PositionTimestamps[p1Index], time);
		Vector3 finalPosition = glm::mix(m_Positions[p0Index], m_Positions[p1Index], scaleFactor);
		return glm::translate(Matrix4(1.0f), finalPosition);
	}

	Matrix4 BoneTransformTrack::InterpolateRotation(float time)
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

		// This returns the last rotation when we are at the last keyframe
		int p0Index = GetRotationIndex(time);
		if (p0Index == m_Rotations.size() - 1)
		{
			return glm::toMat4(m_Rotations[p0Index]);
		}

		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_RotationTimestamps[p0Index], m_RotationTimestamps[p1Index], time);
		Quat finalRotation = glm::slerp(m_Rotations[p0Index],m_Rotations[p1Index], scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return glm::toMat4(finalRotation);
	}

	Matrix4 BoneTransformTrack::InterpolateScale(float time)
	{
		if (m_Scales.size() == 0)
		{
			return Matrix4(1.0f);
		}

		if (1 == m_Scales.size())
		{
			return glm::scale(glm::mat4(1.0f), m_Scales[0]);
		}

		// This returns the last rotation when we are at the last keyframe
		int p0Index = GetScaleIndex(time);
		if (p0Index == m_Scales.size() - 1)
		{
			return glm::scale(Matrix4(1.0f), m_Scales[p0Index]);
		}

		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_ScaleTimestamps[p0Index], m_ScaleTimestamps[p1Index], time);
		Vector3 finalScale = glm::mix(m_Scales[p0Index], m_Scales[p1Index], scaleFactor);
		return glm::scale(Matrix4(1.0f), finalScale);
	}

	json BoneTransformTrack::Serialize()
	{
		BEGIN_SERIALIZE();
		for (uint32_t i = 0; i < m_PositionTimestamps.size(); i++)
		{
			j["m_PositionTimestamps"][i] = m_PositionTimestamps[i];
		}

		for (uint32_t i = 0; i < m_Positions.size(); i++)
		{
			j["m_Positions"][i] = SERIALIZE_VEC3(m_Positions[i]);
		}

		for (uint32_t i = 0; i < m_RotationTimestamps.size(); i++)
		{
			j["m_RotationTimestamps"][i] = m_RotationTimestamps[i];
		}

		for (uint32_t i = 0; i < m_Rotations.size(); i++)
		{
			j["m_Rotations"][i] = SERIALIZE_VEC4(m_Rotations[i]);
		}

		for (uint32_t i = 0; i < m_ScaleTimestamps.size(); i++)
		{
			j["m_ScaleTimestamps"][i] = m_ScaleTimestamps[i];
		}

		for (uint32_t i = 0; i < m_Scales.size(); i++)
		{
			j["m_Scales"][i] = SERIALIZE_VEC3(m_Scales[i]);
		}
		
		END_SERIALIZE();
	}

	bool BoneTransformTrack::Deserialize(const json& j)
	{
		return true;
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

	json SkeletalAnimation::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(m_Name);
		SERIALIZE_VAL(m_Duration);
		SERIALIZE_VAL(m_TicksPerSecond);
		SERIALIZE_VAL(m_CurrentTime);
		SERIALIZE_VAL(m_Loop);

		for (auto& t : m_Tracks)
		{
			j["Tracks"][t.first] = t.second.Serialize();
		}

		return json();
	}

	bool SkeletalAnimation::Deserialize(const json& j)
	{
		m_Name = j["m_Name"];
		m_Duration = j["m_Duration"];
		m_TicksPerSecond = j["m_TicksPerSecond"];
		m_CurrentTime = j["m_CurrentTime"];
		m_Loop = j["m_Loop"];

		for (auto& [trackName, trackData]: j["Tracks"].items())
		{
			BoneTransformTrack track;
			track.Deserialize(j["Tracks"][trackName]);
			m_Tracks[trackName] = std::move(track);
		}
		return true;
	}

}