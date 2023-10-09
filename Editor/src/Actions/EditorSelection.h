#pragma once
#include "src/Core/Core.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Resource/Resource.h"
#include "Engine.h"
#include "src/Rendering/SceneRenderer.h"

enum EditorSelectionType {
	None,
	Entity, 
	File,
	Resource,
	Directory
};

class EditorSelection {
public:

	EditorSelectionType Type = EditorSelectionType::None;

	Nuake::Entity Entity;
	Ref<Nuake::File> File;
	Ref<Nuake::Directory> Directory;
	Nuake::Resource Resource;

	EditorSelection() 
	{
		Type = None;
		if (const auto& scene = Nuake::Engine::GetCurrentScene(); scene)
		{
			Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = 0;
		}
	}

	EditorSelection(const Nuake::Entity& entity)
	{
		Type = EditorSelectionType::Entity;
		Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = (uint32_t)entity.GetHandle();
		Entity = entity;
	}

	EditorSelection(const Ref<Nuake::File>& file)
	{
		Type = EditorSelectionType::File;
		Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = 0;
		File = file;
	}

	EditorSelection(const Ref<Nuake::Directory>& dir)
	{
		Type = EditorSelectionType::Directory;
		Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = 0;
		Directory = dir;
	}

	EditorSelection(const Nuake::Resource& resource)
	{
		Type = EditorSelectionType::Resource;
		Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = 0;
		Resource = resource;
	}

	operator bool()
	{
		return Type != None;
	}
};