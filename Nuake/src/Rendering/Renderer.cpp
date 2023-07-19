#include "Renderer.h"

#include <GL\glew.h>

#include "RenderCommand.h"

#include "src/Rendering/Camera.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Textures/Cubemap.h"
#include "src/Rendering/Shaders/ShaderManager.h"
#include "Engine.h"
#include "src/Core/Core.h"

#include <glm/gtc/type_ptr.hpp>
#include "Buffers/VertexBufferLayout.h"
#include "src/Rendering/Textures/MaterialManager.h"
#include "src/Rendering/Vertex.h"

namespace Nuake
{
    unsigned int depthTexture;
    unsigned int depthFBO;

    Ref<Mesh> Renderer::CubeMesh;

    Shader* Renderer::m_Shader;
    Shader* Renderer::m_SkyboxShader;
    Shader* Renderer::m_BRDShader;
    Shader* Renderer::m_GBufferShader;
    Shader* Renderer::m_DeferredShader;
    Shader* Renderer::m_ProceduralSkyShader;
    Shader* Renderer::m_DebugShader;
    Shader* Renderer::m_ShadowmapShader;

    VertexArray* Renderer::QuadVertexArray;
    VertexBuffer* Renderer::QuadVertexBuffer;
    VertexArray*  Renderer::CubeVertexArray;
    VertexBuffer* Renderer::CubeVertexBuffer;

    Ref<UniformBuffer> Renderer::m_LightsUniformBuffer;
    RenderList Renderer::m_RenderList = RenderList();

    std::vector<Vertex> CubeVertices
    {
        {Vector3(-0.5f, -0.5f, -0.5f), Vector2(0, 0), Vector3(-1, 0, 0)},
        {Vector3( 0.5f, -0.5f, -0.5f), Vector2(1, 0), Vector3(-1, -1, 0)},
        {Vector3( 0.5f,  0.5f, -0.5f), Vector2(0, 1), Vector3(-1, 0, 0)},
        {Vector3(-0.5f,  0.5f, -0.5f), Vector2(1, 1), Vector3(-1, 0, 0)},
        {Vector3(-0.5f, -0.5f,  0.5f), Vector2(0, 1), Vector3(-1, 0, 0)},
        {Vector3( 0.5f, -0.5f,  0.5f), Vector2(1, 0), Vector3(-1, 0, 0)},
        {Vector3( 0.5f,  0.5f,  0.5f), Vector2(1, 1), Vector3(-1, 0, 0)},
        {Vector3(-0.5f,  0.5f,  0.5f), Vector2(1, 1), Vector3(-1, 0, 0)}
    };

    std::vector<unsigned int> CubeIndices
    {
        0, 1, 3, 3, 1, 2,
        1, 5, 2, 2, 5, 6,
        5, 4, 6, 6, 4, 7,
        4, 0, 7, 7, 0, 3,
        3, 2, 7, 7, 2, 6,
        4, 5, 0, 0, 5, 1
    };

    float QuadVertices[] = {
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
        1.0f,  -1.0f, 0.0f,   1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
        1.0f,   1.0f, 0.0f,   1.0f, 1.0f
    };


    void Renderer::Init()
    {
        RenderCommand::SetRendererAPI(RendererPlatforms::OpenGL);

        LoadShaders();

        m_LightsUniformBuffer = CreateRef<UniformBuffer>(128);

        Ref<Material> material = MaterialManager::Get()->GetMaterial("default");
        CubeMesh = CreateRef<Mesh>();
        CubeMesh->AddSurface(CubeVertices, CubeIndices);
        CubeMesh->SetMaterial(material);
        // Cube buffer
        //CubeVertexArray = new VertexArray();
        //CubeVertexArray->Bind();
        //CubeVertexBuffer = new VertexBuffer(CubeVertices, sizeof(CubeVertices));
        //
        VertexBufferLayout vblayout = VertexBufferLayout();
        vblayout.Push<float>(3);
        //CubeVertexArray->AddBuffer(*CubeVertexBuffer, vblayout);

        // Quad buffer
        QuadVertexArray = new VertexArray();
        QuadVertexArray->Bind();
        QuadVertexBuffer = new VertexBuffer(QuadVertices, sizeof(QuadVertices));

        vblayout = VertexBufferLayout();
        vblayout.Push<float>(3);
        vblayout.Push<float>(2);
        QuadVertexArray->AddBuffer(*QuadVertexBuffer, vblayout);
    }

    void Renderer::LoadShaders()
    {
    }

    void Renderer::SubmitMesh(Ref<Mesh> mesh, Matrix4 transform, const int32_t entityId)
    {
        m_RenderList.AddToRenderList(mesh, transform, entityId);
    }

    void Renderer::SubmitCube(Matrix4 transform)
    {
        m_RenderList.AddToRenderList(CubeMesh, transform, -1);
    }

