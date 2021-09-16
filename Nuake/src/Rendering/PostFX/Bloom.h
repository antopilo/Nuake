#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Shaders/ShaderManager.h"
#include "src/Rendering/Buffers/Framebuffer.h"

#include <vector>

namespace Nuake {
	class Bloom
	{
	public:
		float Threshold = 0.3f;
		float BlurAmount = 12.0f;
		Bloom() {}
		Bloom(Ref<Texture> source, unsigned int iteration = 4);

		void Draw();

		Ref<Texture> GetThreshold() const { return m_ThresholdFB->GetTexture(); }

		unsigned int m_Iteration;
		Ref<Texture> m_Source;

		std::vector<Ref<Texture>> m_DownsampleMIP;
		std::vector<Ref<Texture>> m_UpSampleMIP;

		Ref<FrameBuffer> m_FinalFB;
		Ref<Texture> m_Final;
		Ref<FrameBuffer> m_ThresholdFB;


		std::vector<Ref<FrameBuffer>> m_DownSampleFB;
		std::vector<Ref<FrameBuffer>> m_HBlurFB;
		std::vector<Ref<FrameBuffer>> m_VBlurFB;
		std::vector<Ref<FrameBuffer>> m_UpSampleFB;
	};
}