#include "Bloom.h"
#include <dependencies/GLEW/include/GL/glew.h>
#include "src/Rendering/Renderer.h"
#include <src/Vendors/imgui/imgui.h>

namespace Nuake
{
	const float DownsamplingScale = 0.4f;
	Bloom::Bloom(Ref<Texture> source, unsigned int iteration)
	{
		m_Iteration = iteration;
		m_Source = source;

		

		m_FinalFB = CreateRef<FrameBuffer>(false, Vector2(1920, 1080));
		m_FinalFB->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_RGBA, GL_RGBA16F, GL_FLOAT));

		m_ThresholdFB = CreateRef<FrameBuffer>(false, Vector2(1920, 1080));
		m_ThresholdFB->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_RGBA, GL_RGBA16F, GL_FLOAT));
		
		m_DownSampleFB = std::vector<Ref<FrameBuffer>>();
		m_UpSampleFB = std::vector<Ref<FrameBuffer>>();
		m_HBlurFB = std::vector<Ref<FrameBuffer>>();
		m_VBlurFB = std::vector<Ref<FrameBuffer>>();

		Vector2 originalTargetSize = Vector2(source->GetWidth(), source->GetHeight());
		Vector2 currentSize = originalTargetSize;
		for (int i = 0; i < m_Iteration; i++)
		{
			Ref<FrameBuffer> fb = CreateRef<FrameBuffer>(false, currentSize);
			fb->SetTexture(CreateRef<Texture>(currentSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
			m_DownSampleFB.push_back(fb);

			currentSize *= DownsamplingScale;
		}

		for (int i = 0; i < m_Iteration; i++)
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
		Ref<Shader> shader = ShaderManager::GetShader("resources/Shaders/bloom.shader");

		// Threshold
		m_ThresholdFB->Bind();
		{
			m_ThresholdFB->Clear();

			shader->Bind();
			shader->SetUniform1i("u_Stage", 0);
			shader->SetUniform1f("u_Threshold", Threshold);
			m_Source->Bind(1);
			shader->SetUniform1i("u_Source", 1);

			Renderer::DrawQuad(Matrix4());
		}

		for (int i = 0; i < m_Iteration; i++)
		{
			m_DownSampleFB[i]->Bind();
			m_DownSampleFB[i]->Clear();

			shader->SetUniform1i("u_Stage", 1);

			Ref<Texture> downsampleTexture = i == 0 ? m_ThresholdFB->GetTexture() : m_DownSampleFB[i - 1]->GetTexture();
			downsampleTexture->Bind(1);
			shader->SetUniform1i("u_Source", 1);

			if(i == 0)
				shader->SetUniform2f("u_SourceSize", downsampleTexture->GetWidth(), downsampleTexture->GetHeight());
			else
				shader->SetUniform2f("u_SourceSize", m_DownSampleFB[i]->GetTexture()->GetWidth(), m_DownSampleFB[i]->GetTexture()->GetHeight());
			Renderer::DrawQuad(Matrix4());
			m_DownSampleFB[i]->Unbind();
		}

		if (ImGui::Begin("Downsample"))
		{
			ImGui::DragInt("MIP", &downsampleMip, 1.0f, 0, m_Iteration - 1);
			ImGui::Image((void*)m_DownSampleFB[downsampleMip]->GetTexture()->GetID(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();


		for (int i = 0; i < m_Iteration; i++)
		{
			// Horizontal blur
			m_HBlurFB[i]->Bind();
				m_HBlurFB[i]->Clear();

				shader->SetUniform1i("u_Stage", 2);
				shader->SetUniform2f("u_BlurDirection", 0.0f, 1.0f);

				Ref<Texture> blurTexture = m_DownSampleFB[m_Iteration - i - 1]->GetTexture();
				blurTexture->Bind(1);
				shader->SetUniform1i("u_Source", 1);
				shader->SetUniform2f("u_SourceSize", blurTexture->GetWidth(), blurTexture->GetHeight());

				Renderer::DrawQuad(Matrix4());
			m_HBlurFB[i]->Unbind();

			// Vertical blur
			m_VBlurFB[i]->Bind();
			m_VBlurFB[i]->Clear();

				shader->SetUniform1i("u_Stage", 2);
				shader->SetUniform2f("u_BlurDirection", 1.0f, 0.0f);

				m_HBlurFB[i]->GetTexture()->Bind(1);
				shader->SetUniform1i("u_Source", 1);
				shader->SetUniform2f("u_SourceSize", m_HBlurFB[i]->GetTexture()->GetWidth(), m_HBlurFB[i]->GetTexture()->GetHeight());

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
					shader->SetUniform2f("u_Source2Size", m_UpSampleFB[i]->GetTexture()->GetWidth(), m_UpSampleFB[i]->GetTexture()->GetHeight());
				}
				Renderer::DrawQuad(Matrix4());
			m_UpSampleFB[i]->Unbind();
		}

		if (ImGui::Begin("UpSample"))
		{
			ImGui::DragInt("MIP", &upSampleMip, 1.0f, 0, m_Iteration - 1);
			ImGui::Image((void*)m_UpSampleFB[upSampleMip]->GetTexture()->GetID(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();

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
		if (ImGui::Begin("Final"))
		{
			ImGui::Image((void*)m_FinalFB->GetTexture()->GetID(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}
}