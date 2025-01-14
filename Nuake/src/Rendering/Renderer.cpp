#include "Renderer.h"

#include <glad/glad.h>

#include "RenderCommand.h"

#include "src/Rendering/Camera.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Textures/Cubemap.h"
#include "src/Rendering/Shaders/ShaderManager.h"
#include "Engine.h"
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include <glm/gtc/type_ptr.hpp>
#include "Buffers/VertexBufferLayout.h"
#include "src/Rendering/Textures/MaterialManager.h"
#include "src/Rendering/Vertex.h"
#include <imgui/imgui.h>
#include <Tracy.hpp>
#include <vector>
namespace Nuake
{
    uint32_t Renderer::MAX_LIGHT = 42;

    unsigned int depthTexture;
    unsigned int depthFBO;

    Ref<Mesh> Renderer::CubeMesh;
    Ref<Mesh> Renderer::QuadMesh;
    Ref<Mesh> Renderer::SphereMesh;

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
        { Vector3(-1.0f,  1.0f, -1.0f), 0.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(-1.0f, -1.0f, -1.0f), 1.0f, Vector3(-1,-1, 0), 0.0f },
        { Vector3(1.0f, -1.0f, -1.0f),  0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f, -1.0f, -1.0f),  1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f,  1.0f, -1.0f),  0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f,  1.0f, -1.0f), 1.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(-1.0f, -1.0f,  1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f, -1.0f, -1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f,  1.0f, -1.0f), 0.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(-1.0f,  1.0f, -1.0f), 1.0f, Vector3(-1,-1, 0), 0.0f },
        { Vector3(-1.0f,  1.0f,  1.0f), 0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f, -1.0f,  1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f, -1.0f, -1.0f),  0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f, -1.0f,  1.0f),  1.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(1.0f,  1.0f,  1.0f),  1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f,  1.0f,  1.0f),  1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f,  1.0f, -1.0f),  0.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(1.0f, -1.0f, -1.0f),  1.0f, Vector3(-1,-1, 0), 0.0f },
        { Vector3(-1.0f, -1.0f,  1.0f), 0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f,  1.0f,  1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f,  1.0f,  1.0f),  0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f,  1.0f,  1.0f),  1.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(1.0f, -1.0f,  1.0f),  1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f, -1.0f,  1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f,  1.0f, -1.0f), 0.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(1.0f,  1.0f, -1.0f),  1.0f, Vector3(-1,-1, 0), 0.0f },
        { Vector3(1.0f,  1.0f,  1.0f),  0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f,  1.0f,  1.0f),  1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f,  1.0f,  1.0f), 0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f,  1.0f, -1.0f), 1.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(-1.0f, -1.0f, -1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(-1.0f, -1.0f,  1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f, -1.0f, -1.0f),  0.0f, Vector3(-1, 0, 0), 0.0f },
        { Vector3(1.0f, -1.0f, -1.0f),  1.0f, Vector3(-1,-1, 0), 0.0f },
        { Vector3(-1.0f, -1.0f,  1.0f), 0.0f, Vector3(-1, 0, 0), 1.0f },
        { Vector3(1.0f, -1.0f,  1.0f),  1.0f, Vector3(-1, 0, 0), 1.0f }
    };

    std::vector<uint32_t> CubeIndices;

    std::vector<Vertex> QuadVertices
    {
        { Vector3(-1.0f,  1.0f, 0.0f), 0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
        { Vector3(1.0f,  1.0f, 0.0f),  1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
        { Vector3(-1.0f, -1.0f, 0.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
        { Vector3(1.0f,  -1.0f, 0.0f), 1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
        { Vector3(-1.0f, -1.0f, 0.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
        { Vector3(1.0f,   1.0f, 0.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) }
    };


    void Renderer::Init()
    {
        RenderCommand::SetRendererAPI(RendererPlatforms::Vulkan);

        //ShaderManager::LoadShaders();

        //m_LightsUniformBuffer = CreateRef<UniformBuffer>(128);
        //
        //Ref<Material> defaultMaterial = CreateRef<Material>(Vector3{1, 1, 1});
        //defaultMaterial->SetName("white");
        //MaterialManager::Get()->RegisterMaterial(defaultMaterial);
        //
        //CubeIndices.reserve(36);
        //for (int i = 0; i < 36; i++)
        //{
        //    CubeIndices.push_back(i);
        //}
        //
        //CubeMesh = CreateRef<Mesh>();
        //CubeMesh->AddSurface(CubeVertices, CubeIndices);
        //CubeMesh->SetMaterial(defaultMaterial);
        //
        //QuadMesh = CreateRef<Mesh>();
        //QuadMesh->AddSurface(QuadVertices, { 0, 1, 2, 3, 4, 5 });
        //QuadMesh->SetMaterial(defaultMaterial);
        //
        //SphereMesh = CreateSphereMesh();
    }

    void Renderer::LoadShaders()
    {
       
    }

    void Renderer::SubmitMesh(Ref<Mesh> mesh, const Matrix4& transform, const int32_t entityId, const Matrix4& previousTransform)
    {
        m_RenderList.AddToRenderList(mesh, transform, entityId, previousTransform);
    }

    void Renderer::SubmitCube(Matrix4 transform)
    {
        m_RenderList.AddToRenderList(CubeMesh, transform, -1);
    }

    void Renderer::Flush(Shader* shader, bool depthOnly)
    {
        m_RenderList.Flush(shader, depthOnly);
    }

    Vector3 ComputeFaceNormal(Vector3 a, Vector3 b, Vector3 c)
    {
        const float EPSILON = 0.000001f;

        Vector3 normal;     // default return value (0,0,0)
        float nx, ny, nz;

        // find 2 edge vectors: v1-v2, v1-v3
        float ex1 = b.x - a.x;
        float ey1 = b.y - a.y;
        float ez1 = b.z - a.z;
        float ex2 = c.x - a.x;
        float ey2 = c.y - a.y;
        float ez2 = c.z - a.z;

        // cross product: e1 x e2
        nx = ez1 * ey2 - ey1 * ez2;
        ny = ex1 * ez2 - ez1 * ex2;
        nz = ey1 * ex2 - ex1 * ey2;

        // normalize only if the length is > 0
        float length = sqrtf(nx * nx + ny * ny + nz * nz);
        if (length > EPSILON)
        {
            // normalize
            float lengthInv = 1.0f / length;
            normal.x = nx * lengthInv;
            normal.y = ny * lengthInv;
            normal.z = nz * lengthInv;
        }

        return normal * -1.0f;
    }

    Ref<Mesh> Renderer::CreateSphereMesh()
    {
        const float sectorCount = 36;
        const float stackCount = 36;
        const float radius = 0.5f;
        const float PI = acos(-1.0f);

        // new
        std::vector<Vertex> finalVertices;

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
        float s, t;                                     // texCoord

        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                Vertex newVertex;

                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                newVertex.position = Vector3(x, y, z);

                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                newVertex.normal = Vector3(nx, ny, nz) * -1.0f;
                // vertex position
                
                s = (float)j / sectorCount * 4.f;
                t = (float)i / stackCount * 4.f;
                newVertex.uv_x = t;
				newVertex.uv_y = s;

                finalVertices.push_back(newVertex);
            }
        }

        std::vector<uint32_t> finalIndices;
        unsigned int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding 1st and last stacks
                if (i != 0)
                {
                    finalIndices.push_back(k1);
                    finalIndices.push_back(k2);
                    finalIndices.push_back(k1 + 1);
                }

                if (i != (stackCount - 1))
                {
                    finalIndices.push_back(k1 + 1);
                    finalIndices.push_back(k2);
                    finalIndices.push_back(k2 + 1);
                }
            }
        }

        Ref<Mesh> sphereMesh = CreateRef<Mesh>();
        sphereMesh->SetMaterial(CreateRef<Material>());
        sphereMesh->AddSurface(std::move(finalVertices), std::move(finalIndices));
        return sphereMesh;
    }

    void Renderer::BeginDraw(Ref<Camera> camera)
    {
        Shader* lineShader = ShaderManager::GetShader("Resources/Shaders/line.shader");
        lineShader->Bind();
        lineShader->SetUniform("u_Projection", camera->GetPerspective());
        lineShader->SetUniform("u_View", camera->GetTransform());

        m_Shader->Bind();
        m_Shader->SetUniform("u_Projection", camera->GetPerspective());
        m_Shader->SetUniform("u_View", camera->GetTransform());
        m_Shader->SetUniform("u_EyePosition", camera->GetTranslation().x, camera->GetTranslation().y, camera->GetTranslation().z);
    }

    int spotShadowMapCount = 0;
    void Renderer::EndDraw()
    {
        ZoneScoped;

        Shader* deferredShader = ShaderManager::GetShader("Resources/Shaders/deferred.shader");
        deferredShader->Bind();
        deferredShader->SetUniform("LightCount", 0);

        for (int i = 0; i < m_Lights.size(); i++)
        {
            const std::string uniformAccessor = "Lights[" + std::to_string(i) + "].";
            deferredShader->SetUniform(uniformAccessor + "Position", 0, 0, 0);
            deferredShader->SetUniform(uniformAccessor + "Color", 0, 0, 0);
            deferredShader->SetUniform(uniformAccessor + "Type", -1);
            deferredShader->SetUniform(uniformAccessor + "CastShadow", 0);
            deferredShader->SetUniform(uniformAccessor + "ShadowMapID", -1);
        }

        for (int i = 0; i < 8; i++)
        {
            deferredShader->SetUniform("SpotShadowMaps[" + std::to_string(i) + "]", 0);
        }

        m_Lights.clear();
        spotShadowMapCount = 0;
    }

    // List of all lights queued to be used for rendering this frame.
    std::vector<Light> Renderer::m_Lights;

    void Renderer::RegisterDeferredLight(TransformComponent transform, LightComponent light)
    {
        Shader* deferredShader = ShaderManager::GetShader("Resources/Shaders/deferred.shader");
        deferredShader->Bind();
        
        Vector3 direction = light.GetDirection();
        Vector3 pos = transform.GetGlobalTransform()[3];
        Quat lightRotation = transform.GetGlobalRotation();

        const int MaxSpotShadowMap = 8;
        if (light.Type == Directional)
        {
            int shadowmapAmount = 0;

            deferredShader->SetUniform("u_DirectionalLight.Shadow", light.CastShadows);

            if (light.CastShadows)
            {
                for (int i = 0; i < CSM_AMOUNT; i++)
                {
                    //light.m_Framebuffers[i]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(17 + i);
                    const int shadowMapId = shadowmapAmount + i;
                    deferredShader->SetUniform("ShadowMaps[" + std::to_string(shadowMapId) + "]", 17 + i);
                    deferredShader->SetUniform("u_DirectionalLight.CascadeDepth[" + std::to_string(i) + "]", light.mCascadeSplitDepth[i]);
                    deferredShader->SetUniform("u_DirectionalLight.LightTransforms[" + std::to_string(i) + "]", light.mViewProjections[i]);
                }
            }

            deferredShader->SetUniform("u_DirectionalLight.Direction", direction.x, direction.y, direction.z);
            deferredShader->SetUniform("u_DirectionalLight.Color", light.Color.r * light.Strength, light.Color.g * light.Strength, light.Color.b * light.Strength);

            shadowmapAmount += CSM_AMOUNT;
        }
        else
        {
            if (m_Lights.size() == MAX_LIGHT)
            {
                return;
            }

            m_Lights.push_back({ transform , light });
            size_t idx = m_Lights.size();

            const std::string uniformAccessor = "Lights[" + std::to_string(idx - 1) + "].";
            deferredShader->SetUniform(uniformAccessor + "Position", pos.x, pos.y, pos.z);
            deferredShader->SetUniform(uniformAccessor + "Color", light.Color.r * light.Strength, light.Color.g * light.Strength, light.Color.b * light.Strength);
            deferredShader->SetUniform(uniformAccessor + "Type", static_cast<int>(light.Type));
            deferredShader->SetUniform(uniformAccessor + "CastShadow", static_cast<int>(light.CastShadows));

            if (light.Type == Spot)
            {
                direction = transform.GetGlobalRotation() * Vector3(0, 0, -1);
                deferredShader->SetUniform(uniformAccessor + "Direction", direction.x, direction.y, direction.z);
                deferredShader->SetUniform(uniformAccessor + "OuterAngle", glm::cos(Rad(light.OuterCutoff)));
                deferredShader->SetUniform(uniformAccessor + "InnerAngle", glm::cos(Rad(light.Cutoff)));

                if (light.CastShadows && spotShadowMapCount < MaxSpotShadowMap)
                {
                    int shadowMapTextureSlot = 21 + spotShadowMapCount;
                    deferredShader->SetUniform(uniformAccessor + "ShadowMapID", spotShadowMapCount);
                    deferredShader->SetUniform(uniformAccessor + "Transform", light.GetProjection() * glm::inverse(transform.GetGlobalTransform()));

                    //light.m_Framebuffers[0]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(shadowMapTextureSlot);
                    deferredShader->SetUniform("SpotShadowMaps[" + std::to_string(spotShadowMapCount) + "]", shadowMapTextureSlot);
                    spotShadowMapCount++;
                }
            }
            else
            {
                deferredShader->SetUniform(uniformAccessor + "Direction", 0, 0, 0);
                deferredShader->SetUniform(uniformAccessor + "OuterAngle", glm::cos(Rad(light.OuterCutoff)));
                deferredShader->SetUniform(uniformAccessor + "InnerAngle", glm::cos(Rad(light.Cutoff)));
                deferredShader->SetUniform(uniformAccessor + "ShadowMapID", -1);
            }

            deferredShader->SetUniform("LightCount", static_cast<int>(idx));
        }
        
        m_LightsUniformBuffer->Bind();
    }

    void Renderer::DrawLine(Vector3 start, Vector3 end, Color color, Matrix4 transform)
    {
        Shader* shader = ShaderManager::GetShader("Resources/Shaders/line.shader");
        shader->Bind();
        shader->SetUniform("u_Model", transform);
        shader->SetUniform("u_Color", color.r, color.g, color.b, color.a);
        
        std::vector<Vertex> vertices
        {
            {start, 0.0f, Vector3(-1, 0, 0), 1.0f},
            {end, 1.0f, Vector3(-1, -1, 0), 0.0f}
        };

        VertexArray lineVertexArray = VertexArray();
        lineVertexArray.Bind();
        VertexBuffer lineVertexBuffer = VertexBuffer(&vertices, static_cast<int>(size(vertices)));
    
        VertexBufferLayout vblayout = VertexBufferLayout();
        vblayout.Push<float>(3);
        lineVertexArray.AddBuffer(lineVertexBuffer, vblayout);

        //RenderCommand::DrawLines(0, 2);
    }

    void Renderer::DrawLine(Vector3 start, Vector3 end, Vector3 color)
    {

        //m_DebugShader->Bind();
        //m_DebugShader->SetUniform("u_Color", color.r, color.g, color.b, color.a);
    }

    void Renderer::DrawCube(Matrix4 transform)
    {
        ZoneScoped;
        //CubeMesh->Bind();
        //RenderCommand::DrawArrays(0, 36);
    }


    void Renderer::DrawSphere(TransformComponent transform, glm::vec4 color)
    {

    }

    void Renderer::DrawQuad(Matrix4 transform)
    {
		ZoneScoped;

        //QuadMesh->Bind();
        //RenderCommand::DrawArrays(0, 6);
    }
}
