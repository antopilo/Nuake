#pragma once
#include "ComponentPanel.h"


const std::string NET_TEMPLATE_SCRIPT_FIRST = R"(using Nuake.Net;

namespace NuakeShowcase
{
    class )";

const std::string NET_TEMPLATE_SCRIPT_SECOND = R"( : Entity
    {
        public override void OnInit()
        {
            // Called once at the start of the game
        }

        
        public override void OnUpdate(float dt)
        {
            // Called every frame
        }
        
        public override void OnFixedUpdate(float dt)
        {
            // Called every fixed update
        }

        
        public override void OnDestroy()
        {
            // Called at the end of the game fixed update
        }
    }
} 
)";


class NetScriptPanel : ComponentPanel {

public:
    NetScriptPanel() {}

    void Draw(Nuake::Entity entity) override;
};