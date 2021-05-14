#include "Skybox.h"
#include <GL\glew.h>
#include "Renderer.h"
#include "Textures/Cubemap.h"
glm::vec3 Skybox::vertices[36] {
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

Skybox::Skybox(){
    // Setup buffers
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    // TODO: mem leak.
    m_Texture = new CubemapTexture("Res/Textures/Skyboxes/1/japan");
    m_Hdr = new HDRTexture("Res/Textures/Skyboxes/HDR/OldIndustrialHall.jpg");
}

void Skybox::Draw(glm::mat4 projection, glm::mat4 view) {
    
    glDepthMask(GL_FALSE);
    m_Texture->Bind(3);
    m_Hdr->Bind(4);
    Renderer::m_SkyboxShader->Bind();
    m_Hdr->BindCubemap(5);
    Renderer::m_SkyboxShader->SetUniformMat4f("projection", projection);
    Renderer::m_SkyboxShader->SetUniformMat4f("view", view);

    Renderer::m_SkyboxShader->SetUniform1i("isHDR", 0);
    Renderer::m_SkyboxShader->SetUniform1i("equirectangularMap", 4);
    Renderer::m_SkyboxShader->SetUniform1i("skybox", 5);
    
    // ... set view and projection matrix
    glBindVertexArray(VAO);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    //Renderer::m_Shader->SetUniform4f("u_AmbientColor", 1.0f, 1.0f, 1.0f, 1.0f);
}


void Skybox::Push() {
    m_Hdr->BindCubemap(5);
    Renderer::m_Shader->SetUniform1i("u_IrradianceMap", 5);
}


void Skybox::CreateHDRCubemap() {
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
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
    m_Hdr->Bind(5);
    
    // convert HDR equirectangular environment map to cubemap equivalent
    Renderer::m_SkyboxShader->Bind();
    Renderer::m_SkyboxShader->SetUniformMat4f("projection", captureProjection);
    Renderer::m_SkyboxShader->SetUniform1i("convulate", 0);
    Renderer::m_SkyboxShader->SetUniform1i("isHDR", 1);
    Renderer::m_SkyboxShader->SetUniform1i("equirectangularMap", 5);
    glActiveTexture(GL_TEXTURE0);
    
    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        Renderer::m_SkyboxShader->SetUniformMat4f("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    m_Hdr->SetCubemapId(envCubemap);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

