#pragma once
#include "Resource.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"

namespace NuakeUI
{
	class Canvas;
}

namespace Nuake
{
	class FrameBuffer;
	class MyInputManager;
	class Texture;

	class UIResource : public Resource
	{
	public:
		UIResource(const std::string& path);
		~UIResource() = default;

		void Tick();
		void Draw();
		void Resize(const Vector2& size);
		void Reload();

		void SetMousePosition(const Vector2& mousePos);
		Ref<Texture> GetOutputTexture() const;
		Ref<NuakeUI::Canvas> GetCanvas() { return canvas; }

	private:
		Ref<NuakeUI::Canvas> canvas;
		Ref<FrameBuffer> framebuffer;
		static MyInputManager* inputManager;
		std::string filePath;
	};
}