#include "ScriptingEngineNet.h"

#include "src/Core/Logger.h"
#include "src/Core/FileSystem.h"
#include "src/Core/OS.h"
#include "src/Threading/JobSystem.h"
#include "src/Resource/Project.h"
#include "src/Scene/Components/NetScriptComponent.h"

#include "NetModules/EngineNetAPI.h"
#include "NetModules/InputNetAPI.h"
#include "NetModules/SceneNetAPI.h"

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>


void ExceptionCallback(std::string_view InMessage)
{
	const std::string message = std::string("Unhandled native exception: ") + std::string(InMessage);
	Nuake::Logger::Log(message, ".net", Nuake::CRITICAL);
}


namespace Nuake
{
	ScriptingEngineNet::ScriptingEngineNet()
	{
		// Initialize Coral
		// ----------------------------------
		Coral::HostSettings settings =
		{
			.CoralDirectory = "",
			.ExceptionCallback = ExceptionCallback
		};

		m_HostInstance = new Coral::HostInstance();
		m_HostInstance->Initialize(settings);

		// Initialize API modules
		// ----------------------------------
		m_Modules =
		{
			CreateRef<EngineNetAPI>(),
			CreateRef<InputNetAPI>(),
			CreateRef<SceneNetAPI>()
		};

		for (auto& m : m_Modules)
		{
			m->RegisterMethods();
		}
	}

	ScriptingEngineNet::~ScriptingEngineNet()
	{
		m_HostInstance->Shutdown();
	}

	ScriptingEngineNet& ScriptingEngineNet::Get()
	{
		static ScriptingEngineNet instance;
		return instance;
	}

	void ScriptingEngineNet::Initialize()
	{
		// Check if we have an .sln in the project.
		const std::string absoluteAssemblyPath = FileSystem::Root + m_NetDirectory + "/" + m_EngineAssemblyName;

		if (!FileSystem::FileExists(m_EngineAssemblyName, true))
		{
			m_IsInitialized = false;
			return;
		}

		m_LoadContext = m_HostInstance->CreateAssemblyLoadContext(m_ContextName);

		// Load Nuake assembly DLL
		m_NuakeAssembly = m_LoadContext.LoadAssembly(m_EngineAssemblyName);

		// Upload internal calls for each module
		// --------------------------------------------------
		for (const auto& netModule : m_Modules)
		{
			for (const auto& [methodName, methodPtr] : netModule->GetMethods())
			{
				auto namespaceClassSplit = String::Split(methodName, '.');
				m_NuakeAssembly.AddInternalCall(m_Scope + '.' + namespaceClassSplit[0], namespaceClassSplit[1], methodPtr);
			}
		}

		m_NuakeAssembly.UploadInternalCalls();

		m_IsInitialized = true;
	}

	void ScriptingEngineNet::Uninitialize()
	{
		if (!m_IsInitialized)
		{
			return;
		}

		for (auto& [entity, managedObject] : m_EntityToManagedObjects)
		{
			managedObject.Destroy();
		}

		Coral::GC::Collect();

		m_HostInstance->UnloadAssemblyLoadContext(m_LoadContext);

		m_GameEntityTypes.clear();
		m_EntityToManagedObjects.clear();
	}

	void ScriptingEngineNet::BuildProjectAssembly(Ref<Project> project)
	{
		const std::string sanitizedProjectName = String::Sanitize(project->Name);
		if (!FileSystem::FileExists(sanitizedProjectName + ".sln"))
		{
			Logger::Log("Couldn't find .net solution. Have you created a solution?", ".net", CRITICAL);
			return;
		}

		OS::CompileSln(FileSystem::Root + sanitizedProjectName + ".sln");
	}

	void ScriptingEngineNet::LoadProjectAssembly(Ref<Project> project)
	{
		if (!m_IsInitialized)
		{
			return;
		}

		const std::string sanitizedProjectName = String::Sanitize(project->Name);
		const std::string assemblyPath = "/bin/Debug/net8.0/" + sanitizedProjectName + ".dll";

		if (!FileSystem::FileExists(assemblyPath))
		{
			Logger::Log("Couldn't load .net assembly. Did you forget to build the .net project?", ".net", CRITICAL);
			return;
		}

		const std::string absoluteAssemblyPath = FileSystem::Root + assemblyPath;
		m_GameAssembly = m_LoadContext.LoadAssembly(absoluteAssemblyPath);

		for (auto& type : m_GameAssembly.GetTypes())
		{
			Logger::Log(std::string("Detected type: ") + std::string(type->GetFullName()), ".net");
			Logger::Log(std::string("Detected base type: ") + std::string(type->GetBaseType().GetFullName()), ".net");

			const std::string baseTypeName = std::string(type->GetBaseType().GetFullName());
			if (baseTypeName == "Nuake.Net.Entity")
			{
				auto typeSplits = String::Split(type->GetFullName(), '.');
				std::string shortenedTypeName = typeSplits[typeSplits.size() - 1];
				m_GameEntityTypes[shortenedTypeName] = type; // We have found an entity script.
			}
		}
	}

