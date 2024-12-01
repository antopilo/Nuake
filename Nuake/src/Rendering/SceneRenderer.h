#pragma once
#include "Renderer.h"
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Scene/Scene.h"
#include "src/Rendering/Buffers/Framebuffer.h"
#include "src/Rendering/PostFX/Bloom.h"
#include "src/Rendering/PostFX/Volumetric.h"
#include "src/Rendering/PostFX/SSR.h"

#include "src/Physics/PhysicsShapes.h"
#include "Shapes/BoxGizmo.h"
#include "Shapes/SphereGizmo.h"
#include "Shapes/CapsuleGizmo.h"
#include "Shapes/CylinderGizmo.h"
#include <src/Resource/Model.h>
#include <glad/glad.h>

namespace Nuake 
{
	class SceneRenderer 
	{
		struct DebugShape
		{
			Vector3 Position;
			Quat Rotation;
			Color LineColor;
			float Life;
			float Width;
			bool DepthTest = true;
			Ref<Physics::PhysicShape> Shape;
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
		SceneRenderer();

		void Init();
		void Cleanup();

		void Update(const Timestep time, bool isEditorUpdate = false);

		void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vector3& camPos);
		void RenderScene(Scene& scene, FrameBuffer& framebuffer, bool renderUI = true);

		FrameBuffer& GetGBuffer() const
		{
			return *mGBuffer;
		}

		Ref<Texture> GetScaledDepthTexture()
		{
			return mDisplayDepthBuffer->GetTexture(GL_COLOR_ATTACHMENT0);
		}

		Ref<Texture> GetVelocityTexture()
		{
			return mDisplayMotionVector->GetTexture(GL_COLOR_ATTACHMENT0);
		}

		bool IsTempModelLoaded(const std::string& name) const
		{
			return mTempModels.find(name) != mTempModels.end();
		}

		void DrawTemporaryModel(const std::string& name, Ref<Model> model, Matrix4 transform);
		void DrawDebugLine(const Vector3& start, const Vector3& end, const Color& color = Color(1, 0, 0, 1), float life = 0.0f, float width = 1.0f);
		void DrawDebugShape(const Vector3& position, const Quat& rotation, Ref<Physics::PhysicShape> shape, const Color& color = Color(1, 0, 0, 1), float life = 0.0f, float width = 1.0f);

		void ClearTemporaryModels()
		{
			mTempModels.clear();
		}


		float TAAFactor = 0.6f;
		int mOutlineEntityID = 0;
	private:
		Matrix4 mProjection, mView;
		Vector3 mCamPos;

		// GBuffer
		Scope<FrameBuffer> mGBuffer;
		Scope<FrameBuffer> mShadingBuffer;
		Scope<FrameBuffer> mToneMapBuffer;
		Scope<FrameBuffer> mBarrelDistortionBuffer;
		Scope<FrameBuffer> mVignetteBuffer;
		Scope<FrameBuffer> mDOFBuffer;
		Scope<FrameBuffer> mOutlineBuffer;
		Scope<FrameBuffer> mDisplayDepthBuffer;
		Scope<FrameBuffer> mDisplayMotionVector;
		Scope<FrameBuffer> mTempFrameBuffer;
		Scope<FrameBuffer> mPreviousFrame;

		// Shapes
		Ref<BoxGizmo> mBoxGizmo;
		Ref<SphereGizmo> mSphereGizmo;
		Ref<CapsuleGizmo> mCapsuleGizmo;
		Ref<CylinderGizmo> mCylinderGizmo;

		Ref<Mesh> mLineMesh;

		std::map<std::string, TemporaryModels> mTempModels;
		std::vector<DebugLine> mDebugLines;
		std::vector<DebugShape> mDebugShapes;

		static std::vector<Vector2> mJitterOffsets;
		std::vector<Vector2> mScaledJitterOffets;
		uint32_t mJitterIndex = 0;

		void ShadowPass(Scene& scene);
		void GBufferPass(Scene& scene);
		void ShadingPass(Scene& scene, Ref<Texture> previousFrame);
		void PostProcessPass(const Scene& scene);
		void DebugRendererPass(Scene& scene);

		void SetSkeletonBoneTransformRecursive(Scene& scene, SkeletonNode& skeletonNode, Shader* shader);
		
		void UpdateJitterOffsets(const Vector2& viewportSize);
	};
}