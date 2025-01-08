#include "PipelineBuilder.h"
#include "ShaderCompiler.h"
#include "VulkanCheck.h"
#include "VulkanSceneRenderer.h"

#include "src/Rendering/Vulkan/VulkanAllocator.h"
#include "src/Rendering/Vulkan/VulkanInit.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"
#include "src/Rendering/Vulkan/VkResources.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Rendering/Textures/Material.h"


#include <Tracy.hpp>
#include <src/Scene/Components/ModelComponent.h>
#include "Pipeline/RenderPipeline.h"

#include "src/Rendering/Vulkan/DescriptorLayoutBuilder.h"

using namespace Nuake;

VkSceneRenderer::VkSceneRenderer()
{
}

VkSceneRenderer::~VkSceneRenderer()
{
}

void VkSceneRenderer::Init()
{
	// Here we will create the pipeline for rendering a scene
	LoadShaders();
	CreateBuffers();
	CreateSamplers();
	CreateDescriptors();

	SetGBufferSize({ 1280, 720 });
	CreatePipelines();
	ModelMatrixMapping.clear();
	MeshMaterialMapping.clear();
}

void VkSceneRenderer::BeginScene(RenderContext inContext)
{
	Context.CommandBuffer = inContext.CommandBuffer;
	Context.CurrentScene = inContext.CurrentScene;

	// Collect all global transform of things we will render
	BuildMatrixBuffer();
	UpdateTransformBuffer();

	auto& cmd = Context.CommandBuffer;
	auto& scene = Context.CurrentScene;
	auto& vk = VkRenderer::Get();

	// Ensure the command buffer is valid
	if (cmd == VK_NULL_HANDLE) {
		throw std::runtime_error("Invalid command buffer");
	}

	// Ensure the draw image is valid
	if (!vk.DrawImage) {
		throw std::runtime_error("Draw image is not initialized");
	}

	PassRenderContext passCtx = { };
	passCtx.scene = inContext.CurrentScene;
	passCtx.commandBuffer = inContext.CommandBuffer;
	passCtx.resolution = Context.Size;
	GBufferPipeline.Execute(passCtx);
}
ModelPushConstant modelPushConstant{};

