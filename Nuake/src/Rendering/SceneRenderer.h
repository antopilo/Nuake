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

		Scope<SSR> mSSR;

		FrameBuffer& GetGBuffer() const
		{
			return *mGBuffer;
		}

	private:
		Matrix4 mProjection, mView;
		Vector3 mCamPos;

		Scope<FrameBuffer> mGBuffer;
		Scope<FrameBuffer> mShadingBuffer;
		Scope<FrameBuffer> mToneMapBuffer;
		
	private:
		void ShadowPass(Scene& scene);
		void GBufferPass(Scene& scene);
		void ShadingPass(Scene& scene);
		void PostProcessPass(const Scene& scene);
	};
}