#pragma once
#include <string>
#include "../Core/Core.h"
#include "../Scene/Scene.h"
#include "Serializable.h"

#include "FGD/FGDFile.h"

namespace Nuake
{
	struct ProjectSettings
	{
		bool ShowGizmos = true;
		bool ShowAxis = true;
		bool VSync = true;
		float ResolutionScale = 1.0f;
		float OutlineRadius = 4.0f;
		float GizmoSize = 0.25f;
		bool SmoothCamera = false;
		float SmoothCameraSpeed = 0.2f;
		Color PrimaryColor = Color(97.0f / 255.0f, 0, 1.0f, 1.0f);
		int PhysicsStep = 90.0f;
		int MaxPhysicsSubStep = 32;
		int MaxPhysicsBodies = 4096;
		int MaxPhysicsContactConstraints = 2048;
		int MaxPhysicsBodyPair = 2048;

		// Audio
		float GlobalVolume = 1.0f;
		int MaxActiveVoiceCount = 16;

		json Serialize();
		bool Deserialize(const json& j);
	};

	class Project : public ISerializable
	{
	public:
		std::string Name;
		std::string Description;
		std::string FullPath;

		ProjectSettings Settings;

		// Path of the trenchbroom .exe folder
		std::string TrenchbroomPath = "";

		Ref<Scene> DefaultScene;
		Ref<FGDFile> EntityDefinitionsFile;

		Project(const std::string& Name, const std::string& Description, const std::string& FullPath, const std::string& defaultScenePath = "");
		Project();

		void Save();
		void SaveAs(const std::string& FullPath);
		bool FileExist();

		static Ref<Project> New(const std::string& Name, const std::string& Description, const std::string& FullPath);
		static Ref<Project> New();
		static Ref<Project> Load(std::string& path);

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}
