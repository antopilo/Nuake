#include "LoadingSplash.h"

#include "../Misc/InterfaceFonts.h"

#include <Nuake/Core/Maths.h>
#include "Nuake/Window.h"
#include "Nuake/UI/ImUI.h"

#include <imgui/imgui.h>

#include <Nuake/Rendering/Textures/TextureManager.h>
#include <Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h>

using namespace Nuake;

LoadingSplash::LoadingSplash()
{
	_NuakeLogo = Nuake::TextureManager::Get()->GetTexture2(NUAKE_LOGO_PATH);
	_NuakeSplash = Nuake::TextureManager::Get()->GetTexture2(NUAKE_SPLASH_PATH);

	Nuake::Window::Get()->SetDecorated(false);
	Nuake::Window::Get()->SetSize({ 640, 400 });
	Nuake::Window::Get()->Center();
}

void LoadingSplash::Draw(const std::string& project)
{
	// Make viewport fullscreen
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Welcome Screen", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
	{
		const Vector2 splashSize = _NuakeSplash->GetSize();
		const ImVec2 splashImguiSize = ImVec2(splashSize.x, splashSize.y);
		ImGui::Image((ImTextureID)_NuakeSplash->GetImGuiDescriptorSet(), splashImguiSize, {0, 1}, {1, 0});

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
		ImGui::SetCursorPosX(8.0f);
		{
			UIFont font(Fonts::Bold);
			ImGui::Text("Loading project... ");
		}
		ImGui::SetCursorPosX(8.0f);
		ImGui::TextColored({ 1.0, 1.0, 1.0, 0.5 }, project.c_str());
		//ImGui::Text(project.c_str());
	}

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}