#include "TransformSystem.h"
#include "src/Scene/Scene.h"
#include <src/Scene/Components/TransformComponent.h>
#include <src/Scene/Components/ParentComponent.h>

TransformSystem::TransformSystem(Scene* scene)
{
	m_Scene = scene;
}

bool TransformSystem::Init()
{
	UpdateTransform();
	return true;
}

void TransformSystem::Update(Timestep ts)
{
	UpdateTransform();
}

void TransformSystem::FixedUpdate(Timestep ts)
{

}

void TransformSystem::Exit()
{

}

void TransformSystem::UpdateTransform()
{
	auto transformView = m_Scene->m_Registry.view<ParentComponent, TransformComponent>();
	for (auto e : transformView) {
		auto [parent, transform] = transformView.get<ParentComponent, TransformComponent>(e);
		Entity currentParent = Entity((entt::entity)e, m_Scene);
		Vector3 globalPos = Vector3();
		if (parent.HasParent)
		{
			while (currentParent.GetComponent<ParentComponent>().HasParent) 
			{
				currentParent = currentParent.GetComponent<ParentComponent>().Parent;
				globalPos += currentParent.GetComponent<TransformComponent>().Translation;
			}

			transform.GlobalTranslation = globalPos + transform.Translation;
		}
		else
		{
			transform.GlobalTranslation = transform.Translation;
		}
	}
}