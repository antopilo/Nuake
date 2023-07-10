#pragma once
#include "Core/Core.h"
#include "Core/Timestep.h"
#include "Core/Maths.h"

struct GLFWwindow;

namespace Nuake
{
	class Scene;
	class FrameBuffer;

	class Window
	{
	private:
		const std::string DEFAULT_TITLE = "Untitled Window";
		const uint32_t DEFAULT_WIDTH = 1280;
		const uint32_t DEFAULT_HEIGHT = 720;

		GLFWwindow* m_Window;

		std::string m_Title;
		uint32_t m_Width;
		uint32_t m_Height;
		
		Ref<FrameBuffer> m_Framebuffer;
		Ref<Scene> m_Scene;
	
	public:
		Window();
		~Window() = default;

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

		void SetWindowIcon(const std::string& path);
		void SetVSync(bool enabled);

	private:
		void InitImgui();
	};
}
