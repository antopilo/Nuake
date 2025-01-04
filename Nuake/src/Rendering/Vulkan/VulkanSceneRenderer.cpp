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
	CreateDescriptors();
	CreateBasicPipeline();

	ModelMatrixMapping.clear();
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
	
	VkRenderingAttachmentInfo colorAttachment = VulkanInit::AttachmentInfo(vk.DrawImage->GetImageView(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depthAttachment = VulkanInit::DepthAttachmentInfo(vk.DepthImage->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = VulkanInit::RenderingInfo(vk.DrawExtent, &colorAttachment, &depthAttachment);
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

	// Set viewport
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = vk.DrawExtent.width;
	viewport.height = vk.DrawExtent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = vk.DrawExtent.width;
	scissor.extent.height = vk.DrawExtent.height;
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

				ModelPushConstant modelPushConstant{};
				modelPushConstant.Index = ModelMatrixMapping[entity.GetID()];
				vkCmdPushConstants(
					cmd,
					BasicPipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT, // Stage matching the pipeline layout
					0,                          // Offset
					sizeof(ModelPushConstant),           // Size of the push constant
					&modelPushConstant                 // Pointer to the value
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
	vkCmdEndRendering(Context.CommandBuffer);
}

void VkSceneRenderer::CreateBuffers()
{
	CameraData camData{};
	camData.View = Matrix4(1.0f);
	camData.Projection = Matrix4(1.0f);

	// init camera buffer
	GPUResources& resources = GPUResources::Get();
	CameraBuffer = resources.CreateBuffer(sizeof(CameraData), BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY);
	UpdateCameraData(camData);

	ModelBuffer = resources.CreateBuffer(sizeof(Matrix4) * MAX_MODEL_MATRIX, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY);
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
	bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayout layouts[] = { CameraBufferDescriptorLayout, ModelBufferDescriptorLayout, TriangleBufferDescriptorLayout };

	VkPipelineLayoutCreateInfo pipeline_layout_info = VulkanInit::PipelineLayoutCreateInfo();
	pipeline_layout_info.pPushConstantRanges = &bufferRange;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pSetLayouts = layouts;
	pipeline_layout_info.setLayoutCount = 3;
	VK_CALL(vkCreatePipelineLayout(VkRenderer::Get().GetDevice(), &pipeline_layout_info, nullptr, &BasicPipelineLayout));

	//use the triangle layout we created
	PipelineBuilder pipelineBuilder;
	pipelineBuilder.PipelineLayout = BasicPipelineLayout;
	pipelineBuilder.SetShaders(Shaders["basic_vert"]->GetModule(), Shaders["basic_frag"]->GetModule());
	pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.SetMultiSamplingNone();
	pipelineBuilder.EnableBlendingAlphaBlend();
	pipelineBuilder.SetColorAttachment(static_cast<VkFormat>(VkRenderer::Get().DrawImage->GetFormat()));
	pipelineBuilder.SetDepthFormat(static_cast<VkFormat>(VkRenderer::Get().DepthImage->GetFormat()));
	pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
	BasicPipeline = pipelineBuilder.BuildPipeline(VkRenderer::Get().GetDevice());
}

void VkSceneRenderer::CreateDescriptors()
{
	auto vk = VkRenderer::Get();
	auto device = vk.GetDevice();

	// Camera
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		CameraBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL);
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
		ModelBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL);
	}

	auto allocator = vk.GetDescriptorAllocator();
	TriangleBufferDescriptors = allocator.Allocate(device, TriangleBufferDescriptorLayout);
	CameraBufferDescriptors = allocator.Allocate(device, CameraBufferDescriptorLayout);
	ModelBufferDescriptor = allocator.Allocate(device, ModelBufferDescriptorLayout);

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

	uint32_t currentIndex = 0;
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

		currentIndex++;
	}

	ModelTransforms = ModelData{ allTransforms };
}

void VkSceneRenderer::UpdateTransformBuffer()
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