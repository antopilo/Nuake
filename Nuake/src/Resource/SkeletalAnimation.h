#pragma once
#include "src/Core/Core.h"
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

	class BoneTransformTrack
	{
	private:
		std::vector<float> m_PositionTimestamps = {};
		std::vector<float> m_RotationTimestamps = {};
		std::vector<float> m_ScaleTimestamps = {};

		std::vector<Vector3> m_Positions = {};
		std::vector<Quat> m_Rotations = {};
		std::vector<Vector3> m_Scales = {};

	public:
		BoneTransformTrack();
		~BoneTransformTrack() = default;

		void PushPositionKeyframe(float timestamp, const Vector3& position)
		{
			m_PositionTimestamps.push_back(timestamp);
			m_Positions.push_back(position);
		}

		void PushRotationKeyframe(float timestamp, const Quat& rotation)
		{
			m_RotationTimestamps.push_back(timestamp);
			m_Rotations.push_back(rotation);
		}

		void PushScaleKeyframe(float timestamp, const Vector3& scale)
		{
			m_ScaleTimestamps.push_back(timestamp);
			m_Scales.push_back(scale);
		}

		int GetPositionIndex(float animationTime)
		{
			if (m_Positions.size() == 0)
			{
				return 0;
			}

			for (int index = 0; index < m_Positions.size() - 1; index++)
			{
				if (animationTime < m_PositionTimestamps[index + 1])
				{
					return index;
				}
			}
			assert(0);
		}

		/* Gets the current index on mKeyRotations to interpolate to based on the
		current animation time*/
		int GetRotationIndex(float animationTime)
		{
			for (int index = 0; index < m_Rotations.size() - 1; ++index)
			{
				if (animationTime < m_RotationTimestamps[index + 1])
				{
					return index;
				}
			}
			assert(0);
		}

		/* Gets the current index on mKeyScalings to interpolate to based on the
		current animation time */
		int GetScaleIndex(float animationTime)
		{
			for (int index = 0; index < m_Scales.size() - 1; ++index)
			{
				if (animationTime < m_ScaleTimestamps[index + 1])
				{
					return index;
				}
			}
			assert(0);
		}

		float GetScaleFactor(float lastTime, float nextTime, float animationTime);
		Matrix4 InterpolatePosition(float time);
		Matrix4 InterpolateRotation(float time);
		Matrix4 InterpolateScale(float time);
	};

	class SkeletalAnimation
	{
	private:
		std::unordered_map<std::string, BoneTransformTrack> m_Tracks;
		float m_CurrentTime;
		float m_Duration;
		float m_TicksPerSecond;
		std::string m_Name;
		bool m_Loop;

	public:
		SkeletalAnimation() = default;
		SkeletalAnimation(const std::string& name, float duration, float ticksPerSecond);

		~SkeletalAnimation() = default;

		void SetCurrentTime(float time) 
		{
			m_CurrentTime = fmod(time, m_Duration);
		}

		float GetCurrentTime() const { return m_CurrentTime; }
		void SetDuration(float duration) { m_Duration = duration; }
		float GetDuration() const { return m_Duration; }
		float GetTicksPerSecond() const { return m_TicksPerSecond; }
		void SetTicksPerSecond(float ticks) { m_TicksPerSecond = ticks; }

		BoneTransformTrack& GetTrack(const std::string& name);
		std::unordered_map<std::string, BoneTransformTrack>& GetTracks() { return m_Tracks; }
	};
}
