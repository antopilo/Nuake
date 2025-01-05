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
	CreateBasicPipeline();

	ModelMatrixMapping.clear();
	MeshMaterialMapping.clear();
}

void VkSceneRenderer::BeginScene(RenderContext inContext)
{
	Context = inContext;

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
	


	// Ensure the pipeline is valid
	if (BasicPipeline == VK_NULL_HANDLE) {
		throw std::runtime_error("Basic pipeline is not initialized");
	}

	VkClearColorValue clearValue;
	//float flash = std::abs(std::sin(FrameNumber / 120.f));
	clearValue = { { 0.0f, 1.0f, 0.0f, 1.0f } };
	VkImageSubresourceRange clearRange = VulkanInit::ImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
	vkCmdClearColorImage(cmd, GBufferAlbedo->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	vkCmdClearColorImage(cmd, GBufferNormal->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	vkCmdClearColorImage(cmd, GBufferMaterial->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	//VkClearDepthStencilValue clearDepth = { 0.0f, 0 };
	//clearRange = VulkanInit::ImageSubResourceRange(VK_IMAGE_ASPECT_DEPTH_BIT);
	//vkCmdClearDepthStencilImage(cmd, GBufferDepthImage->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearDepth, 1, &clearRange);

	// Optionally, clear the image if you need to reset its contents
	VkClearDepthStencilValue clearValue2 = {};
	clearValue2.depth = 1.0f; // Depth to clear to
	clearValue2.stencil = 0; // Stencil to clear to

	VkImageSubresourceRange range = {};
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	//vkCmdClearDepthStencilImage(cmd, GBufferDepthImage->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue2, 1, &range);

	VulkanUtil::TransitionImage(cmd, GBufferNormal->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, GBufferMaterial->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, GBufferAlbedo->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, GBufferDepthImage->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo colorAttachment = VulkanInit::AttachmentInfo(GBufferAlbedo->GetImageView(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo normalAttachment = VulkanInit::AttachmentInfo(GBufferNormal->GetImageView(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo materialAttachment = VulkanInit::AttachmentInfo(GBufferMaterial->GetImageView(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depthAttachment = VulkanInit::DepthAttachmentInfo(GBufferDepthImage->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	std::vector<VkRenderingAttachmentInfo> attachments = { colorAttachment, normalAttachment, materialAttachment };
	VkRenderingInfo renderInfo = VulkanInit::RenderingInfo(GBufferAlbedo->GetSize(), attachments, &depthAttachment);
	renderInfo.colorAttachmentCount = std::size(attachments);
	renderInfo.pColorAttachments = attachments.data();
	vkCmdBeginRendering(cmd, &renderInfo);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, BasicPipeline);
	
	std::vector<VkDescriptorSet> descriptors = { CameraBufferDescriptors, ModelBufferDescriptor };
	// Bind camera settings
	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		BasicPipelineLayout,
		0,                               // firstSet
		2,                               // descriptorSetCount
		descriptors.data(),        // pointer to the descriptor set(s)
		0,                               // dynamicOffsetCount
		nullptr                          // dynamicOffsets
	);

	// Bind material
	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		BasicPipelineLayout,
		5,                               // firstSet
		1,                               // descriptorSetCount
		&MaterialBufferDescriptor,        // pointer to the descriptor set(s)
		0,                               // dynamicOffsetCount
		nullptr                          // dynamicOffsets
	);
	
	// Set viewport
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = GBufferAlbedo->GetWidth();
	viewport.height = GBufferAlbedo->GetHeight();
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	
	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = GBufferAlbedo->GetWidth();
	scissor.extent.height = GBufferAlbedo->GetWidth();
	vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void VkSceneRenderer::DrawScene()
{
	ZoneScoped;

	auto& cmd = Context.CommandBuffer;
	auto& scene = Context.CurrentScene;
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
					BasicPipelineLayout,
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

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, BasicPipelineLayout, 3, 1, &imageSet, 0, nullptr);

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, BasicPipelineLayout, 4, 1, &SamplerDescriptor, 0, nullptr);

				ModelPushConstant modelPushConstant{};
				modelPushConstant.Index = ModelMatrixMapping[entity.GetID()];
				modelPushConstant.MaterialIndex = MeshMaterialMapping[vkMesh->GetID()];

				vkCmdPushConstants(
					cmd,
					BasicPipelineLayout,
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

	// Quake
	{

	}
}

void VkSceneRenderer::EndScene()
{
	auto& vk = VkRenderer::Get();
	auto& cmd = Context.CommandBuffer;

	vkCmdEndRendering(Context.CommandBuffer);

	VulkanUtil::TransitionImage(cmd, GBufferAlbedo->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, GBufferNormal->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, GBufferMaterial->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, vk.DrawImage->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanUtil::CopyImageToImage(cmd, GBufferMaterial->GetImage(), vk.GetDrawImage()->GetImage(), GBufferAlbedo->GetSize(), vk.DrawImage->GetSize());
	VulkanUtil::TransitionImage(cmd, vk.DrawImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	VulkanUtil::TransitionImage(cmd, GBufferMaterial->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	VulkanUtil::TransitionImage(cmd, GBufferNormal->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	VulkanUtil::TransitionImage(cmd, GBufferAlbedo->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
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

void VkSceneRenderer::CreateBasicPipeline()
{
	VkPushConstantRange bufferRange{};
	bufferRange.offset = 0;
	auto sizeOfThing = sizeof(ModelPushConstant);
	bufferRange.size = sizeOfThing;
	bufferRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	VkDescriptorSetLayout layouts[] = { CameraBufferDescriptorLayout, ModelBufferDescriptorLayout, TriangleBufferDescriptorLayout, ImageDescriptorLayout, SamplerDescriptorLayout, MaterialBufferDescriptorLayout };

	VkPipelineLayoutCreateInfo pipeline_layout_info = VulkanInit::PipelineLayoutCreateInfo();
	pipeline_layout_info.pPushConstantRanges = &bufferRange;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pSetLayouts = layouts;
	pipeline_layout_info.setLayoutCount = 6;
	VK_CALL(vkCreatePipelineLayout(VkRenderer::Get().GetDevice(), &pipeline_layout_info, nullptr, &BasicPipelineLayout));

	//use the triangle layout we created
	PipelineBuilder pipelineBuilder;
	pipelineBuilder.PipelineLayout = BasicPipelineLayout;
	pipelineBuilder.SetShaders(Shaders["basic_vert"]->GetModule(), Shaders["basic_frag"]->GetModule());
	pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.SetMultiSamplingNone();
	pipelineBuilder.EnableBlendingAlphaBlend();	// Target 0
	pipelineBuilder.EnableBlendingAlphaBlend(); // Target 1
	pipelineBuilder.EnableBlendingAlphaBlend(); // Target 2
	std::vector<VkFormat> formats = { static_cast<VkFormat>(GBufferAlbedo->GetFormat()), static_cast<VkFormat>(GBufferNormal->GetFormat()), static_cast<VkFormat>(GBufferMaterial->GetFormat())};
	pipelineBuilder.SetColorAttachments(formats);
	pipelineBuilder.SetDepthFormat(static_cast<VkFormat>(GBufferDepthImage->GetFormat()));
	pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
	//pipelineBuilder.DisableDepthTest();
	BasicPipeline = pipelineBuilder.BuildPipeline(VkRenderer::Get().GetDevice());
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

void VkSceneRenderer::SetGBufferSize(const Vector2& size)
{
	GBufferAlbedo = CreateRef<VulkanImage>(ImageFormat::RGBA16F, size);
	GBufferDepthImage = CreateRef<VulkanImage>(ImageFormat::D32F, size, ImageUsage::Depth);
	GBufferNormal = CreateRef<VulkanImage>(ImageFormat::RGBA16F, size);
	GBufferMaterial = CreateRef<VulkanImage>(ImageFormat::RGBA8, size);
}

void VkSceneRenderer::UpdateCameraData(const CameraData& data)
{
	CameraData adjustedData = data;
	adjustedData.View = Matrix4(1.0f); //data.View;
	adjustedData.View = data.View;
	adjustedData.Projection = data.Projection;
	//adjustedData.Projection[1][1] *= -1;

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