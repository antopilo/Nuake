#pragma once
#include "src/Core/Core.h"
#include "SkeletalAnimation.h"

namespace Nuake
{
	class Animator
	{
	private:
		float m_CurrentTime;
		float m_DeltaTime;

		uint32_t m_CurrentAnimation;
		uint32_t m_AnimationCount;
		std::vector<SkeletalAnimation> m_Animations;

	public:
		Animator(std::vector<SkeletalAnimation>& animations);
		~Animator() = default;

		void Update(float deltaTime);
		void PlayAnimation(uint32_t index);

		void CalculateBoneTransform(Matrix4& parentTransform);

		std::vector<Matrix4>& GetFinalBoneMatrices();
	};
}