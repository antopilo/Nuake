#include "Bloom.h"
#include <glad/glad.h>
#include "src/Rendering/Renderer.h"
#include <src/Vendors/imgui/imgui.h>
#include "Engine.h"

namespace Nuake
{
	const float DownsamplingScale = 0.4f;
	Bloom::Bloom(unsigned int iteration)
	{
		m_Iteration = iteration;
	}

	void Bloom::SetSource(Ref<Texture> source)
	{
		if (m_Source == source)
			return;

		m_Source = source;
		m_Size = source->GetSize();
		Init();
	}

	void Bloom::Init()
	{
		if (!m_Source)
			return;

		m_FinalFB = CreateRef<FrameBuffer>(false, m_Size);
		m_FinalFB->SetTexture(CreateRef<Texture>(m_Size, GL_RGBA, GL_RGBA16F, GL_FLOAT));

		m_ThresholdFB = CreateRef<FrameBuffer>(false, m_Size);
		m_ThresholdFB->SetTexture(CreateRef<Texture>(m_Size, GL_RGBA, GL_RGBA16F, GL_FLOAT));

		m_DownSampleFB = std::vector<Ref<FrameBuffer>>();
		m_UpSampleFB = std::vector<Ref<FrameBuffer>>();
		m_HBlurFB = std::vector<Ref<FrameBuffer>>();
		m_VBlurFB = std::vector<Ref<FrameBuffer>>();

		Vector2 currentSize = m_Size / 2.0f;
		for (uint32_t i = 0; i < m_Iteration; i++)
		{
			Ref<FrameBuffer> fb = CreateRef<FrameBuffer>(false, currentSize);
			fb->SetTexture(CreateRef<Texture>(currentSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
			m_DownSampleFB.push_back(fb);

			currentSize *= DownsamplingScale;
		}

		for (uint32_t i = 0; i < m_Iteration; i++)
		{
			Ref<FrameBuffer> fb = CreateRef<FrameBuffer>(false, currentSize);
			fb->SetTexture(CreateRef<Texture>(currentSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
			m_UpSampleFB.push_back(fb);

			fb = CreateRef<FrameBuffer>(false, currentSize);
			fb->SetTexture(CreateRef<Texture>(currentSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
			m_HBlurFB.push_back(fb);

			fb = CreateRef<FrameBuffer>(false, currentSize);
			fb->SetTexture(CreateRef<Texture>(currentSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
			m_VBlurFB.push_back(fb);

			currentSize /= DownsamplingScale;
		}
	}

	int downsampleMip = 0;
	int upSampleMip = 0;
	void Bloom::Draw()
	{
		if (!m_Source) return;

		Shader* shader = ShaderManager::GetShader("Resources/Shaders/bloom.shader");

		m_ThresholdFB->Bind();
		{
			m_ThresholdFB->Clear();
			shader->Bind();
			shader->SetUniform1i("u_Stage", 0);
			shader->SetUniform1f("u_Threshold", m_Threshold);
			m_Source->Bind(1);
			shader->SetUniform1i("u_Source", 1);

			Renderer::DrawQuad(Matrix4());
		}

		for (uint32_t i = 0; i < m_Iteration; i++)
		{
			m_DownSampleFB[i]->Bind();
			m_DownSampleFB[i]->Clear();

			shader->SetUniform1i("u_Stage", 1);

			Ref<Texture> downsampleTexture = i == 0 ? m_ThresholdFB->GetTexture() : m_DownSampleFB[i - 1]->GetTexture();
			shader->SetUniformTex("u_Source", downsampleTexture.get(), 1);

			Vector2 size = i == 0 ? downsampleTexture->GetSize() : m_DownSampleFB[i]->GetTexture()->GetSize();
			shader->SetUniformVec2("u_SourceSize", size);

			Renderer::DrawQuad(Matrix4());
			m_DownSampleFB[i]->Unbind();
		}

		for (uint32_t i = 0; i < m_Iteration; i++)
		{
			// Horizontal blur
			m_HBlurFB[i]->Bind();
				m_HBlurFB[i]->Clear();

				shader->SetUniform1i("u_Stage", 2);
				shader->SetUniform2f("u_BlurDirection", 0.0f, 1.0f);

				Ref<Texture> blurTexture = m_DownSampleFB[m_Iteration - i - 1]->GetTexture();
				blurTexture->Bind(1);
				shader->SetUniform1i("u_Source", 1);
				shader->SetUniform2f("u_SourceSize", (float)blurTexture->GetWidth(), (float)blurTexture->GetHeight());

				Renderer::DrawQuad(Matrix4());
			m_HBlurFB[i]->Unbind();

			// Vertical blur
			m_VBlurFB[i]->Bind();
			m_VBlurFB[i]->Clear();

				shader->SetUniform1i("u_Stage", 2);
				shader->SetUniform2f("u_BlurDirection", 1.0f, 0.0f);

				m_HBlurFB[i]->GetTexture()->Bind(1);
				shader->SetUniform1i("u_Source", 1);
				shader->SetUniform2f("u_SourceSize", (float)m_HBlurFB[i]->GetTexture()->GetWidth(), (float)m_HBlurFB[i]->GetTexture()->GetHeight());

				Renderer::DrawQuad(Matrix4());
			m_VBlurFB[i]->Unbind();

			
			// Upsampling
			m_UpSampleFB[i]->Bind();
				m_UpSampleFB[i]->Clear();
				if (i == 0)
				{
					shader->SetUniform1i("u_Stage", 3);
					shader->SetUniform1i("u_Source", 1);
					m_VBlurFB[i]->GetTexture()->Bind(1);
				}
				if (i > 0)
				{
					shader->SetUniform1i("u_Stage", 4);

					m_VBlurFB[i]->GetTexture()->Bind(1);
					shader->SetUniform1i("u_Source", 1);

					m_UpSampleFB[i - 1]->GetTexture()->Bind(2);
					shader->SetUniform1i("u_Source2", 2);
					shader->SetUniform2f("u_Source2Size", (float)m_UpSampleFB[i]->GetTexture()->GetWidth(), (float)m_UpSampleFB[i]->GetTexture()->GetHeight());
				}
				Renderer::DrawQuad(Matrix4());
			m_UpSampleFB[i]->Unbind();
		}
		m_FinalFB->Bind();
		m_FinalFB->Clear();
		{
			shader->SetUniform1i("u_Stage", 5);
			shader->SetUniform1i("u_Source", 1);
			shader->SetUniform1i("u_Source2", 2);
			m_UpSampleFB[m_Iteration - 1]->GetTexture()->Bind(1);
			m_Source->Bind(2);
			Renderer::DrawQuad(Matrix4());
		}
		
		m_FinalFB->Unbind();
	}

	void Bloom::Resize(Vector2 size)
	{
		if (m_Size == size)
			return;
		m_Size = size;
		Init();
	}
}