#include "Renderer2D.h"
#include "GL/glew.h"

Ref<Shader> Renderer2D::UIShader;
unsigned int Renderer2D::VAO;
unsigned int Renderer2D::VBO;

Matrix4 Renderer2D::Projection;

void Renderer2D::Init()
{
	UIShader = CreateRef<Shader>("resources/Shaders/ui.shader");
	Projection = glm::ortho(0.f, 1920.f, 0.0f, 1080.f, -0.5f, 100.0f);

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

void Renderer2D::BeginDraw()
{
	UIShader->Bind();
	UIShader->SetUniformMat4f("projection", Projection);
}

void Renderer2D::DrawRect()
{
	glBindVertexArray(Renderer2D::VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Renderer2D::EndDraw()
{

}
