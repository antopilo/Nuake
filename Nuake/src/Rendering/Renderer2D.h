#pragma once
#include <src/Rendering/Shaders/Shader.h>
#include <src/Core/Core.h>
#include <src/Core/Maths.h>
class Renderer2D
{
private:
	static unsigned int VAO;
	static unsigned int VBO;
public:
	static Ref<Shader> UIShader;
	static Matrix4 Projection;

	static void Init();
	static void BeginDraw();
	static void DrawRect();
	static void EndDraw();
};