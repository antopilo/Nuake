#include "ThumbnailManager.h"

#include <Nuake/FileSystem/FileTypes.h>
#include <Nuake/FileSystem/File.h>

#include <Nuake/Scene/Components/CameraComponent.h>
#include <Nuake/Scene/Components/ModelComponent.h>

#include <Nuake/Rendering/SceneRenderer.h>

#include <Nuake/Resource/Prefab.h>
#include <Nuake/Resource/ResourceLoader.h>

#include <glm/ext/matrix_clip_space.hpp>


ThumbnailManager::ThumbnailManager()
{
	using namespace Nuake;

}

ThumbnailManager& ThumbnailManager::Get()
{
	static ThumbnailManager instance;
	return instance;
}

bool ThumbnailManager::IsThumbnailLoaded(const std::string& path) const
{
	return m_Thumbnails.find(path) != m_Thumbnails.end();
}

Ref<Nuake::Texture> ThumbnailManager::GetThumbnail(const std::string& path)
{

	if (IsThumbnailLoaded(path))
	{
		return m_Thumbnails[path];
	}

	using namespace Nuake;

	if (m_ThumbnailGeneratedThisFrame > MAX_THUMBNAIL_PER_FRAME)
	{
		return nullptr;
	}

	// Generate Thumbnail
	return nullptr;
}

void ThumbnailManager::MarkThumbnailAsDirty(const std::string & path)
{
	if (IsThumbnailLoaded(path))
	{
		m_Thumbnails.erase(path);
	}
}

