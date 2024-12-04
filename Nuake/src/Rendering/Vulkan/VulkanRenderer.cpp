#include "VulkanRenderer.h"

#include "src/Core/Logger.h"
#include "ShaderCompiler.h"
#include "src/Rendering/RenderCommand.h"
#include "VulkanShader.h"

#include "src/Window.h"
#include <GLFW/glfw3.h>


using namespace Nuake;


bool NKUseValidationLayer = true;

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
	
	// Pass the vulkanFunctions struct to the VMA allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = GPU;
	allocatorInfo.device = Device;
	allocatorInfo.instance = Instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocatorInfo.pVulkanFunctions = {
		allocatorInfo.pVulkanFunctions = new VmaVulkanFunctions{
			.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
			.vkGetDeviceProcAddr = vkGetDeviceProcAddr,
			.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
			.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
			.vkAllocateMemory = vkAllocateMemory,
			.vkFreeMemory = vkFreeMemory,
			.vkMapMemory = vkMapMemory,
			.vkUnmapMemory = vkUnmapMemory,
			.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
			.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
			.vkBindBufferMemory = vkBindBufferMemory,
			.vkBindImageMemory = vkBindImageMemory,
			.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
			.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
			.vkCreateBuffer = vkCreateBuffer,
			.vkDestroyBuffer = vkDestroyBuffer,
			.vkCreateImage = vkCreateImage,
			.vkDestroyImage = vkDestroyImage,
			.vkCmdCopyBuffer = vkCmdCopyBuffer,
			.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
			.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR,
			.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR,
			.vkBindImageMemory2KHR = vkBindImageMemory2KHR,
		}
	};

	vmaCreateAllocator(&allocatorInfo, &Allocator);

	MainDeletionQueue.push_function([&]() {
		vmaDestroyAllocator(Allocator);
	});

	CreateSwapchain(Window::Get()->GetSize());

	// Now lets grab the Queues
	GPUQueue = VkbDevice.get_queue(vkb::QueueType::graphics).value();
	GPUQueueFamily = VkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	InitCommands();

	InitSync();

	InitDescriptors();

	BackgroundShader = ShaderCompiler::Get().CompileShader("../Resources/Shaders/Vulkan/background.comp");

	InitPipeline();

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
	vkb::PhysicalDeviceSelector selector{ VkbInstance };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 3)
		.set_required_features_13(features)
		.set_required_features_12(features12)
		.set_surface(Surface)
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
}

void VkRenderer::CreateSwapchain(const Vector2& size)
{
	vkb::SwapchainBuilder swapchainBuilder{ GPU, Device, Surface};

	SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ .format = SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
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

	//hardcoding the draw format to 32 bit float
	DrawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	DrawImage.imageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = VulkanInit::ImageCreateInfo(DrawImage.imageFormat, drawImageUsages, drawImageExtent);

	//for the draw image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	VK_CALL(vmaCreateImage(Allocator, &rimg_info, &rimg_allocinfo, &DrawImage.image, &DrawImage.allocation, nullptr));

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo rview_info = VulkanInit::ImageviewCreateInfo(DrawImage.imageFormat, DrawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CALL(vkCreateImageView(Device, &rview_info, nullptr, &DrawImage.imageView));

	//add to deletion queues
	//ainDeletionQueue.push_function([=]() {
	//	vkDestroyImageView(Device, DrawImage.imageView, nullptr);
	//	vmaDestroyImage(Allocator, DrawImage.image, DrawImage.allocation);
	//);
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
	}
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
}

