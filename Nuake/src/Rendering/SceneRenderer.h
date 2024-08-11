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
		struct DebugSphere
		{
			Vector3 Position;
			float Radius;
			Color SphereColor;
			float Life;
		};

		struct DebugQuad
		{
			Vector3 Position;
			Vector3 Size;
			Color SphereColor;
			float Life;
		};

		struct DebugLine
		{
			Vector3 Start;
			Vector3 End;
			Color LineColor;
			float Life;
			float Width;
			bool DepthTest = true;
		};

		struct TemporaryModels
		{
			Ref<Model> ModelResource;
			Matrix4 Transform;
		};

	public:
		void Init();
		void Cleanup();

		void Update(const Timestep time);

		void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vector3& camPos);
		void RenderScene(Scene& scene, FrameBuffer& framebuffer);

		FrameBuffer& GetGBuffer() const
		{
			return *mGBuffer;
		}

		bool IsTempModelLoaded(const std::string& name) const
		{
			return mTempModels.find(name) != mTempModels.end();
		}

		void DrawTemporaryModel(const std::string& name, Ref<Model> model, Matrix4 transform);
		void DrawDebugLine(const Vector3& start, const Vector3& end, const Color& color = Color(1, 0, 0, 1), float life = 0.0f);
		void ClearTemporaryModels()
		{
			mTempModels.clear();
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

		Ref<Mesh> mLineMesh;


		std::map<std::string, TemporaryModels> mTempModels;
		std::vector<DebugLine> mDebugLines;

		void ShadowPass(Scene& scene);
		void GBufferPass(Scene& scene);
		void ShadingPass(Scene& scene);
		void PostProcessPass(const Scene& scene);
		void DebugRendererPass(Scene& scene);

		void SetSkeletonBoneTransformRecursive(Scene& scene, SkeletonNode& skeletonNode, Shader* shader);
	};
}