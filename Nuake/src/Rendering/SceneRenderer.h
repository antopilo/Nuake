#pragma once
#include "Renderer.h"
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Scene/Scene.h"
#include "src/Rendering/Buffers/Framebuffer.h"
#include "src/Rendering/PostFX/Bloom.h"
#include "src/Rendering/PostFX/Volumetric.h"
#include "src/Rendering/PostFX/SSR.h"

namespace Nuake 
{
	class SceneRenderer 
	{
	public:
		void Init();
		void Cleanup();

		void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vector3& camPos);
		void RenderScene(Scene& scene, FrameBuffer& framebuffer);

		FrameBuffer& GetGBuffer() const
		{
			return *mGBuffer;
		}

		uint32_t mOutlineEntityID = 0;
	private:
		Matrix4 mProjection, mView;
		Vector3 mCamPos;

		
		Scope<FrameBuffer> mGBuffer;
		Scope<FrameBuffer> mShadingBuffer;
		Scope<FrameBuffer> mToneMapBuffer;
		Scope<FrameBuffer> mBarrelDistortionBuffer;
		Scope<FrameBuffer> mVignetteBuffer;
		Scope<FrameBuffer> mDOFBuffer;
		Scope<FrameBuffer> mOutlineBuffer;

		void ShadowPass(Scene& scene);
		void GBufferPass(Scene& scene);
		void ShadingPass(Scene& scene);
		void PostProcessPass(const Scene& scene);

		void SetSkeletonBoneTransformRecursive(Scene& scene, SkeletonNode& skeletonNode, Shader* shader);
	};
}