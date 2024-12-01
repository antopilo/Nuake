#include "AnimationSystem.h"

#include <Engine.h>
#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/SkinnedModelComponent.h"
#include <src/Scene/Components/BoneComponent.h>
#include <future>

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
		if (Engine::GetGameState() == GameState::Paused)
		{
			return;
		}

		auto view = m_Scene->m_Registry.view<TransformComponent, SkinnedModelComponent>();
		for (auto e : view)
		{
			auto [transformComponent, skinnedComponent] = view.get<TransformComponent, SkinnedModelComponent>(e);

			Ref<SkinnedModel> model = skinnedComponent.ModelResource;

			// Load the model if it's not already loaded
			// TODO: [WiggleWizard] Needs some sort of flag to infer that we've tried loading it once already, so
			// we don't try loading the model every frame if it's invalid.
			if (skinnedComponent.ModelPath.Exist() && skinnedComponent.ModelResource == nullptr)
			{
				skinnedComponent.LoadModel(e, m_Scene);
			}
			
			if (!model)
			{
				continue;
			}

			float animTime = 0.0f;
			float prevAnimTime = 0.0f;
			Ref<SkeletalAnimation> anim = model->GetCurrentAnimation();
			Ref<SkeletalAnimation> prevAnim = model->GetPreviousAnimation();
			float blendWeight = model->GetCurrentBlendTime();
			if (anim && model->IsPlaying)
			{
				animTime = anim->GetCurrentTime() + (ts * anim->GetTicksPerSecond());
				anim->SetCurrentTime(animTime);

				model->SetCurrentBlendTime(blendWeight - ts);
				Logger::Log("Blend time: " + std::to_string(model->GetCurrentBlendTime()));
				prevAnimTime = prevAnim->GetCurrentTime() + (ts * prevAnim->GetTicksPerSecond());
				prevAnim->SetCurrentTime(prevAnimTime);
			}

			auto& rootBone = model->GetSkeletonRootNode();
			UpdateBonePositionTraversal(rootBone, anim, prevAnim, animTime, prevAnimTime, model->IsPlaying, model->GetCurrentBlendTime());
		}
	}

	void AnimationSystem::UpdateBonePositionTraversal(SkeletonNode& bone, Ref<SkeletalAnimation> animation, Ref<SkeletalAnimation> prevAnimation, float time, float prevTime, bool isPlaying, float blendTime)
	{
		if (Entity boneEnt = m_Scene->GetEntityByID(bone.EntityHandle); 
			boneEnt.IsValid())
		{
			// Update the bone transform
			auto& transformComponent = boneEnt.GetComponent<TransformComponent>();
			bone.FinalTransform = transformComponent.GetGlobalTransform() * bone.Offset;
			
			if (animation)
			{
				if (auto& animationTrack = animation->GetTrack(bone.Name); 
					!animationTrack.IsEmpty() && isPlaying)
				{
					// Get Update transform
					animationTrack.Update(time);

					const Matrix4& finalTransform = animationTrack.GetFinalTransform();

					Vector3 localPosition;
					Quat localRotation;
					Vector3 localScale;
					Decompose(finalTransform, localPosition, localRotation, localScale);

					if (auto& prevAnimTrack = prevAnimation->GetTrack(bone.Name); !prevAnimTrack.IsEmpty())
					{
						prevAnimTrack.Update(prevTime);
						Matrix4 prevTransform = prevAnimTrack.GetFinalTransform();

						Vector3 prevLocalPosition;
						Quat prevLocalRotation;
						Vector3 prevLocalScale;
						Decompose(prevTransform, prevLocalPosition, prevLocalRotation, prevLocalScale);

						localPosition = glm::mix(localPosition, prevLocalPosition, blendTime);
						localRotation = glm::slerp(localRotation, prevLocalRotation, blendTime);
						prevLocalScale = glm::mix(localScale, prevLocalScale, blendTime);
					}

					Matrix4 finalLocalTransform = Matrix4(1.0f);
					auto translateMatrix = glm::translate(Matrix4(1.0), localPosition);
					auto RotationMatrix = glm::mat4_cast(localRotation);
					auto scaleMatrix = glm::scale(Matrix4(1.0f), localScale);
					
					auto finalMatrix = translateMatrix * RotationMatrix * scaleMatrix;
					transformComponent.SetLocalPosition(localPosition);
					transformComponent.SetLocalRotation(localRotation);
					transformComponent.SetLocalScale(localScale);
					transformComponent.SetLocalTransform(finalMatrix);
				}
			}
			
		}

		for (auto& childBone : bone.Children)
		{
			UpdateBonePositionTraversal(childBone, animation, prevAnimation, time, prevTime, isPlaying, blendTime);
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