	void ScriptingEngineNet::RegisterEntityScript(Entity& entity)
	{
		if (!m_IsInitialized)
		{
			return;
		}

		if (!entity.IsValid())
		{
			Logger::Log("Failed to register entity .net script: Entity not valid.", ".net", CRITICAL);
			return;
		}

		if (!entity.HasComponent<NetScriptComponent>())
		{
			Logger::Log("Failed to register entity .net script: Entity doesn't have a .net script component.", ".net", CRITICAL);
			return;
		}

		auto& component = entity.GetComponent<NetScriptComponent>();
		const auto& filePath = component.ScriptPath;
		if (filePath.empty())
		{
			Logger::Log("Skipped .net entity script since it was empty.", ".net", VERBOSE);
			return;
		}

		if (!FileSystem::FileExists(filePath))
		{
			Logger::Log("Skipped .net entity script: The file path doesn't exist.", ".net", WARNING);
			return;
		}

		// We can now scan the file and look for this pattern: class XXXXX : Entity
		// Warning, this doesnt do any bound check so if there is a semicolon at the end 
		// of the file. IT MIGHT CRASH HERE. POTENTIALLY - I have not tested.
		// -----------------------------------------------------
		std::string fileContent = FileSystem::ReadFile(filePath);
		fileContent = fileContent.substr(3, fileContent.size());
		fileContent = String::RemoveWhiteSpace(fileContent);

		// Find class token
		size_t classTokenPos = fileContent.find("class");
		if (classTokenPos == std::string::npos)
		{
			Logger::Log("Skipped .net entity script: file doesnt contain entity class.", ".net", WARNING);
			return;
		}

		size_t classNameStartIndex = classTokenPos + 5; // 4 letter: class + 1 for next char

		// Find semi-colon token
		size_t semiColonPos = fileContent.find(":");
		if (semiColonPos == std::string::npos || semiColonPos < classTokenPos)
		{
			Logger::Log("Skipped .net entity script: Not class inheriting Entity was found.", ".net", WARNING);
			return;
		}

		size_t classNameLength = semiColonPos - classNameStartIndex;

		const std::string className = fileContent.substr(classNameStartIndex, classNameLength);
		if(m_GameEntityTypes.find(className) == m_GameEntityTypes.end())
		{
			// The class name parsed in the file was not found in the game's DLL.
			const std::string& msg = "Skipped .net entity script: \n Class: " + 
				className + " not found in " + std::string(m_GameAssembly.GetName());
			Logger::Log(msg, ".net", CRITICAL);
			return;
		}

		auto classInstance = m_GameEntityTypes[className]->CreateInstance();

		int handle = entity.GetHandle();
		int id = entity.GetID();
		classInstance.SetPropertyValue("ECSHandle", handle);
		classInstance.SetPropertyValue("ID", id);

		m_EntityToManagedObjects.emplace(entity.GetID(), classInstance);
	}

	Coral::ManagedObject ScriptingEngineNet::GetEntityScript(const Entity& entity)
	{
		if (!HasEntityScriptInstance(entity))
		{
			std::string name = entity.GetComponent<NameComponent>().Name;
			Logger::Log(name);
			Logger::Log("Failed to get entity .Net script instance, doesn't exist", ".net", CRITICAL);
			return Coral::ManagedObject();
		}

		return m_EntityToManagedObjects[entity.GetID()];
	}

	bool ScriptingEngineNet::HasEntityScriptInstance(const Entity& entity)
	{
		if (!m_IsInitialized)
		{
			return false;
		}

		return m_EntityToManagedObjects.find(entity.GetID()) != m_EntityToManagedObjects.end();
	}

	void ScriptingEngineNet::CopyNuakeNETAssemblies(const std::string& path)
	{
		// Create .Net directory in projects folder.
		const auto netDirectionPath = '/' + m_NetDirectory + '/';
		const auto netDir = path + netDirectionPath;
		if (!FileSystem::DirectoryExists(netDirectionPath))
		{
			FileSystem::MakeDirectory(netDirectionPath);
		}

		// Copy engine assembly to projects folder.
		const std::vector<std::string> dllToCopy =
		{
			m_EngineAssemblyName
		};

		for (const auto& fileToCopy : dllToCopy)
		{
			std::filesystem::copy_file(fileToCopy, netDir + fileToCopy, std::filesystem::copy_options::overwrite_existing);
		}
	}

	void ScriptingEngineNet::GenerateSolution(const std::string& path, const std::string& projectName)
	{
		CopyNuakeNETAssemblies(path);

		// Generate premake5 templates
		// ----------------------------------------
		const std::string cleanProjectName = String::Sanitize(projectName);
		const std::string premakeScript = R"(
workspace ")" + cleanProjectName + R"("
configurations { "Debug", "Release" }
	project ")" + cleanProjectName + R"("
		language "C#"
		dotnetframework "net8.0"
		kind "SharedLib"
			clr "Unsafe"
	
		-- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

		files 
		{
			"**.cs"
		}
    
		links 
		{
			".net/NuakeNet"
		}
)";

		// Writting premake5 templates in project's directory
		// ----------------------------------------
		FileSystem::BeginWriteFile("premake5.lua");
		FileSystem::WriteLine(premakeScript);
		FileSystem::EndWriteFile();

		// Execute premake script, generating .sln
		OS::ExecuteCommand("cd " + path + " && premake5 vs2022");

		// Open solution file in visual studio
		OS::OpenIn(path + cleanProjectName + ".sln");

		// Delete premake script
		FileSystem::DeleteFileFromPath(FileSystem::Root + "/premake5.lua");
	}

	void ScriptingEngineNet::CreateEntityScript(const std::string & path, const std::string& entityName)
	{
		const std::string scriptTemplate = R"(
using )" + m_Scope + R"(;

namespace NuakeShowcase 
{
	class )" + entityName + R"( 
}
)";
	}

}