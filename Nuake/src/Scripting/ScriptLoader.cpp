#include "ScriptLoader.h"
#include <windows.h>
#include <libloaderapi.h>

bool ScriptLoader::RegisterScript(ScriptableEntity* script)
{
    return false;
}

bool ScriptLoader::LoadModule(const std::string& path)
{
    HMODULE loadedLib = LoadLibraryA(path.c_str());
    if (!loadedLib) {
        printf("Failed to load library\n");
        return false;
    }

    printf("Library loaded\n");

    ScriptModuleRegister* functionPointer = (ScriptModuleRegister*)GetProcAddress(loadedLib, "Register");

    functionPointer();

    return true;

}


