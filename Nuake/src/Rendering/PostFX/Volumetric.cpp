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
		mFinalFramebuffer = CreateScope<FrameBuffer>(false, mSize);
		mFinalFramebuffer->SetTexture(CreateRef<Texture>(mSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
	}

	void Volumetric::Resize(Vector2 size)
	{
		if (mSize == size)
			return;

		mSize = size * mRenderRatio;
		Init();	
	}

	void Volumetric::Draw(Matrix4 projection, Matrix4 view, std::vector<LightComponent>& lights)
	{
		mFinalFramebuffer->Bind();
			mFinalFramebuffer->Clear();
			RenderCommand::Disable(RendererEnum::DEPTH_TEST);
			RenderCommand::Disable(RendererEnum::FACE_CULL);

			Shader* volumetricShader = ShaderManager::GetShader("resources/Shaders/volumetric.shader");
			volumetricShader->Bind();
			volumetricShader->SetUniformMat4f("u_Projection", projection);
			volumetricShader->SetUniformMat4f("u_View", view);
			volumetricShader->SetUniformTex("u_Depth", mDepth, 0);
			volumetricShader->SetUniformVec3("u_CamPosition", view[3]);
			volumetricShader->SetUniform1i("u_StepCount", mStepCount);
			volumetricShader->SetUniform1f("u_FogAmount", mFogAmount);
			volumetricShader->SetUniform1i("u_LightCount", lights.size());

			for (uint16_t i = 0; i < lights.size(); i++)
			{
				LightComponent& light = lights[i];

				std::string u_light = "u_Lights[" + std::to_string(i) + "].";
				volumetricShader->SetUniformMat4f(u_light + "transform", light.mViewProjections[0]);
				volumetricShader->SetUniformVec3(u_light + "color", light.Color);
				volumetricShader->SetUniformVec3(u_light + "direction", light.GetDirection());

				volumetricShader->SetUniformTex(u_light + "shadowmap", light.m_Framebuffers[0]->GetTexture(GL_DEPTH_ATTACHMENT).get(), 1 + i);
			}

			Renderer::DrawQuad();
		mFinalFramebuffer->Unbind();
	}
}