void VkSceneRenderer::EndScene()
{
	auto& vk = VkRenderer::Get();
	auto& cmd = Context.CommandBuffer;

	auto& albedo = GBufferPipeline.GetRenderPass("GBuffer").GetAttachment("Albedo");
	VulkanUtil::TransitionImage(cmd, albedo.Image->GetImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, vk.DrawImage->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanUtil::CopyImageToImage(cmd, albedo.Image->GetImage(), vk.GetDrawImage()->GetImage(), albedo.Image->GetSize(), vk.DrawImage->GetSize());
	VulkanUtil::TransitionImage(cmd, vk.DrawImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	VulkanUtil::TransitionImage(cmd, albedo.Image->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
}

void VkSceneRenderer::CreateBuffers()
{
	CameraData camData{};
	camData.View = Matrix4(1.0f);
	camData.Projection = Matrix4(1.0f);

	// init camera buffer
	GPUResources& resources = GPUResources::Get();
	CameraBuffer = resources.CreateBuffer(sizeof(CameraData), BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "CameraBuffer");
	UpdateCameraData(camData);

	ModelBuffer = resources.CreateBuffer(sizeof(Matrix4) * MAX_MODEL_MATRIX, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "TransformBuffer");
	MaterialBuffer = resources.CreateBuffer(sizeof(MaterialBufferStruct) * MAX_MATERIAL, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "MaterialBuffer");
}

void VkSceneRenderer::LoadShaders()
{
	ShaderCompiler& shaderCompiler = ShaderCompiler::Get();
	Shaders["basic_frag"] = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/triangle.frag");
	Shaders["basic_vert"] = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/triangle.vert");
}

void VkSceneRenderer::CreateSamplers()
{
	auto device = VkRenderer::Get().GetDevice();
	VkSamplerCreateInfo sampler = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;

	vkCreateSampler(device, &sampler, nullptr, &SamplerNearest);

	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	vkCreateSampler(device, &sampler, nullptr, &SamplerLinear);
}

void VkSceneRenderer::CreateDescriptors()
{
	auto vk = VkRenderer::Get();
	auto device = vk.GetDevice();

	// Camera
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		CameraBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT);
	}

	{
		// Triangle vertex buffer layout
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		TriangleBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT);
	}

	{
		// Matrices
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		ModelBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT);
	}

	// Textures
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		ImageDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER);
		SamplerDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	// Material
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		MaterialBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	auto allocator = vk.GetDescriptorAllocator();
	TriangleBufferDescriptors = allocator.Allocate(device, TriangleBufferDescriptorLayout);
	CameraBufferDescriptors = allocator.Allocate(device, CameraBufferDescriptorLayout);
	ModelBufferDescriptor = allocator.Allocate(device, ModelBufferDescriptorLayout);
	SamplerDescriptor = allocator.Allocate(device, SamplerDescriptorLayout);
	MaterialBufferDescriptor = allocator.Allocate(device, MaterialBufferDescriptorLayout);
	//SamplerDescriptor = allocator.Allocate(device, SamplerDescriptorLayout);

	// Update descriptor set for camera
	VkDescriptorBufferInfo camBufferInfo{};
	camBufferInfo.buffer = CameraBuffer->GetBuffer();
	camBufferInfo.offset = 0;
	camBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet bufferWriteCam = {};
	bufferWriteCam.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bufferWriteCam.pNext = nullptr;
	bufferWriteCam.dstBinding = 0;
	bufferWriteCam.dstSet = CameraBufferDescriptors;
	bufferWriteCam.descriptorCount = 1;
	bufferWriteCam.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferWriteCam.pBufferInfo = &camBufferInfo;
	vkUpdateDescriptorSets(device, 1, &bufferWriteCam, 0, nullptr);

	// Update Descriptor Set for Texture
	VkDescriptorImageInfo textureInfo = {};
	textureInfo.sampler = SamplerNearest;  // Your VkSampler object
	textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Update Descriptor Set for Sampler
	VkWriteDescriptorSet samplerWrite = {};
	samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	samplerWrite.dstBinding = 0;  // Binding for sampler (in shader)
	samplerWrite.dstSet = SamplerDescriptor;  // The allocated descriptor set for the sampler
	samplerWrite.descriptorCount = 1;
	samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	samplerWrite.pImageInfo = &textureInfo;  // Sampler info (same as texture)

	vkUpdateDescriptorSets(device, 1, &samplerWrite, 0, nullptr);
}

