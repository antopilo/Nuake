#pragma once
#include "Core/Timestep.h"
#include "Rendering/Framebuffer.h"
#include "Scene/Scene.h"
#include "Core/Core.h"
struct GLFWwindow;
class __declspec(dllexport) Window
{
private:
	static Window* s_Instance;
	GLFWwindow* m_Window;
	Ref<FrameBuffer> m_Framebuffer;
	GBuffer* m_GBuffer;
	Ref<Scene> m_Scene;
	FrameBuffer* m_DeferredFrambuffer;
	int Width, Height;
    
public:
	Window();
	~Window();

	static Window* Get();
	GLFWwindow* GetHandle();

	

	int Init();
	void Update(Timestep ts);
	void Draw();
	void EndDraw();

	bool ShouldClose();

	Ref<Texture> GetSceneRenderTexture();

	void DrawQuad();
	Ref<Scene> GetScene();
	bool SetScene(Ref<Scene> scene);

	Ref<FrameBuffer> GetFrameBuffer() const;
};
