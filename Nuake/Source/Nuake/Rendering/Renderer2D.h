#pragma once
#include "Nuake/Rendering/Shaders/Shader.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"

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
		static void EndDraw();
	};
}
