#include "Animator.h"

#include "src/Core/Logger.h"

namespace Nuake
{
	Animator::Animator(std::vector<SkeletalAnimation>& animations)
	{
		m_AnimationCount = static_cast<uint32_t>(animations.size());
		m_CurrentAnimation = 0;

		m_Animations = animations;
		m_CurrentTime = 0.0f;
		m_DeltaTime = 0.0f;
	}

	void Animator::PlayAnimation(uint32_t index)
	{
		if (index < m_AnimationCount)
		{
			m_CurrentAnimation = index;
		}
		else
		{
			m_CurrentAnimation = m_AnimationCount - 1;
			Logger::Log("Animation index out of range", "animator", WARNING);
		}
	}
}