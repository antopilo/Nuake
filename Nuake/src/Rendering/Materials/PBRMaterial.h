#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/Shaders/Shader.h"

namespace Nuake {
	class PBRMaterial 
	{
	public:
		static Ref<Shader> mShader;

		PBRMaterial();

		void Bind();
	private:
	};
}