#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include <volk/volk.h>
#include "vkb/VkBootstrap.h"

#include "VulkanImage/VulkanImage.h"
#include "VulkanTypes.h"
#include "VkVertex.h"

#include "VulkanAllocatedBuffer.h"

#include <functional>
#include <span>


#include "vk_mem_alloc.h"

namespace Nuake
{
	class VulkanShader;
	class GPUMeshBuffers;
	

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

		AllocatedBuffer CameraStagingBuffer;

		// Semaphore are for GPU -> GPU sync
		// Fence are for CPU -> GPU
		// Two, one for window, other for rendering
		VkSemaphore SwapchainSemaphore, RenderSemaphore;
		VkFence RenderFence; // CPU wait for next frame

		DeletionQueue LocalDeletionQueue; // Local when destroying this frame
	};


	struct DescriptorLayoutBuilder 
	{

		std::vector<VkDescriptorSetLayoutBinding> Bindings;

		void AddBinding(uint32_t binding, VkDescriptorType type);
		void Clear();
		VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
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

	constexpr unsigned int FRAME_OVERLAP = 2;

	struct CameraData
	{
		Matrix4 Model;
		Matrix4 View;
		Matrix4 Projection;
	};

	class VkRenderer
	{
	private:
		bool IsInitialized = false;
		Vector2 SurfaceSize;
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

		Ref<VulkanImage> DrawImage;
		VkExtent2D DrawExtent;

		VkQueue GPUQueue;
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
		Ref<GPUMeshBuffers> rectangle;

		AllocatedBuffer CameraBuffer;

	public:
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
		void DrawGeometry(VkCommandBuffer cmd);
		void InitImgui();

		void BeginScene(const Matrix4& view, const Matrix4& projection);

		void Draw();
		void DrawBackground(VkCommandBuffer cmd);
		void DrawImgui(VkCommandBuffer cmd, VkImageView targetImageView);

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

		void UploadCameraData(const CameraData& data);
		Ref<GPUMeshBuffers> UploadMesh(std::vector<uint32_t> indices, std::vector<VkVertex> vertices);

		VkDescriptorSet GetViewportDescriptor() const { return DrawImageDescriptors; }
		Ref<VulkanImage> GetDrawImage() const { return DrawImage; }
	};
	
}