void VkRenderer::InitDescriptors()
{
	//create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	GlobalDescriptorAllocator.InitPool(Device, 1, sizes);

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		DrawImageDescriptorLayout = builder.Build(Device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	DrawImageDescriptors = GlobalDescriptorAllocator.Allocate(Device, DrawImageDescriptorLayout);

	UpdateDescriptorSets();
}

void VkRenderer::UpdateDescriptorSets()
{
	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = DrawImage.imageView;

	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;

	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = DrawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(Device, 1, &drawImageWrite, 0, nullptr);
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

void VkRenderer::Draw()
{
	VK_CALL(vkWaitForFences(Device, 1, &GetCurrentFrame().RenderFence, true, 1000000000));

	//GetCurrentFrame().LocalDeletionQueue.flush();

	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(Device, Swapchain, 1000000000, GetCurrentFrame().SwapchainSemaphore, nullptr, &swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || SurfaceSize != Window::Get()->GetSize())
	{
		RecreateSwapchain();
		//DestroySwapchain();
		//CreateSwapchain(Window::Get()->GetSize());
		return;
	}

	VK_CALL(vkResetFences(Device, 1, &GetCurrentFrame().RenderFence));

	// Note: this will be the meat of the engine that should be here.
	VkCommandBuffer cmd = GetCurrentFrame().CommandBuffer;
	VK_CALL(vkResetCommandBuffer(cmd, 0));

	// Begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	// Note: We might reuse them later!!!
	VkCommandBufferBeginInfo cmdBeginInfo = VulkanInit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	DrawExtent.width = DrawImage.imageExtent.width;
	DrawExtent.height = DrawImage.imageExtent.height;

	// Create commands
	VK_CALL(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
	{
		// Transfer rendering image to general layout
		VulkanUtil::TransitionImage(cmd, DrawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		// Execute compute shader that writes to the image
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout, 0, 1, &DrawImageDescriptors, 0, nullptr);

		VulkanUtil::TransitionImage(cmd, DrawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		vkCmdDispatch(cmd, std::ceil(DrawExtent.width / 16.0), std::ceil(DrawExtent.height / 16.0), 1);

		// Transition rendering iamge to transfert onto swapchain images
		VulkanUtil::TransitionImage(cmd, DrawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Execute a copy from the rendering image into the swapchain
		VulkanUtil::CopyImageToImage(cmd, DrawImage.image, SwapchainImages[swapchainImageIndex], DrawExtent, SwapchainExtent);
		
		// Transition the swapchain image to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presentation
		VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
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
	//VkClearColorValue clearValue;
	//float flash = std::abs(std::sin(FrameNumber / 120.f));
	//clearValue = { { 0.0f, 0.0f, flash, 1.0f } };
	//VkImageSubresourceRange clearRange = VulkanInit::ImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
	//vkCmdClearColorImage(cmd, DrawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

	// This doesnt!!
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout, 0, 1, &DrawImageDescriptors, 0, nullptr);
	vkCmdDispatch(cmd, std::ceil(DrawExtent.width / 16.0), std::ceil(DrawExtent.height / 16.0), 1);
}


// Since well only be using those settings, this is just a shorthand.
// Well only be using VK_COMMAND_BUFFER_LEVEL_PRIMARY
VkCommandPoolCreateInfo VulkanInit::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;
	return info;
}

VkCommandBufferAllocateInfo VulkanInit::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	return info;
}

VkFenceCreateInfo VulkanInit::FenceCreateInfo(VkFenceCreateFlags flags)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = flags;
	return info;
}

VkSemaphoreCreateInfo VulkanInit::SemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = flags;
	return info;
}

VkCommandBufferBeginInfo VulkanInit::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
{
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;

	info.pInheritanceInfo = nullptr;
	info.flags = flags;
	return info;
}

VkImageSubresourceRange VulkanInit::ImageSubResourceRange(VkImageAspectFlags aspectMask)
{
	VkImageSubresourceRange subImage{};
	subImage.aspectMask = aspectMask;
	subImage.baseMipLevel = 0;
	subImage.levelCount = VK_REMAINING_MIP_LEVELS;
	subImage.baseArrayLayer = 0;
	subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

	return subImage;
}

VkSemaphoreSubmitInfo VulkanInit::SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
	VkSemaphoreSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.semaphore = semaphore;
	submitInfo.stageMask = stageMask;
	submitInfo.deviceIndex = 0;
	submitInfo.value = 1;

	return submitInfo;
}

VkCommandBufferSubmitInfo VulkanInit::CommandBufferSubmitInfo(VkCommandBuffer cmd)
{
	VkCommandBufferSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	info.pNext = nullptr;
	info.commandBuffer = cmd;
	info.deviceMask = 0;

	return info;
}

VkSubmitInfo2 VulkanInit::SubmitInfo(VkCommandBufferSubmitInfo * cmd, VkSemaphoreSubmitInfo * signalSemaphoreInfo, VkSemaphoreSubmitInfo * waitSemaphoreInfo)
{
	VkSubmitInfo2 info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	info.pNext = nullptr;

	info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
	info.pWaitSemaphoreInfos = waitSemaphoreInfo;

	info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
	info.pSignalSemaphoreInfos = signalSemaphoreInfo;

	info.commandBufferInfoCount = 1;
	info.pCommandBufferInfos = cmd;

	return info;
}

VkImageCreateInfo VulkanInit::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;

	info.format = format;
	info.extent = extent;

	info.mipLevels = 1;
	info.arrayLayers = 1;

	//for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
	info.samples = VK_SAMPLE_COUNT_1_BIT;

	//optimal tiling, which means the image is stored on the best gpu format
	info.tiling = VK_IMAGE_TILING_LINEAR;
	info.usage = usageFlags;

	return info;
}

VkImageViewCreateInfo VulkanInit::ImageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
	// build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;

	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.aspectMask = aspectFlags;

	return info;
}

// This is a helper to transtion images between readable, writable layouts.
void VulkanUtil::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier2 imageBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
	imageBarrier.pNext = nullptr;

	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	imageBarrier.oldLayout = currentLayout;
	imageBarrier.newLayout = newLayout;

	VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange = VulkanInit::ImageSubResourceRange(aspectMask);
	imageBarrier.image = image;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = nullptr;

	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(cmd, &depInfo);
}

void VulkanUtil::CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize)
{
	VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
	blitInfo.dstImage = destination;
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = source;
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(cmd, &blitInfo);
}

void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding newbind{};
	newbind.binding = binding;
	newbind.descriptorCount = 1;
	newbind.descriptorType = type;

	Bindings.push_back(newbind);
}

void DescriptorLayoutBuilder::Clear()
{
	Bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void * pNext, VkDescriptorSetLayoutCreateFlags flags)
{
	for (auto& b : Bindings) {
		b.stageFlags |= shaderStages;
	}

	VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	info.pNext = pNext;

	info.pBindings = Bindings.data();
	info.bindingCount = (uint32_t)Bindings.size();
	info.flags = flags;

	VkDescriptorSetLayout set;
	VK_CALL(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

	return set;
}

void DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSizeRatio ratio : poolRatios) {
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = ratio.type,
			.descriptorCount = uint32_t(ratio.ratio * maxSets)
		});
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
