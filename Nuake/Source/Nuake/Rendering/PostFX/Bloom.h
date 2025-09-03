#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Textures/Texture.h"
#include "Nuake/Rendering/Shaders/ShaderManager.h"
#include "Nuake/Rendering/Buffers/Framebuffer.h"

#include <vector>

namespace Nuake {
	class Bloom
	{
		const float MAX_THRESHOLD = 500.0f;
		const float MIN_THRESHOLD = 0.0f;
		const float MAX_BLUR = 50.0f;
		const float MIN_BLUR = 0.0f;
		const unsigned int MAX_ITERATION = 5;
		const unsigned int MIN_ITERATION = 1;

	private:
		float m_Threshold = 2.3f;
		float m_BlurAmount = 12.0f;
		float m_LensDirtIntensity = 1.0f;

		uint32_t m_Iteration = 4;
		Vector2 m_Size = Vector2();

		Ref<Texture> m_Source;

		bool m_HasLensDirt = false;
		Ref<Texture> m_LensDirt;
		Ref<FrameBuffer> m_FinalFB;
		Ref<FrameBuffer> m_ThresholdFB;
		std::vector<Ref<FrameBuffer>> m_DownSampleFB;
		std::vector<Ref<FrameBuffer>> m_HBlurFB;
		std::vector<Ref<FrameBuffer>> m_VBlurFB;
		std::vector<Ref<FrameBuffer>> m_UpSampleFB;
	public:
		Bloom() {}
		Bloom(unsigned int iteration = 4);

		Ref<Texture> GetLensDirt() { return m_LensDirt; }
		void SetLensDirt(Ref<Texture> texture);
		void ClearLensDirt();

		float GetLensDirtIntensity() const { return m_LensDirtIntensity; }
		void SetLensDirtIntensity(float intensity) { m_LensDirtIntensity = intensity; }

		void Init();
		void Draw();

		void Resize(Vector2 size);

		inline float GetThreshold() const { return m_Threshold; }
		void SetThreshold(float threshold)
		{
			if (threshold <= MAX_THRESHOLD && threshold >= MIN_THRESHOLD)
				m_Threshold = threshold;
		}

		void SetIteration(unsigned int iteration)
		{
			if (iteration > MAX_ITERATION || iteration < MIN_ITERATION)
				return;

			m_Iteration = iteration;

			Init();
		};
		inline unsigned int GetIteration() const { return m_Iteration; }

		void SetSource(Ref<Texture> source);
		Ref<Texture> GetOutput() const { return m_FinalFB->GetTexture(); }

	};
}