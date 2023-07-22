#pragma once
#include "ComponentPanel.h"


const std::string TEMPLATE_SCRIPT_FIRST = R"(import "Nuake:Engine" for Engine 
import "Nuake:ScriptableEntity" for ScriptableEntity 
import "Nuake:Input" for Input 
import "Nuake:Math" for Vector3, Math 
import "Nuake:Scene" for Scene 

class )";

const std::string TEMPLATE_SCRIPT_SECOND = R"( is ScriptableEntity { 
        construct new() { 
        } 

        // Called when the scene gets initialized 
        init() { 
            // Engine.Log("Hello World!") 
        } 
 
        // Called every update 
        update(ts) { 
        } 
 
        // Called 90 times per second 
        fixedUpdate(ts) { 
        }  
         
        // Called on shutdown 
        exit() { 
        } 
} 
)";


class ScriptPanel : ComponentPanel {

public:
    ScriptPanel() {}

    void Draw(Nuake::Entity entity) override;
};