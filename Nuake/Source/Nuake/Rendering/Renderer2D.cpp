#include "Renderer2D.h"
#include "Engine.h"
#include "Shaders/ShaderManager.h"

#include "Vendors/glm/gtx/matrix_decompose.hpp"
namespace Nuake
{
	Shader* Renderer2D::UIShader;
	Shader* Renderer2D::TextShader;

	unsigned int Renderer2D::VAO;
	unsigned int Renderer2D::VBO;

	Matrix4 Renderer2D::Projection;

	void Renderer2D::Init()
	{
		Logger::Log("Renderer 2D initiaizing", "render2d");
		UIShader = ShaderManager::GetShader("Resources/Shaders/ui.shader");
		TextShader = ShaderManager::GetShader("Resources/Shaders/sdf_text.shader");

		float quad_Vertices[] = {
			// positions      texture Coords
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f
		};

		
		Logger::Log("Renderer 2D create VAOS", "render2d");
		// setup plane VAO
		//glGenVertexArrays(1, &Renderer2D::VAO);
		//glGenBuffers(1, &Renderer2D::VBO);
		//glBindVertexArray(Renderer2D::VAO);
		//glBindBuffer(GL_ARRAY_BUFFER, Renderer2D::VBO);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(quad_Vertices), &quad_Vertices, GL_STATIC_DRAW);
		//glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		//glEnableVertexAttribArray(1);
		//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	void Renderer2D::BeginDraw(Vector2 size)
	{
		//glDisable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
		Projection = glm::ortho(0.f, size.x, size.y, 0.f, -1.f, 1000.0f);
		UIShader->Bind();
		UIShader->SetUniform("projection", Projection);
	}

	void Renderer2D::DrawRect()
	{
		//glBindVertexArray(Renderer2D::VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
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
		UIShader->SetUniform("model", transform);
		UIShader->SetUniform("u_border_radius", borderRadius);
		UIShader->SetUniform("u_size", scale.x, scale.y);

		//glBindVertexArray(Renderer2D::VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Renderer2D::DrawRect(Vector2 position, Vector2 size, Color color, float borderRadius )
	{
		Matrix4 model = Matrix4(1.f);
		model = glm::translate(model, Vector3(position.x, position.y, 0.f));
		model = glm::scale(model, Vector3(size.x, size.y, 1.f));

		UIShader->SetUniform("model", model);
		UIShader->SetUniform("u_BorderRadius", borderRadius);
		UIShader->SetUniform("u_Size", size.x, size.y);
		UIShader->SetUniform("u_BackgroundColor", color.r, color.g, color.b, color.a);
		
		//glBindVertexArray(Renderer2D::VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

