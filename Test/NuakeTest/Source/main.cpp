#include "catch2/catch_amalgamated.hpp"
#define CATCH_CONFIG_MAIN

#include "Engine.h"


namespace Engine
{
    TEST_CASE("Window creation", "[window]")
    {
        Nuake::Engine::Init();

        REQUIRE(Nuake::Engine::GetCurrentWindow() != nullptr);
    }

    TEST_CASE("Window shutdown", "[window]")
    {
        Nuake::Engine::Init();

        Nuake::Engine::GetCurrentWindow()->Close();

        REQUIRE(Nuake::Engine::GetCurrentWindow()->ShouldClose());
    }
}