Ref<Nuake::Texture> ThumbnailManager::GenerateThumbnail(const std::string& path, Ref<Nuake::Texture> texture)
{
	using namespace Nuake;

	const Matrix4 ortho = glm::orthoLH(-0.6f, 0.6f, -0.6f, 0.6f, -100.0f, 100.0f);
	Matrix4 view = Matrix4(1.0f);
	view = glm::lookAt(Vector3(1, -1.0, 0), Vector3(0, 0, 0), Vector3(0, 1, 0));;
	
	auto file = FileSystem::GetFile(path);
	if (file->GetFileType() == FileType::Prefab)
	{
		//Ref<Scene> scene = Scene::New();
		//auto cam = scene->CreateEntity("Camera");
		//TransformComponent& camTransform = cam.GetComponent<TransformComponent>();
		//camTransform.SetLocalPosition({ 0.0f, 0.0f, 2.0f });
		//
		//auto& previewLight = scene->CreateEntity("_directionalLight").AddComponent<LightComponent>();
		//previewLight.SetCastShadows(false);
		//previewLight.Type = LightType::Directional;
		//
		//scene->GetEnvironment()->CurrentSkyType = SkyType::ProceduralSky;
		//scene->GetEnvironment()->ProceduralSkybox->SunDirection = { 0.58f, 0.34f, -0.74f };
		//auto& camComponent = cam.AddComponent<CameraComponent>();
		//camComponent.CameraInstance->Fov = 45.0f;
		//camComponent.CameraInstance->AspectRatio = 1.0f;
		//m_ShadedFramebuffer->SetTexture(texture);
		//
		//Ref<Prefab> prefab = Prefab::InstanceInScene(path, scene.get());
		//
		//scene->Update(0.01f);
		//scene->Draw(*m_ShadedFramebuffer.get());

		// Gbuffer pass
		//m_Framebuffer->Bind();
		//{
		//	RenderCommand::SetClearColor({ 0.3f, 0.3f, 0.3f, 0.3f });
		//	m_Framebuffer->Clear();
		//
		//	RenderCommand::Disable(RendererEnum::BLENDING);
		//	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
		//	auto shader = ShaderManager::GetShader("Resources/Shaders/gbuffer.shader");
		//	shader->Bind();
		//
		//	auto cam = Engine::GetCurrentScene()->GetCurrentCamera();
		//	shader->SetUniform("u_View", view);
		//	shader->SetUniform("u_Projection", ortho);
		//	shader->SetUniform("u_Model", Matrix4(1.0f));
		//	Renderer::SphereMesh->Draw(shader, true);
		//}
		//m_Framebuffer->Unbind();
		//m_ShadedFramebuffer->SetTexture(texture);
		//m_ShadedFramebuffer->Bind();
		//{
		//	//RenderCommand::Enable(RendererEnum::BLENDING);
		//	RenderCommand::SetClearColor({ 0.2, 0.2, 0.2, 1 });
		//	m_ShadedFramebuffer->Clear();
		//	RenderCommand::Disable(RendererEnum::DEPTH_TEST);
		//	RenderCommand::Disable(RendererEnum::FACE_CULL);
		//	auto shader = ShaderManager::GetShader("Resources/Shaders/deferred.shader");
		//	shader->Bind();
		//	shader->SetUniform("u_EyePosition", Vector3(1, 0, 0));
		//	shader->SetUniform("LightCount", 0);
		//	auto dir = Engine::GetCurrentScene()->GetEnvironment()->ProceduralSkybox->GetSunDirection();
		//	shader->SetUniform("u_DirectionalLight.Direction", 0.6, -0.6, 0.6);
		//	shader->SetUniform("u_DirectionalLight.Color", 10.0f, 10.0f, 10.0f);
		//	shader->SetUniform("u_DirectionalLight.Shadow", 0);
		//	shader->SetUniform("u_DisableSSAO", 1);
		//	shader->SetUniform("u_View", view);
		//	shader->SetUniform("u_Projection", ortho);
		//
		//	m_Framebuffer->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(5);
		//	m_Framebuffer->GetTexture(GL_COLOR_ATTACHMENT0)->Bind(6);
		//	m_Framebuffer->GetTexture(GL_COLOR_ATTACHMENT1)->Bind(7);
		//	m_Framebuffer->GetTexture(GL_COLOR_ATTACHMENT2)->Bind(8);
		//	m_Framebuffer->GetTexture(GL_COLOR_ATTACHMENT4)->Bind(10);
		//
		//	shader->SetUniform("m_Depth", 5);
		//	shader->SetUniform("m_Albedo", 6);
		//	shader->SetUniform("m_Normal", 7);
		//	shader->SetUniform("m_Material", 8);
		//	shader->SetUniform("m_Emissive", 10);
		//
		//	Renderer::DrawQuad(Matrix4());
		//}
		//m_ShadedFramebuffer->Unbind();
	}
	else if (file->GetFileType() == FileType::Mesh)
	{
		Ref<Scene> scene = Scene::New();
		Ref<Environment> env = scene->GetEnvironment();
		env->CurrentSkyType = SkyType::ClearColor;
		env->ProceduralSkybox->SunDirection = { 0.88f, 0.34f, -0.14f };

		Entity camera = scene->CreateEntity("Camera");
		TransformComponent& camTransform = camera.GetComponent<TransformComponent>();
		camTransform.SetLocalPosition({ 0.0f, 0.5f, 2.0f });

		CameraComponent& camComponent = camera.AddComponent<CameraComponent>();
		camComponent.CameraInstance->Fov = 45.0f;
		camComponent.CameraInstance->AspectRatio = 1.0f;

		LightComponent& previewLight = scene->CreateEntity("_directionalLight").AddComponent<LightComponent>();
		previewLight.SetCastShadows(false);
		previewLight.Type = LightType::Directional;
		previewLight.Strength = 5.0f;
		previewLight.SyncDirectionWithSky = true;

		m_ShadedFramebuffer->SetTexture(texture);

		//ModelComponent& modelComp = scene->CreateEntity("Mesh").AddComponent<ModelComponent>();
		//modelComp.ModelResource = ResourceLoader::LoadModel(file->GetRelativePath());
		scene->Update(0.01f);
		scene->Draw(*m_ShadedFramebuffer.get());
	}
	else if (file->GetFileType() == FileType::Material)
	{
		// Gbuffer pass
		m_Framebuffer->Bind();
		{
			//RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
			//m_Framebuffer->Clear();
			//
			//RenderCommand::Disable(RendererEnum::BLENDING);
			//RenderCommand::Enable(RendererEnum::DEPTH_TEST);
			//auto shader = ShaderManager::GetShader("Resources/Shaders/gbuffer.shader");
			//shader->Bind();
			//
			//auto cam = Engine::GetCurrentScene()->GetCurrentCamera();
			//shader->SetUniform("u_View", view);
			//shader->SetUniform("u_Projection", ortho);
			//shader->SetUniform("u_Model", Matrix4(1.0f));
			//Ref<Material> material = ResourceLoader::LoadMaterial(path);
			//material->Bind(shader);
			//Renderer::SphereMesh->Draw(shader, false);
		}
		m_Framebuffer->Unbind();
		m_ShadedFramebuffer->SetTexture(texture);
		m_ShadedFramebuffer->Bind();
		{
			//RenderCommand::Enable(RendererEnum::BLENDING);
			RenderCommand::SetClearColor({ 0.2, 0.2, 0.2, 1 });
			m_ShadedFramebuffer->Clear();
			RenderCommand::Disable(RendererEnum::DEPTH_TEST);
			RenderCommand::Disable(RendererEnum::FACE_CULL);
			auto shader = ShaderManager::GetShader("Resources/Shaders/deferred.shader");
			shader->Bind();
			shader->SetUniform("u_EyePosition", Vector3(1, 0, 0));
			shader->SetUniform("LightCount", 0);
			auto dir = Engine::GetCurrentScene()->GetEnvironment()->ProceduralSkybox->GetSunDirection();
			shader->SetUniform("u_DirectionalLight.Direction", 0.6, -0.6, 0.6);
			shader->SetUniform("u_DirectionalLight.Color", 10.0f, 10.0f, 10.0f);
			shader->SetUniform("u_DirectionalLight.Shadow", 0);
			shader->SetUniform("u_DisableSSAO", 1);
			shader->SetUniform("u_View", view);
			shader->SetUniform("u_Projection", ortho);


			shader->SetUniform("m_Depth", 5);
			shader->SetUniform("m_Albedo", 6);
			shader->SetUniform("m_Normal", 7);
			shader->SetUniform("m_Material", 8);
			shader->SetUniform("m_Emissive", 10);

			Renderer::DrawQuad(Matrix4());
		}
		m_ShadedFramebuffer->Unbind();
	}
	

	return nullptr;
}

void ThumbnailManager::OnEndFrame()
{
	m_ThumbnailGeneratedThisFrame = 0;
}
