#include "SceneRenderer.h"
#include "src/Rendering/Shaders/ShaderManager.h"

#include <src/Scene/Components/BSPBrushComponent.h>
#include <GL\glew.h>

namespace Nuake {
	void SceneRenderer::Init()
	{
		mGBuffer = CreateScope<FrameBuffer>(false, Vector2(1920, 1080));
		mGBuffer->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_DEPTH_COMPONENT), GL_DEPTH_ATTACHMENT);
		mGBuffer->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_RGB), GL_COLOR_ATTACHMENT0);
		mGBuffer->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_RGB), GL_COLOR_ATTACHMENT1);
		mGBuffer->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_RGB), GL_COLOR_ATTACHMENT2);
		mGBuffer->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_RGB), GL_COLOR_ATTACHMENT3);
	
		mShadingBuffer = CreateScope<FrameBuffer>(true, Vector2(1920, 1080));
		Ref<Texture> shadedTexture = CreateRef<Texture>(Vector2(1920, 1080), GL_RGB, GL_RGB16F, GL_FLOAT);
		mShadingBuffer->SetTexture(shadedTexture);

		mBloom = CreateScope<Bloom>(4);
		mBloom->SetSource(shadedTexture);
		
		mVolumetric = CreateScope<Volumetric>();

		mSSR = CreateScope<SSR>();

		mToneMapBuffer = CreateScope<FrameBuffer>(false, Vector2(1920, 1080));
		mToneMapBuffer->SetTexture(CreateRef<Texture>(Vector2(1920, 1080), GL_RGB), GL_COLOR_ATTACHMENT0);

	}

	void SceneRenderer::Cleanup()
	{
		
	}

	void SceneRenderer::BeginRenderScene(const Matrix4& projection, const Matrix4& view)
	{
		mProjection = projection;
		mView = view;
	}

	void SceneRenderer::RenderScene(Scene& scene, FrameBuffer& framebuffer) 
	{
		ShadowPass(scene);

		mGBuffer->QueueResize(framebuffer.GetSize());
		GBufferPass(scene);
		 
		mShadingBuffer->QueueResize(framebuffer.GetSize());
		ShadingPass(scene);

		Texture* finalOutput = mShadingBuffer->GetTexture().get();
		if (scene.GetEnvironment()->BloomEnabled)
		{
			scene.GetEnvironment()->mBloom->SetSource(mShadingBuffer->GetTexture());
			scene.GetEnvironment()->mBloom->Resize(framebuffer.GetSize());
			scene.GetEnvironment()->mBloom->Draw();

			finalOutput = scene.GetEnvironment()->mBloom->GetOutput().get();
		}

		auto view = scene.m_Registry.view<LightComponent>();
		std::vector<LightComponent> lightList = std::vector<LightComponent>();
		for (auto l : view)
		{
			auto& lc = view.get<LightComponent>(l);
			if (lc.Type == Directional && lc.IsVolumetric)
				lightList.push_back(lc);
		}
		
		if (scene.GetEnvironment()->VolumetricEnabled)
		{
			mVolumetric->Resize(framebuffer.GetSize());
			mVolumetric->SetDepth(mGBuffer->GetTexture(GL_DEPTH_ATTACHMENT).get());
			mVolumetric->Draw(mProjection , mView, lightList);

			finalOutput = mVolumetric->GetFinalOutput();
		}
		
		// Copy final output to target framebuffer
		mToneMapBuffer->QueueResize(framebuffer.GetSize());
		mToneMapBuffer->Bind();
		{
			RenderCommand::Clear();
			Shader* shader = ShaderManager::GetShader("resources/Shaders/tonemap.shader");
			shader->Bind();

			shader->SetUniform1f("u_Exposure", scene.GetEnvironment()->Exposure);
			shader->SetUniform1f("u_Gamma", scene.GetEnvironment()->Gamma);
			shader->SetUniformTex("u_Source", finalOutput);
			Renderer::DrawQuad();
		}
		mToneMapBuffer->Unbind();

		//mSSR->Resize(framebuffer.GetSize());
		//mSSR->Draw(mGBuffer.get(), framebuffer.GetTexture(), mView, mProjection, scene.GetCurrentCamera());

		framebuffer.Bind();
		{
			RenderCommand::Clear();
			Shader* shader = ShaderManager::GetShader("resources/Shaders/combine.shader");
			shader->Bind();

			shader->SetUniformTex("u_Source", mToneMapBuffer->GetTexture().get(), 0);
			shader->SetUniformTex("u_Source2", mToneMapBuffer->GetTexture().get(), 1);
			Renderer::DrawQuad();
		}
		framebuffer.Unbind();


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

		auto meshView = scene.m_Registry.view<TransformComponent, MeshComponent>();
		auto quakeView = scene.m_Registry.view<TransformComponent, BSPBrushComponent>();
		auto view = scene.m_Registry.view<TransformComponent, LightComponent>();
		for (auto l : view)
		{
			auto [lightTransform, light] = view.get<TransformComponent, LightComponent>(l);
			if (light.Type != LightType::Directional || !light.CastShadows)
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
						auto [transform, mesh] = meshView.get<TransformComponent, MeshComponent>(e);
						for (auto& m : mesh.ModelResource->GetMeshes())
							Renderer::SubmitMesh(m, transform.GetGlobalTransform());
					}

					for (auto e : quakeView)
					{
						auto [transform, model] = quakeView.get<TransformComponent, BSPBrushComponent>(e);

						if (model.IsTransparent)
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

			auto view = scene.m_Registry.view<TransformComponent, MeshComponent, ParentComponent>();
			for (auto e : view)
			{
				auto [transform, mesh, parent] = view.get<TransformComponent, MeshComponent, ParentComponent>(e);
				
				if (mesh.ModelResource)
				{
					for (auto& m : mesh.ModelResource->GetMeshes())
						Renderer::SubmitMesh(m, transform.GetGlobalTransform());
				}
			}

			glCullFace(GL_BACK);
			RenderCommand::Disable(RendererEnum::FACE_CULL);
			Renderer::Flush(gBufferShader, false);

			auto quakeView = scene.m_Registry.view<TransformComponent, BSPBrushComponent, ParentComponent>();
			for (auto e : quakeView)
			{
				auto [transform, model, parent] = quakeView.get<TransformComponent, BSPBrushComponent, ParentComponent>(e);

				if (model.IsTransparent)
					continue;

				for (auto& b : model.Meshes)
					Renderer::SubmitMesh(b, transform.GetGlobalTransform());
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