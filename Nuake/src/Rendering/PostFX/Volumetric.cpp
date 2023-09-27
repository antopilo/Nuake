#include "Volumetric.h"
#include "src/Rendering/Shaders/ShaderManager.h"
#include "src/Rendering/Renderer.h"
#include <dependencies/GLEW/include/GL/glew.h>
#include <src/Vendors/imgui/imgui.h>
namespace Nuake {
	Volumetric::Volumetric()
	{
		mSize = Vector2(1920, 1080);
		Init();
	}

	void Volumetric::SetDepth(Texture* depth)
	{
		Resize(depth->GetSize());
		mDepth = depth;
	}

	void Volumetric::Init()
	{
		mFinalFramebuffer = CreateScope<FrameBuffer>(true, mSize);
		mFinalFramebuffer->SetTexture(CreateRef<Texture>(mSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));

		mVolumetricFramebuffer = CreateScope<FrameBuffer>(true, mSize);
		mVolumetricFramebuffer->SetTexture(CreateRef<Texture>(mSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
	}

	void Volumetric::Resize(Vector2 size)
	{
		if (mSize == size)
			return;

		mSize = size * mRenderRatio;
		mVolumetricFramebuffer->QueueResize(mSize);
		mVolumetricFramebuffer->QueueResize(mSize);
	}

	void Volumetric::Draw(Matrix4 projection, Matrix4 view, const Vector3& camPos, std::vector<LightComponent>& lights)
	{
		mVolumetricFramebuffer->Bind();
		{
			mVolumetricFramebuffer->Clear();
			RenderCommand::Disable(RendererEnum::FACE_CULL);

			auto cameraPosition = Vector3(view[3]);
			Shader* volumetricShader = ShaderManager::GetShader("Resources/Shaders/volumetric.shader");
			volumetricShader->Bind();
			volumetricShader->SetUniformMat4f("u_Projection", projection);
			volumetricShader->SetUniformMat4f("u_View", view);
			volumetricShader->SetUniformTex("u_Depth", mDepth, 1);
			volumetricShader->SetUniformVec3("u_CamPosition", camPos);
			volumetricShader->SetUniform1i("u_StepCount", mStepCount);
			volumetricShader->SetUniform1f("u_FogAmount", mFogAmount);
			volumetricShader->SetUniform1i("u_LightCount", static_cast<int>(lights.size()));

			for (uint16_t i = 0; i < lights.size(); i++)
			{
				LightComponent& light = lights[i];

				std::string u_light = "u_Lights[" + std::to_string(i) + "].";
				volumetricShader->SetUniformMat4f(u_light + "transform", light.mViewProjections[0]);
				volumetricShader->SetUniformVec3(u_light + "color", light.Color);
				volumetricShader->SetUniformVec3(u_light + "direction", light.GetDirection());

				volumetricShader->SetUniformTex(u_light + "shadowmap", light.m_Framebuffers[0]->GetTexture(GL_DEPTH_ATTACHMENT).get(), 5 + i);
				volumetricShader->SetUniform1f(u_light + "strength", light.Strength);
			}

			Renderer::DrawQuad();
		}
		mVolumetricFramebuffer->Unbind();

		// Blur
		mFinalFramebuffer->Bind();
		{
			mFinalFramebuffer->Clear();
			Shader* blurShader = ShaderManager::GetShader("Resources/Shaders/blur.shader");
			blurShader->Bind();
			//blurShader->SetUniformTex("u_Depth", mDepth, 1);
			blurShader->SetUniformTex("u_Input", mVolumetricFramebuffer->GetTexture().get());

			Renderer::DrawQuad();
		}
		mFinalFramebuffer->Unbind();


		//if (ImGui::Begin("Volumetric debug"))
		//{
		//	ImGui::Image((void*)(uintptr_t)(mFinalFramebuffer->GetTexture()->GetID()), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		//}
		//ImGui::End();

		/*
		if (ImGui::Begin("Volumetric debug depth"))
		{
			ImGui::Image((void*)(uintptr_t)(mDepth->GetID()), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		}
		
		ImGui::End();*/
	}
}