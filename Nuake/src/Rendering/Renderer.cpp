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

namespace Nuake
{
    unsigned int depthTexture;
    unsigned int depthFBO;

    Ref<Shader> Renderer::m_Shader;
    Ref<Shader> Renderer::m_SkyboxShader;
    Ref<Shader> Renderer::m_BRDShader;
    Ref<Shader> Renderer::m_GBufferShader;
    Ref<Shader> Renderer::m_DeferredShader;
    Ref<Shader> Renderer::m_ProceduralSkyShader;
    Ref<Shader> Renderer::m_DebugShader;
    Ref<Shader> Renderer::m_ShadowmapShader;

    VertexArray* Renderer::QuadVertexArray;
    VertexBuffer* Renderer::QuadVertexBuffer;
    VertexArray*  Renderer::CubeVertexArray;
    VertexBuffer* Renderer::CubeVertexBuffer;

    RenderList Renderer::m_RenderList = RenderList();

    glm::vec3 CubeVertices[36]{
           glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f,  -0.5f, -0.5f), glm::vec3(0.5f,   0.5f, -0.5f),
           glm::vec3(0.5f,   0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
           glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.5f,  -0.5f,  0.5f), glm::vec3(0.5f,   0.5f,  0.5f),
           glm::vec3(0.5f,   0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f),
           glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
           glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f),
           glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec3(0.5f, -0.5f, -0.5f),
           glm::vec3(0.5f, -0.5f, -0.5f),  glm::vec3(0.5f, -0.5f,  0.5f),  glm::vec3(0.5f,  0.5f,  0.5f),
           glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f,  -0.5f, -0.5f), glm::vec3(0.5f,  -0.5f,  0.5f),
           glm::vec3(0.5f,  -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
           glm::vec3(-0.5f, 0.5f, -0.5f),  glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec3(0.5f,  0.5f,  0.5f),
           glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec3(-0.5f, 0.5f,  0.5f),  glm::vec3(-0.5f, 0.5f, -0.5f)
    };

    float QuadVertices[] = {
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
        0.5f,   0.5f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,
        0.5f,  -0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
        0.5f,   0.5f, 0.0f,   1.0f, 1.0f
    };


    void Renderer::Init()
    {
        RenderCommand::SetRendererAPI(RendererPlatforms::OpenGL);

        LoadShaders();

        // Cube buffer
        CubeVertexArray = new VertexArray();
        CubeVertexArray->Bind();
        CubeVertexBuffer = new VertexBuffer(CubeVertices, sizeof(CubeVertices));

        VertexBufferLayout vblayout = VertexBufferLayout();
        vblayout.Push<float>(3);
        CubeVertexArray->AddBuffer(*CubeVertexBuffer, vblayout);

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
        m_ShadowmapShader   = ShaderManager::GetShader("resources/Shaders/shadowMap.shader");
        m_SkyboxShader      = ShaderManager::GetShader("resources/Shaders/skybox.shader");
        m_BRDShader         = ShaderManager::GetShader("resources/Shaders/BRD.shader");
        m_GBufferShader     = ShaderManager::GetShader("resources/Shaders/gbuffer.shader");
        m_DeferredShader    = ShaderManager::GetShader("resources/Shaders/deferred.shader");
        m_ProceduralSkyShader = ShaderManager::GetShader("resources/Shaders/atmospheric_sky.shader");
        m_DebugShader       = ShaderManager::GetShader("resources/Shaders/debug.shader");
        m_Shader            = ShaderManager::GetShader("resources/Shaders/basic.shader");
    }

    void Renderer::SubmitMesh(Ref<Mesh> mesh, Matrix4 transform)
    {
        m_RenderList.AddToRenderList(mesh, transform);
    }

    void Renderer::Flush()
    {
        m_RenderList.Flush();
    }

    void Renderer::BeginDraw(Ref<Camera> camera)
    {
        RenderCommand::Clear();

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

    void Renderer::RegisterLight(TransformComponent transform, LightComponent light, Ref<Camera> cam)
    {
        if (m_Lights.size() == 20)
            return;

        m_Lights.push_back({ transform , light });

        int idx = m_Lights.size();

        Vector3 direction = light.GetDirection();
        Vector3 pos = transform.GlobalTranslation;
        Matrix4 lightView = glm::lookAt(pos, pos - direction, glm::vec3(0.0f, 1.0f, 0.0f));

        //light.m_Framebuffer->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(17);

        if (light.CastShadows)
        {
            light.m_Framebuffers[0]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(17);
            light.m_Framebuffers[1]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(18);
            light.m_Framebuffers[2]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(19);
            light.m_Framebuffers[3]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(20);
        }

        m_Shader->SetUniform1i("LightCount", idx);
        m_Shader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].Type", light.Type);
        m_Shader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].ShadowMaps[0]", 17);
        m_Shader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].ShadowMaps[1]", 18);
        m_Shader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].ShadowMaps[2]", 19);
        m_Shader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].ShadowMaps[3]", 20);
        m_Shader->SetUniformMat4f("Lights[" + std::to_string(idx - 1) + "].LightTransforms[0]", light.mViewProjections[0]);
        m_Shader->SetUniformMat4f("Lights[" + std::to_string(idx - 1) + "].LightTransforms[1]", light.mViewProjections[1]);
        m_Shader->SetUniformMat4f("Lights[" + std::to_string(idx - 1) + "].LightTransforms[2]", light.mViewProjections[2]);
        m_Shader->SetUniformMat4f("Lights[" + std::to_string(idx - 1) + "].LightTransforms[3]", light.mViewProjections[3]);
        m_Shader->SetUniform1f("Lights[" + std::to_string(idx - 1) + "].CascadeDepth[0]", light.mCascadeSplitDepth[0]);
        m_Shader->SetUniform1f("Lights[" + std::to_string(idx - 1) + "].CascadeDepth[1]", light.mCascadeSplitDepth[1]);
        m_Shader->SetUniform1f("Lights[" + std::to_string(idx - 1) + "].CascadeDepth[2]", light.mCascadeSplitDepth[2]);
        m_Shader->SetUniform1f("Lights[" + std::to_string(idx - 1) + "].CascadeDepth[3]", light.mCascadeSplitDepth[3]);
        m_Shader->SetUniformMat4f("Lights[" + std::to_string(idx - 1) + "].LightTransform", light.GetProjection() * lightView);
        m_Shader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Position", transform.GlobalTranslation.x, transform.GlobalTranslation.y, transform.GlobalTranslation.z);
        m_Shader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Direction", direction.x, direction.y, direction.z);
        m_Shader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Color", light.Color.r * light.Strength, light.Color.g * light.Strength, light.Color.b * light.Strength);
        m_Shader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].Volumetric", light.IsVolumetric);
    }

    void Renderer::RegisterDeferredLight(TransformComponent transform, LightComponent light, Camera* cam)
    {
        m_Lights.push_back({ transform , light });

        int idx = m_Lights.size();

        Vector3 direction = light.GetDirection();
        Vector3 pos = transform.Translation;
        Matrix4 lightView = glm::lookAt(pos, pos - direction, Vector3(0.0f, 1.0f, 0.0f));

        light.m_Framebuffer->GetTexture()->Bind(11);

        m_DeferredShader->SetUniform1i("LightCount", idx);
        m_DeferredShader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].Type", light.Type);
        m_DeferredShader->SetUniform1i("Lights[" + std::to_string(idx - 1) + "].ShadowMap", 11);
        m_DeferredShader->SetUniformMat4f("Lights[" + std::to_string(idx - 1) + "].LightTransform", light.GetProjection() * lightView);
        m_DeferredShader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Position", transform.Translation.x, transform.Translation.y, transform.Translation.z);
        m_DeferredShader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Direction", direction.x, direction.y, direction.z);
        m_DeferredShader->SetUniform3f("Lights[" + std::to_string(idx - 1) + "].Color", light.Color.r * light.Strength, light.Color.g * light.Strength, light.Color.b * light.Strength);
    }

    void Renderer::DrawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color) 
    {
        //m_DebugShader->Bind();
        //m_DebugShader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);
    }

    // TODO: Maybe have a debug primitive draw list
    void Renderer::DrawCube(TransformComponent transform, glm::vec4 color)
    {
        //glDisable(GL_DEPTH_TEST);
        m_DebugShader->SetUniformMat4f("u_Model", transform.GetTransform());
        m_DebugShader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);

        CubeVertexArray->Bind();
        RenderCommand::DrawArrays(0, 36);
        //glBindVertexArray(CubeVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        //m_DebugShader->SetUniform4f("u_Color", color.r, color.g, color.b, 1.0f);

        //glPolygonMode(GL_FRONT, GL_LINE);
        //glPolygonMode(GL_BACK, GL_LINE);
        //glLineWidth(4);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //
        //// Turn off wireframe mode
        //glPolygonMode(GL_FRONT, GL_FILL);
        //glPolygonMode(GL_BACK, GL_FILL);
        //glEnable(GL_DEPTH_TEST);

    }

    // TODO: See above
    void Renderer::DrawSphere(TransformComponent transform, glm::vec4 color)
    {

    }

    void Renderer::DrawQuad(Matrix4 transform)
    {
        QuadVertexArray->Bind();
        RenderCommand::DrawArrays(0, 6);
    }
}
