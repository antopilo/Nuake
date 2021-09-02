#pragma once
#include "Core/Timestep.h"
#include "Rendering/Buffers/Framebuffer.h"
#include "Scene/Scene.h"
#include "Core/Core.h"

struct GLFWwindow;

namespace Nuake
{
	class Window
	{
	private:
		static Ref<Window> s_Instance;

		int m_Width = 1280;
		int m_Height = 720;

		GLFWwindow* m_Window;

		Ref<FrameBuffer> m_Framebuffer;
		Ref<FrameBuffer> m_GBuffer;
		Ref<FrameBuffer> m_DeferredBuffer;

		Ref<Scene> m_Scene;
		Vector2 m_FramebufferOffset;

	public:
		Window();
		~Window();

		static Ref<Window> Get(); // Get the window instance
		GLFWwindow* GetHandle();

		bool ShouldClose();

		int Init();
		void Update(Timestep ts);
		void FixedUpdate(Timestep ts);
		void Draw();
		void EndDraw();

		Ref<FrameBuffer> GetFrameBuffer() const;
		Ref<FrameBuffer> GetGBuffer() const;
		Ref<FrameBuffer> GetDeferredBuffer() const;
		Vector2 GetSize();

		Ref<Scene> GetScene();
		bool SetScene(Ref<Scene> scene);
	};
}

