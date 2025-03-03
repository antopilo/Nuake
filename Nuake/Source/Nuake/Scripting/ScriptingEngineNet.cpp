#include "ScriptingEngineNet.h"

#include "Nuake/Core/Logger.h"
#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/Core/OS.h"
#include "Nuake/Threading/JobSystem.h"
#include "Nuake/Resource/Project.h"
#include "Nuake/Scene/Components/NetScriptComponent.h"

#include "NetModules/EngineNetAPI.h"
#include "NetModules/EngineSubsystemNetAPI.h"
#include "NetModules/InputNetAPI.h"
#include "NetModules/SceneNetAPI.h"
#include "NetModules/UINetAPI.h"

#include "Nuake/Scene/Components/BSPBrushComponent.h"

#include <filesystem>
#include <sstream>
#include <random>
#include <regex>


void ExceptionCallback(std::string_view InMessage)
{
	const std::string message = std::string("Unhandled native exception: ") + std::string(InMessage);
	Nuake::Logger::Log(message, ".net", Nuake::COMPILATION);
}


void MessageCallback(std::string_view message, Coral::MessageLevel level)
{
	Nuake::Logger::Log(std::string(message), ".net", Nuake::VERBOSE);
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
			.MessageCallback = MessageCallback,
			.ExceptionCallback = ExceptionCallback,
		};

		hostInstance = new Coral::HostInstance();
		Coral::CoralInitStatus status = hostInstance->Initialize(settings);
		if (status != Coral::CoralInitStatus::Success)
		{
			Logger::Log("Failed to initialize Coral: " + std::to_string((int)status));
			delete hostInstance;
			return;
		}

		// Initialize API modules
		// ----------------------------------
		modules =
		{
			CreateRef<EngineNetAPI>(),
			CreateRef<EngineSubsystemNetAPI>(),
			CreateRef<InputNetAPI>(),
			CreateRef<SceneNetAPI>(),
			CreateRef<UINetAPI>()
		};

		for (auto& m : modules)
		{
			m->RegisterMethods();
		}

		// Check if we have an .sln in the project.
		const std::string absoluteAssemblyPath = FileSystem::Root + m_NetDirectory + "/" + m_EngineAssemblyName;

		if (!FileSystem::FileExists(m_EngineAssemblyName, true))
		{
			isInitialized = false;
			return;
		}
	}

	ScriptingEngineNet::~ScriptingEngineNet()
	{
		if (isInitialized)
			hostInstance->Shutdown();
		delete hostInstance;
	}

	std::vector<CompilationError> ScriptingEngineNet::ExtractErrors(const std::string& output)
	{
		std::vector<CompilationError> errors;

		std::istringstream stream(output);
		std::string line;

		while (std::getline(stream, line)) 
		{
			auto trimmed = String::RemoveWhiteSpace(line);
			auto parenSplit = String::Split(trimmed, '(');
			if (parenSplit.size() > 1)
			{
				const std::string filePath = parenSplit[0];
				const std::string restOfLine = parenSplit[1];

				auto numbersString = String::Split(restOfLine, ')');
				if (numbersString.size() > 1)
				{
					auto lineCharNums = String::Split(numbersString[0], ',');

					if (lineCharNums.size() == 1)
					{
						CompilationError compilationError;
						compilationError.message = "";
						compilationError.file = filePath;
						compilationError.line = 0;
						errors.push_back(compilationError);
					}
					else
					{
						int lineNum = std::stoi(lineCharNums[0]);
						int charNum = std::stoi(lineCharNums[1]);
						bool isWarning = false;
						// error message
						std::string errMesg = "";
						int i = 0;
						for (auto s : String::Split(line, ':'))
						{
							if (String::BeginsWith(String::RemoveWhiteSpace(s), "warning"))
							{
								isWarning = true;
							}
							if (i >= 3)
							{
								errMesg += s;
							}
							i++;
						}

						CompilationError compilationError;
						compilationError.message = errMesg;
						compilationError.file = filePath;
						compilationError.line = lineNum;
						compilationError.isWarning = isWarning;
						errors.push_back(compilationError);
					}
					
				}
			}

		}

		return errors;
	}

	ScriptingEngineNet& ScriptingEngineNet::Get()
	{
		static ScriptingEngineNet instance;
		return instance;
	}

	Coral::ManagedAssembly ScriptingEngineNet::ReloadEngineAPI(Coral::AssemblyLoadContext& context)
	{
		auto assembly = context.LoadAssembly(m_EngineAssemblyName);

		// Upload internal calls for each module
		// --------------------------------------------------
		for (const auto& netModule : modules)
		{
			for (const auto& [methodName, methodPtr] : netModule->GetMethods())
			{
				auto namespaceClassSplit = String::Split(methodName, '.');
				assembly.AddInternalCall(m_Scope + '.' + namespaceClassSplit[0], namespaceClassSplit[1], methodPtr);
			}
		}

		assembly.UploadInternalCalls();

		return assembly;
	}

	void ScriptingEngineNet::Initialize()
	{
		if (!isInitialized)
			return;

		loadContext = hostInstance->CreateAssemblyLoadContext(m_ContextName);

		nuakeAssembly = ReloadEngineAPI(loadContext);

		isInitialized = true;

		gameEntityTypes.clear();
		brushEntityTypes.clear();
		pointEntityTypes.clear();
		uiWidgets.clear();
	}

	void ScriptingEngineNet::Uninitialize()
	{
		if (!isInitialized)
		{
			return;
		}

		for (auto& [entity, managedObject] : entityToManagedObjects)
		{
			managedObject.Destroy();
		}
		
		for (auto& [widgetUUID, managedObject] : widgetUUIDToManagedObjects)
		{
			managedObject.Destroy();
		}

		onUninitializeDelegate.Broadcast();

		Coral::GC::Collect();
		Coral::GC::WaitForPendingFinalizers();

		GetHostInstance()->UnloadAssemblyLoadContext(loadContext);

		entityToManagedObjects.clear();
		widgetUUIDToManagedObjects.clear();
	}

	void ScriptingEngineNet::UpdateEntityWithExposedVar(Entity entity)
	{
		if (!entity.HasComponent<NetScriptComponent>())
		{
			return;
		}

		std::vector<std::string> detectedExposedVar;
		NetScriptComponent& component = entity.GetComponent<NetScriptComponent>();
		for (auto& e : Nuake::ScriptingEngineNet::Get().GetExposedVarForTypes(entity))
		{
			bool found = false;
			for (auto& c : component.ExposedVar)
			{
				if (e.Name == c.Name)
				{
					c.Type = (Nuake::NetScriptExposedVarType)e.Type;
					c.DefaultValue = e.Value;
					found = true;
				}
			}

			detectedExposedVar.push_back(e.Name);

			if (!found)
			{
				Nuake::NetScriptExposedVar exposedVar;
				exposedVar.Name = e.Name;
				exposedVar.Value = e.Value;
				exposedVar.DefaultValue = e.Value;
				exposedVar.Type = (Nuake::NetScriptExposedVarType)e.Type;
				component.ExposedVar.push_back(exposedVar);
			}
		}

		std::erase_if(component.ExposedVar,
			[&](Nuake::NetScriptExposedVar& var)
			{
				return std::find(detectedExposedVar.begin(), detectedExposedVar.end(), var.Name) == detectedExposedVar.end();
			}
		);
	}

	std::vector<ExposedVar> ScriptingEngineNet::GetExposedVarForTypes(Entity entity)
	{
		if (!entity.HasComponent<NetScriptComponent>())
		{
			return std::vector<ExposedVar>();
		}

		auto& component = entity.GetComponent<NetScriptComponent>();
		const auto& filePath = component.ScriptPath;

		std::string className;
		if (String::EndsWith(filePath, ".cs"))
		{
			className = FindClassNameInScript(filePath);;
		}
		else
		{
			className = filePath;
		}

		if (gameEntityTypes.find(className) == gameEntityTypes.end())
		{
			// The class name parsed in the file was not found in the game's DLL.
			const std::string& msg = "Skipped .net entity script: \n Class: " +
				className + " not found in " + std::string(gameAssembly.GetName());
			Logger::Log(msg, ".net", CRITICAL);
			return std::vector<ExposedVar>();
		}

		return gameEntityTypes[className].exposedVars;
	}

	bool ScriptingEngineNet::HasUIWidget(const std::string& widgetName)
	{
		return uiWidgets.contains(widgetName);
	}

	UIWidgetObject& ScriptingEngineNet::GetUIWidget(const std::string& widgetName)
	{
		ASSERT(HasUIWidget(widgetName));
		return uiWidgets[widgetName];
	}

	void ScriptingEngineNet::RegisterCustomWidgetInstance(const UUID& canvasUUID, const UUID& nodeUUID, const std::string& widgetTypeName)
	{
		if (uiWidgets.find(widgetTypeName) == uiWidgets.end())
		{
			// The class name parsed in the file was not found in the game's DLL.
			const std::string& msg = "Skipped .net widget script: \n Class: " +
				widgetTypeName + " not found in " + std::string(gameAssembly.GetName());
			Logger::Log(msg, ".net", CRITICAL);
			return;
		}

		auto classInstance = uiWidgets[widgetTypeName].coralType->CreateInstance();
		classInstance.SetFieldValue("CanvasUUID", Coral::String::New(std::to_string(canvasUUID)));
		classInstance.SetFieldValue("UUID", Coral::String::New(std::to_string(nodeUUID)));
		widgetUUIDToManagedObjects[std::make_pair(canvasUUID, nodeUUID)] = classInstance;
	}

	bool ScriptingEngineNet::HasCustomWidgetInstance(const UUID& canvasUUID, const UUID& uuid)
	{
		return widgetUUIDToManagedObjects.contains(std::make_pair(canvasUUID, uuid));
	}

	Coral::ManagedObject ScriptingEngineNet::GetCustomWidgetInstance(const UUID& canvasUUID, const UUID& uuid)
	{
		if (!HasCustomWidgetInstance(canvasUUID, uuid))
		{
			Logger::Log("Failed to get custom widget .Net script instance, doesn't exist", ".net", CRITICAL);
			return Coral::ManagedObject();
		}

		return widgetUUIDToManagedObjects[std::make_pair(canvasUUID, uuid)];
	}

	std::string ScriptingEngineNet::GetGameAssemblyPath(Ref<Project> project) const
	{
		const std::string sanitizedProjectName = String::Sanitize(project->Name);
		const std::string assemblyPath = "/bin/Debug/net8.0/" + sanitizedProjectName + ".dll";
		return assemblyPath;
	}

	std::vector<CompilationError> ScriptingEngineNet::BuildProjectAssembly(Ref<Project> project)
	{
		const std::string sanitizedProjectName = String::Sanitize(project->Name);
		if (!FileSystem::FileExists(sanitizedProjectName + ".sln"))
		{
			Logger::Log("Couldn't find .net solution. Have you created a solution?", ".net", CRITICAL);
			return std::vector<CompilationError>();
		}

		std::string result = OS::CompileSln(FileSystem::Root + sanitizedProjectName + ".sln");
		return ExtractErrors(result);
		//if (errors.size() > 0)
		//{
		//	Logger::Log("Build failed!", ".net", CRITICAL);
		//	for (auto& err : errors)
		//	{
		//		Logger::Log(err.file + " line " + std::to_string(err.line) + " : " + err.message, ".net", CRITICAL);
		//	}
		//}
	}

	void ScriptingEngineNet::LoadProjectAssembly(Ref<Project> project)
	{
		if (!isInitialized)
		{
			return;
		}

		const std::string gameAssemblyPath = GetGameAssemblyPath(project);
		if (!FileSystem::FileExists(gameAssemblyPath))
		{
			Logger::Log("Couldn't load .net assembly. Did you forget to build the .net project?", ".net", CRITICAL);
			return;
		}

		const std::string absoluteAssemblyPath = FileSystem::Root + gameAssemblyPath;
		gameAssembly = loadContext.LoadAssembly(absoluteAssemblyPath);

		prefabType = gameAssembly.GetType("Nuake.Net.Prefab");
		auto entityScriptType = gameAssembly.GetType("Nuake.Net.Entity");
		auto& exposedFieldAttributeType = gameAssembly.GetType("Nuake.Net.ExposedAttribute");
		auto& brushScriptAttributeType = gameAssembly.GetType("Nuake.Net.BrushScript");
		auto& pointScriptAttributeType = gameAssembly.GetType("Nuake.Net.PointScript");
		auto& uiWidgetType = gameAssembly.GetType("Nuake.Net.UIWidget");
		auto& uiWidgetExternalLayoutType = gameAssembly.GetType("Nuake.Net.ExternalHTML");

		for (auto& type : gameAssembly.GetTypes())
		{
			// Brush
			bool isBrushScript = false;
			std::string brushDescription;
			bool isTrigger = false;

			// Point
			bool isPointScript = false;
			std::string pointDescription;
			std::string pointBase;
			for (auto& attribute : type->GetAttributes())
			{
				if (attribute.GetType() == brushScriptAttributeType)
				{
					brushDescription = attribute.GetFieldValue<Coral::String>("Description");
					isTrigger = attribute.GetFieldValue<Coral::Bool32>("IsTrigger");
					isBrushScript = true;
				}

				if (attribute.GetType() == pointScriptAttributeType)
				{
					pointDescription = attribute.GetFieldValue<Coral::String>("Description");
					pointBase = attribute.GetFieldValue<Coral::String>("Base");
					isPointScript = true;
				}
			}

			if (type->IsSubclassOf(entityScriptType))
			{
				const std::string baseTypeName = std::string(type->GetBaseType().GetFullName());
				if (baseTypeName != "Nuake.Net.Entity")
				{
					continue;
				}

				baseEntityType = type->GetBaseType();

				auto typeSplits = String::Split(type->GetFullName(), '.');
				std::string shortenedTypeName = typeSplits[typeSplits.size() - 1];

				NetGameScriptObject gameScriptObject;
				gameScriptObject.coralType = type;
				gameScriptObject.exposedVars = std::vector<ExposedVar>();
				gameScriptObject.Base = baseTypeName;
				for (auto& f : type->GetFields())
				{
					for (auto& a : f.GetAttributes())
					{
						if (a.GetType() == exposedFieldAttributeType)
						{
							ExposedVar exposedVar;
							exposedVar.Name = f.GetName();

							auto typeName = f.GetType().GetFullName();
							ExposedVarTypes varType = ExposedVarTypes::Unsupported;
							bool hasDefaultValue = a.GetFieldValue<Coral::Bool32>("HasDefaultvalue");
							if (typeName == "System.Single")
							{
								varType = ExposedVarTypes::Float;

								if (hasDefaultValue)
								{
									exposedVar.Value = a.GetFieldValue<float>("DefaultValueInternalFloat");
								}
							}
							else if (typeName == "System.Double")
							{
								varType = ExposedVarTypes::Double;
							}
							else if (typeName == "System.String")
							{
								varType = ExposedVarTypes::String;

								if (hasDefaultValue)
								{
									exposedVar.Value = a.GetFieldValue<Coral::String>("DefaultValueInternalString");
								}
							}
							else if (typeName == "System.Boolean")
							{
								varType = ExposedVarTypes::Bool;

								if (hasDefaultValue)
								{
									exposedVar.Value = (bool)a.GetFieldValue<Coral::Bool32>("DefaultValueInternalBool");
								}
							}
							else if (typeName == "System.Numerics.Vector2")
							{
								varType = ExposedVarTypes::Vector2;
							}
							else if (typeName == "System.Numerics.Vector3")
							{
								varType = ExposedVarTypes::Vector3;
							}
							else if (typeName == "Nuake.Net.Entity")
							{
								varType = ExposedVarTypes::Entity;
							}
							else if (typeName == "Nuake.Net.Prefab")
							{
								varType = ExposedVarTypes::Prefab;
							}

							if (varType != ExposedVarTypes::Unsupported)
							{
								exposedVar.Type = varType;
								gameScriptObject.exposedVars.push_back(exposedVar);
							}

							Logger::Log("Exposed field detected: " + std::string(f.GetName()) + " of type " + std::string(f.GetType().GetFullName()));
						}
					}
				}

				// Add to the brush map to retrieve when exporting FGD
				if (isBrushScript)
				{
					gameScriptObject.Description = brushDescription;
					gameScriptObject.isTrigger = isTrigger;
					brushEntityTypes[shortenedTypeName] = gameScriptObject;
				}

				if (isPointScript)
				{
					gameScriptObject.Description = pointDescription;
					pointEntityTypes[shortenedTypeName] = gameScriptObject;
				}
				gameEntityTypes[shortenedTypeName] = gameScriptObject;
			}

			if (type->IsSubclassOf(uiWidgetType))
			{
				auto typeSplits = String::Split(type->GetFullName(), '.');
				std::string shortenedTypeName = typeSplits[typeSplits.size() - 1];
				for (auto& attribute : type->GetAttributes())
				{
					if (attribute.GetType() == uiWidgetExternalLayoutType)
					{
						Coral::String htmlPath = attribute.GetFieldValue<Coral::String>("HTMLPath");
						if (FileSystem::FileExists(htmlPath))
						{
							uiWidgets[shortenedTypeName] = { type, htmlPath };
						}
						else
						{
							Logger::Log("Couldn't register UI Widget: " + shortenedTypeName + " \nExternal HTML file doesn't exist with path: " + std::string(htmlPath), "UI", WARNING);
						}
					}
				}
			}

			onGameAssemblyLoadedDelegate.Broadcast();
		}
	}

	void ScriptingEngineNet::RegisterEntityScript(Entity& entity)
	{
		if (!isInitialized)
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
		
		std::string className;
		if (String::EndsWith(filePath, ".cs"))
		{
			className = FindClassNameInScript(filePath);
		}
		else
		{
			className = filePath;
		}

		if(gameEntityTypes.find(className) == gameEntityTypes.end())
		{
			// The class name parsed in the file was not found in the game's DLL.
			const std::string& msg = "Skipped .net entity script: \n Class: " + 
				className + " not found in " + std::string(gameAssembly.GetName());
			Logger::Log(msg, ".net", CRITICAL);
			return;
		}

		auto classInstance = gameEntityTypes[className].coralType->CreateInstance();

		int handle = entity.GetHandle();
		int id = entity.GetID();
		classInstance.SetPropertyValue("ECSHandle", handle);
		classInstance.SetPropertyValue("ID", id);
		
		if (entity.HasComponent<BSPBrushComponent>())
		{
			BSPBrushComponent& brushComponent = entity.GetComponent<BSPBrushComponent>();
		}

		std::vector<std::string> detectedExposedVar;
		detectedExposedVar.reserve(std::size(gameEntityTypes[className].exposedVars));
		// Update new default values if they have changed in the code.
		for (auto& exposedVar : gameEntityTypes[className].exposedVars)
		{
			const std::string varName = exposedVar.Name;
			detectedExposedVar.push_back(varName);

			switch (exposedVar.Type)
			{
				case ExposedVarTypes::Float:
				{
					break;
				}
				case ExposedVarTypes::Double:
				{
					break;
				}
				case ExposedVarTypes::String:
				{
					try {
						//exposedVar.Value = std::string(classInstance.GetFieldValue<Coral::String>(varName));
					}
					catch (...)
					{
					}
					
					break;
				}
				case ExposedVarTypes::Bool:
				{
					break;
				}
				case ExposedVarTypes::Vector2:
				{
					break;
				}
				case ExposedVarTypes::Vector3:
				{
					break;
				}
				case ExposedVarTypes::Entity:
				{
					struct EntityWrapper
					{
						int32_t id;
						int32_t handle;
					};

					break;
				}
				case ExposedVarTypes::Prefab:
				{

					break;
				}
			}
		}

		

		// Override with user values set in the editor.
		for (auto& exposedVarUserValue : component.ExposedVar)
		{
			if (!exposedVarUserValue.Value.has_value())
			{
				continue;
			}

			if (exposedVarUserValue.Type == NetScriptExposedVarType::String)
			{
				std::string stringValue = std::any_cast<std::string>(exposedVarUserValue.Value);
				Coral::String coralString = Coral::String::New(stringValue);
				classInstance.SetFieldValue(exposedVarUserValue.Name, coralString);
			}
			else if (exposedVarUserValue.Type == NetScriptExposedVarType::Entity)
			{
				if (exposedVarUserValue.Value.has_value())
				{
					int entityId = std::any_cast<int>(exposedVarUserValue.Value);
					Entity scriptEntity = entity.GetScene()->GetEntityByID(entityId);
					if (scriptEntity.IsValid())
					{
						if (HasEntityScriptInstance(scriptEntity))
						{
							// In the case the entity has a script & instance, we pass that.
							// This gives access to the objects scripts.
							//Coral::ManagedObject* scriptInstance = GetEntityScript(scriptEntity);
							//classInstance->SetFieldValue<Coral::ManagedObject>(exposedVarUserValue.Name, scriptInstance);
						}
						else
						{
							// In the case where the entity doesnt have an instance, we create one
							auto newEntity = baseEntityType.CreateInstance(scriptEntity.GetHandle());
							classInstance.SetFieldValue<Coral::ManagedObject>(exposedVarUserValue.Name, newEntity);
						}
					}
					else
					{
						Logger::Log("Invalid entity exposed variable set", ".net", CRITICAL);
					}
				}
			}
			else if (exposedVarUserValue.Type == NetScriptExposedVarType::Prefab)
			{
				if (exposedVarUserValue.Value.has_value())
				{
					std::string path = std::any_cast<std::string>(exposedVarUserValue.Value);
					if (FileSystem::FileExists(path))
					{
						// In the case where the entity doesnt have an instance, we create one
						auto newPrefab = prefabType.CreateInstance(Coral::String::New(path));
						classInstance.SetFieldValue<Coral::ManagedObject>(exposedVarUserValue.Name, newPrefab);
					}
					else
					{
						Logger::Log("Invalid prefab exposed variable set", ".net", CRITICAL);
					}
				}
			}
			else
			{
				classInstance.SetFieldValue(exposedVarUserValue.Name, exposedVarUserValue.Value);
			}
		}

		entityToManagedObjects.emplace(entity.GetID(), std::move(classInstance));
	}

	Coral::ManagedObject* ScriptingEngineNet::GetEntityScript(const Entity& entity)
	{
		if (!HasEntityScriptInstance(entity))
		{
			std::string name = entity.GetComponent<NameComponent>().Name;
			Logger::Log(name);
			Logger::Log("Failed to get entity .Net script instance, doesn't exist", ".net", CRITICAL);
			return nullptr;
		}

		return &entityToManagedObjects[entity.GetID()];
	}

	bool ScriptingEngineNet::HasEntityScriptInstance(const Entity& entity)
	{
		if (!isInitialized)
		{
			return false;
		}

		return entityToManagedObjects.find(entity.GetID()) != entityToManagedObjects.end();
	}

	void ScriptingEngineNet::CopyNuakeNETAssemblies(const std::string& path)
	{
		// Create .Net directory in projects folder.
		const auto netDirectionPath = "/";
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

		const std::string cleanProjectName = String::Sanitize(projectName);
		const std::string csprojFilePath = cleanProjectName + ".csproj";
		const std::string& csProjFileContent = R"(<?xml version="1.0" encoding="utf-8"?>
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net8.0</TargetFramework>
    <OutputType>Library</OutputType>
    <LangVersion>latest</LangVersion>
  </PropertyGroup>
  <ItemGroup>
	<Reference Include="NuakeNet">
		<HintPath>NuakeNet.dll</HintPath>
	</Reference>
  </ItemGroup>
</Project>
)";
		FileSystem::BeginWriteFile(csprojFilePath);
		FileSystem::WriteLine(csProjFileContent);
		FileSystem::EndWriteFile();

		const std::string slnFilePath = cleanProjectName + ".sln";
		const std::string guid = GenerateGUID(); // Generate GUID

		const std::string& slnFileContent = R"(Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
