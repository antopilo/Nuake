#pragma once
#include "RenderList.h"

#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "src/Rendering/Camera.h"
#include "src/Scene/Components/LightComponent.h"
#include "src/Scene/Components/TransformComponent.h"

#include "Shaders/Shader.h"
#include "Buffers/UniformBuffer.h"
#include "Buffers/VertexArray.h"


namespace Nuake
{
	struct Light
	{
		TransformComponent transform;
		LightComponent light;
	};

	struct LightData
	{
		int ShadowMapsIDs[4];
		float CascadeDepth[4];
		Matrix4 LightTransforms[4];
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
		static uint32_t MAX_LIGHT;

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
		static Ref<Mesh> QuadMesh;
		static Ref<Mesh> SphereMesh;

		static void Init();
		static void LoadShaders();

		static void SubmitMesh(Ref<Mesh> mesh, const Matrix4& transform, const int32_t entityId = -1, const Matrix4& previousTransform = Matrix4(1.0f));
		static void SubmitCube(Matrix4 transform);
		static void Flush(Shader* shader, bool depthOnly = false);

		static Ref<Mesh> CreateSphereMesh();

		// Drawing states
		static void BeginDraw(Ref<Camera> camera);
		static void EndDraw();

		// Lights
		static std::vector<Light> m_Lights;
		static void RegisterDeferredLight(TransformComponent transform, LightComponent light);

		// Debug
		static void DrawLine(Vector3 start, Vector3 end, Color color, Matrix4 transform = Matrix4());
		static void DrawLine(Vector3 start, Vector3 end, Vector3 color);
		static void DrawCube(TransformComponent transform, glm::vec4 color);
		static void DrawSphere(TransformComponent transform, glm::vec4 color);
		static void DrawQuad(Matrix4 transform = Matrix4());
	};
}
