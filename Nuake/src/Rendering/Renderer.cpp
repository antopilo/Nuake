#include "Renderer.h"
#include <GL\glew.h>
#include "Camera.h"
#include "Textures/Texture.h"
#include "Textures/Cubemap.h"
#include "../../Engine.h"
#include <glm/gtc/type_ptr.hpp>

namespace Nuake
{
    unsigned int depthTexture;
    unsigned int depthFBO;

    Shader* Renderer::m_Shader;
    Shader* Renderer::m_SkyboxShader;
    Shader* Renderer::m_BRDShader;
    Shader* Renderer::m_GBufferShader;
    Shader* Renderer::m_DeferredShader;
    Shader* Renderer::m_ProceduralSkyShader;
    Shader* Renderer::m_DebugShader;

    unsigned int CubeVAO;
    unsigned int CubeVBO;
    unsigned int QuadVAO;
    unsigned int QuadVBO;


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
        // TODO: Make shader storage somewhere.
        m_ShadowmapShader = new Shader("resources/Shaders/shadowMap.shader");
        m_SkyboxShader = new Shader("resources/Shaders/skybox.shader");
        m_BRDShader = new Shader("resources/Shaders/BRD.shader");
        m_GBufferShader = new Shader("resources/Shaders/gbuffer.shader");
        m_DeferredShader = new Shader("resources/Shaders/deferred.shader");
        m_ProceduralSkyShader = new Shader("resources/Shaders/atmospheric_sky.shader");
        m_DebugShader = new Shader("resources/Shaders/debug.shader");
        m_Shader = new Shader("resources/Shaders/basic.shader");
        m_Shader->Bind();

        // TODO: Use buffer abstraction.
        // Setup buffers
        glGenVertexArrays(1, &CubeVAO);
        glBindVertexArray(CubeVAO);

        glGenBuffers(1, &CubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);

        glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
        glEnableVertexAttribArray(0);

        // Quad
        glGenVertexArrays(1, &QuadVAO);
        glBindVertexArray(QuadVAO);

        glGenBuffers(1, &QuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);

        glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    }

    // TODO: Move to shader storage
    Shader* Renderer::m_ShadowmapShader;

    void Renderer::BeginDraw(Ref<Camera> camera)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Shader->SetUniformMat4f("u_Projection", camera->GetPerspective());
        m_Shader->SetUniformMat4f("u_View", camera->GetTransform());
        m_Shader->SetUniform3f("u_EyePosition", camera->GetTranslation().x, camera->GetTranslation().y, camera->GetTranslation().z);
        m_Shader->Bind();
    }

    void Renderer::EndDraw()
    {
        m_Lights.clear(); // Clean up lights.
    }


    // List of all lights queued to be used for rendering this frame.
    std::vector<Light> Renderer::m_Lights;

    void Renderer::RegisterLight(TransformComponent transform, LightComponent light, Ref<Camera> cam)
    {
        if (m_Lights.size() == 20)
            return;
        m_Lights.push_back({ transform , light });

        // What light idx is this?
        int idx = m_Lights.size();

        glm::vec3 direction = light.GetDirection();
        glm::vec3 pos = transform.GlobalTranslation;
        glm::mat4 lightView = glm::lookAt(pos, pos - direction, glm::vec3(0.0f, 1.0f, 0.0f));

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

        // What light idx is this?
        int idx = m_Lights.size();

        glm::vec3 direction = light.GetDirection();
        glm::vec3 pos = transform.Translation;
        glm::mat4 lightView = glm::lookAt(pos, pos - direction, glm::vec3(0.0f, 1.0f, 0.0f));

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
        Ref<Window> window = Engine::GetCurrentWindow();
        Ref<FrameBuffer> fb = window->GetFrameBuffer();


        m_DebugShader->SetUniformMat4f("u_Model", transform.GetTransform());
        m_DebugShader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        m_DebugShader->SetUniform4f("u_Color", color.r, color.g, color.b, 1.0f);

        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        glLineWidth(4);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Turn off wireframe mode
        glPolygonMode(GL_FRONT, GL_FILL);
        glPolygonMode(GL_BACK, GL_FILL);
        //glEnable(GL_DEPTH_TEST);

    }

    // TODO: See above
    void Renderer::DrawSphere(TransformComponent transform, glm::vec4 color)
    {

    }

    void Renderer::DrawQuad(Matrix4 transform)
    {
        glBindVertexArray(QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
