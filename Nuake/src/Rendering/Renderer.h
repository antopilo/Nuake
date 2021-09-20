#pragma once
#include "src/Core/Maths.h"
#include "Shaders/Shader.h"
#include "src/Scene/Components/Components.h"
#include "src/Core/Core.h"
#include "Buffers/VertexArray.h"
#include "RenderList.h"
#include "Buffers/UniformBuffer.h"

namespace Nuake
{
	struct Light 
	{
		TransformComponent transform;
		LightComponent light;
	};

	const int MAX_LIGHT = 64;
	//struct LightDataArray
	//{
	//	LightData Lights[MAX_LIGHT];
	//};

	struct LightData
	{
		int ShadowMapsIDs[CSM_AMOUNT];
		float CascadeDepth[CSM_AMOUNT];
		Matrix4 LightTransforms[CSM_AMOUNT];
		int Type;
		Vector3 Position;
		Vector3 Direction;
		Vector3 Color;
		int Volumetric;
	};

	class Renderer 
	{
	private:
		static RenderList m_RenderList;
	public:
		static VertexArray* Renderer::QuadVertexArray;
		static VertexBuffer* Renderer::QuadVertexBuffer;
		static VertexArray* Renderer::CubeVertexArray;
		static VertexBuffer* Renderer::CubeVertexBuffer;

		static Shader* m_Shader;
		static Shader* m_ShadowmapShader;
		static Shader* m_SkyboxShader;
		static Shader* m_BRDShader;
		static Shader* m_GBufferShader;
		static Shader* m_DeferredShader;
		static Shader* m_ProceduralSkyShader;
		static Shader* m_DebugShader;

		static Ref<UniformBuffer> m_LightsUniformBuffer;

		static void Init();
		static void LoadShaders();

		static void BeginScene();
		static void SubmitMesh(Ref<Mesh> mesh, Matrix4 transform);
		static void Flush(Shader* shader, bool depthOnly = false);

		// Drawing states
		static void BeginDraw(Ref<Camera> camera);
		static void EndDraw();

		// Lights
		static std::vector<Light> m_Lights;
		static void RegisterLight(TransformComponent transform, LightComponent light);
		static void RegisterDeferredLight(TransformComponent transform, LightComponent light);

		

		// Debug
		static void DrawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color);
		static void DrawCube(TransformComponent transform, glm::vec4 color);
		static void DrawSphere(TransformComponent transform, glm::vec4 color);
		static void DrawQuad(Matrix4 transform = Matrix4());
	};
}
