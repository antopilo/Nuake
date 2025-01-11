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

#include <functional>
#include <span>

#include "vk_mem_alloc.h"
#include "VkMesh.h"
#include "DescriptorAllocatorGrowable.h"

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
	constexpr uint32_t MAX_MODEL_MATRIX = 3000;
	constexpr uint32_t MAX_MATERIAL = 1000;
	constexpr uint32_t MAX_TEXTURES = 500;
	constexpr uint32_t MAX_CAMERAS = 100;
	constexpr uint32_t MAX_LIGHTS = 100;
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

		// Descriptors
		DescriptorAllocator GlobalDescriptorAllocator;

		VkDescriptorSet DrawImageDescriptors;
		VkDescriptorSetLayout DrawImageDescriptorLayout;

		VkDescriptorSet TriangleBufferDescriptors;
		VkDescriptorSetLayout TriangleBufferDescriptorLayout;

		VkDescriptorSet CameraBufferDescriptors;
		VkDescriptorSetLayout CameraBufferDescriptorLayout;

		// Pipeline
		VkPipeline Pipeline;
		VkPipelineLayout PipelineLayout;

		VkPipelineLayout TrianglePipelineLayout;
		VkPipeline TrianglePipeline;

		Ref<VulkanShader> BackgroundShader;
		Ref<VulkanShader> TriangleVertShader;
		Ref<VulkanShader> TriangleFragShader;
		
		// Imgui
		VkFence ImguiFence;
		VkCommandBuffer ImguiCommandBuffer;
		VkCommandPool ImguiCommandPool;

		// Buffers
		Ref<VkMesh> rectangle;

		Ref<AllocatedBuffer> CameraBuffer;
		Ref<VkSceneRenderer> SceneRenderer;

	public:
		VkQueue GPUQueue;

		Ref<VulkanImage> DrawImage;
		Ref<VulkanImage> DepthImage;
		VkExtent2D DrawExtent;
		FrameData& GetCurrentFrame() { return Frames[FrameNumber % FRAME_OVERLAP]; };

		static VkRenderer& Get()
		{
			static VkRenderer instance;
			return instance;
		}

		VkRenderer() = default;
		~VkRenderer();

		VkDevice GetDevice() const
		{
			return Device;
		}

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
		void InitPipeline();
		void InitBackgroundPipeline();
		void InitTrianglePipeline();
		void DrawScene(RenderContext ctx);
		void DrawGeometry(VkCommandBuffer cmd);
		void InitImgui();

		void BeginScene(const UUID& camera);

		bool Draw();

		void EndDraw();

		DescriptorAllocator GetDescriptorAllocator()
		{
			return GlobalDescriptorAllocator;
		}

		VkCommandBuffer GetCurrentCmdBuffer()
		{
			return GetCurrentFrame().CommandBuffer;
		}

		void DrawBackground(VkCommandBuffer cmd);
		void DrawImgui(VkCommandBuffer cmd, VkImageView targetImageView);

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

		void UploadCameraData(const CameraData& data);
		auto& GetRenderPipeline() { return this->SceneRenderer->GetRenderPipeline(); }
		VkDescriptorSet GetViewportDescriptor() const { return DrawImageDescriptors; }
		Ref<VulkanImage> GetDrawImage() const { return DrawImage; }
	};
	
}