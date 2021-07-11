#include "SkyboxHDR.h"
#include <GL\glew.h>
#include "Renderer.h"
#include "Textures/Cubemap.h"

glm::vec3 SkyboxHDR::vertices[36]{
       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f,  -0.5f, -0.5f),
       glm::vec3(0.5f,   0.5f, -0.5f),
       glm::vec3(0.5f,   0.5f, -0.5f),
       glm::vec3(-0.5f,  0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),

       glm::vec3(-0.5f, -0.5f,  0.5f),
       glm::vec3(0.5f,  -0.5f,  0.5f),
       glm::vec3(0.5f,   0.5f,  0.5f),
       glm::vec3(0.5f,   0.5f,  0.5f),
       glm::vec3(-0.5f,  0.5f,  0.5f),
       glm::vec3(-0.5f, -0.5f,  0.5f),

       glm::vec3(-0.5f,  0.5f,  0.5f),
       glm::vec3(-0.5f,  0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f,  0.5f),
       glm::vec3(-0.5f,  0.5f,  0.5f),

       glm::vec3(0.5f,  0.5f,  0.5f),
       glm::vec3(0.5f,  0.5f, -0.5f),
       glm::vec3(0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f, -0.5f,  0.5f),
       glm::vec3(0.5f,  0.5f,  0.5f),

       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f,  -0.5f, -0.5f),
       glm::vec3(0.5f,  -0.5f,  0.5f),
       glm::vec3(0.5f,  -0.5f,  0.5f),
       glm::vec3(-0.5f, -0.5f,  0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),

       glm::vec3(-0.5f, 0.5f, -0.5f) ,
       glm::vec3(0.5f,  0.5f, -0.5f) ,
       glm::vec3(0.5f,  0.5f,  0.5f) ,
       glm::vec3(0.5f,  0.5f,  0.5f) ,
       glm::vec3(-0.5f, 0.5f,  0.5f) ,
       glm::vec3(-0.5f, 0.5f, -0.5f)
       
       
      
};

SkyboxHDR::SkyboxHDR(const std::string path) {
    // Setup buffers
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);


    m_HDRTexture = new HDRTexture(path);
    // Create cubemap.
    CreateHDRCubemap();
    // Create convulated from cubemap.
    CreateConvulatedCubemap();
    // Create specular mip maps.
    CreateSpecularCubemaps();

    CreateBRDLUT();
    // TODO: mem leak.
    //m_Texture = new CubemapTexture("Res/Textures/Skyboxes/1/japan");
    //m_Hdr = new HDRTexture("Res/Textures/Skyboxes/HDR/lilienstein_4k.hdr");
}

void SkyboxHDR::Draw(glm::mat4 projection, glm::mat4 view) {
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Cubemap);

    m_HDRTexture->Bind(6);

    Renderer::m_SkyboxShader->Bind();
    Renderer::m_SkyboxShader->SetUniformMat4f("projection", projection);
    Renderer::m_SkyboxShader->SetUniformMat4f("view", view);
    Renderer::m_SkyboxShader->SetUniform1i("convulate", 0);
    Renderer::m_SkyboxShader->SetUniform1i("prefilter", 0);
    Renderer::m_SkyboxShader->SetUniform1i("isHDR", 0);
    Renderer::m_SkyboxShader->SetUniform1i("equirectangularMap", 6);
    Renderer::m_SkyboxShader->SetUniform1i("skybox", 5);

    // ... set view and projection matrix
    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    //Renderer::m_Shader->SetUniform4f("u_AmbientColor", 1.0f, 1.0f, 1.0f, 1.0f);
}

void SkyboxHDR::Push() {
    //m_Hdr->BindCubemap(5);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ConvulatedCubemap);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SpecularCubemap);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, m_brdLut);
    
    Renderer::m_Shader->SetUniform1i("u_IrradianceMap", 1);
    Renderer::m_Shader->SetUniform1i("prefilterMap", 2);
    Renderer::m_Shader->SetUniform1i("brdfLUT", 3);

    Renderer::m_DeferredShader->SetUniform1i("u_IrradianceMap", 1);
    Renderer::m_DeferredShader->SetUniform1i("u_PrefilterMap", 2);
    Renderer::m_DeferredShader->SetUniform1i("u_BrdfLUT", 3);
}

