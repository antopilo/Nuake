#include "VulkanRenderer.h"

#include "src/Core/Logger.h"
#include "ShaderCompiler.h"
#include "src/Rendering/RenderCommand.h"
#include "VulkanShader.h"

#include "src/Window.h"

#include "src/Resource/StaticResources.h"
#include "VulkanInit.h"
#include "VulkanAllocator.h"

#include "VulkanCheck.h"
#include "PipelineBuilder.h"
#include "VulkanAllocatedBuffer.h"

#include "VkResources.h"

#include "src/Rendering/Vertex.h"
#include "VulkanSceneRenderer.h"

#include "DescriptorLayoutBuilder.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include "GLFW/glfw3.h"

#include "vk_mem_alloc.h"

#include <array>

bool NKUseValidationLayer = true;

using namespace Nuake;

VkRenderer::~VkRenderer()
{
	CleanUp();
}

void VkRenderer::Initialize()
{
	VkResult result = volkInitialize();
	if (result != VK_SUCCESS)
	{
		Logger::Log("Volk failed to initialize", "vulkan", CRITICAL);
	}

	GetInstance();

	volkLoadInstance(Instance);

	// Create window surface
	const auto glfwHandle = Window::Get()->GetHandle();
	result = glfwCreateWindowSurface(Instance, glfwHandle, NULL, &Surface);
	if (result != VK_SUCCESS)
	{
		Logger::Log("Failed to create Vulkan window surface", "vulkan", CRITICAL);
	}

	SelectGPU();
	
	// Initialize allocator
	VulkanAllocator::Get().Initialize(Instance, GPU, Device);

	// TODO: Find a better way to handle deletion
	//MainDeletionQueue.push_function([&]() {
	//	vmaDestroyAllocator(allocator.GetAllocator());
	//});

	CreateSwapchain(Window::Get()->GetSize());

	// Now lets grab the Queues
	GPUQueue = VkbDevice.get_queue(vkb::QueueType::graphics).value();
	GPUQueueFamily = VkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	InitCommands();

	InitSync();

	ShaderCompiler& shaderCompiler = ShaderCompiler::Get();
	TriangleVertShader = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/triangle.vert");
	BackgroundShader = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/background.comp");
	TriangleFragShader = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/triangle.frag");

	std::vector<Vertex> rect_vertices;
	rect_vertices.resize(4);

	rect_vertices[0].position = {  1.0f, -1.0f, 0 };
	rect_vertices[1].position = {  1.0f,  1.0f, 0 };
	rect_vertices[2].position = { -1.0f, -1.0f, 0 };
	rect_vertices[3].position = { -1.0f,  1.0f, 0 };

	rect_vertices[0].normal = { 0, 0, 1 };
	rect_vertices[1].normal = { 0.5, 0.5,0.5 };
	rect_vertices[2].normal = { 1.0, 0.0, 1.0f };
	rect_vertices[3].normal = { 0, 1,0 };

	std::vector<uint32_t> rect_indices;
	rect_indices.resize(6);

	rect_indices[0] = 0;
	rect_indices[1] = 1;
	rect_indices[2] = 2;

	rect_indices[3] = 2;
	rect_indices[4] = 1;
	rect_indices[5] = 3;

	// Init global pool
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 8 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 50 }
	};

	GlobalDescriptorAllocator.InitPool(Device, 1000, sizes);


	GPUResources& resources = GPUResources::Get();
	rectangle = resources.CreateMesh(rect_vertices, rect_indices);

	CameraData camData{};
	camData.View = Matrix4(1.0f);
	camData.Projection = Matrix4(1.0f);

	// init camera buffer
	CameraBuffer = resources.CreateBuffer(sizeof(CameraData), BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "CameraBuffer");
	UploadCameraData(camData);

	InitDescriptors();

	InitPipeline();
	InitTrianglePipeline();

	InitImgui();

	SceneRenderer = CreateRef<VkSceneRenderer>();
	SceneRenderer->Init();

	IsInitialized = true;
}

