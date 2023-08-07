#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "glm/gtc/matrix_transform.hpp"
#include "src/Core/Maths.h"
#include "src/Rendering/Textures/Texture.h"

namespace Nuake
{
	struct ShaderSource 
	{
		std::string VertexShader;
		std::string FragmentShader;
		std::string GeometryShader;
		std::string ComputeShader;
	};

	class Shader 
	{
	public:
		std::string Path;
		ShaderSource Source;
		unsigned int ProgramId;

		std::unordered_map<std::string, int> UniformCache;

		Shader(const std::string& filePath);

		bool Rebuild();
		void Bind() const;
		void Unbind() const;

		void SetUniformVec4(const std::string& name, Vector4 vec);
		void SetUniformVec3(const std::string& name, Vector3 vec);
		void SetUniformVec2(const std::string& name, Vector2 vec);
		void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
		void SetUniform3f(const std::string& name, float v0, float v1, float v2);
		void SetUniform2f(const std::string& name, float v0, float v1);
		void SetUniform1f(const std::string& name, float v0);
		void SetUniform1b(const std::string& name, bool v0);
		void SetUniformTex(const std::string& name, Texture* texture, unsigned int slot = 0);
		void SetUniform1i(const std::string& name, int v0);
		void SetUniform1i(uint32_t location, int v0);

		void SetUniform1iv(const std::string& name, int size, int* value);
		void SetUniform1fv(const std::string& name, int size, float* value);
		void SetUniformMat3f(const std::string& name, Matrix3 mat);

		void SetUniformMat4f(uint32_t name, const Matrix4& mat);
		void SetUniformMat4f(const std::string& name, const Matrix4& mat);

		int FindUniformLocation(std::string uniform);

	private:
		ShaderSource ParseShader(const std::string& filePath);
		unsigned int CreateProgram(ShaderSource source);
		unsigned int Compile(unsigned int type, ShaderSource source);
	};
}
