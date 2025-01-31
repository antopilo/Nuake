#pragma once
#include "Nuake/Core/Maths.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Buffers/Framebuffer.h"

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
		float Radius = 0.997f;
		float Bias = 0.045f;
		float Area = 0.0075f;
		float Falloff = 0.002f;
		float Strength = 0.2f;
		void Resize(const Vector2& size);
		void Clear();
		SSAO();
		void Draw(FrameBuffer* gBuffer, const Matrix4& projection, const Matrix4& view);
		Ref<FrameBuffer> GetOuput() const;
	};
}