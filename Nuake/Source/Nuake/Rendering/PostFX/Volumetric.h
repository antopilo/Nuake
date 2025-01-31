#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Buffers/Framebuffer.h"
#include "Nuake/Scene/Components/LightComponent.h"
#include <vector>

namespace Nuake {
	class Volumetric 
	{
	private:
		uint32_t mStepCount = 50;
		float mRenderRatio = 0.75f;
		float mFogAmount = 0.4f;
		float mFogExponant = 1.0f;
		Vector2 mSize;
		Texture* mDepth;

		Scope<FrameBuffer> mVolumetricFramebuffer;
		Scope<FrameBuffer> mFinalFramebuffer;

	public:
		Volumetric();

		void SetDepth(Texture* depth);
		void Init();
		void Resize(Vector2 size);
		void Draw(Matrix4 projection, Matrix4 view, const Vector3& camPos, std::vector<LightComponent>& lights);

		inline int GetStepCount() const { return mStepCount;}

		inline void SetStepCount(unsigned int amount) { mStepCount = amount; }

		void SetFogExponant(float expo) { mFogExponant = expo; }
		float GetFogExponant() const { return mFogExponant; }

		inline float GetFogAmount() const { return mFogAmount; }
		void SetFogAmount(float amount) 
		{
			if (amount > 0) mFogAmount = amount;
		}

		Ref<Texture> GetFinalOutput() 
		{
			return mFinalFramebuffer->GetTexture();
		}
	};
}