void VkSceneRenderer::CreatePipelines()
{
	GBufferPipeline = RenderPipeline();

	auto& gBufferPass = GBufferPipeline.AddPass("GBuffer");
	gBufferPass.SetShaders(Shaders["basic_vert"], Shaders["basic_frag"]);
	gBufferPass.AddAttachment("Albedo", ImageFormat::RGBA8);
	gBufferPass.AddAttachment("Normal", ImageFormat::RGBA16F);
	gBufferPass.AddAttachment("Material", ImageFormat::RGBA8);
	gBufferPass.AddAttachment("Depth", ImageFormat::D32F, ImageUsage::Depth);
	gBufferPass.SetPushConstant<ModelPushConstant>(modelPushConstant);

	gBufferPass.SetPreRender([&](PassRenderContext& ctx) {
		std::vector<VkDescriptorSet> descriptors2 = { CameraBufferDescriptors, ModelBufferDescriptor };
		vkCmdBindDescriptorSets(
			ctx.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			ctx.renderPass->PipelineLayout,
			0,                               // firstSet
			2,                               // descriptorSetCount
			descriptors2.data(),        // pointer to the descriptor set(s)
			0,                               // dynamicOffsetCount
			nullptr                          // dynamicOffsets
		);

		// Bind material
		vkCmdBindDescriptorSets(
			ctx.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			ctx.renderPass->PipelineLayout,
			5,                               // firstSet
			1,                               // descriptorSetCount
			&MaterialBufferDescriptor,        // pointer to the descriptor set(s)
			0,                               // dynamicOffsetCount
			nullptr                          // dynamicOffsets
		);
	});

	gBufferPass.SetRender([&](PassRenderContext& ctx){
		auto& cmd = ctx.commandBuffer;
		auto& scene = ctx.scene;
		auto& vk = VkRenderer::Get();

		// Draw the scene
		{
			ZoneScopedN("Render Models");
			auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
			for (auto e : view)
			{
				auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
				if (!mesh.ModelResource || !visibility.Visible)
				{
					continue;
				}

				Entity entity = Entity((entt::entity)e, scene.get());
				for (auto& m : mesh.ModelResource->GetMeshes())
				{
					Ref<VkMesh> vkMesh = m->GetVkMesh();
					Matrix4 globalTransform = transform.GetGlobalTransform();

					auto descSet = vkMesh->GetDescriptorSet();
					vkCmdBindDescriptorSets(
						cmd,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						ctx.renderPass->PipelineLayout,
						2,                               // firstSet
						1,                               // descriptorSetCount
						&descSet,     // pointer to the descriptor set(s)
						0,                               // dynamicOffsetCount
						nullptr                          // dynamicOffsets
					);

					// Bind texture descriptor set
					Ref<Material> material = m->GetMaterial();
					Ref<VulkanImage> albedo = GPUResources::Get().GetTexture(material->AlbedoImage);

					//bind a texture
					VkDescriptorSet imageSet = vk.GetCurrentFrame().FrameDescriptors.Allocate(vk.GetDevice(), ImageDescriptorLayout);
					{
						DescriptorWriter writer;
						writer.WriteImage(0, albedo->GetImageView(), SamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
						writer.UpdateSet(vk.GetDevice(), imageSet);
					}

					vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.renderPass->PipelineLayout, 3, 1, &imageSet, 0, nullptr);

					vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.renderPass->PipelineLayout, 4, 1, &SamplerDescriptor, 0, nullptr);

					modelPushConstant.Index = ModelMatrixMapping[entity.GetID()];
					modelPushConstant.MaterialIndex = MeshMaterialMapping[vkMesh->GetID()];

					vkCmdPushConstants(
						cmd,
						ctx.renderPass->PipelineLayout,
						VK_SHADER_STAGE_ALL_GRAPHICS,			// Stage matching the pipeline layout
						0,									// Offset
						sizeof(ModelPushConstant),          // Size of the push constant
						&modelPushConstant                  // Pointer to the value
					);

					vkCmdBindIndexBuffer(cmd, vkMesh->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(cmd, vkMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t), 1, 0, 0, 0);
				}
			}
		}
		
	});

	auto& shadingPass = GBufferPipeline.AddPass("Shading");

	shadingPass.AddAttachment("Output", ImageFormat::RGBA16F);


	GBufferPipeline.Build();
}

void VkSceneRenderer::SetGBufferSize(const Vector2& size)
{
	Context.Size = size;
}

void VkSceneRenderer::UpdateCameraData(const CameraData& data)
{
	CameraData adjustedData = data;
	adjustedData.View = data.View;
	adjustedData.Projection = data.Projection;

	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (VkRenderer::Get().GetCurrentFrame().CameraStagingBuffer->GetAllocation()), &mappedData);
	memcpy(mappedData, &adjustedData, sizeof(CameraData));

	VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
		VkBufferCopy copy{ 0 };
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = sizeof(CameraData);

		vkCmdCopyBuffer(cmd, VkRenderer::Get().GetCurrentFrame().CameraStagingBuffer->GetBuffer(), CameraBuffer->GetBuffer(), 1, &copy);
	});

	vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), VkRenderer::Get().GetCurrentFrame().CameraStagingBuffer->GetAllocation());
}

