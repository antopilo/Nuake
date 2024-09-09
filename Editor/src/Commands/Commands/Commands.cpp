#include "Commands.h"

#include <Engine.h>

#include "src/FileSystem/FileSystem.h"

#include <filesystem>

namespace NuakeEditor
{
	bool SaveSceneCommand::Execute()
	{
		return true;
	}

	bool SaveProjectCommand::Execute()
	{
		Engine::GetProject()->Save();
		Engine::GetCurrentScene()->Save();

		return true;
	}

	bool SetGameState::Execute()
	{
		GameState gameState = Engine::GetGameState();
		if (gameState == GameState::Stopped && mGameState == GameState::Playing)
		{
			Engine::EnterPlayMode();
		}
		else if (mGameState == GameState::Stopped)
		{
			Engine::ExitPlayMode();
		}

		Engine::SetGameState(mGameState);

		return true;
	}

	bool CreateTrenchbroomGameConfig::Execute()
	{
        const std::string& cleanProjectName = String::Sanitize(mProject->Name);
        const std::string& gameConfigFolderPath = mProject->TrenchbroomPath + "/../games/" + cleanProjectName + "/";

        if (!FileSystem::DirectoryExists(gameConfigFolderPath))
        {
            FileSystem::MakeDirectory(gameConfigFolderPath, true);
        }

        const std::string& gameConfigFilePath = gameConfigFolderPath + "GameConfig.cfg";
        FileSystem::BeginWriteFile(gameConfigFilePath, true);

        std::string templateFile = R"(
			{
    "version": 8,
    "name": ")" + cleanProjectName + R"(",
    "icon": "Icon.png",
    "experimental": true,
    "fileformats": [
        { "format": "Valve" }, { "format": "Standard" }
    ],
    "filesystem": {
        "searchpath": "",
        "packageformat": { "extension": ".zip", "format": "zip" }
    },
    "textures": {
        "root": "textures",
        "extensions": [ ".jpg", ".png", ".tga" ]
    },
    "entities": {
        "definitions": [ ")" + cleanProjectName + R"(.fgd" ],
        "defaultcolor": "0.6 0.6 0.6 1.0"
    },
    "tags": {
        "brush": [
            {
                "name": "Trigger",
                "attribs": [ "transparent" ],
                "match": "classname",
                "pattern": "Trigger*",
                "texture": "trigger.png"
            }
        ],
        "brushface": [
            {
                "name": "Caulk",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "*caulk"
            },            
            {
                "name": "Clip",
                "attribs": [ "transparent" ],
                "match": "surfaceparm",
                "pattern": [ "playerclip", "monsterclip" ]
            },
            {
                "name": "Detail",
                "match": "contentflag",
                "flags": [ "detail" ]
            },            
            {
                "name": "Hint",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "common/hint*"
            },
            {
                "name": "Liquid",
                "match": "surfaceparm",
                "pattern": [ "water", "lava", "slime" ]
            },
            {
                "name": "Skip",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "common/*skip"
            },                        
            {
                "name": "Translucent",
                "attribs": [ "transparent" ],
                "match": "surfaceparm",
                "pattern": [ "trans", "fog" ]
            }
        ]
    },
    "faceattribs": {
        "defaults": {
            "scale": [1.0, 1.0]
        },
        "surfaceflags": [],
        "contentflags": [
           
        ]
    },
    "softMapBounds":"-65536 -65536 -65536 65536 65536 65536"
}

 )";
        FileSystem::WriteLine(templateFile);
        FileSystem::EndWriteFile();

        // Copy Icon.png
        if (FileSystem::FileExists("icon.png"))
        {
            std::filesystem::copy_file(FileSystem::RelativeToAbsolute("icon.png"), gameConfigFolderPath + "Icon.png", std::filesystem::copy_options::overwrite_existing);
        }
		return true;
	}

    bool SetVSync::Execute()
    {
        Engine::GetCurrentWindow()->SetVSync(value);

        return true;
    }
}