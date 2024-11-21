#pragma once

#include "Core/Core.h"
#include "Core/Maths.h"
#include "Core/MulticastDelegate.h"
#include "Core/Timestep.h"

#include <functional>

struct GLFWwindow;

namespace Nuake
{
	class Scene;
	class FrameBuffer;

	DECLARE_MULTICAST_DELEGATE(OnWindowSetSceneDelegate, Ref<Scene>, Ref<Scene>)
	
	class Window
	{
	public:
		Window();
		~Window() = default;

		static Ref<Window> Get(); // Get the window instance

	public:
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
		bool SetScene(Ref<Scene> newScene);

		void SetTitle(const std::string& title);
		std::string GetTitle();
		void ShowTitleBar(bool visible);
		void SetWindowIcon(const std::string& path);
		void SetVSync(bool enabled);
		void SetDecorated(bool enabled);
		void SetFullScreen(bool enabled);
		void Maximize();
		bool IsMaximized();
		void OnWindowFocused(Window& window, bool focused);
		void OnWindowClosed(Window& window);
		void OnDragNDropCallback(Window& window, const std::vector<std::string>& paths);
		void TitlebarHitTest(Window& window, int x, int y, bool& hit);

		void SetOnWindowFocusedCallback(std::function<void(Window& window, bool focused)> callback);
		void SetOnWindowClosedCallback(std::function<void(Window& window)> callback);
		void SetOnDragNDropCallback(std::function<void(Window&, const std::vector<std::string>& paths)> callback);
		void SetTitlebarHitTestCallback(std::function<void(Window&, int x, int y, bool& hit)> callback);

		// Delegate is broadcasted BEFORE the actual internal scene has been reassigned, this is to keep
		// the potential old scene relevant before its ultimate destruction.
		OnWindowSetSceneDelegate& OnWindowSetScene() { return windowSetSceneDelegate; }

	private:
		const std::string DEFAULT_TITLE = "Nuake Engine";
		const uint32_t DEFAULT_WIDTH = 1280;
		const uint32_t DEFAULT_HEIGHT = 720;

		GLFWwindow* window;

		std::string title;
		uint32_t width;
		uint32_t height;
		Vector2 position;

		Ref<FrameBuffer> framebuffer;
		Ref<Scene> scene;

		// Callbacks
		std::function<void(Window&)> onWindowClosedCallback;
		std::function<void(Window&, bool)> onWindowFocusedCallback;
		std::function<void(Window&, const std::vector<std::string>& paths)> onDragNDropCallback;
		std::function<void(Window&, int x, int y, bool& hit)> titleBarHitTestCallback;

		void InitImgui();

		OnWindowSetSceneDelegate windowSetSceneDelegate;

	private:
	};
}
