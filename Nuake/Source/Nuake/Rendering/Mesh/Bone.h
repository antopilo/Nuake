#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"


namespace Nuake
{
	struct KeyPosition
	{
		Vector3 Position;
		float Timestamp;
	};

	struct KeyRotation
	{
		Quat Orientation;
		float Timestamp;
	};

	struct KeyScale
	{
		Vector3 Scale;
		float Timestamp;
	};

	struct BoneVertexWeight
	{
		uint32_t VertexID;
		float Weight;
	};

	class Bone
	{
	public:
		std::string Name;
		uint32_t Id;
		Matrix4 Offset;
		std::vector<BoneVertexWeight> VertexWeights;

	private:
		std::vector<KeyPosition> m_Positions;
		std::vector<KeyRotation> m_Rotations;
		std::vector<KeyScale> m_Scales;
		uint32_t m_NumPositions;
		uint32_t m_NumRotations;
		uint32_t m_NumScales;

		Matrix4 m_LocalTransform;

	public:
		Bone() = default;
		Bone(const std::string& name, int id);

		void PushPositionKeyframe(KeyPosition& key);
		void PushRotationframe(KeyRotation& key);
		void PushScaleKeyframe(KeyScale& key);

		void Update(float time);

		Matrix4 GetLocalTransform() const;
		int GetPositionIndex(float time) const;
		int GetRotationIndex(float time) const;
		int GetScaleIndex(float time) const;

	private:
		float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const;

		Matrix4 InterpolatePosition(float animationTime) const ;
		Matrix4 InterpolateRotation(float animationTime) const;
		Matrix4 InterpolateScaling(float animationTime) const;
	};
}