    void Renderer::Flush(Shader* shader, bool depthOnly)
    {
        m_RenderList.Flush(shader, depthOnly);
    }

    void Renderer::BeginDraw(Ref<Camera> camera)
    {
        Shader* lineShader = ShaderManager::GetShader("resources/Shaders/line.shader");
        lineShader->Bind();
        lineShader->SetUniformMat4f("u_Projection", camera->GetPerspective());
        lineShader->SetUniformMat4f("u_View", camera->GetTransform());

        m_Shader->Bind();
        m_Shader->SetUniformMat4f("u_Projection", camera->GetPerspective());
        m_Shader->SetUniformMat4f("u_View", camera->GetTransform());
        m_Shader->SetUniform3f("u_EyePosition", camera->GetTranslation().x, camera->GetTranslation().y, camera->GetTranslation().z);
    }

    void Renderer::EndDraw()
    {
        m_Lights.clear();
    }

    // List of all lights queued to be used for rendering this frame.
    std::vector<Light> Renderer::m_Lights;

    void Renderer::RegisterDeferredLight(TransformComponent transform, LightComponent light)
    {
        if (m_Lights.size() == 20)
            return;

        Shader* deferredShader = ShaderManager::GetShader("resources/Shaders/deferred.shader");
        deferredShader->Bind();
        m_Lights.push_back({ transform , light });

        size_t idx = m_Lights.size();

        Vector3 direction = light.GetDirection();
        Vector3 pos = transform.GetGlobalPosition();

        //light.m_Framebuffer->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(17);

        int shadowmapAmount = 0;
        if (light.CastShadows && light.Type == Directional)
        {
            for (unsigned int i = 0; i < CSM_AMOUNT; i++)
            {
                light.m_Framebuffers[i]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(17 + i);
                deferredShader->SetUniform1i("ShadowMaps[" + std::to_string(shadowmapAmount + i) + "]", 17 + i);
                deferredShader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].ShadowMapsIDs[" + std::to_string(i) + "]", shadowmapAmount + i);
                deferredShader->SetUniform1f("Lights[" + std::to_string(idx - 1) + "].CascadeDepth[" + std::to_string(i) + "]", light.mCascadeSplitDepth[i]);
                deferredShader->SetUniformMat4f("Lights[" + std::to_string(idx - 1) + "].LightTransforms[" + std::to_string(i) + "]", light.mViewProjections[i]);
            }
            shadowmapAmount += CSM_AMOUNT;
        }

       deferredShader->SetUniform1i("LightCount", (int)idx);
       deferredShader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].Type", light.Type);
       deferredShader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Position", pos.x, pos.y, pos.z);
       deferredShader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Direction", direction.x, direction.y, direction.z);
       deferredShader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Color", light.Color.r * light.Strength, light.Color.g * light.Strength, light.Color.b * light.Strength);
       deferredShader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].Volumetric", light.IsVolumetric);

       m_LightsUniformBuffer->Bind();
       //m_LightsUniformBuffer->UpdateData()
    }

    void Renderer::DrawLine(Vector3 start, Vector3 end, Color color, Matrix4 transform)
    {
        Shader* shader = ShaderManager::GetShader("resources/Shaders/line.shader");
        shader->Bind();
        shader->SetUniformMat4f("u_Model", transform);
        shader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);
        
        std::vector<Vertex> vertices
        {
            {start, Vector2(0, 0), Vector3(-1, 0, 0)},
            {end, Vector2(1, 0), Vector3(-1, -1, 0)}
        };

        VertexArray lineVertexArray = VertexArray();
        lineVertexArray.Bind();
        VertexBuffer lineVertexBuffer = VertexBuffer(&vertices, static_cast<int>(size(vertices)));
    
        VertexBufferLayout vblayout = VertexBufferLayout();
        vblayout.Push<float>(3);
        lineVertexArray.AddBuffer(lineVertexBuffer, vblayout);

        RenderCommand::DrawLines(0, 2);
    }

    void Renderer::DrawDebugLine(Vector3 start, Vector3 end, Vector3 color)
    {
        //m_DebugShader->Bind();
        //m_DebugShader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);
    }

    void Renderer::DrawCube(TransformComponent transform, glm::vec4 color)
    {
        //glDisable(GL_DEPTH_TEST);
        m_DebugShader->SetUniformMat4f("u_Model", transform.GetGlobalTransform());
        m_DebugShader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);

        CubeVertexArray->Bind();
        RenderCommand::DrawArrays(0, 36);
    }


    void Renderer::DrawSphere(TransformComponent transform, glm::vec4 color)
    {

    }

    void Renderer::DrawQuad(Matrix4 transform)
    {
        QuadVertexArray->Bind();
        RenderCommand::DrawArrays(0, 6);
    }
}