void VkRenderer::CleanUp()
{
	if (!IsInitialized)
	{
		return;
	}

	// Wait for GPU to finish the submitted commands.
	vkDeviceWaitIdle(Device);

	// Destroy command pools, we cannot destroy command buffers
	// Destroy sync
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		vkDestroyCommandPool(Device, Frames[i].CommandPool, nullptr);

		//destroy sync objects
		vkDestroyFence(Device, Frames[i].RenderFence, nullptr);
		vkDestroySemaphore(Device, Frames[i].RenderSemaphore, nullptr);
		vkDestroySemaphore(Device, Frames[i].SwapchainSemaphore, nullptr);

		Frames[i].LocalDeletionQueue.flush();
	}
	
	MainDeletionQueue.flush();

	DestroySwapchain();

	vkDestroySurfaceKHR(Instance, Surface, nullptr);
	vkDestroyDevice(Device, nullptr);

	vkb::destroy_debug_utils_messenger(Instance, VkDebugMessenger);
	vkDestroyInstance(Instance, nullptr);
}


void VkRenderer::GetInstance()
{
	vkb::InstanceBuilder builder;

	//make the vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Nuake Engine")
		.request_validation_layers(NKUseValidationLayer)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	VkbInstance = inst_ret.value();
	Instance = VkbInstance.instance;
	VkDebugMessenger = VkbInstance.debug_messenger;
}

void VkRenderer::SelectGPU()
{
	VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;
	VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

	std::vector<const char*> requiredExtensions = { VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };


	vkb::PhysicalDeviceSelector selector{ VkbInstance };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 3)
		.set_required_features_13(features)
		.set_required_features_12(features12)
		.set_surface(Surface)
		.add_required_extensions(requiredExtensions)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	VkbDevice = deviceBuilder.build().value();
	Device = VkbDevice.device;
	GPU = physicalDevice.physical_device;
}

void VkRenderer::RecreateSwapchain()
{
	vkQueueWaitIdle(GPUQueue);

	DestroySwapchain();
	CreateSwapchain(Window::Get()->GetSize());
	UpdateDescriptorSets();

	// Update sceneRenderer
	SceneRenderer->SetGBufferSize(Window::Get()->GetSize());
}

void VkRenderer::CreateSwapchain(const Vector2& size)
{
	if (size.x == 0.0f || size.y == 0.0f)
	{
		return;
	}

	vkb::SwapchainBuilder swapchainBuilder{ GPU, Device, Surface};

	SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ .format = SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
		.set_desired_extent(static_cast<int>(size.x), static_cast<int>(size.y))
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();

	SwapchainExtent = vkbSwapchain.extent;
	//store swapchain and its related images
	DrawExtent = VkExtent2D{ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };
	Swapchain = vkbSwapchain.swapchain;
	SwapchainImages = vkbSwapchain.get_images().value();
	SwapchainImageViews = vkbSwapchain.get_image_views().value();

	SurfaceSize = size;

	//draw image size will match the window
	VkExtent3D drawImageExtent = {
		size.x,
		size.y,
		1
	};

	DrawImage = CreateRef<VulkanImage>(ImageFormat::RGBA16F, size);
	DepthImage = CreateRef<VulkanImage>(ImageFormat::D32F, size, ImageUsage::Depth);
}

void VkRenderer::DestroySwapchain()
{
	vkDestroySwapchainKHR(Device, Swapchain, nullptr);

	// Destroy swapchain resources
	for (int i = 0; i < SwapchainImageViews.size(); i++)
	{
		vkDestroyImageView(Device, SwapchainImageViews[i], nullptr);
	}
}

void VkRenderer::InitCommands()
{
	VkCommandPoolCreateInfo cmdPoolInfo = VulkanInit::CommandPoolCreateInfo(GPUQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; i++) {

		vkCreateCommandPool(Device, &cmdPoolInfo, nullptr, &Frames[i].CommandPool);

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = VulkanInit::CommandBufferAllocateInfo(Frames[i].CommandPool, 1);

		VK_CALL(vkAllocateCommandBuffers(Device, &cmdAllocInfo, &Frames[i].CommandBuffer));

		GPUResources& resources = GPUResources::Get();
		Frames[i].CameraStagingBuffer = resources.CreateBuffer(sizeof(CameraData), BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_ONLY, "CameraStaging" + std::to_string(i) );
		Frames[i].ModelStagingBuffer = resources.CreateBuffer(sizeof(Matrix4) * MAX_MODEL_MATRIX, BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_ONLY, "TransformStaging" + std::to_string(i));
		Frames[i].MaterialStagingBuffer = resources.CreateBuffer(sizeof(MaterialBufferStruct) * MAX_MATERIAL, BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_ONLY, "MaterialStaging" + std::to_string(i));
	}

	VK_CALL(vkCreateCommandPool(Device, &cmdPoolInfo, nullptr, &ImguiCommandPool));

	// allocate the command buffer for immediate submits
	VkCommandBufferAllocateInfo cmdAllocInfo = VulkanInit::CommandBufferAllocateInfo(ImguiCommandPool, 1);

	VK_CALL(vkAllocateCommandBuffers(Device, &cmdAllocInfo, &ImguiCommandBuffer));

	MainDeletionQueue.push_function([=]() {
		vkDestroyCommandPool(Device, ImguiCommandPool, nullptr);
	});
}

