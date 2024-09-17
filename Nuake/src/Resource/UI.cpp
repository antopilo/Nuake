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
	framebuffer = CreateRef<FrameBuffer>(true, defaultSize);
	framebuffer->SetTexture(CreateRef<Texture>(defaultSize, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE));

	if (!inputManager)
	{
		inputManager = new MyInputManager(*Engine::GetCurrentWindow());
	}

	canvas = Canvas::New();
	canvas->uuid = this->ID;
	canvas = CanvasParser::Get().Parse(canvas, FileSystem::RelativeToAbsolute(path));

	if (!canvas)
	{
		canvas->SetInputManager(inputManager);
		canvas->ComputeLayout(defaultSize);
	}
}

void UIResource::Tick()
{
	if (canvas)
	{
		canvas->Tick();
	}
}

void UIResource::Draw()
{
	framebuffer->Bind();
	{
		RenderCommand::SetClearColor({ 0, 0, 0, 0 });
		RenderCommand::Clear();

		if (canvas)
		{
			canvas->Draw();
		}
	}
	framebuffer->Unbind();
}

void UIResource::Resize(const Vector2& size)
{
	framebuffer->QueueResize(size);
	if (canvas)
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
	if (canvas)
	{
		canvas->SetInputManager(inputManager);
		canvas->ComputeLayout(framebuffer->GetSize());
	}
}

Ref<Texture> UIResource::GetOutputTexture() const
{
	return framebuffer->GetTexture();
}
