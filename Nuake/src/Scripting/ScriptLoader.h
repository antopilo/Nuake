#pragma once

#include "ScriptModule.h"
#include <map>
#include <string>
#include "../Scene/Entities/ScriptableEntity.h"


typedef void (ScriptModuleRegister)();
//#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
// Now we create a typedef for this function pointer
//typedef X_INPUT_GET_STATE(x_input_get_state);
extern "C"
{
    class ScriptLoader
    {
    private:
        static std::map<std::string, ScriptableEntity*> m_Scripts;

    public:
        static bool RegisterScript(ScriptableEntity* script);


        static ScriptableEntity* GetScript(std::string script) {
            return m_Scripts[script];
        }

        static bool LoadModule(const std::string& path);

    };
}
