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

			mShader->SetUniform("rayStep", RayStep);
			mShader->SetUniform("iterationCount", IterationCount);
			mShader->SetUniform("distanceBias", DistanceBias);
			mShader->SetUniform("enableSSR", (int)1);
			mShader->SetUniform("sampleCount", SampleCount);
			mShader->SetUniform("isSamplingEnabled", SamplingEnabled);
			mShader->SetUniform("isExponentialStepEnabled", ExpoStep);
			mShader->SetUniform("isAdaptiveStepEnabled", AdaptiveStep);
			mShader->SetUniform("isBinarySearchEnabled", BinarySearch);
			mShader->SetUniform("debugDraw", DebugDraw);
			mShader->SetUniform("samplingCoefficient", SampleingCoefficient);
			mShader->SetUniform("view", view);
			mShader->SetUniform("proj", projection);
			mShader->SetUniform("invProj", glm::inverse(projection));

			mShader->SetUniform("textureDepth", gBuffer->GetTexture(GL_DEPTH_ATTACHMENT).get(), 1);
			mShader->SetUniform("textureNorm", gBuffer->GetTexture(GL_COLOR_ATTACHMENT1).get(), 2);
			mShader->SetUniform("textureMetallic", gBuffer->GetTexture(GL_COLOR_ATTACHMENT2).get(), 3);

			mShader->SetUniform("textureAlbedo", gBuffer->GetTexture(GL_COLOR_ATTACHMENT0).get(), 5);
			mShader->SetUniform("textureFrame", previousFrame.get(), 7);

			Renderer::DrawQuad(Matrix4());
		}
		OutputFramebuffer->Unbind();
	}

}
