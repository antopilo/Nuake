#include "Renderer2D.h"
#include "GL/glew.h"
#include "../Core/Maths.h"
Ref<Shader> Renderer2D::UIShader;
Ref<Shader> Renderer2D::TextShader;

unsigned int Renderer2D::VAO;
unsigned int Renderer2D::VBO;

Matrix4 Renderer2D::Projection;

void Renderer2D::Init()
{
	UIShader = CreateRef<Shader>("resources/Shaders/ui.shader");
	TextShader = CreateRef<Shader>("resources/Shaders/sdf_text.shader");

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
	Projection = glm::ortho(0.f, size.x, size.y, 0.f, -0.5f, 1000.0f);
	UIShader->Bind();
	UIShader->SetUniformMat4f("projection", Projection);
}

void Renderer2D::DrawRect()
{
	glBindVertexArray(Renderer2D::VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

Vector2 Renderer2D::CalculateStringSize(const std::string& str, Ref<Font> font, Vector2 position, float fontSize)
{
	float advance = 0.0f;
	float Y = 0.0f; 
	for (char const& c : str) {
		Char& letter = font->GetChar((int)c);
		advance += letter.Advance * fontSize;
		if(letter.PlaneBounds.top - letter.PlaneBounds.bottom > Y)
			Y = letter.PlaneBounds.top - letter.PlaneBounds.bottom;
	}
	return Vector2(advance, Y);
}

void Renderer2D::DrawString(const std::string& str, Ref<Font> font, Vector2 position, float fontSize)
{
	TextShader->Bind();
	TextShader->SetUniformMat4f("projection", Projection);
	float advance = 0.0f;
	font->FontAtlas->Bind(5);
	TextShader->SetUniform1i("msdf", 5);
	for (char const& c : str) {
		Char& letter = font->GetChar((int)c);

		Matrix4 mat = Matrix4(1.0f);
		mat = glm::translate(mat, Vector3(position.x + advance, position.y - (letter.PlaneBounds.top * fontSize), 0.f));
		float scaleX = letter.PlaneBounds.right - letter.PlaneBounds.left;
		float scaleY = letter.PlaneBounds.top - letter.PlaneBounds.bottom;
		mat = glm::scale(mat, Vector3(scaleX * fontSize, scaleY * fontSize, 0.f));

		TextShader->SetUniform2f("texPos", letter.AtlasBounds.Pos.x, letter.AtlasBounds.Pos.y);
		TextShader->SetUniform2f("texScale", letter.AtlasBounds.Size.x, letter.AtlasBounds.Size.y);
		TextShader->SetUniformMat4f("model", mat);
		//TextShader->SetUniform4f("bgColor", 0, 0, 1, 1);
		TextShader->SetUniform4f("bgColor", 0.f, 0.f, 0.f, 0.f);
		TextShader->SetUniform4f("fgColor", 1.f, 1.f, 1.f, 1.f);
		//TextShader->SetUniform1f("pxRange", 32);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		advance += letter.Advance * fontSize;
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

}
