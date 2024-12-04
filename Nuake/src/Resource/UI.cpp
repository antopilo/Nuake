#include "UI.h"
#include "Engine.h"
#include "src/FileSystem/FileSystem.h"
#include "src/Rendering/Buffers/Framebuffer.h"
#include "src/UI/NuakeUI.h"
#include "src/UI/Parsers/CanvasParser.h"
#include "src/UI/UIInputManager.h"


using namespace Nuake;
using namespace NuakeUI;

MyInputManager* UIResource::inputManager = nullptr;

UIResource::UIResource(const std::string& path) :
	filePath(path)
{
	const Vector2 defaultSize = { 1280, 720 };
	framebuffer = CreateRef<FrameBuffer>(false, defaultSize);
	framebuffer->SetTexture(CreateRef<Texture>(defaultSize, GL_RGBA));
	framebuffer->SetTexture(CreateRef<Texture>(defaultSize, GL_DEPTH_COMPONENT), 0x8D00); // Depth

	if (!inputManager)
	{
		inputManager = new MyInputManager(*Engine::GetCurrentWindow());
	}

	canvas = Canvas::New();
	canvas->uuid = this->ID;
	canvas = CanvasParser::Get().Parse(canvas, FileSystem::RelativeToAbsolute(path));

	if (canvas != nullptr)
	{
		canvas->SetInputManager(inputManager);
		canvas->ComputeLayout(defaultSize);
	}
}

void UIResource::Tick()
{
	if (canvas != nullptr)
	{
		canvas->Tick();
	}
}

void UIResource::Draw()
{
	RenderCommand::SetClearColor({ 0, 0, 0, 1.0f });
	framebuffer->Bind();
	{
		RenderCommand::Clear();

		if (canvas != nullptr)
		{
			//canvas->Draw();
		}
	}
	framebuffer->Unbind();
}

void UIResource::Resize(const Vector2& size)
{
	framebuffer->QueueResize(size);
	if (canvas != nullptr)
	{
		canvas->ComputeLayout(size);
	}
}

void UIResource::Reload()
{
	if (!FileSystem::FileExists(filePath))
	{
		return;
	}

	canvas = CanvasParser::Get().Parse(canvas, FileSystem::RelativeToAbsolute(filePath));
	if (canvas != nullptr)
	{
		canvas->SetInputManager(inputManager);
		canvas->ComputeLayout(framebuffer->GetSize());
	}
}

void Nuake::UIResource::SetMousePosition(const Vector2 & mousePos)
{
	canvas->SetOverrideMousePosition(mousePos);
}

Ref<Texture> UIResource::GetOutputTexture() const
{
	return framebuffer->GetTexture();
}
