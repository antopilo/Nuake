#include "SceneRenderer.h"
#include "src/Rendering/Shaders/ShaderManager.h"

#include <src/Scene/Components/BSPBrushComponent.h>
#include <GL\glew.h>

namespace Nuake 
{
	void SceneRenderer::Init()
	{
		const auto defaultResolution = Vector2(1920, 1080);
		mGBuffer = CreateScope<FrameBuffer>(false, defaultResolution);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_DEPTH_COMPONENT), GL_DEPTH_ATTACHMENT);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT1);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT2);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RED_INTEGER, GL_R32I, GL_INT), GL_COLOR_ATTACHMENT3);

		mShadingBuffer = CreateScope<FrameBuffer>(true, defaultResolution);
		mShadingBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB, GL_RGB16F, GL_FLOAT));

		mSSR = CreateScope<SSR>();
		mToneMapBuffer = CreateScope<FrameBuffer>(false, defaultResolution);
		mToneMapBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0);
	}

	void SceneRenderer::Cleanup()
	{
		
	}

	void SceneRenderer::BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vector3& camPos)
	{
		mProjection = projection;
		mView = view;
		mCamPos = camPos;
	}

	/// <summary>
	/// Renders a scene to a framebuffer. The size of the framebuffer will be used.
	/// </summary>
	/// <param name="scene">Scene to render</param>
	/// <param name="framebuffer">Framebuffer to render the scene to. Should be in the right size</param>
	void SceneRenderer::RenderScene(Scene& scene, FrameBuffer& framebuffer) 
	{
		// Renders all shadow maps
		ShadowPass(scene);

		mGBuffer->QueueResize(framebuffer.GetSize());
		GBufferPass(scene);

		const auto& sceneEnv = scene.GetEnvironment();
		Ref<Texture> finalOutput = mShadingBuffer->GetTexture();
		if (scene.GetEnvironment()->BloomEnabled)
		{
			sceneEnv->mBloom->SetSource(mShadingBuffer->GetTexture());
			sceneEnv->mBloom->Resize(framebuffer.GetSize());
			sceneEnv->mBloom->Draw();

			finalOutput = scene.GetEnvironment()->mBloom->GetOutput();
		}

		const auto view = scene.m_Registry.view<LightComponent>();
		auto lightList = std::vector<LightComponent>();
		for (auto l : view)
		{
			auto& lc = view.get<LightComponent>(l);
			if (lc.Type == Directional && lc.IsVolumetric && lc.CastShadows)
				lightList.push_back(lc);
		}
		
		if (scene.GetEnvironment()->VolumetricEnabled)
		{
			sceneEnv->mVolumetric->Resize(framebuffer.GetSize());
			sceneEnv->mVolumetric->SetDepth(mGBuffer->GetTexture(GL_DEPTH_ATTACHMENT).get());
			sceneEnv->mVolumetric->Draw(mProjection , mView, mCamPos, lightList);

			//finalOutput = mVolumetric->GetFinalOutput().get();

			// combine
			framebuffer.Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("resources/Shaders/combine.shader");
				shader->Bind();

				shader->SetUniformTex("u_Source", finalOutput.get(), 0);
				shader->SetUniformTex("u_Source2", sceneEnv->mVolumetric->GetFinalOutput().get(), 1);
				Renderer::DrawQuad();
			}
			framebuffer.Unbind();
		}
		else
		{
			framebuffer.Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("resources/Shaders/copy.shader");
				shader->Bind();

				shader->SetUniformTex("u_Source", finalOutput.get(), 0);
				Renderer::DrawQuad();
			}
		}

		finalOutput = framebuffer.GetTexture();

		// SSAO
		sceneEnv->mSSAO->Resize(framebuffer.GetSize());
		sceneEnv->mSSAO->Draw(mGBuffer.get(), mProjection, mView);

		// Copy final output to target framebuffer
		mToneMapBuffer->QueueResize(framebuffer.GetSize());
		mToneMapBuffer->Bind();
		{
			RenderCommand::Clear();
			Shader* shader = ShaderManager::GetShader("resources/Shaders/tonemap.shader");
			shader->Bind();

			shader->SetUniform1f("u_Exposure", scene.GetEnvironment()->Exposure);
			shader->SetUniform1f("u_Gamma", scene.GetEnvironment()->Gamma);
			shader->SetUniformTex("u_Source", finalOutput.get());
			Renderer::DrawQuad();
		}
		mToneMapBuffer->Unbind();

		mSSR->Resize(framebuffer.GetSize());
		mSSR->Draw(mGBuffer.get(), framebuffer.GetTexture(), mView, mProjection, scene.GetCurrentCamera());

		framebuffer.Bind();
		{
			RenderCommand::Clear();
			Shader* shader = ShaderManager::GetShader("resources/Shaders/combine.shader");
			shader->Bind();

			shader->SetUniformTex("u_Source", mToneMapBuffer->GetTexture().get(), 0);
			shader->SetUniformTex("u_Source2", mSSR->OutputFramebuffer->GetTexture().get(), 1);
			Renderer::DrawQuad();
		}
		framebuffer.Unbind();



		mShadingBuffer->QueueResize(framebuffer.GetSize());
		ShadingPass(scene);

		RenderCommand::Enable(RendererEnum::DEPTH_TEST);
		Renderer::EndDraw();
	}
	
	void SceneRenderer::ShadowPass(Scene& scene)
	{
		RenderCommand::Enable(RendererEnum::DEPTH_TEST);

		Shader* shader = ShaderManager::GetShader("resources/Shaders/shadowMap.shader");
		shader->Bind();

		RenderCommand::Disable(RendererEnum::FACE_CULL);
		glCullFace(GL_BACK);

		auto meshView = scene.m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
		auto quakeView = scene.m_Registry.view<TransformComponent, BSPBrushComponent, VisibilityComponent>();
		auto view = scene.m_Registry.view<TransformComponent, LightComponent, VisibilityComponent>();
		for (auto l : view)
		{
			auto [lightTransform, light, visibility] = view.get<TransformComponent, LightComponent, VisibilityComponent>(l);
			if (light.Type != LightType::Directional || !light.CastShadows || !visibility.Visible)
				continue;

			light.CalculateViewProjection(mView, mProjection);

			for (int i = 0; i < CSM_AMOUNT; i++)
			{
				light.m_Framebuffers[i]->Bind();
				light.m_Framebuffers[i]->Clear();
				{
					shader->SetUniformMat4f("u_LightTransform", light.mViewProjections[i]);
					for (auto e : meshView)
					{
						auto [transform, mesh, visibility] = meshView.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
						if (mesh.ModelResource != nullptr && visibility.Visible)
						{
							for (auto& m : mesh.ModelResource->GetMeshes())
								Renderer::SubmitMesh(m, transform.GetGlobalTransform());
						}
					}

					for (auto e : quakeView)
					{
						auto [transform, model, visibility] = quakeView.get<TransformComponent, BSPBrushComponent, VisibilityComponent>(e);

						if (model.IsTransparent || !visibility.Visible)
							continue;

						for (Ref<Mesh>& m : model.Meshes)
						{
							Renderer::SubmitMesh(m, transform.GetGlobalTransform());
						}
					}
					Renderer::Flush(shader, true);
				}

				light.m_Framebuffers[i]->Unbind();
			}
		}
	}

	void SceneRenderer::GBufferPass(Scene& scene)
	{
		mGBuffer->Bind();
		mGBuffer->Clear();
		{
			RenderCommand::Enable(RendererEnum::FACE_CULL);
			Shader* gBufferShader = ShaderManager::GetShader("resources/Shaders/gbuffer.shader");
			gBufferShader->Bind();
			gBufferShader->SetUniformMat4f("u_Projection", mProjection);
			gBufferShader->SetUniformMat4f("u_View", mView);

			auto view = scene.m_Registry.view<TransformComponent, ModelComponent, ParentComponent, VisibilityComponent>();
			for (auto e : view)
			{
				auto [transform, mesh, parent, visibility] = view.get<TransformComponent, ModelComponent, ParentComponent, VisibilityComponent>(e);
				
				if (mesh.ModelResource && visibility.Visible)
				{
					for (auto& m : mesh.ModelResource->GetMeshes())
					{
						Renderer::SubmitMesh(m, transform.GetGlobalTransform(), (uint32_t)e);
					}
				}
			}

			glCullFace(GL_BACK);
			RenderCommand::Disable(RendererEnum::FACE_CULL);
			Renderer::Flush(gBufferShader, false);

			auto quakeView = scene.m_Registry.view<TransformComponent, BSPBrushComponent, ParentComponent, VisibilityComponent>();
			for (auto e : quakeView)
			{
				auto [transform, model, parent, visibility] = quakeView.get<TransformComponent, BSPBrushComponent, ParentComponent, VisibilityComponent>(e);

				if (model.IsTransparent || !visibility.Visible)
					continue;

				for (auto& b : model.Meshes)
				{
					Renderer::SubmitMesh(b, transform.GetGlobalTransform(), (uint32_t)e);
				}
			}
			Renderer::Flush(gBufferShader, false);
		}
		mGBuffer->Unbind();
	}

	void SceneRenderer::ShadingPass(Scene& scene)
	{
		mShadingBuffer->Bind();
		mShadingBuffer->Clear();
		{
			RenderCommand::Disable(RendererEnum::DEPTH_TEST);
			RenderCommand::Disable(RendererEnum::FACE_CULL);
			Ref<Environment> environment = scene.GetEnvironment();
			if (environment->CurrentSkyType == SkyType::ProceduralSky)
			{
				RenderCommand::Clear();
				RenderCommand::SetClearColor(Color(0, 0, 0, 1));
				environment->ProceduralSkybox->Draw(mProjection, mView);
			}
			else if (environment->CurrentSkyType == SkyType::ClearColor)
			{
				RenderCommand::SetClearColor(environment->AmbientColor);
				RenderCommand::Clear();
			}
			RenderCommand::Enable(RendererEnum::FACE_CULL);

			Shader* shadingShader = ShaderManager::GetShader("resources/Shaders/deferred.shader");
			shadingShader->Bind();
			shadingShader->SetUniformMat4f("u_Projection", mProjection);
			shadingShader->SetUniformMat4f("u_View", mView);
			shadingShader->SetUniformVec3("u_EyePosition", scene.GetCurrentCamera()->Translation);

			shadingShader->SetUniformTex("m_SSAO", scene.GetEnvironment()->mSSAO->GetOuput()->GetTexture().get(), 9);

			Ref<Environment> env = scene.GetEnvironment();

			auto view = scene.m_Registry.view<TransformComponent, LightComponent, ParentComponent>();
			for (auto l : view)
			{
				auto [transform, light, parent] = view.get<TransformComponent, LightComponent, ParentComponent>(l);

				if (light.SyncDirectionWithSky)
					light.Direction = env->ProceduralSkybox->GetSunDirection();

				Renderer::RegisterDeferredLight(transform, light);
			}

			mGBuffer->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(5);
			mGBuffer->GetTexture(GL_COLOR_ATTACHMENT0)->Bind(6);
			mGBuffer->GetTexture(GL_COLOR_ATTACHMENT1)->Bind(7);
			mGBuffer->GetTexture(GL_COLOR_ATTACHMENT2)->Bind(8);

			shadingShader->SetUniform1i("m_Depth", 5);
			shadingShader->SetUniform1i("m_Albedo", 6);
			shadingShader->SetUniform1i("m_Normal", 7);
			shadingShader->SetUniform1i("m_Material", 8);

			RenderCommand::Disable(RendererEnum::FACE_CULL);

			Renderer::DrawQuad(Matrix4());
		}
		mShadingBuffer->Unbind();
	}

	void SceneRenderer::PostProcessPass(const Scene& scene)
	{
	}
}