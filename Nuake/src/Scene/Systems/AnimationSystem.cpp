#include "AnimationSystem.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/SkinnedModelComponent.h"

namespace Nuake
{
	AnimationSystem::AnimationSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool AnimationSystem::Init()
	{
		return true;
	}

	void AnimationSystem::Update(Timestep ts)
	{
		auto view = m_Scene->m_Registry.view<TransformComponent, SkinnedModelComponent>();
		for (auto e : view)
		{
			auto [transformComponent, skinnedComponent] = view.get<TransformComponent, SkinnedModelComponent>(e);

			auto& model = skinnedComponent.ModelResource;
			if (!model)
			{
				continue;
			}

			Ref<SkeletalAnimation> animation = model->GetCurrentAnimation();
			float newAnimationTime = animation->GetCurrentTime() + (ts * animation->GetTicksPerSecond());
			animation->SetCurrentTime(newAnimationTime);

			auto& rootBone = model->GetSkeletonRootNode();
			UpdateBonePositionTraversal(rootBone, animation, animation->GetCurrentTime());
		}
	}

	void AnimationSystem::UpdateBonePositionTraversal(SkeletonNode& bone, Ref<SkeletalAnimation> animation, float time)
	{
		const std::string& boneName = bone.Name;
		

		Entity& boneEntity = m_Scene->GetEntity(boneName);
		if (boneEntity.GetHandle() != -1)
		{
			auto& animationTrack = animation->GetTrack(boneName);
			auto& transformComponent = boneEntity.GetComponent<TransformComponent>();

			// Get Update transform
			const Matrix4 newPosition = animationTrack.InterpolatePosition(time);
			const Matrix4 newRotation = animationTrack.InterpolateRotation(time);
			const Matrix4 newScale = animationTrack.InterpolateScale(time);
			const Matrix4 finalTransform = newPosition * newRotation * newScale;

			Vector3 localPosition;
			Quat localRotation;
			Vector3 localScale;
			Decompose(finalTransform, localPosition, localRotation, localScale);

			transformComponent.SetLocalPosition(localPosition);
			transformComponent.SetLocalRotation(localRotation);
			transformComponent.SetLocalScale(localScale);
			transformComponent.SetLocalTransform(finalTransform);
		}

		for (auto& childBone : bone.Children)
		{
			UpdateBonePositionTraversal(childBone, animation, time);
		}
	}

	void AnimationSystem::FixedUpdate(Timestep ts)
	{
		
	}

	void AnimationSystem::EditorUpdate()
	{

	}

	void AnimationSystem::Exit()
	{

	}
}
