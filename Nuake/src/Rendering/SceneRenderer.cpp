#include "SceneRenderer.h"
#include "src/Rendering/Shaders/ShaderManager.h"

#include "src/Scene/Components/BSPBrushComponent.h"
#include "src/Scene/Components/SpriteComponent.h"
#include "src/Scene/Components/ParticleEmitterComponent.h"

#include <glad/glad.h>
#include <src/Scene/Components/SkinnedModelComponent.h>
#include <src/Vendors/imgui/imgui.h>


namespace Nuake 
{
	void SceneRenderer::Init()
	{
		const auto defaultResolution = Vector2(1920, 1080);
		mGBuffer = CreateScope<FrameBuffer>(false, defaultResolution);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_DEPTH_COMPONENT), GL_DEPTH_ATTACHMENT);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0); // Albedo
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT1); //
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGBA), GL_COLOR_ATTACHMENT2); // Material + unlit
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RED_INTEGER, GL_R32I, GL_INT), GL_COLOR_ATTACHMENT3);
		mGBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RED, GL_R16F, GL_FLOAT), GL_COLOR_ATTACHMENT4); // Emissive

		mShadingBuffer = CreateScope<FrameBuffer>(true, defaultResolution);
		mShadingBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB, GL_RGB16F, GL_FLOAT));

		mToneMapBuffer = CreateScope<FrameBuffer>(false, defaultResolution);
		mToneMapBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0);

		mBarrelDistortionBuffer = CreateScope<FrameBuffer>(false, defaultResolution);
		mBarrelDistortionBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0);

		mVignetteBuffer = CreateScope<FrameBuffer>(false, defaultResolution);
		mVignetteBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0);

		mDOFBuffer = CreateScope<FrameBuffer>(false, defaultResolution);
		mDOFBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0);
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
		if (sceneEnv->SSAOEnabled)
		{
			sceneEnv->mSSAO->Resize(framebuffer.GetSize());
			sceneEnv->mSSAO->Draw(mGBuffer.get(), mProjection, mView);
		}
		else
		{
			sceneEnv->mSSAO->Clear();
		}

		mShadingBuffer->QueueResize(framebuffer.GetSize());
		ShadingPass(scene);

		Ref<Texture> finalOutput = mShadingBuffer->GetTexture();
		if (scene.GetEnvironment()->BloomEnabled)
		{
			sceneEnv->mBloom->SetSource(mShadingBuffer->GetTexture());
			sceneEnv->mBloom->Resize(framebuffer.GetSize());
			sceneEnv->mBloom->Draw();

			finalOutput = sceneEnv->mBloom->GetOutput();
		}

		const auto view = scene.m_Registry.view<LightComponent>();
		auto lightList = std::vector<LightComponent>();
		for (auto l : view)
		{
			auto& lc = view.get<LightComponent>(l);
			if (lc.Type == Directional && lc.IsVolumetric && lc.CastShadows)
				lightList.push_back(lc);
		}

		if (sceneEnv->VolumetricEnabled)
		{
			sceneEnv->mVolumetric->Resize(framebuffer.GetSize());
			sceneEnv->mVolumetric->SetDepth(mGBuffer->GetTexture(GL_DEPTH_ATTACHMENT).get());
			sceneEnv->mVolumetric->Draw(mProjection, mView, mCamPos, lightList);

			//finalOutput = mVolumetric->GetFinalOutput().get();

			// combine
			framebuffer.Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/combine.shader");
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
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/copy.shader");
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
			Shader* shader = ShaderManager::GetShader("Resources/Shaders/tonemap.shader");
			shader->Bind();

			shader->SetUniform1f("u_Exposure", scene.GetEnvironment()->Exposure);
			shader->SetUniform1f("u_Gamma", scene.GetEnvironment()->Gamma);
			shader->SetUniformTex("u_Source", finalOutput.get());
			Renderer::DrawQuad();
		}
		mToneMapBuffer->Unbind();

		if (sceneEnv->SSREnabled)
		{
			sceneEnv->mSSR->Resize(framebuffer.GetSize());
			sceneEnv->mSSR->Draw(mGBuffer.get(), framebuffer.GetTexture(), mView, mProjection, scene.GetCurrentCamera());

			framebuffer.Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/combine.shader");
				shader->Bind();

				shader->SetUniformTex("u_Source", mToneMapBuffer->GetTexture().get(), 0);
				shader->SetUniformTex("u_Source2", sceneEnv->mSSR->OutputFramebuffer->GetTexture().get(), 1);
				Renderer::DrawQuad();
			}
			framebuffer.Unbind();
		}
		else
		{
			framebuffer.Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/copy.shader");
				shader->Bind();

				shader->SetUniformTex("u_Source", mToneMapBuffer->GetTexture().get(), 0);
				Renderer::DrawQuad();
			}
			framebuffer.Unbind();
		}

		static float focalDepth = 100.0f;
		static float focalLength = 16.0f;
		static float fstop = 6.0f;
		static bool autoFocus = false;
		static bool showFocus = false;
		static bool manualdof = true;
		static int samples = 3;
		static int rings = 3;
		static float ndofstart = 1.0f;
		static float ndofDist = 2.0f;
		static float fdofstart = 1.0f;
		static float fdofdist = 3.0f;
		static float coc = 0.03f;
		static float maxBlue = 1.0f;
		static float threshold = 0.7f;
		static float gain = 100.0f;
		static float biaos = 0.0f;
		static float fringe = 0.0f;
		static float nammount = 0.0001;
		static float dbsize = 1.25f;
		static float feather = 1.0f;

		mDOFBuffer->QueueResize(framebuffer.GetSize());
		mDOFBuffer->Bind();
		{
			RenderCommand::Clear();
			Shader* shader = ShaderManager::GetShader("Resources/Shaders/dof.shader");
			shader->Bind();

			shader->SetUniform1f("focalDepth", focalDepth);
			shader->SetUniform1f("focalLength", focalLength);
			shader->SetUniform1f("fstop", fstop);
			shader->SetUniform1i("showFocus", showFocus);
			shader->SetUniform1i("autofocus", autoFocus);
			shader->SetUniform1i("samples", samples);
			shader->SetUniform1i("manualdof", manualdof);
			shader->SetUniform1f("rings", rings);
			shader->SetUniform1f("ndofstart", ndofstart);
			shader->SetUniform1f("ndofdist", ndofDist);
			shader->SetUniform1f("fdofstart", fdofstart);
			shader->SetUniform1f("fdofdist", fdofdist);
			shader->SetUniform1f("CoC", coc);
			shader->SetUniform1f("maxblur", maxBlue);
			shader->SetUniform1f("threshold", threshold);
			shader->SetUniform1f("gain", gain);
			shader->SetUniform1f("bias", biaos);
			shader->SetUniform1f("fringe", fringe);
			shader->SetUniform1f("namount", nammount);
			shader->SetUniform1f("dbsize", dbsize);
			shader->SetUniform1f("feather", feather);
			shader->SetUniform1f("u_Distortion", sceneEnv->BarrelDistortion);
			shader->SetUniform1f("height", finalOutput->GetHeight());
			shader->SetUniform1f("width", finalOutput->GetWidth());
			shader->SetUniformTex("depthTex", mGBuffer->GetTexture(GL_DEPTH_ATTACHMENT).get(), 0);
			shader->SetUniformTex("renderTex", finalOutput.get(), 1);
			Renderer::DrawQuad();
		}
		mDOFBuffer->Unbind();

		if (sceneEnv->BarrelDistortionEnabled)
		{
			mBarrelDistortionBuffer->QueueResize(framebuffer.GetSize());
			mBarrelDistortionBuffer->Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/barrel_distortion.shader");
				shader->Bind();

				shader->SetUniform1f("u_Distortion", sceneEnv->BarrelDistortion);
				shader->SetUniform1f("u_DistortionEdge", sceneEnv->BarrelEdgeDistortion);
				shader->SetUniform1f("u_Scale", sceneEnv->BarrelScale);

				if (sceneEnv->DOFEnabled)
				{
					shader->SetUniformTex("u_Source", mDOFBuffer->GetTexture().get(), 0);
				}
				else
				{
					shader->SetUniformTex("u_Source", finalOutput.get(), 0);
				}

				
				Renderer::DrawQuad();
			}
			mBarrelDistortionBuffer->Unbind();

			framebuffer.Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/copy.shader");
				shader->Bind();

				shader->SetUniformTex("u_Source", mBarrelDistortionBuffer->GetTexture().get(), 0);
				Renderer::DrawQuad();
			}
			framebuffer.Unbind();
		}

		if (sceneEnv->VignetteEnabled)
		{
			mVignetteBuffer->QueueResize(framebuffer.GetSize());
			mVignetteBuffer->Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/vignette.shader");
				shader->Bind();

				shader->SetUniform1f("u_Intensity", sceneEnv->VignetteIntensity);
				shader->SetUniform1f("u_Extend", sceneEnv->VignetteExtend);
				shader->SetUniformTex("u_Source", finalOutput.get(), 0);
				Renderer::DrawQuad();
			}
			mVignetteBuffer->Unbind();

			framebuffer.Bind();
			{
				RenderCommand::Clear();
				Shader* shader = ShaderManager::GetShader("Resources/Shaders/copy.shader");
				shader->Bind();

				shader->SetUniformTex("u_Source", mVignetteBuffer->GetTexture().get(), 0);
				Renderer::DrawQuad();
			}
			framebuffer.Unbind();
		}
		

		// Barrel distortion
		//mVignetteBuffer->Bind();
		//{
		//	RenderCommand::Clear();
		//	Shader* shader = ShaderManager::GetShader("Resources/Shaders/vignette.shader");
		//	shader->Bind();
		//
		//	shader->SetUniform1f("u_Intensity", sceneEnv->VignetteIntensity);
		//	shader->SetUniform1f("u_Extend", sceneEnv->VignetteExtend);
		//	shader->SetUniformTex("u_Source", mBarrelDistortionBuffer->GetTexture().get(), 0);
		//	Renderer::DrawQuad();
		//}
		//mVignetteBuffer->Unbind();


		RenderCommand::Enable(RendererEnum::DEPTH_TEST);
		Renderer::EndDraw();
	}
	
	void SceneRenderer::ShadowPass(Scene& scene)
	{
		RenderCommand::Enable(RendererEnum::DEPTH_TEST);

		Shader* shader = ShaderManager::GetShader("Resources/Shaders/shadowMap.shader");
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

						auto finalQuadTransform = transform.GetGlobalTransform();
						if (sprite.Billboard)
						{
							if (sprite.PositionFacing)
							{
								const Matrix4& invView = glm::inverse(mView);
								const Vector3& cameraPosition = Vector3(invView[3][0], invView[3][1], invView[3][2]);
								const Vector3& spritePosition = Vector3(finalQuadTransform[3][0], finalQuadTransform[3][1], finalQuadTransform[3][2]);
								const Vector3& direction = cameraPosition - spritePosition;
								finalQuadTransform = glm::inverse(glm::lookAt(Vector3(), direction, Vector3(0, 1, 0)));
							}
							else
							{
								finalQuadTransform = glm::inverse(mView);

							}

							if (sprite.LockYRotation)
							{
								// This locks the pitch rotation on the billboard, useful for trees, lamps, etc.
								finalQuadTransform[1] = Vector4(0, 1, 0, 0);
								finalQuadTransform[2] = Vector4(finalQuadTransform[2][0], 0, finalQuadTransform[2][2], 0);
								finalQuadTransform = finalQuadTransform;
							}

							finalQuadTransform[3] = Vector4(Vector3(transform.GetGlobalTransform()[3]), 1.0f);

							// Scale
							finalQuadTransform = glm::scale(finalQuadTransform, transform.GetGlobalScale());
						}

						Renderer::SubmitMesh(sprite.SpriteMesh, finalQuadTransform, (uint32_t)e);
					}

					Renderer::Flush(shader, true);
				}
			}
		}

		Shader* gBufferSkinnedMeshShader = ShaderManager::GetShader("Resources/Shaders/shadowMap_skinned.shader");
		gBufferSkinnedMeshShader->Bind();
		const uint32_t modelMatrixUniformLocation = gBufferSkinnedMeshShader->FindUniformLocation("u_Model");
		gBufferSkinnedMeshShader->SetUniformMat4f(modelMatrixUniformLocation, Matrix4(1.0f));
		
		auto skinnedView = scene.m_Registry.view<TransformComponent, SkinnedModelComponent, VisibilityComponent>();
		for (auto l : view)
		{
			auto [lightTransform, light, visibility] = view.get<TransformComponent, LightComponent, VisibilityComponent>(l);
			if (light.Type != LightType::Directional || !light.CastShadows || !visibility.Visible)
			{
				continue;
			}
		
			for (int i = 0; i < CSM_AMOUNT; i++)
			{
				light.m_Framebuffers[i]->Bind();
				{
					gBufferSkinnedMeshShader->SetUniformMat4f("u_LightTransform", light.mViewProjections[i]);
					for (auto e : skinnedView)
					{
						auto [transform, mesh, visibility] = skinnedView.get<TransformComponent, SkinnedModelComponent, VisibilityComponent>(e);
						if (mesh.ModelResource != nullptr && visibility.Visible)
						{
							auto& rootBoneNode = mesh.ModelResource->GetSkeletonRootNode();
							SetSkeletonBoneTransformRecursive(scene, rootBoneNode, gBufferSkinnedMeshShader);

							for (auto& m : mesh.ModelResource->GetMeshes())
							{
								m->Draw(gBufferSkinnedMeshShader, false);
							}
						}
					}
				}
			}
		}
	}

	void SceneRenderer::GBufferPass(Scene& scene)
	{
		mGBuffer->Bind();
		mGBuffer->Clear();
		{
			RenderCommand::Disable(RendererEnum::BLENDING);

			// Init
			RenderCommand::Enable(RendererEnum::FACE_CULL);
			Shader* gBufferShader = ShaderManager::GetShader("Resources/Shaders/gbuffer.shader");
			Shader* gBufferSkinnedMeshShader = ShaderManager::GetShader("Resources/Shaders/gbuffer_skinned.shader");

			gBufferShader->Bind();
			gBufferShader->SetUniformMat4f("u_Projection", mProjection);
			gBufferShader->SetUniformMat4f("u_View", mView);

			// Models
			glDisable(GL_CULL_FACE);
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

			RenderCommand::Disable(RendererEnum::FACE_CULL);

			// Sprites
			auto spriteView = scene.m_Registry.view<TransformComponent, SpriteComponent, VisibilityComponent>();
			for (auto& e : spriteView)
			{
				auto [transform, sprite, visibility] = spriteView.get<TransformComponent, SpriteComponent, VisibilityComponent>(e);

				if (!visibility.Visible || !sprite.SpriteMesh)
					continue;

				auto finalQuadTransform = transform.GetGlobalTransform();
				if (sprite.Billboard)
				{
					if (sprite.PositionFacing)
					{
						const Matrix4& invView = glm::inverse(mView);
						const Vector3& cameraPosition = Vector3(invView[3][0], invView[3][1], invView[3][2]);
						const Vector3& spritePosition = Vector3(finalQuadTransform[3][0], finalQuadTransform[3][1], finalQuadTransform[3][2]);
						const Vector3& direction = cameraPosition - spritePosition;
						finalQuadTransform = glm::inverse(glm::lookAt(Vector3(), direction, Vector3(0, 1, 0)));
					}
					else
					{
						finalQuadTransform = glm::inverse(mView);
						
					}

					if (sprite.LockYRotation)
					{
						// This locks the pitch rotation on the billboard, useful for trees, lamps, etc.
						finalQuadTransform[1] = Vector4(0, 1, 0, 0);
						finalQuadTransform[2] = Vector4(finalQuadTransform[2][0], 0, finalQuadTransform[2][2], 0);
						finalQuadTransform = finalQuadTransform;
					}

					finalQuadTransform[3] = Vector4(Vector3(transform.GetGlobalTransform()[3]), 1.0f);

					// Scale
					finalQuadTransform = glm::scale(finalQuadTransform, transform.GetGlobalScale());
				}

				Renderer::SubmitMesh(sprite.SpriteMesh, finalQuadTransform, (uint32_t)e);
			}
			Renderer::Flush(gBufferShader, false);

			// Particles
			Ref<Material> previousMaterial = Renderer::QuadMesh->GetMaterial();

			auto particleEmitterView = scene.m_Registry.view<TransformComponent, ParticleEmitterComponent, VisibilityComponent>();
			for (auto& e : particleEmitterView)
			{
				auto [transform, emitterComponent, visibility] = particleEmitterView.get<TransformComponent, ParticleEmitterComponent, VisibilityComponent>(e);

				if (!visibility.Visible || !emitterComponent.ParticleMaterial)
					continue;

				Renderer::QuadMesh->SetMaterial(emitterComponent.ParticleMaterial);

				Vector3 oldColor = Renderer::QuadMesh->GetMaterial()->data.m_AlbedoColor;
				auto initialTransform = transform.GetGlobalTransform();
				for (auto& p : emitterComponent.Emitter.Particles)
				{
					Matrix4 particleTransform = initialTransform;
					particleTransform = glm::inverse(mView);

					// Translation
					Vector3 particleGlobalPosition;
					if (emitterComponent.GlobalSpace)
					{
						particleGlobalPosition = p.Position;
					}
					else
					{
						particleGlobalPosition = Vector3(initialTransform[3]) + p.Position;
					}
					particleTransform[3] = Vector4(particleGlobalPosition, 1.0f);

					// Scale
					Vector3 finalScale = emitterComponent.ParticleScale;
					if (p.Scale != 1.0f)
					{
						finalScale += emitterComponent.ParticleScale * p.Scale;
					}

					particleTransform = glm::scale(particleTransform, finalScale);
					
					Renderer::SubmitMesh(Renderer::QuadMesh, particleTransform, (uint32_t)e);
				}

				Renderer::QuadMesh->SetMaterial(emitterComponent.ParticleMaterial);
				Renderer::Flush(gBufferShader, false);

				Renderer::QuadMesh->SetMaterial(previousMaterial);
			}

			// Reset material on quadmesh
			//Renderer::QuadMesh->SetMaterial(previousMaterial);

			// Skinned mesh at the end because we switch shader
			gBufferSkinnedMeshShader->Bind();
			gBufferSkinnedMeshShader->SetUniformMat4f("u_Projection", mProjection);
			gBufferSkinnedMeshShader->SetUniformMat4f("u_View", mView);

			RenderCommand::Disable(RendererEnum::FACE_CULL);

			// Skinned Models
			const uint32_t entityIdUniformLocation = gBufferSkinnedMeshShader->FindUniformLocation("u_EntityID");
			const uint32_t modelMatrixUniformLocation = gBufferSkinnedMeshShader->FindUniformLocation("u_Model");
			gBufferSkinnedMeshShader->SetUniformMat4f(modelMatrixUniformLocation, Matrix4(1.0f));
			auto skinnedModelView = scene.m_Registry.view<TransformComponent, SkinnedModelComponent, VisibilityComponent>();
			for (auto e : skinnedModelView)
			{
				auto [transform, mesh, visibility] = skinnedModelView.get<TransformComponent, SkinnedModelComponent, VisibilityComponent>(e);
				auto& meshResource = mesh.ModelResource;

				if (meshResource && visibility.Visible)
				{
					auto& rootBoneNode = meshResource->GetSkeletonRootNode();
					SetSkeletonBoneTransformRecursive(scene, rootBoneNode, gBufferSkinnedMeshShader);

					for (auto& m : mesh.ModelResource->GetMeshes())
					{
						m->GetMaterial()->Bind(gBufferSkinnedMeshShader);
						
						gBufferSkinnedMeshShader->SetUniform1i(entityIdUniformLocation, (uint32_t)e + 1);
						m->Draw(gBufferSkinnedMeshShader, true);
					}
				}
			}
		}

		RenderCommand::Enable(RendererEnum::BLENDING);
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

			Shader* shadingShader = ShaderManager::GetShader("Resources/Shaders/deferred.shader");
			shadingShader->Bind();
			shadingShader->SetUniformMat4f("u_Projection", mProjection);
			shadingShader->SetUniformMat4f("u_View", mView);
			shadingShader->SetUniformVec3("u_EyePosition", scene.GetCurrentCamera()->Translation);
			shadingShader->SetUniform1f("u_AmbientTerm", environment->AmbientTerm);

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
			mGBuffer->GetTexture(GL_COLOR_ATTACHMENT4)->Bind(10);

			shadingShader->SetUniform1i("m_Depth", 5);
			shadingShader->SetUniform1i("m_Albedo", 6);
			shadingShader->SetUniform1i("m_Normal", 7);
			shadingShader->SetUniform1i("m_Material", 8);
			shadingShader->SetUniform1i("m_Emissive", 10);

			RenderCommand::Disable(RendererEnum::FACE_CULL);

			Renderer::DrawQuad(Matrix4());
		}
	}

	void SceneRenderer::PostProcessPass(const Scene& scene)
	{
	}

	void SceneRenderer::SetSkeletonBoneTransformRecursive(Scene& scene, SkeletonNode& skeletonNode, Shader* shader)
	{
		for (auto& child : skeletonNode.Children)
		{
			if (auto entity = scene.GetEntity(child.Name); entity.GetHandle() != -1)
			{
				const std::string boneMatrixUniformName = "u_FinalBonesMatrice[" + std::to_string(child.Id) + "]";
				shader->SetUniformMat4f(boneMatrixUniformName, child.FinalTransform);
			}

			SetSkeletonBoneTransformRecursive(scene, child, shader);
		}
	}

}