VisualStudioVersion = 17.0.32327.299
MinimumVisualStudioVersion = 10.0.40219.1
Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = ")" + cleanProjectName + R"(", ")" + cleanProjectName + R"(.csproj", ")" + guid + R"("
  EndProject
Global
  GlobalSection(SolutionConfigurationPlatforms) = preSolution
    Debug|Any CPU = Debug|Any CPU
    Release|Any CPU = Release|Any CPU
  EndGlobalSection
  GlobalSection(ProjectConfigurationPlatforms) = postSolution
    {)" + guid + R"(}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
    {)" + guid + R"(}.Debug|Any CPU.Build.0 = Debug|Any CPU
    {)" + guid + R"(}.Release|Any CPU.ActiveCfg = Release|Any CPU
    {)" + guid + R"(}.Release|Any CPU.Build.0 = Release|Any CPU
  EndGlobalSection
  GlobalSection(SolutionProperties) = preSolution
    HideSolutionNode = FALSE
  EndGlobalSection
EndGlobal
)";
		FileSystem::BeginWriteFile(slnFilePath);
		FileSystem::WriteLine(slnFileContent);
		FileSystem::EndWriteFile();

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

	std::string ScriptingEngineNet::FindClassNameInScript(const std::string& filePath)
	{
		if (filePath.empty())
		{
			Logger::Log("Skipped .net entity script since it was empty.", ".net", VERBOSE);
			return "";
		}

		if (!FileSystem::FileExists(filePath))
		{
			Logger::Log("Skipped .net entity script: The file path doesn't exist.", ".net", WARNING);
			return "";
		}

		// We can now scan the file and look for this pattern: class XXXXX : Entity
		// Warning, this doesnt do any bound check so if there is a semicolon at the end 
		// of the file. IT MIGHT CRASH HERE. POTENTIALLY - I have not tested.
		// -----------------------------------------------------
		std::string fileContent = FileSystem::ReadFile(filePath);
		if (fileContent.empty())
		{
			return "";
		}

		fileContent = fileContent.substr(3, fileContent.size());
		fileContent = String::RemoveWhiteSpace(fileContent);

		// Find class token
		size_t classTokenPos = fileContent.find("class");
		if (classTokenPos == std::string::npos)
		{
			Logger::Log("Skipped .net entity script: file doesnt contain entity class.", ".net", WARNING);
			return "";
		}

		size_t classNameStartIndex = classTokenPos + 5; // 4 letter: class + 1 for next char

		// Find semi-colon token
		size_t semiColonPos = fileContent.find(":");
		if (semiColonPos == std::string::npos || semiColonPos < classTokenPos)
		{
			Logger::Log("Skipped .net entity script: Not class inheriting Entity was found.", ".net", WARNING);
			return "";
		}

		size_t classNameLength = semiColonPos - classNameStartIndex;
		return fileContent.substr(classNameStartIndex, classNameLength);
	}

	std::string ScriptingEngineNet::GenerateGUID()
	{
		// Random number generator
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 15);

		auto randomHexDigit = [&]() {
			static const char hexDigits[] = "0123456789ABCDEF";
			return hexDigits[dis(gen)];
			};

		// Generate a UUID (this is a simplified version and not a real UUID)
		std::ostringstream oss;
		for (int i = 0; i < 8; ++i) oss << randomHexDigit();
		oss << '-';
		for (int i = 0; i < 4; ++i) oss << randomHexDigit();
		oss << '-';
		oss << '4'; // UUID version 4
		for (int i = 0; i < 3; ++i) oss << randomHexDigit();
		oss << '-';
		oss << '8'; // UUID variant
		for (int i = 0; i < 3; ++i) oss << randomHexDigit();
		oss << '-';
		for (int i = 0; i < 12; ++i) oss << randomHexDigit();

		return oss.str();
	}

}
