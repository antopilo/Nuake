#include "SSAO.h"
#include "src/Core/Core.h"
#include "src/Rendering/Textures/Texture.h"
#include <src/Rendering/Shaders/Shader.h>
#include "src/Rendering/Shaders/ShaderManager.h"
#include "src/Rendering/Renderer.h"

#include <glad/glad.h>

#include <random>
#include <src/Vendors/imgui/imgui.h>
#include <Tracy.hpp>

namespace Nuake 
{
	SSAO::SSAO()
	{
		GenerateKernel();
		GenerateNoise();

		_size = { 1280, 720 };
		
		_ssaoFramebuffer = CreateRef<FrameBuffer>(false, Vector2(_size));

		auto renderTarget = CreateRef<Texture>(_size, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
		_ssaoFramebuffer->SetTexture(renderTarget, GL_COLOR_ATTACHMENT0);

		_ssaoBlurFramebuffer = CreateRef<FrameBuffer>(false, Vector2(_size));

		auto renderTargetBlur = CreateRef<Texture>(_size, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
		_ssaoBlurFramebuffer->SetTexture(renderTargetBlur);
	}

	void SSAO::Resize(const Vector2& size)
	{
		ZoneScoped;

		if (_size == size)
			return;

		_size = size;
		_ssaoFramebuffer->QueueResize(size);
		_ssaoBlurFramebuffer->QueueResize(size);
	}

	void SSAO::Clear()
	{
		ZoneScoped;

		_ssaoBlurFramebuffer->Bind();
		{
			RenderCommand::SetClearColor({ 1, 1, 1, 1});
			_ssaoBlurFramebuffer->Clear();
		}
		_ssaoBlurFramebuffer->Unbind();
	}

	void SSAO::GenerateKernel()
	{
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		std::vector<Vector3> ssaoKernel;
		for (unsigned int i = 0; i < 64; ++i)
		{
			Vector3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);

			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = (float)i / 64.0f;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			_ssaoKernel.push_back(sample);
		}
	}

	void SSAO::GenerateNoise()
	{
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
		std::default_random_engine generator;

		for (unsigned int i = 0; i < 16; i++)
		{
			Vector3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			_ssaoNoise.push_back(noise);
		}

		_ssaoNoiseTexture = CreateRef<Texture>(Vector2(4, 4), GL_RGB, GL_RGB16F, GL_FLOAT, &_ssaoNoise[0]);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void SSAO::Draw(FrameBuffer* gBuffer, const Matrix4& projection, const Matrix4& view)
	{
		ZoneScoped;

		_ssaoFramebuffer->Bind();
		{
			_ssaoFramebuffer->Clear();
			auto depthTexture = gBuffer->GetTexture(GL_DEPTH_ATTACHMENT);
			auto normalTexture = gBuffer->GetTexture(GL_COLOR_ATTACHMENT1);

			ImGui::Begin("debug");
			//ImGui::Image((ImTextureID)normalTexture->GetID(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 });
			ImGui::End();

			Shader* shader = ShaderManager::GetShader("Resources/Shaders/ssao.shader");
			shader->Bind();
			shader->SetUniform("u_Projection", projection);
			shader->SetUniform("u_View", view);
			shader->SetUniform("u_Depth", depthTexture.get(), 2);
			shader->SetUniform("u_Normal", normalTexture.get(), 3);
			shader->SetUniform("u_Noise", _ssaoNoiseTexture.get(), 6);
			shader->SetUniform("u_Radius", Radius);
			shader->SetUniform("u_Bias", Bias);
			shader->SetUniform("u_Strength", Strength);
			shader->SetUniform("u_NoiseScale", Vector2(_size.x / 4, _size.y / 4) );

			int i = 0;
			for (const auto& k : _ssaoKernel)
			{
				const std::string& uniformName = "u_Samples[" + std::to_string(i) + "]";
				shader->SetUniform(uniformName, k.x, k.y, k.z);
				i++;
			}

			//shader->SetUniformv("u_Samples", 64 * 3, (float*)&(_ssaoKernel.begin()));
			Renderer::DrawQuad(Matrix4(1.0));
		}
		_ssaoFramebuffer->Unbind();

		_ssaoBlurFramebuffer->Bind();
		{
			Shader* shader = ShaderManager::GetShader("Resources/Shaders/blur.shader");
			shader->Bind();

			shader->SetUniform("u_Input", _ssaoFramebuffer->GetTexture().get(), 2);

			Renderer::DrawQuad(Matrix4(1.0));
		}
		_ssaoBlurFramebuffer->Unbind();
	}

	Ref<FrameBuffer> SSAO::GetOuput() const
	{
		return _ssaoBlurFramebuffer;
	}
}