#include "SceneRenderer.h"
#include "src/Rendering/Shaders/ShaderManager.h"

#include "src/Scene/Components/BSPBrushComponent.h"
#include "src/Scene/Components/SpriteComponent.h"
#include "src/Scene/Components/ParticleEmitterComponent.h"

#include <GL\glew.h>
#include <src/Scene/Components/SkinnedModelComponent.h>


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

		// SSAO
		const auto& sceneEnv = scene.GetEnvironment();
		sceneEnv->mSSAO->Resize(framebuffer.GetSize());
		sceneEnv->mSSAO->Draw(mGBuffer.get(), mProjection, mView);

		mShadingBuffer->QueueResize(framebuffer.GetSize());
		ShadingPass(scene);

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
			{
				continue;
			}

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

					auto spriteView = scene.m_Registry.view<TransformComponent, SpriteComponent, VisibilityComponent>();
					for (auto e : spriteView)
					{
						auto [transform, sprite, visibility] = spriteView.get<TransformComponent, SpriteComponent, VisibilityComponent>(e);

						if (!visibility.Visible || !sprite.SpriteMesh)
							continue;

						auto& finalQuadTransform = transform.GetGlobalTransform();
						if (sprite.Billboard)
						{
							finalQuadTransform = glm::inverse(mView);

							if (sprite.LockYRotation)
							{
								// This locks the pitch rotation on the billboard, useful for trees, lamps, etc.
								finalQuadTransform[1] = Vector4(0, 1, 0, 0);
								finalQuadTransform[2] = Vector4(finalQuadTransform[2][0], 0, finalQuadTransform[2][2], 0);
								finalQuadTransform = finalQuadTransform;
							}

							// Translation
							finalQuadTransform[3] = Vector4(transform.GetGlobalPosition(), 1.0f);

							// Scale
							finalQuadTransform = glm::scale(finalQuadTransform, transform.GetGlobalScale());
						}

						Renderer::SubmitMesh(sprite.SpriteMesh, finalQuadTransform, (uint32_t)e);
					}

					Renderer::Flush(shader, true);
				}
			}
		}
	}

	void SceneRenderer::GBufferPass(Scene& scene)
	{
		mGBuffer->Bind();
		mGBuffer->Clear();
		{
			// Init
			RenderCommand::Enable(RendererEnum::FACE_CULL);
			Shader* gBufferShader = ShaderManager::GetShader("resources/Shaders/gbuffer.shader");
			Shader* gBufferSkinnedMeshShader = ShaderManager::GetShader("resources/Shaders/gbuffer_skinned.shader");

			gBufferShader->Bind();
			gBufferShader->SetUniformMat4f("u_Projection", mProjection);
			gBufferShader->SetUniformMat4f("u_View", mView);

			// Models
			auto view = scene.m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
			for (auto e : view)
			{
				auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
				
				if (mesh.ModelResource && visibility.Visible)
				{
					for (auto& m : mesh.ModelResource->GetMeshes())
					{
						Renderer::SubmitMesh(m, transform.GetGlobalTransform(), (uint32_t)e);
					}
				}
			}

			glCullFace(GL_FRONT);
			Renderer::Flush(gBufferShader, false);

			// Quake BSPs
			auto quakeView = scene.m_Registry.view<TransformComponent, BSPBrushComponent, VisibilityComponent>();
			for (auto e : quakeView)
			{
				auto [transform, model, visibility] = quakeView.get<TransformComponent, BSPBrushComponent, VisibilityComponent>(e);

				if (model.IsTransparent || !visibility.Visible)
					continue;

				for (auto& b : model.Meshes)
				{
					Renderer::SubmitMesh(b, transform.GetGlobalTransform(), (uint32_t)e);
				}
			}
			Renderer::Flush(gBufferShader, false);

			// Sprites
			auto spriteView = scene.m_Registry.view<TransformComponent, SpriteComponent, VisibilityComponent>();
			for (auto& e : spriteView)
			{
				auto [transform, sprite, visibility] = spriteView.get<TransformComponent, SpriteComponent, VisibilityComponent>(e);

				if (!visibility.Visible || !sprite.SpriteMesh)
					continue;

				auto& finalQuadTransform = transform.GetGlobalTransform();
				if (sprite.Billboard)
				{
					finalQuadTransform = glm::inverse(mView);

					if (sprite.LockYRotation)
					{
						// This locks the pitch rotation on the billboard, useful for trees, lamps, etc.
						finalQuadTransform[1] = Vector4(0, 1, 0, 0);
						finalQuadTransform[2] = Vector4(finalQuadTransform[2][0], 0, finalQuadTransform[2][2], 0);
						finalQuadTransform = finalQuadTransform;
					}

					// Translation
					finalQuadTransform[3] = Vector4(transform.GetGlobalPosition(), 1.0f);

					// Scale
					finalQuadTransform = glm::scale(finalQuadTransform, transform.GetGlobalScale());
				}

				Renderer::SubmitMesh(sprite.SpriteMesh, finalQuadTransform, (uint32_t)e);
			}
			Renderer::Flush(gBufferShader, false);

			// Particles
			auto particleEmitterView = scene.m_Registry.view<TransformComponent, ParticleEmitterComponent, VisibilityComponent>();
			for (auto& e : particleEmitterView)
			{
				auto [transform, emitterComponent, visibility] = particleEmitterView.get<TransformComponent, ParticleEmitterComponent, VisibilityComponent>(e);

				if (!visibility.Visible)
					continue;

				Vector3 oldColor = Renderer::QuadMesh->GetMaterial()->data.m_AlbedoColor;
				auto initialTransform = transform.GetGlobalTransform();
				for (auto& p : emitterComponent.Emitter.Particles)
				{
					Matrix4 particleTransform = initialTransform;
					particleTransform = glm::inverse(mView);

					// Translation
					const Vector3& particleGlobalPosition = transform.GetGlobalPosition() + p.Position;
					particleTransform[3] = Vector4(particleGlobalPosition, 1.0f);

					// Scale
					particleTransform = glm::scale(particleTransform, transform.GetGlobalScale());

					Renderer::QuadMesh->GetMaterial()->data.u_HasAlbedo = 0;
					Renderer::QuadMesh->GetMaterial()->data.m_AlbedoColor = p.Color;
					Renderer::SubmitMesh(Renderer::QuadMesh, particleTransform, (uint32_t)e);
				}

				Renderer::Flush(gBufferShader, false);
				Renderer::QuadMesh->GetMaterial()->data.m_AlbedoColor = oldColor;
			}

			// Skinned mesh at the end because we switch shader
			gBufferSkinnedMeshShader->Bind();
			gBufferSkinnedMeshShader->SetUniformMat4f("u_Projection", mProjection);
			gBufferSkinnedMeshShader->SetUniformMat4f("u_View", mView);

			// Skinned Models
			const uint32_t entityIdUniformLocation = gBufferSkinnedMeshShader->FindUniformLocation("u_EntityID");
			const uint32_t modelMatrixUniformLocation = gBufferSkinnedMeshShader->FindUniformLocation("u_Model");

			auto skinnedModelView = scene.m_Registry.view<TransformComponent, SkinnedModelComponent, VisibilityComponent>();
			for (auto e : skinnedModelView)
			{
				auto [transform, mesh, visibility] = skinnedModelView.get<TransformComponent, SkinnedModelComponent, VisibilityComponent>(e);

				if (mesh.ModelResource && visibility.Visible)
				{
					for (auto& m : mesh.ModelResource->GetMeshes())
					{
						m->GetMaterial()->Bind(gBufferSkinnedMeshShader);
						
						uint32_t boneId = 0;
						for (auto& b : m->GetBones())
						{
							const std::string boneMatrixUniformName = "u_FinalBonesMatrice[" + std::to_string(boneId) + "]";
							gBufferSkinnedMeshShader->SetUniformMat4f(boneMatrixUniformName, b.Offset);
							boneId++;
						}

						gBufferSkinnedMeshShader->SetUniformMat4f(modelMatrixUniformLocation, transform.GetGlobalTransform());
						gBufferSkinnedMeshShader->SetUniform1i(entityIdUniformLocation, (uint32_t)e + 1);
						m->Draw(gBufferSkinnedMeshShader, true);
					}
				}
			}
		}
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
	}

	void SceneRenderer::PostProcessPass(const Scene& scene)
	{
	}
}