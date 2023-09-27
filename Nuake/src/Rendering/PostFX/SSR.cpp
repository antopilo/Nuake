#include "SSR.h"
#include <glad/glad.h>
#include "src/Rendering/RenderCommand.h"
#include "src/Rendering/Renderer.h"
#include <src/Vendors/imgui/imgui.h>

namespace Nuake {
	SSR::SSR()
	{
		mSize = Vector2(1280, 720);
		mShader = ShaderManager::GetShader("Resources/Shaders/ssr.shader");
	}

	void SSR::Init()
	{
		OutputFramebuffer = CreateRef<FrameBuffer>(false, mSize);
		OutputFramebuffer->SetTexture(CreateRef<Texture>(mSize, GL_RGBA, GL_RGBA16F, GL_FLOAT));
	}

	void SSR::Resize(Vector2 size)
	{
		Vector2 newSize = size * 0.5f;
		if (mSize == newSize)
			return;

		mSize = newSize;
		Init();
	}

	void SSR::Draw(FrameBuffer* gBuffer, Ref<Texture> previousFrame, Matrix4 view, Matrix4 projection, Ref<Camera> cam)
	{
		OutputFramebuffer->Bind();
		{
			RenderCommand::Clear();

			mShader->Bind();

			mShader->SetUniform1f("rayStep", RayStep);
			mShader->SetUniform1i("iterationCount", IterationCount);
			mShader->SetUniform1f("distanceBias", DistanceBias);
			mShader->SetUniform1i("enableSSR", (int)1);
			mShader->SetUniform1i("sampleCount", SampleCount);
			mShader->SetUniform1i("isSamplingEnabled", SamplingEnabled);
			mShader->SetUniform1i("isExponentialStepEnabled", ExpoStep);
			mShader->SetUniform1i("isAdaptiveStepEnabled", AdaptiveStep);
			mShader->SetUniform1i("isBinarySearchEnabled", BinarySearch);
			mShader->SetUniform1i("debugDraw", DebugDraw);
			mShader->SetUniform1f("samplingCoefficient", SampleingCoefficient);
			mShader->SetUniformMat4f("view", view);
			mShader->SetUniformMat4f("invView", glm::inverse(view));
			mShader->SetUniformMat4f("proj", projection);
			mShader->SetUniformMat4f("invProj", glm::inverse(projection));

			mShader->SetUniformTex("textureDepth", gBuffer->GetTexture(GL_DEPTH_ATTACHMENT).get(), 1);
			mShader->SetUniformTex("textureNorm", gBuffer->GetTexture(GL_COLOR_ATTACHMENT1).get(), 2);
			mShader->SetUniformTex("textureMetallic", gBuffer->GetTexture(GL_COLOR_ATTACHMENT2).get(), 3);

			mShader->SetUniformTex("textureAlbedo", gBuffer->GetTexture(GL_COLOR_ATTACHMENT0).get(), 5);
			mShader->SetUniformTex("textureFrame", previousFrame.get(), 7);

			Renderer::DrawQuad(Matrix4());
		}
		OutputFramebuffer->Unbind();
	}

}
