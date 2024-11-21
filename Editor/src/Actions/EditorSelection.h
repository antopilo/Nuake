#pragma once
#include "src/Core/Core.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Rendering/SceneRenderer.h"
#include "src/Resource/Resource.h"
#include "Engine.h"



enum EditorSelectionType 
{
	None,
	Entity, 
	File,
	Resource,
	Directory
};


class EditorSelection 
{

public:
	EditorSelectionType Type = EditorSelectionType::None;

	Ref<Nuake::File> File;
	Ref<Nuake::Directory> Directory;
	Nuake::Entity Entity;
	Nuake::Resource Resource;

public:
	EditorSelection() 
	{
		Type = None;
		if (const auto& scene = Nuake::Engine::GetCurrentScene(); scene)
		{
			Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = -1;
		}
	}

	EditorSelection(const Nuake::Entity& entity)
	{
		Type = EditorSelectionType::Entity;
		Entity = entity;
	}

	EditorSelection(const Ref<Nuake::File>& file)
	{
		Type = EditorSelectionType::File;
		Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = -1;
		File = file;
	}

	EditorSelection(const Ref<Nuake::Directory>& dir)
	{
		Type = EditorSelectionType::Directory;
		Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = -1;
		Directory = dir;
	}

	EditorSelection(const Nuake::Resource& resource)
	{
		Type = EditorSelectionType::Resource;
		Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = -1;
		Resource = resource;
	}

	operator bool()
	{
		return Type != None;
	}
};