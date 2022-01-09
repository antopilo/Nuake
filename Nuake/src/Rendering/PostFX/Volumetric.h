#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Buffers/FrameBuffer.h"
#include "src/Scene/Components/LightComponent.h"
#include <vector>

namespace Nuake {
	class Volumetric 
	{
	private:
		uint32_t mStepCount = 50;
		float mRenderRatio = 1.0f;
		float mFogAmount = 0.9f;

		Vector2 mSize;
		Texture* mDepth;
		Scope<FrameBuffer> mFinalFramebuffer;

	public:
		Volumetric();

		void SetDepth(Texture* depth);
		void Init();
		void Resize(Vector2 size);
		void Draw(Matrix4 projection, Matrix4 view, std::vector<LightComponent>& lights);

		inline int GetStepCount() const { return mStepCount;}

		inline void SetStepCount(unsigned int amount) { mStepCount = amount; }

		inline float GetFogAmount() const { return mFogAmount; }
		void SetFogAmount(float amount) 
		{
			if (amount > 0) mFogAmount = amount;
		}

		Texture* GetFinalOutput() 
		{
			return mFinalFramebuffer->GetTexture().get();
		}
	};
}