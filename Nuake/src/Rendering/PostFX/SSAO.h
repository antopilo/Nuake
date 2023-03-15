#pragma once
#include "src/Core/Maths.h"
#include "src/Core/Core.h"
#include "src/Rendering/Buffers/Framebuffer.h"

#include <vector>

namespace Nuake 
{
	class SSAO 
	{
	private:
		std::vector<glm::f32vec3> _ssaoKernel;
		std::vector<Vector3> _ssaoNoise;

		Ref<FrameBuffer> _ssaoFramebuffer;
		Ref<FrameBuffer> _ssaoBlurFramebuffer;
		Ref<Texture> _ssaoNoiseTexture;

		Vector2 _size;

		void GenerateKernel();
		void GenerateNoise();
		float lerp(float a, float b, float f)
		{
			return a + f * (b - a);
		}

	public:
		float Radius = 0.07f;
		float Bias = 0.003f;
		float Area = 0.0075f;
		float Falloff = 0.002f;
		float Strength = 1.0f;
		void Resize(const Vector2& size);
		SSAO();
		void Draw(FrameBuffer* gBuffer, const Matrix4& projection, const Matrix4& view);
		Ref<FrameBuffer> GetOuput() const;
	};
}