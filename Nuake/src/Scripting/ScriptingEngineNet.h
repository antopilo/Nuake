#pragma once
#include "src/Core/Core.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scripting/NetModules/NetAPIModule.h"

// For some reason HostInstance.hpp doesn't include filesystem but it needs it.
#include <filesystem>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>
#include <Coral/Assembly.hpp>

#include <any>


namespace Coral
{
	class HostInstance;
	class Type;
}

namespace Nuake 
{
	class Project;

	enum class ExposedVarTypes
	{
		Bool,
		Float,
		Double,
		String,
		Entity,
		Prefab,
		Vector2,
		Vector3,
		Vector4,
		Int,
		Unsupported
	};

	struct ExposedVar
	{
		ExposedVarTypes Type;
		std::any Value;
		std::string Name;
	};

	struct UIWidgetObject
	{
		Coral::Type* coralType;
		std::string htmlPath;
	};

	struct NetGameScriptObject 
	{
		Coral::Type* coralType;
		std::vector<ExposedVar> exposedVars;

		std::string Base = "";
		bool isTrigger = false;
		std::string Description = "";
		AABB aabb = AABB({0, 0, 0}, { 1, 1, 1 });
	};

	struct CompilationError
	{
		std::string file;
		int line;
		std::string message;
	};

	class ScriptingEngineNet
	{
	public:
		static ScriptingEngineNet& Get();

		void Initialize();
		void Uninitialize();
		bool IsInitialized() const { return isInitialized; }

		Coral::HostInstance* GetHostInstance() { return hostInstance; }
		Coral::AssemblyLoadContext& GetLoadContext() { return loadContext; }
		Coral::ManagedAssembly GetNuakeAssembly() const { return nuakeAssembly; }

		Coral::ManagedAssembly ReloadEngineAPI(Coral::AssemblyLoadContext & context);

		void GenerateSolution(const std::string& path, const std::string& projectName);
		void CopyNuakeNETAssemblies(const std::string& path);
		std::vector<CompilationError> BuildProjectAssembly(Ref<Project> project);
		void LoadProjectAssembly(Ref<Project> project);

		void CreateEntityScript(const std::string& path, const std::string& entityName);
		void RegisterEntityScript(Entity& entity);
		Coral::ManagedObject GetEntityScript(const Entity& entity);
		bool HasEntityScriptInstance(const Entity& entity);
		std::string FindClassNameInScript(const std::string& filePath);

		void UpdateEntityWithExposedVar(Entity entity);
		std::vector<ExposedVar> GetExposedVarForTypes(Entity entity);

		bool HasUIWidget(const std::string& widgetName);
		UIWidgetObject& GetUIWidget(const std::string& widgetName);
		void RegisterCustomWidgetInstance(const UUID& uuid, const std::string& widgetTypeName);
		bool HasCustomWidgetInstance(const UUID& uuid);
		Coral::ManagedObject GetCustomWidgetInstance(const UUID& uuid);

		std::unordered_map<std::string, NetGameScriptObject> GetBrushEntities() const { return brushEntityTypes; }
		std::unordered_map<std::string, NetGameScriptObject> GetPointEntities() const { return pointEntityTypes; }
		std::unordered_map<std::string, UIWidgetObject> GetUIWidgets() const { return uiWidgets; }

	private:
		const std::string m_Scope = "Nuake.Net";
		const std::string m_EngineAssemblyName = "NuakeNet.dll";
		const std::string m_NetDirectory = ".net";
		const std::string m_ContextName = "NuakeEngineContext";

		Coral::Type baseEntityType;
		Coral::Type prefabType;
		bool isInitialized = false;

		Coral::HostInstance* hostInstance;
		Coral::AssemblyLoadContext loadContext;

		std::unordered_map<std::string, Coral::AssemblyLoadContext*> loadedAssemblies;
		std::vector<Ref<NetAPIModule>> modules;

		Coral::ManagedAssembly nuakeAssembly; // Nuake DLL
		Coral::ManagedAssembly gameAssembly;	// Game DLL

		// This is a map of all the entity scripts detected in the game's assembly.
		// This is filled when loading the game's assembly.

		// This is a map that contains all the instances of entity scripts.
		std::unordered_map<std::string, NetGameScriptObject> gameEntityTypes;
		std::unordered_map<std::string, NetGameScriptObject> brushEntityTypes;
		std::unordered_map<std::string, NetGameScriptObject> pointEntityTypes;
		std::unordered_map<std::string, UIWidgetObject> uiWidgets;

		std::unordered_map<uint32_t, Coral::ManagedObject> entityToManagedObjects;
		std::unordered_map<UUID, Coral::ManagedObject> widgetUUIDToManagedObjects;
		ScriptingEngineNet();
		~ScriptingEngineNet();

	private:
		std::string GenerateGUID();
		std::vector<CompilationError> ExtractErrors(const std::string& input);
	};
}