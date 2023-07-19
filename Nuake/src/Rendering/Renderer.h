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
		static VertexArray* QuadVertexArray;
		static VertexBuffer* QuadVertexBuffer;
		static VertexArray* CubeVertexArray;
		static VertexBuffer* CubeVertexBuffer;

		static Shader* m_Shader;
		static Shader* m_ShadowmapShader;
		static Shader* m_SkyboxShader;
		static Shader* m_BRDShader;
		static Shader* m_GBufferShader;
		static Shader* m_DeferredShader;
		static Shader* m_ProceduralSkyShader;
		static Shader* m_DebugShader;

		static Ref<UniformBuffer> m_LightsUniformBuffer;

		static Ref<Mesh> CubeMesh;

		static void Init();
		static void LoadShaders();

		static void SubmitMesh(Ref<Mesh> mesh, Matrix4 transform, const int32_t entityId = -1);
		static void SubmitCube(Matrix4 transform);
		static void Flush(Shader* shader, bool depthOnly = false);

		// Drawing states
		static void BeginDraw(Ref<Camera> camera);
		static void EndDraw();

		// Lights
		static std::vector<Light> m_Lights;
		static void RegisterLight(TransformComponent transform, LightComponent light);
		static void RegisterDeferredLight(TransformComponent transform, LightComponent light);

		// Debug
		static void DrawLine(Vector3 start, Vector3 end, Color color, Matrix4 transform = Matrix4());
		static void DrawDebugLine(Vector3 start, Vector3 end, Vector3 color);
		static void DrawCube(TransformComponent transform, glm::vec4 color);
		static void DrawSphere(TransformComponent transform, glm::vec4 color);
		static void DrawQuad(Matrix4 transform = Matrix4());
	};
}
