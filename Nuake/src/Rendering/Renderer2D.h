#pragma once
#include <src/Rendering/Shaders/Shader.h>
#include <src/Core/Core.h>
#include <src/Core/Maths.h>
#include "src/UI/Font/Font.h"

class Renderer2D
{
private:
	static unsigned int VAO;
	static unsigned int VBO;
public:
	static Ref<Shader> UIShader;
	static Ref<Shader> TextShader;
	static Matrix4 Projection;

	static void Init();
	static void BeginDraw(Vector2 size);
	static void DrawRect();
	static Vector2 CalculateStringSize(const std::string& str, Ref<Font> font, Vector2 position, float fontSize);
	static void DrawString(const std::string& str, Ref<Font> font, Vector2 position, float fontSize);
	static void DrawChar(Char& letter, Ref<Font> font, Vector2 position, Vector2 size);
	static void EndDraw();
};