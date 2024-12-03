#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include <volk/volk.h>
#include "vkb/VkBootstrap.h"


namespace Nuake
{
	// GPU has Vk Queue family type
	// 2x Queue -> All operation
	// 2x Queue -> Computer operation
	// 2x Queue -> Data transfer operation
	// Not: reminds me of data bus
	//
	// You have one command pool that can be executed at at time on queue
	// command pool is allocator for command buffer
	// you can have X amount of command buffer from a pool
	// you can only write to a command buffer from 1 thread
	// 
	// All on the same thread: 
	// ~ vkBeginCommandBuffer
	// ~ vkEndCommandBuffer
	// ~ vkQueueSubmit 
	// ~ Frame data

	// Note:
	// You should double buffer the command pool and command buffer
	// To write commands while the GPU is executing the previous
	struct FrameData
	{
		VkCommandPool CommandPool; // This is like the allocator for a buffer.
		VkCommandBuffer CommandBuffer; // You send commands in there.

		// Semaphore are for GPU -> GPU sync
		// Fence are for CPU -> GPU
		// Two, one for window, other for rendering
		VkSemaphore SwapchainSemaphore, RenderSemaphore;
		VkFence RenderFence; // CPU wait for next frame
	};

	constexpr unsigned int FRAME_OVERLAP = 2;

	class VkRenderer
	{
	private:
		bool IsInitialized = false;

		VkInstance Instance;
		vkb::Instance VkbInstance;
		VkDebugUtilsMessengerEXT VkDebugMessenger;
		VkPhysicalDevice GPU;
		VkDevice Device; 
		vkb::Device VkbDevice;

		VkSurfaceKHR Surface;

		// Swap chain
		VkFormat SwapchainImageFormat;
		VkSwapchainKHR Swapchain;
		VkExtent2D SwapchainExtent;
		std::vector<VkImage> SwapchainImages;
		std::vector<VkImageView> SwapchainImageViews;

		// Frame data
		uint32_t FrameNumber = 0;
		FrameData Frames[FRAME_OVERLAP];
		FrameData& GetCurrentFrame() { return Frames[FrameNumber % FRAME_OVERLAP]; };

		VkQueue GPUQueue;
		uint32_t GPUQueueFamily;

	public:
		static VkRenderer& Get()
		{
			static VkRenderer instance;
			return instance;
		}

		VkRenderer() = default;
		~VkRenderer();

	public:
		void Initialize();
		void CleanUp();

		void GetInstance();
		void SelectGPU();
		void CreateSwapchain(const Vector2& size);
		void DestroySwapchain();

		void InitCommands();
		void InitSync();

		void Draw();
	};


	class VulkanInit
	{
	public:
		VulkanInit() = delete;
		~VulkanInit() = delete;

		static VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		static VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);
		static VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
		static VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

		static VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
		static VkImageSubresourceRange  ImageSubResourceRange(VkImageAspectFlags  aspectMask);
		static VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
		static VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);
		static VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);

	};

	class VulkanUtil
	{
	public:
		VulkanUtil() = delete;
		~VulkanUtil() = delete;

		static void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	};
}