void VkRenderer::InitSync()
{
	// create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = VulkanInit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = VulkanInit::SemaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{
		VK_CALL(vkCreateFence(Device, &fenceCreateInfo, nullptr, &Frames[i].RenderFence));

		VK_CALL(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &Frames[i].SwapchainSemaphore));
		VK_CALL(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &Frames[i].RenderSemaphore));
	}

	VK_CALL(vkCreateFence(Device, &fenceCreateInfo, nullptr, &ImguiFence));
	MainDeletionQueue.push_function([=]() { vkDestroyFence(Device, ImguiFence, nullptr); });
}

void VkRenderer::InitDescriptors()
{
	//create a descriptor pool that will hold 10 sets with 1 image each

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		DrawImageDescriptorLayout = builder.Build(Device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	// Camera buffer
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		CameraBufferDescriptorLayout = builder.Build(Device, VK_SHADER_STAGE_VERTEX_BIT);
	}

	// Triangle vertex buffer layout
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		TriangleBufferDescriptorLayout = builder.Build(Device, VK_SHADER_STAGE_ALL);
	}

	DrawImageDescriptors = GlobalDescriptorAllocator.Allocate(Device, DrawImageDescriptorLayout);
	TriangleBufferDescriptors = GlobalDescriptorAllocator.Allocate(Device, TriangleBufferDescriptorLayout);
	CameraBufferDescriptors = GlobalDescriptorAllocator.Allocate(Device, CameraBufferDescriptorLayout);

	UpdateDescriptorSets();

	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{
		// create a descriptor pool
		std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = 
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};

		Frames[i].FrameDescriptors = DescriptorAllocatorGrowable{};
		Frames[i].FrameDescriptors.Init(Device, 1000, frame_sizes);
	}
}

void VkRenderer::UpdateDescriptorSets()
{
	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = DrawImage->GetImageView();

	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;

	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = DrawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;
	vkUpdateDescriptorSets(Device, 1, &drawImageWrite, 0, nullptr);

	// Update descriptor set for cameras
	//VkDescriptorBufferInfo camBufferInfo{};
	//camBufferInfo.buffer = CameraBuffer->GetBuffer();
	//camBufferInfo.offset = 0;
	//camBufferInfo.range = VK_WHOLE_SIZE;
	//
	//VkWriteDescriptorSet bufferWriteCam = {};
	//bufferWriteCam.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//bufferWriteCam.pNext = nullptr;
	//bufferWriteCam.dstBinding = 0;
	//bufferWriteCam.dstSet = CameraBufferDescriptors;
	//bufferWriteCam.descriptorCount = 1;
	//bufferWriteCam.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	//bufferWriteCam.pBufferInfo = &camBufferInfo;
	//vkUpdateDescriptorSets(Device, 1, &bufferWriteCam, 0, nullptr);
	//
	//// Update descriptor set for TriangleBufferDescriptors
	//VkDescriptorBufferInfo bufferInfo{};
	//bufferInfo.buffer = rectangle->GetVertexBuffer()->GetBuffer();
	//bufferInfo.offset = 0;
	//bufferInfo.range = VK_WHOLE_SIZE;
	//
	//VkWriteDescriptorSet bufferWrite = {};
	//bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//bufferWrite.pNext = nullptr;
	//bufferWrite.dstBinding = 0;
	//bufferWrite.dstSet = TriangleBufferDescriptors;
	//bufferWrite.descriptorCount = 1;
	//bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	//bufferWrite.pBufferInfo = &bufferInfo;
	//vkUpdateDescriptorSets(Device, 1, &bufferWrite, 0, nullptr);
}

void VkRenderer::InitPipeline()
{
	InitBackgroundPipeline();
}

