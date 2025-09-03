#include "Bone.h"
#include "Nuake/Core/Logger.h"

namespace Nuake
{
	Bone::Bone(const std::string& name, int id)
	{
		Name = name;
		Id = id;
		m_LocalTransform = Matrix4(1.0f);
	}

	void Bone::PushPositionKeyframe(KeyPosition& key)
	{
		m_Positions.push_back(std::move(key));
	}

	void Bone::PushRotationframe(KeyRotation& key)
	{
		m_Rotations.push_back(std::move(key));
	}

	void Bone::PushScaleKeyframe(KeyScale& key)
	{
		m_Scales.push_back(std::move(key));
	}

	void Bone::Update(float time)
	{
		Matrix4 translation = InterpolatePosition(time);
		Matrix4 rotation = InterpolateRotation(time);
		Matrix4 scale = InterpolateScaling(time);
		m_LocalTransform = translation * rotation * scale;
	}

	Matrix4 Bone::GetLocalTransform() const
	{
		return m_LocalTransform;
	}

	int Bone::GetPositionIndex(float time) const
	{
		for (uint32_t i = 0; i < m_NumPositions - 1; i++)
		{
			if (time < m_Positions[i + 1].Timestamp)
			{
				return i;
			}
		}

		Logger::Log("Bone position keyframe not found", "Bone system", CRITICAL);
		assert(false);

		return 0;
	}

	int Bone::GetRotationIndex(float time) const
	{
		for (uint32_t i = 0; i < m_NumRotations - 1; i++)
		{
			if (time < m_Rotations[i + 1].Timestamp)
			{
				return i;
			}
		}

		Logger::Log("Bone rotation keyframe not found", "Bone system", CRITICAL);
		assert(false);

		return 0;
	}

	int Bone::GetScaleIndex(float time) const
	{
		for (uint32_t i = 0; i < m_NumScales - 1; i++)
		{
			if (time < m_Scales[i + 1].Timestamp)
			{
				return i;
			}
		}

		Logger::Log("Bone scale keyframe not found", "Bone system", CRITICAL);
		assert(false);

		return 0;
	}

	float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	Matrix4 Bone::InterpolatePosition(float animationTime) const
	{
		if (m_NumPositions == 1)
		{
			return glm::translate(Matrix4(1.0f), m_Positions[0].Position);
		}

		const int keyFramePresentIndex = GetPositionIndex(animationTime);
		const int keyFrameFutureIndex = keyFramePresentIndex + 1;

		const KeyPosition& keyFramePresent = m_Positions[keyFramePresentIndex];
		const KeyPosition& keyFrameFuture = m_Positions[keyFrameFutureIndex];

		const float scaleFactor = GetScaleFactor(keyFramePresent.Timestamp, keyFrameFuture.Timestamp, animationTime);
		const Vector3 finalPosition = glm::mix(keyFramePresent.Position, keyFrameFuture.Position, scaleFactor);
		return glm::translate(Matrix4(1.0f), finalPosition);
	}

	Matrix4 Bone::InterpolateRotation(float animationTime) const
	{
		if (m_NumRotations == 1)
		{
			auto rotation = glm::normalize(m_Rotations[0].Orientation);
			return glm::toMat4(rotation);
		}

		const int keyFramePresentIndex = GetRotationIndex(animationTime);
		const int keyFrameFutureIndex = keyFramePresentIndex + 1;

		const KeyRotation& keyFramePresent = m_Rotations[keyFramePresentIndex];
		const KeyRotation& keyFrameFuture = m_Rotations[keyFrameFutureIndex];

		const float scaleFactor = GetScaleFactor(keyFramePresent.Timestamp, keyFrameFuture.Timestamp, animationTime);
		const Quat& finalRotation = glm::slerp(keyFramePresent.Orientation, keyFramePresent.Orientation, scaleFactor);
		return glm::toMat4(finalRotation);
	}
	
	Matrix4 Bone::InterpolateScaling(float animationTime) const
	{
		if (m_NumScales == 1)
		{
			return glm::scale(Matrix4(1.0f), m_Scales[0].Scale);
		}

		const int keyFramePresentIndex = GetScaleIndex(animationTime);
		const int keyFrameFutureIndex = keyFramePresentIndex + 1;

		const KeyScale& keyFramePresent = m_Scales[keyFramePresentIndex];
		const KeyScale& keyFrameFuture = m_Scales[keyFrameFutureIndex];

		const float scaleFactor = GetScaleFactor(keyFramePresent.Timestamp, keyFrameFuture.Timestamp, animationTime);
		const Vector3 finalScale = glm::mix(keyFramePresent.Scale, keyFramePresent.Scale, scaleFactor);
		return glm::scale(Matrix4(1.0f), finalScale);
	}
}