void VkSceneRenderer::BuildMatrixBuffer()
{
	// This will scan and build the matrix buffer for the next frame.
	// It will create a mapping between UUID and the corresponding index for the model matrix
	ZoneScopedN("Build Matrix Buffer");
	auto& scene = Context.CurrentScene;

	std::array<Matrix4, MAX_MODEL_MATRIX> allTransforms;
	std::array<MaterialBufferStruct, MAX_MATERIAL> allMaterials;

	uint32_t currentIndex = 0;
	uint32_t currentMaterialIndex = 0;
	auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
	for (auto e : view)
	{
		// Check if we've reached the maximum capacity of the array
		if (currentIndex >= MAX_MODEL_MATRIX)
		{
			assert(false);
			break;
		}

		auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);

		if (!mesh.ModelResource || !visibility.Visible)
		{
			continue;
		}

		allTransforms[currentIndex] = transform.GetGlobalTransform();

		Entity entity = Entity((entt::entity)e, scene.get());
		ModelMatrixMapping[entity.GetID()] = currentIndex;

		for (auto& m : mesh.ModelResource->GetMeshes())
		{
			Ref<Material> material = m->GetMaterial();
			if (!material)
			{
				continue;
			}

			// TODO: Avoid duplicated materials
			MaterialBufferStruct materialBuffer = {};
			materialBuffer.hasAlbedo = material->HasAlbedo();
			materialBuffer.albedo = material->data.m_AlbedoColor;
			materialBuffer.hasNormal = material->HasNormal();
			materialBuffer.hasMetalness = material->HasMetalness();
			materialBuffer.metalnessValue = material->data.u_MetalnessValue;
			materialBuffer.hasAO = material->HasAO();
			materialBuffer.aoValue = material->data.u_AOValue;
			materialBuffer.hasRoughness = material->HasRoughness();
			materialBuffer.roughnessValue = material->data.u_RoughnessValue;
			allMaterials[currentMaterialIndex] = materialBuffer;
			MeshMaterialMapping[m->GetVkMesh()->GetID()] = currentMaterialIndex;

			currentMaterialIndex++;
		}

		currentIndex++;
	}

	ModelTransforms = ModelData{ allTransforms };
	MaterialDataContainer = MaterialData{ allMaterials };
}

void VkSceneRenderer::UpdateTransformBuffer()
{
	// Update tranform buffer
	{
		void* mappedData;
		vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (VkRenderer::Get().GetCurrentFrame().ModelStagingBuffer->GetAllocation()), &mappedData);
		memcpy(mappedData, &ModelTransforms, sizeof(ModelData));

		VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
			VkBufferCopy copy{ 0 };
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = sizeof(ModelData);

			vkCmdCopyBuffer(cmd, VkRenderer::Get().GetCurrentFrame().ModelStagingBuffer->GetBuffer(), ModelBuffer->GetBuffer(), 1, &copy);
		});

		vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), VkRenderer::Get().GetCurrentFrame().ModelStagingBuffer->GetAllocation());

		// Update descriptor set for camera
		VkDescriptorBufferInfo transformBufferInfo{};
		transformBufferInfo.buffer = ModelBuffer->GetBuffer();
		transformBufferInfo.offset = 0;
		transformBufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet bufferWriteModel = {};
		bufferWriteModel.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWriteModel.pNext = nullptr;
		bufferWriteModel.dstBinding = 0;
		bufferWriteModel.dstSet = ModelBufferDescriptor;
		bufferWriteModel.descriptorCount = 1;
		bufferWriteModel.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bufferWriteModel.pBufferInfo = &transformBufferInfo;
		vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWriteModel, 0, nullptr);
	}

	// Update material buffer
	{
		void* mappedData;
		vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (VkRenderer::Get().GetCurrentFrame().MaterialStagingBuffer->GetAllocation()), &mappedData);
		memcpy(mappedData, &MaterialDataContainer, sizeof(MaterialData));

		VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
			VkBufferCopy copy{ 0 };
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = sizeof(MaterialData);

			vkCmdCopyBuffer(cmd, VkRenderer::Get().GetCurrentFrame().MaterialStagingBuffer->GetBuffer(), MaterialBuffer->GetBuffer(), 1, &copy);
		});

		vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), VkRenderer::Get().GetCurrentFrame().MaterialStagingBuffer->GetAllocation());

		// Update descriptor set for camera
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = MaterialBuffer->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet bufferWrite = {};
		bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWrite.pNext = nullptr;
		bufferWrite.dstBinding = 0;
		bufferWrite.dstSet = MaterialBufferDescriptor;
		bufferWrite.descriptorCount = 1;
		bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bufferWrite.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWrite, 0, nullptr);
	}
}