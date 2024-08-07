#pragma once
#include "Core/Core.h"
#include "Core/Maths.h"
#include "Core/Timestep.h"

struct GLFWwindow;

namespace Nuake
{
	class Scene;
	class FrameBuffer;

	class Window
	{
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

		void SetPosition(const Vector2& position);

		void SetMonitor(int monitor);

		void Center();

		Ref<Scene> GetScene();
		bool SetScene(Ref<Scene> scene);

		void SetTitle(const std::string& title);
		std::string GetTitle();

		void SetWindowIcon(const std::string& path);
		void SetVSync(bool enabled);
		void SetDecorated(bool enabled);
		void SetFullScreen(bool enabled);
		void Maximize();
	private:
		const std::string DEFAULT_TITLE = "Untitled Window";
		const uint32_t DEFAULT_WIDTH = 1280;
		const uint32_t DEFAULT_HEIGHT = 720;

		GLFWwindow* m_Window;

		std::string m_Title;
		uint32_t m_Width;
		uint32_t m_Height;
		Vector2 m_Position;

		Ref<FrameBuffer> m_Framebuffer;
		Ref<Scene> m_Scene;

		void InitImgui();
	};
}
