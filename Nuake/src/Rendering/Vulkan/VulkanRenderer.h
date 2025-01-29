#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include <volk/volk.h>
#include "vkb/VkBootstrap.h"

#include "VulkanImage/VulkanImage.h"
#include "VulkanTypes.h"
#include "VkVertex.h"
#include "RenderContext.h"
#include "VulkanAllocatedBuffer.h"
#include "VulkanSceneRenderer.h"

#include "src/Resource/UUID.h"

#include <functional>
#include <span>

#include "vk_mem_alloc.h"
#include "VkMesh.h"
#include "DescriptorAllocatorGrowable.h"

#include "src/Rendering/Vulkan/Cmd.h"

namespace Nuake
{
	class VulkanShader;
	class GPUMeshBuffers;
	class Scene;

	struct AllocatedImage {
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D imageExtent;
		VkFormat imageFormat;
	};

	// Since we need to delete things in order, and manually doing it in the cleanup is 
	// too tedious to maintain. We will use a queue of destructor to execute in inverse order
	// of insertion
	struct DeletionQueue
	{
		std::deque<std::function<void()>> Deletors;

		void push_function(std::function<void()>&& function)
		{
			Deletors.push_back(function);
		}

		void flush()
		{
			// reverse iterate the deletion queue to execute all the functions
			for (auto it = Deletors.rbegin(); it != Deletors.rend(); it++)
			{
				(*it)(); //call functors
			}

			Deletors.clear();
		}
	};


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

		Ref<AllocatedBuffer> CameraStagingBuffer;	// Current camera
		Ref<AllocatedBuffer> ModelStagingBuffer;	// Matrices
		Ref<AllocatedBuffer> MaterialStagingBuffer;	// Materials
		Ref<AllocatedBuffer> LightStagingBuffer;	// Lights
		Ref<AllocatedBuffer> CamerasStagingBuffer;	// Draw image

		// Semaphore are for GPU -> GPU sync
		// Fence are for CPU -> GPU
		// Two, one for window, other for rendering
		VkSemaphore SwapchainSemaphore, RenderSemaphore;
		VkFence RenderFence; // CPU wait for next frame

		DescriptorAllocatorGrowable FrameDescriptors;

		DeletionQueue LocalDeletionQueue; // Local when destroying this frame
	};

	struct DescriptorAllocator 
	{
		uint32_t NumAllocation = 0;
		struct PoolSizeRatio 
		{
			VkDescriptorType type;
			float ratio;
		};

		VkDescriptorPool pool;

		void InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void ClearDescriptors(VkDevice device);
		void DestroyPool(VkDevice device);

		VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);
	};

	struct CameraData
	{
		Matrix4 View;
		Matrix4 Projection;
		Matrix4 InvView;
		Matrix4 InvProjection;
		Vector3 Position;
	};

	// Renderer configuration
	constexpr uint32_t FRAME_OVERLAP = 2;
	
	class Viewport;

	class VkRenderer
	{
	private:
		bool FrameSkipped = false;
		bool IsInitialized = false;
		Vector2 SurfaceSize;
		VkInstance Instance;
		vkb::Instance VkbInstance;
		VkDebugUtilsMessengerEXT VkDebugMessenger;
		VkPhysicalDevice GPU;
		VkDevice Device; 
		vkb::Device VkbDevice;

		VkSurfaceKHR Surface;
		uint32_t swapchainImageIndex;
		// Swap chain
		VkFormat SwapchainImageFormat;
		VkSwapchainKHR Swapchain;
		VkExtent2D SwapchainExtent;
		std::vector<VkImage> SwapchainImages;
		std::vector<VkImageView> SwapchainImageViews;

		// Frame data
		uint32_t FrameNumber = 0;
		FrameData Frames[FRAME_OVERLAP];
		
		uint32_t GPUQueueFamily;

		DeletionQueue MainDeletionQueue;
		DescriptorAllocator GlobalDescriptorAllocator;



	public:
		VkDescriptorSet DrawImageDescriptors;
		VkDescriptorSetLayout DrawImageDescriptorLayout;
		
		// Imgui stuff
		VkFence ImguiFence;
		VkCommandBuffer ImguiCommandBuffer;
		VkCommandPool ImguiCommandPool;

		Ref<VkSceneRenderer> SceneRenderer;



	public:
		static VkRenderer& Get()
		{
			static VkRenderer instance;
			return instance;
		}

		VkRenderer() = default;
		~VkRenderer();

	public:
		Ref<VkMesh> Rect;
		VkQueue GPUQueue;

		Ref<VulkanImage> DrawImage;
		Ref<VulkanImage> DepthImage;
		VkExtent2D DrawExtent;

		std::map<UUID, Ref<Viewport>> Viewports;
		std::map<Ref<Scene>, std::vector<UUID>> SceneViewports;

	public:
		void Initialize();
		void CleanUp();

		void GetInstance();
		void SelectGPU();
		
		void RecreateSwapchain();
		void CreateSwapchain(const Vector2& size);
		void DestroySwapchain();

		void InitCommands();
		void InitSync();
		void InitDescriptors();
		void UpdateDescriptorSets();
		void DrawScene(RenderContext ctx);
		void InitImgui();

		Ref<Viewport> CreateViewport(const UUID& viewId, const Vector2& size);
		void RemoveViewport(const UUID& viewportId);

		void RegisterSceneViewport(const Ref<Scene>& scene, const UUID& viewportId);
		void UnRegisterSceneViewport(const Ref<Scene>& scene, const UUID& viewportId);

		void PrepareSceneData(RenderContext ctx);
		void DrawScenes();
		void DrawSceneViewport(const Ref<Scene>& scene, const UUID& viewportId);

		void BeginScene(const UUID& camera);
		bool Draw();
		void EndDraw();

	public:
		VkDevice GetDevice() const
		{
			return Device;
		}

		FrameData& GetCurrentFrame() 
		{ 
			return Frames[FrameNumber % FRAME_OVERLAP]; 
		};

		DescriptorAllocator& GetDescriptorAllocator()
		{
			return GlobalDescriptorAllocator;
		}

		Cmd GetCurrentCmdBuffer()
		{
			return Cmd(GetCurrentFrame().CommandBuffer);
		}

		VkDescriptorSet GetViewportDescriptor() const { return DrawImageDescriptors; }
		Ref<VulkanImage> GetDrawImage() const { return DrawImage; }

		void DrawImgui(VkCommandBuffer cmd, VkImageView targetImageView);
		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	};
}