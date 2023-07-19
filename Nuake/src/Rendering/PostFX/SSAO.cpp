#include "SSAO.h"
#include "src/Core/Core.h"
#include "src/Rendering/Textures/Texture.h"
#include <src/Rendering/Shaders/Shader.h>
#include "src/Rendering/Shaders/ShaderManager.h"
#include "src/Rendering/Renderer.h"

#include <GL\glew.h>

#include <random>
#include <src/Vendors/imgui/imgui.h>

namespace Nuake 
{
	SSAO::SSAO()
	{
		GenerateKernel();
		GenerateNoise();

		_size = { 1280, 720 };
		
		_ssaoFramebuffer = CreateRef<FrameBuffer>(false, Vector2(_size));

		auto& renderTarget = CreateRef<Texture>(_size, GL_RGBA, GL_RGBA16F, GL_FLOAT);
		_ssaoFramebuffer->SetTexture(renderTarget, GL_COLOR_ATTACHMENT0);

		_ssaoBlurFramebuffer = CreateRef<FrameBuffer>(false, Vector2(_size));

		auto& renderTargetBlur = CreateRef<Texture>(_size, GL_RGBA, GL_RGBA16F, GL_FLOAT);
		_ssaoBlurFramebuffer->SetTexture(renderTargetBlur);
	}

	void SSAO::Resize(const Vector2& size)
	{
		if (_size == size)
			return;

		_size = size;
		_ssaoFramebuffer->QueueResize(size);
		_ssaoBlurFramebuffer->QueueResize(size);
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

		_ssaoNoiseTexture = CreateRef<Texture>(Vector2(4, 4), GL_RGB, GL_RGBA16F, GL_FLOAT, &_ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void SSAO::Draw(FrameBuffer* gBuffer, const Matrix4& projection, const Matrix4& view)
	{
		_ssaoFramebuffer->Bind();
		{
			_ssaoFramebuffer->Clear();
			const auto& depthTexture = gBuffer->GetTexture(GL_DEPTH_ATTACHMENT);
			const auto& normalTexture = gBuffer->GetTexture(GL_COLOR_ATTACHMENT1);
			Shader* shader = ShaderManager::GetShader("resources/Shaders/ssao.shader");
			shader->Bind();
			shader->SetUniformMat4f("u_Projection", projection);
			shader->SetUniformMat4f("u_View", view);
			shader->SetUniformTex("u_Depth", depthTexture.get(), 2);
			shader->SetUniformTex("u_Normal", normalTexture.get(), 3);
			shader->SetUniformTex("u_Noise", _ssaoNoiseTexture.get(), 6);
			shader->SetUniform1i("u_KernelSize", 64);
			shader->SetUniform1f("u_Radius", Radius);
			shader->SetUniform1f("u_Bias", Bias);
			shader->SetUniform1f("u_Falloff", Falloff);
			shader->SetUniform1f("u_Area", Area);
			shader->SetUniform1f("u_Strength", Strength);
			shader->SetUniformVec2("u_NoiseScale", Vector2(_size.x / 4, _size.y / 4) );

			int i = 0;
			for (const auto& k : _ssaoKernel)
			{
				const std::string& uniformName = "u_Samples[" + std::to_string(i) + "]";
				shader->SetUniform3f(uniformName, k.x, k.y, k.z);
				i++;
			}

			//shader->SetUniform1fv("u_Samples", 64 * 3, (float*)&(_ssaoKernel.begin()));
			Renderer::DrawQuad(Matrix4(1.0));
		}
		_ssaoFramebuffer->Unbind();

		_ssaoBlurFramebuffer->Bind();
		{
			Shader* shader = ShaderManager::GetShader("resources/Shaders/blur.shader");
			shader->Bind();

			shader->SetUniformTex("u_Input", _ssaoFramebuffer->GetTexture().get(), 2);

			Renderer::DrawQuad(Matrix4(1.0));
		}
		_ssaoBlurFramebuffer->Unbind();

		/*
		if (ImGui::Begin("SSAO debug"))
		{
			ImGui::Image((void*)(uintptr_t)(_ssaoFramebuffer->GetTexture(GL_COLOR_ATTACHMENT0)->GetID()), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
		*/
	}

	Ref<FrameBuffer> SSAO::GetOuput() const
	{
		return _ssaoBlurFramebuffer;
	}
}