void VkRenderer::InitBackgroundPipeline()
{
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &DrawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VK_CALL(vkCreatePipelineLayout(Device, &computeLayout, nullptr, &PipelineLayout));

	VkPipelineShaderStageCreateInfo stageinfo{};
	stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageinfo.pNext = nullptr;
	stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageinfo.module = BackgroundShader->GetModule();
	stageinfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = PipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	VK_CALL(vkCreateComputePipelines(Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &Pipeline));

	MainDeletionQueue.push_function([&]() {
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
		vkDestroyPipeline(Device, Pipeline, nullptr);
	});
}

void VkRenderer::InitTrianglePipeline()
{
	VkPushConstantRange bufferRange{};
	bufferRange.offset = 0;
	bufferRange.size = sizeof(GPUDrawPushConstants);
	bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayout layouts[] = { CameraBufferDescriptorLayout, TriangleBufferDescriptorLayout };

	VkPipelineLayoutCreateInfo pipeline_layout_info = VulkanInit::PipelineLayoutCreateInfo();
	pipeline_layout_info.pPushConstantRanges = &bufferRange;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pSetLayouts = layouts;
	pipeline_layout_info.setLayoutCount = 2;
	VK_CALL(vkCreatePipelineLayout(Device, &pipeline_layout_info, nullptr, &TrianglePipelineLayout));

	//use the triangle layout we created
	PipelineBuilder pipelineBuilder;
	pipelineBuilder.PipelineLayout = TrianglePipelineLayout;
	pipelineBuilder.SetShaders(TriangleVertShader->GetModule(), TriangleFragShader->GetModule());
	pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.SetMultiSamplingNone();
	pipelineBuilder.DisableBlending();
	pipelineBuilder.DisableDepthTest();
	pipelineBuilder.SetColorAttachment(static_cast<VkFormat>(DrawImage->GetFormat()));
	pipelineBuilder.SetDepthFormat(VK_FORMAT_UNDEFINED);
	TrianglePipeline = pipelineBuilder.BuildPipeline(Device);

	MainDeletionQueue.push_function([&]() {
		vkDestroyPipelineLayout(Device, TrianglePipelineLayout, nullptr);
		vkDestroyPipeline(Device, TrianglePipeline, nullptr);
	});
}

void VkRenderer::DrawScene(RenderContext ctx)
{
	SceneRenderer->BeginScene(ctx);
	SceneRenderer->DrawScene();
	SceneRenderer->EndScene();
}


void VkRenderer::DrawGeometry(VkCommandBuffer cmd)
{
	//begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo colorAttachment = VulkanInit::AttachmentInfo(DrawImage->GetImageView(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


	//VkRenderingInfo renderInfo = VulkanInit::RenderingInfo({ DrawExtent.width, DrawExtent.height }, &colorAttachment, nullptr);
	//vkCmdBeginRendering(cmd, &renderInfo);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TrianglePipeline);
	
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TrianglePipelineLayout, 0, 2, std::array{ CameraBufferDescriptors, TriangleBufferDescriptors }.data(), 0, nullptr);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = DrawExtent.width;
	viewport.height = DrawExtent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = DrawExtent.width;
	scissor.extent.height = DrawExtent.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);



	vkCmdBindIndexBuffer(cmd, rectangle->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
	//launch a draw command to draw 3 vertices
	//vkCmdDraw(cmd, 3, 1, 0, 0);

	vkCmdEndRendering(cmd);
}