void SkyboxHDR::CreateHDRCubemap() {
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


    glGenTextures(1, &m_Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Cubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    m_HDRTexture->Bind(5);
    // convert HDR equirectangular environment map to cubemap equivalent
    Renderer::m_SkyboxShader->Bind();
    Renderer::m_SkyboxShader->SetUniformMat4f("projection", captureProjection);
    Renderer::m_SkyboxShader->SetUniform1i("isHDR", 1);
    Renderer::m_SkyboxShader->SetUniform1i("prefilter", 0);
    Renderer::m_SkyboxShader->SetUniform1i("convulate", 0);
    Renderer::m_SkyboxShader->SetUniform1i("equirectangularMap", 5);

    glActiveTexture(GL_TEXTURE0);

    glViewport(0, 0, 1024, 1024); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        Renderer::m_SkyboxShader->SetUniformMat4f("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_Cubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Cubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void SkyboxHDR::CreateConvulatedCubemap() {
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    glGenTextures(1, &m_ConvulatedCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ConvulatedCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
            GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Cubemap);

    Renderer::m_SkyboxShader->Bind();
    Renderer::m_SkyboxShader->SetUniformMat4f("projection", captureProjection);
    Renderer::m_SkyboxShader->SetUniform1i("convulate", 1);
    Renderer::m_SkyboxShader->SetUniform1i("prefilter", 0);
    Renderer::m_SkyboxShader->SetUniform1i("isHDR", 0);
    Renderer::m_SkyboxShader->SetUniform1i("equirectangularMap", 6);
    Renderer::m_SkyboxShader->SetUniform1i("skybox", 5);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        Renderer::m_SkyboxShader->SetUniformMat4f("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_ConvulatedCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SkyboxHDR::CreateSpecularCubemaps() {
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glGenTextures(1, &m_SpecularCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SpecularCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    Renderer::m_SkyboxShader->Bind();
    Renderer::m_SkyboxShader->SetUniformMat4f("projection", captureProjection);
    Renderer::m_SkyboxShader->SetUniform1i("convulate", 0);
    Renderer::m_SkyboxShader->SetUniform1i("prefilter", 1);
    Renderer::m_SkyboxShader->SetUniform1i("isHDR", 0);
    Renderer::m_SkyboxShader->SetUniform1i("equirectangularMap", 6);
    Renderer::m_SkyboxShader->SetUniform1i("skybox", 5);


    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = 1024 * std::pow(0.5, mip);
        unsigned int mipHeight = 1024 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        Renderer::m_SkyboxShader->SetUniform1f("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            Renderer::m_SkyboxShader->SetUniformMat4f("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_SpecularCubemap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


unsigned int quadVAO = 0;
unsigned int quadVBO;

void SkyboxHDR::CreateBRDLUT() {
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


    glGenTextures(1, &m_brdLut);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, m_brdLut);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdLut, 0);

    glViewport(0, 0, 512, 512);
    Renderer::m_BRDShader->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //RenderQuad();
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::vec3 vertices[36]{
       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f,  -0.5f, -0.5f),
       glm::vec3(0.5f,   0.5f, -0.5f),
       glm::vec3(0.5f,   0.5f, -0.5f),
       glm::vec3(-0.5f,  0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),

       glm::vec3(-0.5f, -0.5f,  0.5f),
       glm::vec3(0.5f,  -0.5f,  0.5f),
       glm::vec3(0.5f,   0.5f,  0.5f),
       glm::vec3(0.5f,   0.5f,  0.5f),
       glm::vec3(-0.5f,  0.5f,  0.5f),
       glm::vec3(-0.5f, -0.5f,  0.5f),

       glm::vec3(-0.5f,  0.5f,  0.5f),
       glm::vec3(-0.5f,  0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(-0.5f, -0.5f,  0.5f),
       glm::vec3(-0.5f,  0.5f,  0.5f),

       glm::vec3(0.5f,  0.5f,  0.5f),
       glm::vec3(0.5f,  0.5f, -0.5f),
       glm::vec3(0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f, -0.5f,  0.5f),
       glm::vec3(0.5f,  0.5f,  0.5f),

       glm::vec3(-0.5f, -0.5f, -0.5f),
       glm::vec3(0.5f,  -0.5f, -0.5f),
       glm::vec3(0.5f,  -0.5f,  0.5f),
       glm::vec3(0.5f,  -0.5f,  0.5f),
       glm::vec3(-0.5f, -0.5f,  0.5f),
       glm::vec3(-0.5f, -0.5f, -0.5f),

       glm::vec3(-0.5f, 0.5f, -0.5f) ,
       glm::vec3(0.5f,  0.5f, -0.5f) ,
       glm::vec3(0.5f,  0.5f,  0.5f) ,
       glm::vec3(0.5f,  0.5f,  0.5f) ,
       glm::vec3(-0.5f, 0.5f,  0.5f) ,
       glm::vec3(-0.5f, 0.5f, -0.5f)
};