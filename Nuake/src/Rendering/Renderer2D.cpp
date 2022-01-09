#include "Renderer2D.h"
#include "GL/glew.h"
#include <Engine.h>
#include "src/UI/Nodes/TextNode.h"
#include "Shaders/ShaderManager.h"

#include "src/Vendors/glm/gtx/matrix_decompose.hpp"
namespace Nuake
{
	Shader* Renderer2D::UIShader;
	Shader* Renderer2D::TextShader;

	unsigned int Renderer2D::VAO;
	unsigned int Renderer2D::VBO;

	Matrix4 Renderer2D::Projection;

	void Renderer2D::Init()
	{
		UIShader = ShaderManager::GetShader("resources/Shaders/ui.shader");
		TextShader = ShaderManager::GetShader("resources/Shaders/sdf_text.shader");

		float quad_Vertices[] = {
			// positions        // texture Coords
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
			0.0f,  1.0f, 1.0f, 0.0f, 1.0f,
			 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 1.0f, 1.0f
		};

		// setup plane VAO
		glGenVertexArrays(1, &Renderer2D::VAO);
		glGenBuffers(1, &Renderer2D::VBO);
		glBindVertexArray(Renderer2D::VAO);
		glBindBuffer(GL_ARRAY_BUFFER, Renderer2D::VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_Vertices), &quad_Vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	}

	void Renderer2D::BeginDraw(Vector2 size)
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		Projection = glm::ortho(0.f, size.x, size.y, 0.f, -1.f, 1000.0f);
		UIShader->Bind();
		UIShader->SetUniformMat4f("projection", Projection);
	}

	void Renderer2D::DrawRect()
	{
		glBindVertexArray(Renderer2D::VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Renderer2D::DrawRect(Matrix4 transform, Vector2 size, Color color, float borderRadius)
	{
		Vector3 translation;
		glm::quat rotation;
		Vector3 scale;
		Vector3 skew;
		Vector4 perspective;
		glm::decompose(transform, scale, rotation, translation, skew, perspective);

		BeginDraw({ 1920.0f, 1080.0f });
		UIShader->SetUniformMat4f("model", transform);
		UIShader->SetUniform1f("u_border_radius", borderRadius);
		UIShader->SetUniform2f("u_size", scale.x, scale.y);

		glBindVertexArray(Renderer2D::VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Renderer2D::DrawRect(Vector2 position, Vector2 size, Color color, float borderRadius )
	{
		Matrix4 model = Matrix4(1.f);
		model = glm::translate(model, Vector3(position.x, position.y, 0.f));
		model = glm::scale(model, Vector3(size.x, size.y, 1.f));

		UIShader->SetUniformMat4f("model", model);
		UIShader->SetUniform1f("u_BorderRadius", borderRadius);
		UIShader->SetUniform2f("u_Size", size.x, size.y);
		UIShader->SetUniform4f("u_BackgroundColor", color.r, color.g, color.b, color.a);
		
		glBindVertexArray(Renderer2D::VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	Vector2 Renderer2D::CalculateStringSize(const std::string& str, const TextStyle& style)
	{
		float lineHeight = 0.0f;
		for (char const& c : str)
		{
			Char& letter = style.font->GetChar((int)c);

			if (letter.PlaneBounds.top - letter.PlaneBounds.bottom > lineHeight)
				lineHeight = letter.PlaneBounds.top - letter.PlaneBounds.bottom;
		}

		float advance = 0.0f;
		for (char const& c : str)
		{
			Char& letter = style.font->GetChar((int)c);
			advance += letter.Advance * style.fontSize;
		}
		return Vector2(advance, lineHeight * style.fontSize);
	}

	void Renderer2D::DrawString(const std::string& str, TextStyle style, Matrix4 model)
	{
		TextShader->Bind();
		TextShader->SetUniformMat4f("projection", Projection);

		style.font->FontAtlas->Bind(5);
		TextShader->SetUniform1i("msdf", 5);

		float lineHeight = 0.0f;
		for (char const& c : str)
		{
			Char& letter = style.font->GetChar((int)c);

			if (letter.PlaneBounds.top - letter.PlaneBounds.bottom > lineHeight)
				lineHeight = letter.PlaneBounds.top - letter.PlaneBounds.bottom;
		}

		float advance = 0.0f;
		for (char const& c : str) 
		{
			Char& letter = style.font->GetChar((int)c);

			Matrix4 mat = glm::translate(model, Vector3(advance, (-(letter.PlaneBounds.top ) + (lineHeight)) * style.fontSize, 0.f));
			float scaleX = letter.PlaneBounds.right - letter.PlaneBounds.left;
			float scaleY = letter.PlaneBounds.top - letter.PlaneBounds.bottom;
			
			mat = glm::scale(mat, Vector3(scaleX * style.fontSize, scaleY * style.fontSize, 0.f));
			TextShader->SetUniformMat4f("model", mat);

			TextShader->SetUniform2f("texPos", letter.AtlasBounds.Pos.x, letter.AtlasBounds.Pos.y);
			TextShader->SetUniform2f("texScale", letter.AtlasBounds.Size.x, letter.AtlasBounds.Size.y);
			TextShader->SetUniform4f("bgColor", 1.f, 1.f, 1.f, 0.f);
			TextShader->SetUniform4f("fgColor", 1.f, 1.f, 1.f, 1.f);
			TextShader->SetUniform1f("pxRange", style.fontSize);
			//TextShader->SetUniform1f("pxRange", 32);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			advance += letter.Advance * style.fontSize;
		}
	}

	void Renderer2D::DrawChar(Char& letter, Ref<Font> font, Vector2 position, Vector2 size)
	{
		TextShader->Bind();
		TextShader->SetUniformMat4f("projection", Projection);

		Matrix4 mat = Matrix4(1.0f);
		mat = glm::translate(mat, Vector3(position.x, position.y, 0.f));
		mat = glm::scale(mat, Vector3(letter.AtlasBounds.Size.x, letter.AtlasBounds.Size.y, 0.f));

		//TextShader->SetUniform1f("u_border_radius", 8.1f);
		//TextShader->SetUniform2f("u_size", 100.f, 100.f);

		font->FontAtlas->Bind(5);
		//
		TextShader->SetUniform1i("msdf", 5);
		TextShader->SetUniform2f("texPos", letter.AtlasBounds.Pos.x, letter.AtlasBounds.Pos.y);
		TextShader->SetUniform2f("texScale", letter.AtlasBounds.Size.x, letter.AtlasBounds.Size.y);
		TextShader->SetUniformMat4f("model", mat);
		//TextShader->SetUniform4f("bgColor", 0, 0, 1, 1);
		TextShader->SetUniform4f("bgColor", 0.f, 0.f, 0.f, 0.f);
		TextShader->SetUniform4f("fgColor", 1.f, 1.f, 1.f, 1.f);
		//TextShader->SetUniform1f("pxRange", 32);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Renderer2D::EndDraw()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
}