void VkRenderer::InitImgui()
{
	// 1: create descriptor pool for IMGUI
	//  the size of the pool is very oversize, but it's copied from imgui demo
	//  itself.
	VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	VK_CALL(vkCreateDescriptorPool(Device, &pool_info, nullptr, &imguiPool));

	// 2: initialize imgui library

	// this initializes the core structures of imgui
	ImGui::CreateContext();

	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->AddFontFromMemoryTTF(StaticResources::Resources_Fonts_Poppins_Regular_ttf, StaticResources::Resources_Fonts_Poppins_Regular_ttf_len, 16.0);

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		ImGui::StyleColorsDark();

		ImGuiStyle& s = ImGui::GetStyle();
		s.WindowMenuButtonPosition = ImGuiDir_None;
		s.GrabRounding = 2.0f;
		s.CellPadding = ImVec2(8, 8);
		s.WindowPadding = ImVec2(4, 4);
		s.ScrollbarRounding = 9.0f;
		s.ScrollbarSize = 15.0f;
		s.GrabMinSize = 32.0f;
		s.TabRounding = 0;
		s.WindowRounding = 4.0f;
		s.ChildRounding = 4.0f;
		s.FrameRounding = 4.0f;
		s.GrabRounding = 0;
		s.FramePadding = ImVec2(8, 4);
		s.ItemSpacing = ImVec2(8, 4);
		s.ItemInnerSpacing = ImVec2(4, 4);
		s.TabRounding = 4.0f;
		s.WindowBorderSize = 0.0f;
		s.IndentSpacing = 12.0f;
		s.ChildBorderSize = 0.0f;
		s.PopupRounding = 4.0f;
		s.FrameBorderSize = 0.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.078f, 0.078f, 0.078f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.078f, 0.078f, 0.078f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.93f, 0.27f, 0.27f, 0.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.08f, 0.49f, 0.97f, 0.28f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	auto load_vk_func = [&](const char* fn) {
		if (auto proc = vkGetDeviceProcAddr(Device, fn)) return proc;
		return vkGetInstanceProcAddr(Instance, fn);
		};
	ImGui_ImplVulkan_LoadFunctions([](const char* fn, void* data) {
		return (*(decltype(load_vk_func)*)data)(fn);
	}, &load_vk_func);

	// this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(Window::Get()->GetHandle(), true);

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Instance;
	init_info.PhysicalDevice = GPU;
	init_info.Device = Device;
	init_info.Queue = GPUQueue;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.UseDynamicRendering = true;

	//dynamic rendering parameters for imgui to use
	init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &SwapchainImageFormat;

	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info);

	ImGui_ImplVulkan_CreateFontsTexture();

	// add the destroy the imgui created structures
	MainDeletionQueue.push_function([=]() {
		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(Device, imguiPool, nullptr);
	});
}

void VkRenderer::BeginScene(const Matrix4& view, const Matrix4& projection)
{
	CameraData newData = { view, projection };
	//UploadCameraData(newData);
	SceneRenderer->UpdateCameraData(newData);
}


