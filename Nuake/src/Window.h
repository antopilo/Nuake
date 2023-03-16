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
		const std::string DEFAULT_TITLE = "Untitled Window";

		static Ref<Window> s_Instance;

		std::string m_Title;
		int m_Width = 1280;
		int m_Height = 720;

		GLFWwindow* m_Window;

		Ref<FrameBuffer> m_Framebuffer;

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
		Vector2 GetSize() const;
		void SetSize(const Vector2& size);

		void SetMonitor(int monitor);

		Ref<Scene> GetScene();
		bool SetScene(Ref<Scene> scene);

		void SetTitle(const std::string& title);
		std::string GetTitle();
	};
}

