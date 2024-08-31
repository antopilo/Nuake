#pragma once
#include "src/Core/Core.h"
#include "src/Scene/Entities/Entity.h"

#include "src/Scripting/NetModules/NetAPIModule.h"
#include <any>

namespace Coral
{
	class HostInstance;
	class Type;
}

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>
#include <Coral/Assembly.hpp>

namespace Nuake {

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

	struct NetGameScriptObject 
	{
		Coral::Type* coralType;
		std::vector<ExposedVar> exposedVars;

		bool isTrigger = false;
		std::string Description = "";
	};

	struct CompilationError
	{
		std::string file;
		int line;
		std::string message;
	};

	class ScriptingEngineNet
	{
	private:
		const std::string m_Scope = "Nuake.Net";
		const std::string m_EngineAssemblyName = "NuakeNet.dll";
		const std::string m_NetDirectory = ".net";
		const std::string m_ContextName = "NuakeEngineContext";

		Coral::Type m_BaseEntityType;
		Coral::Type m_PrefabType;
		bool m_IsInitialized = false;

		Coral::HostInstance* m_HostInstance;
		Coral::AssemblyLoadContext m_LoadContext;

		std::unordered_map<std::string, Coral::AssemblyLoadContext*> m_LoadedAssemblies;
		std::vector<Ref<NetAPIModule>> m_Modules;

		Coral::ManagedAssembly m_NuakeAssembly; // Nuake DLL
		Coral::ManagedAssembly m_GameAssembly;	// Game DLL

		// This is a map of all the entity scripts detected in the game's assembly.
		// This is filled when loading the game's assembly.

		// This is a map that contains all the instances of entity scripts.
		std::unordered_map<std::string, NetGameScriptObject> m_GameEntityTypes;
		std::unordered_map<std::string, NetGameScriptObject> m_BrushEntityTypes;
		std::unordered_map<uint32_t, Coral::ManagedObject> m_EntityToManagedObjects;

		ScriptingEngineNet();
		~ScriptingEngineNet();

		std::vector<CompilationError> ExtractErrors(const std::string& input);

	public:
		static ScriptingEngineNet& Get();

		Coral::HostInstance* GetHostInstance() { return m_HostInstance; }
		Coral::AssemblyLoadContext& GetLoadContext() { return m_LoadContext; }

		Coral::ManagedAssembly ReloadEngineAPI(Coral::AssemblyLoadContext & context);

		void Initialize();
		void Uninitialize();


		bool IsInitialized() const { return m_IsInitialized; }

		void UpdateEntityWithExposedVar(Entity entity);
		std::vector<ExposedVar> GetExposedVarForTypes(Entity entity);

		std::vector<CompilationError> BuildProjectAssembly(Ref<Project> project);
		void LoadProjectAssembly(Ref<Project> project);

		void RegisterEntityScript(Entity& entity);
		Coral::ManagedObject GetEntityScript(const Entity& entity);
		bool HasEntityScriptInstance(const Entity& entity);

		void CopyNuakeNETAssemblies(const std::string& path);
		void GenerateSolution(const std::string& path, const std::string& projectName);
		void CreateEntityScript(const std::string& path, const std::string& entityName);

		std::string FindClassNameInScript(const std::string& filePath);

		Coral::ManagedAssembly GetNuakeAssembly() const { return m_NuakeAssembly; }

		std::unordered_map<std::string, NetGameScriptObject> GetBrushEntities() const { return m_BrushEntityTypes; }
	private:
		std::string GenerateGUID();
	};
}