bool VkRenderer::Draw()
{
	VK_CALL(vkWaitForFences(Device, 1, &GetCurrentFrame().RenderFence, true, 1000000000));

	if (SurfaceSize != Window::Get()->GetSize())
	{
		RecreateSwapchain();
		FrameSkipped = true;
		return false;
	}

	FrameSkipped = false;

	GetCurrentFrame().FrameDescriptors.ClearPools(Device);

	//request image from the swapchain
	VkResult result = vkAcquireNextImageKHR(Device, Swapchain, 1000000000, GetCurrentFrame().SwapchainSemaphore, nullptr, &swapchainImageIndex);
	
	VK_CALL(vkResetFences(Device, 1, &GetCurrentFrame().RenderFence));

	// Note: this will be the meat of the engine that should be here.
	VkCommandBuffer cmd = GetCurrentFrame().CommandBuffer;
	VK_CALL(vkResetCommandBuffer(cmd, 0));

	// Begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	// Note: We might reuse them later!!!
	VkCommandBufferBeginInfo cmdBeginInfo = VulkanInit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	DrawExtent.width = DrawImage->GetSize().x;
	DrawExtent.height = DrawImage->GetSize().y;

	// Create commands
	VK_CALL(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
	// Transfer rendering image to general layout
	VulkanUtil::TransitionImage(cmd, DrawImage->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// Execute compute shader that writes to the image
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout, 0, 1, &DrawImageDescriptors, 0, nullptr);

	VulkanUtil::TransitionImage(cmd, DrawImage->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	//vkCmdDispatch(cmd, std::ceil(DrawExtent.width / 16.0), std::ceil(DrawExtent.height / 16.0), 1);
	//DrawBackground(cmd);

	// Transition rendering iamge to transfert onto swapchain images
	//VulkanUtil::TransitionImage(cmd, DrawImage->GetImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanUtil::TransitionImage(cmd, DepthImage->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	//DrawGeometry(cmd);

	return true;
}

void VkRenderer::EndDraw()
{
	if (FrameSkipped)
	{
		return;
	}

	VkCommandBuffer cmd = GetCurrentFrame().CommandBuffer;
	VulkanUtil::TransitionImage(cmd, DrawImage->GetImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	//draw imgui into the swapchain image
	DrawImgui(cmd, SwapchainImageViews[swapchainImageIndex]);

	// set swapchain image layout to Attachment Optimal so we can draw it
	VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	// set swapchain image layout to Present so we can draw it
	VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Transition the swapchain image to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presentation
	VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	VK_CALL(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = VulkanInit::CommandBufferSubmitInfo(cmd);

	// Wait for both semaphore of swapchain and the texture of the frame.
	VkSemaphoreSubmitInfo waitInfo = VulkanInit::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().SwapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = VulkanInit::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().RenderSemaphore);
	VkSubmitInfo2 submit = VulkanInit::SubmitInfo(&cmdinfo, &signalInfo, &waitInfo);

	// Submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CALL(vkQueueSubmit2(GPUQueue, 1, &submit, GetCurrentFrame().RenderFence));

	// Prepare present
	// This will put the image we just rendered to into the visible window.
	// We want to wait on the _renderSemaphore for that, 
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &Swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &GetCurrentFrame().RenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CALL(vkQueuePresentKHR(GPUQueue, &presentInfo));

	// Increase the number of frames drawn
	FrameNumber++;
}

void VkRenderer::DrawBackground(VkCommandBuffer cmd)
{
	// This works
	VkClearColorValue clearValue;
	//float flash = std::abs(std::sin(FrameNumber / 120.f));
	clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	VkImageSubresourceRange clearRange = VulkanInit::ImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
	vkCmdClearColorImage(cmd, DrawImage->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

	// This doesnt!!
	//vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
	//vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout, 0, 1, &DrawImageDescriptors, 0, nullptr);
	//vkCmdDispatch(cmd, std::ceil(DrawExtent.width / 16.0), std::ceil(DrawExtent.height / 16.0), 1);
}

void VkRenderer::DrawImgui(VkCommandBuffer cmd, VkImageView targetImageView)
{
	VkRenderingAttachmentInfo colorAttachment = VulkanInit::AttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	std::vector<VkRenderingAttachmentInfo> attachments = { colorAttachment };
	VkRenderingInfo renderInfo = VulkanInit::RenderingInfo({ SwapchainExtent.width, SwapchainExtent.height }, attachments, nullptr);

	vkCmdBeginRendering(cmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vkCmdEndRendering(cmd);
}

void VkRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VK_CALL(vkResetFences(Device, 1, &ImguiFence));
	VK_CALL(vkResetCommandBuffer(ImguiCommandBuffer, 0));

	VkCommandBuffer cmd = ImguiCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = VulkanInit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CALL(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CALL(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = VulkanInit::CommandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = VulkanInit::SubmitInfo(&cmdinfo, nullptr, nullptr);

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CALL(vkQueueSubmit2(GPUQueue, 1, &submit, ImguiFence));

	VK_CALL(vkWaitForFences(Device, 1, &ImguiFence, true, 9999999999));
}

void VkRenderer::UploadCameraData(const CameraData& data)
{
	CameraData adjustedData = data;
	adjustedData.View = Matrix4(1.0f); //data.View;
	adjustedData.View = data.View;
	adjustedData.Projection = glm::perspective(glm::radians(70.f), (float)DrawExtent.width / (float)DrawExtent.height, 0.0001f, 10000.0f);
	//adjustedData.Projection[1][1] *= -1;

	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (GetCurrentFrame().CameraStagingBuffer->GetAllocation()), &mappedData);
	memcpy(mappedData, &adjustedData, sizeof(CameraData));

	ImmediateSubmit([&](VkCommandBuffer cmd) {
		VkBufferCopy copy{ 0 };
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = sizeof(CameraData);
	
		vkCmdCopyBuffer(cmd, GetCurrentFrame().CameraStagingBuffer->GetBuffer(), CameraBuffer->GetBuffer(), 1, &copy);
	});

	vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), GetCurrentFrame().CameraStagingBuffer->GetAllocation());
}



void DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSizeRatio ratio : poolRatios) 
	{
		poolSizes.push_back(
			VkDescriptorPoolSize
			{
				.type = ratio.type,
				.descriptorCount = uint32_t(ratio.ratio * maxSets)
			}
		);
	}

	VkDescriptorPoolCreateInfo pool_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	pool_info.flags = 0;
	pool_info.maxSets = maxSets;
	pool_info.poolSizeCount = (uint32_t)poolSizes.size();
	pool_info.pPoolSizes = poolSizes.data();

	VK_CALL(vkCreateDescriptorPool(device, &pool_info, nullptr, &pool));
}

void DescriptorAllocator::ClearDescriptors(VkDevice device)
{
	vkResetDescriptorPool(device, pool, 0);
}

void DescriptorAllocator::DestroyPool(VkDevice device)
{
	vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout)
{
	VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet ds;
	VK_CALL(vkAllocateDescriptorSets(device, &allocInfo, &ds));

	return ds;
}
