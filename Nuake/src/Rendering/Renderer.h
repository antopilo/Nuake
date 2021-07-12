#pragma once
#include "Shaders/Shader.h"
#include "src/Scene/Components/Components.h"
#include "src/Core/Core.h"
#include "Buffers/VertexArray.h"

namespace Nuake
{
	struct Light 
	{
		TransformComponent transform;
		LightComponent light;
	};

	class Renderer 
	{
	public:
		static VertexArray* Renderer::QuadVertexArray;
		static VertexBuffer* Renderer::QuadVertexBuffer;

		static Ref<Shader> m_Shader;
		static Ref<Shader> m_ShadowmapShader;
		static Ref<Shader> m_SkyboxShader;
		static Ref<Shader> m_BRDShader;
		static Ref<Shader> m_GBufferShader;
		static Ref<Shader> m_DeferredShader;
		static Ref<Shader> m_ProceduralSkyShader;
		static Ref<Shader> m_DebugShader;

		static void Init();
		static void LoadShaders();

		static void BeginScene();
		static void Flush();

		// Drawing states
		static void BeginDraw(Ref<Camera> camera);
		static void EndDraw();

		// Lights
		static std::vector<Light> m_Lights;
		static void RegisterLight(TransformComponent transform, LightComponent light, Ref<Camera> cam);
		static void RegisterDeferredLight(TransformComponent transform, LightComponent light, Camera* cam);

		static void SubmitMesh(Ref<Mesh> mesh);

		// Debug
		static void DrawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color);
		static void DrawCube(TransformComponent transform, glm::vec4 color);
		static void DrawSphere(TransformComponent transform, glm::vec4 color);
		static void DrawQuad(Matrix4 transform);
	};
}
