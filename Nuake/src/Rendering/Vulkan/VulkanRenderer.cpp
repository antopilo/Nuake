#include "VulkanRenderer.h"

#include "src/Core/Logger.h"
#include <src/Rendering/RenderCommand.h>

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
	
	CreateSwapchain(Window::Get()->GetSize());

	// Now lets grab the Queues
	GPUQueue = VkbDevice.get_queue(vkb::QueueType::graphics).value();
	GPUQueueFamily = VkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	InitCommands();

	InitSync();

	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
	vulkanFunctions.vkFreeMemory = vkFreeMemory;
	vulkanFunctions.vkMapMemory = vkMapMemory;
	vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
	vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
	vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
	vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
	vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
	vulkanFunctions.vkCreateImage = vkCreateImage;
	vulkanFunctions.vkDestroyImage = vkDestroyImage;
	vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

	// Optional: Include extension functions if needed
	vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
	vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
	vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;

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

	Swapchain = vkbSwapchain.swapchain;
	SwapchainImages = vkbSwapchain.get_images().value();
	SwapchainImageViews = vkbSwapchain.get_image_views().value();
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

		vkAllocateCommandBuffers(Device, &cmdAllocInfo, &Frames[i].CommandBuffer);
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
		vkCreateFence(Device, &fenceCreateInfo, nullptr, &Frames[i].RenderFence);

		vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &Frames[i].SwapchainSemaphore);
		vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &Frames[i].RenderSemaphore);
	}
}

void VkRenderer::Draw()
{
	vkWaitForFences(Device, 1, &GetCurrentFrame().RenderFence, true, 1000000000);
	vkResetFences(Device, 1, &GetCurrentFrame().RenderFence);

	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(Device, Swapchain, 1000000000, GetCurrentFrame().SwapchainSemaphore, nullptr, &swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		DestroySwapchain();
		CreateSwapchain(Window::Get()->GetSize());
		return;
	}

	// Note: this will be the meat of the engine that should be here.
	VkCommandBuffer cmd = GetCurrentFrame().CommandBuffer;
	vkResetCommandBuffer(cmd, 0);

	// Begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	// Note: We might reuse them later!!!
	VkCommandBufferBeginInfo cmdBeginInfo = VulkanInit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// Start the command buffer recording
	vkBeginCommandBuffer(cmd, &cmdBeginInfo);
	{
		// We transfer current frame from any layout to general layout.
		VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		float flash = std::abs(std::sin(FrameNumber / 120.f));
		VkClearColorValue clearValue;
		clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

		VkImageSubresourceRange clearRange = VulkanInit::ImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		// TODO: Move to -> RenderCommand::Clear();
		vkCmdClearColorImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

		// From writable, to presentable layout
		VulkanUtil::TransitionImage(cmd, SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	vkEndCommandBuffer(cmd);

	VkCommandBufferSubmitInfo cmdinfo = VulkanInit::CommandBufferSubmitInfo(cmd);

	// Wait for both semaphore of swapchain and the texture of the frame.
	VkSemaphoreSubmitInfo waitInfo = VulkanInit::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().SwapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = VulkanInit::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().RenderSemaphore);

	// Submit command!!
	VkSubmitInfo2 submit = VulkanInit::SubmitInfo(&cmdinfo, &signalInfo, &waitInfo);

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	vkQueueSubmit2(GPUQueue, 1, &submit, GetCurrentFrame().RenderFence);

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &Swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &GetCurrentFrame().RenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	vkQueuePresentKHR(GPUQueue, &presentInfo);

	//increase the number of frames drawn
	FrameNumber++;
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
