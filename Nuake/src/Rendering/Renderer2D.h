#pragma once
#include "src/Rendering/Shaders/Shader.h"
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/UI/Font/Font.h"

namespace Nuake
{
	struct TextStyle;
	class Renderer2D
	{
	private:
		static unsigned int VAO;
		static unsigned int VBO;
	public:
		static Shader* UIShader;
		static Shader* TextShader;
		static Matrix4 Projection;

		static void Init();
		static void BeginDraw(Vector2 size);
		static void DrawRect();
		static void DrawRect(Matrix4 transform, Vector2 size, Color color, float borderRadius = 0.0f);
		static void DrawRect(Vector2 position, Vector2 size, Color color, float borderRadius = 0.0f);

		static Vector2 CalculateStringSize(const std::string& str, const TextStyle& style);
		static void DrawString(const std::string& str, TextStyle style, Matrix4 model);
		static void DrawChar(Char& letter, Ref<Font> font, Vector2 position, Vector2 size);
		static void EndDraw();
	};
}
