#pragma once
#include "Renderer.h"
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Scene/Scene.h"
#include "src/Rendering/Buffers/Framebuffer.h"
#include "src/Rendering/PostFX/Bloom.h"
#include "src/Rendering/PostFX/Volumetric.h"

namespace Nuake {
	class SceneRenderer {
	public:
		void Init();
		void Cleanup();

		void BeginRenderScene(const Matrix4& projection, const Matrix4& view);
		void RenderScene(Scene& scene, FrameBuffer& framebuffer);
	private:
		Matrix4 mProjection, mView;

		Scope<FrameBuffer> mGBuffer;
		Scope<FrameBuffer> mShadingBuffer;
		Scope<Bloom> mBloom;
		Scope<Volumetric> mVolumetric;

		void ShadowPass(Scene& scene);
		void GBufferPass(Scene& scene);
		void ShadingPass(Scene& scene);
		void PostProcessPass(const Scene& scene);
	};
}