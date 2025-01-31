#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Buffers/Framebuffer.h"
#include "Nuake/Rendering/Shaders/ShaderManager.h"
#include "Nuake/Rendering/Camera.h"

namespace Nuake {
	class SSR {
	private:
		Vector2 mSize;
		Shader* mShader;
	public:
		Ref<FrameBuffer> OutputFramebuffer;

		float RayStep = 0.2f;
		int IterationCount = 100;
		float DistanceBias = 0.05f;
		int SampleCount = 1;
		bool SamplingEnabled = true;
		bool ExpoStep = true;
		bool AdaptiveStep = true;
		bool BinarySearch = true;
		bool DebugDraw = false;
		float SampleingCoefficient = 0.000f;

		SSR();
		void Init();
		void Resize(Vector2 size);
		void Draw(FrameBuffer* gBuffer, Ref<Texture> previousFrame, Matrix4 view, Matrix4 projection, Ref<Camera> cam);
	};
}