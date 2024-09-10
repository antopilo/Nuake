#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "glm/gtc/matrix_transform.hpp"
#include "src/Core/Maths.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Core/Core.h"

namespace Nuake
{
	struct ShaderSource 
	{
		std::string VertexShader;
		std::string FragmentShader;
		std::string GeometryShader;
		std::string ComputeShader;
	};

	enum class UniformTypes
	{
		Int, Uint, Float, Bool, Vec2, Vec3, Vec4, Mat3, Mat4, Sampler2D
	};

	union UniformValue 
	{
		float valueFloat;
		int valueInt;
		unsigned int valueUInt;
		Vector2 valueVec2;
		Vector3 valueVec3;
		Vector4 valueVec4;
		Matrix3 valueMat3;
		Matrix4 valueMat4;
	};

	struct UniformVariable
	{
		std::string name;
		UniformValue value;
		UniformTypes type;

		UniformVariable(const std::string& uniformName, float uniformValue)
		{
			name = uniformName;
			value.valueFloat = uniformValue;
			type = UniformTypes::Float;
		}

		UniformVariable(const std::string& uniformName, int uniformValue)
		{
			name = uniformName;
			value.valueInt = uniformValue;
			type = UniformTypes::Int;
		}

		UniformVariable(const std::string& uniformName, bool uniformValue)
		{
			name = uniformName;
			value.valueInt = static_cast<int>(uniformValue);
			type = UniformTypes::Int;
		}

		UniformVariable(const std::string& uniformName, Vector2 uniformValue)
		{
			name = uniformName;
			value.valueVec2 = uniformValue;
			type = UniformTypes::Vec2;
		}

		UniformVariable(const std::string& uniformName, Vector3 uniformValue)
		{
			name = uniformName;
			value.valueVec3 = uniformValue;
			type = UniformTypes::Vec3;
		}

		UniformVariable(const std::string& uniformName, Vector4 uniformValue)
		{
			name = uniformName;
			value.valueVec4 = uniformValue;
			type = UniformTypes::Vec4;
		}

		UniformVariable(const std::string& uniformName, Matrix3 uniformValue)
		{
			name = uniformName;
			value.valueMat3 = uniformValue;
			type = UniformTypes::Mat3;
		}

		UniformVariable(const std::string& uniformName, Matrix4 uniformValue)
		{
			name = uniformName;
			value.valueMat4 = uniformValue;
			type = UniformTypes::Mat4;
		}
	};

	class Shader 
	{
	private:
		uint32_t programId;
		std::string Path;
		ShaderSource Source;

		std::unordered_map<std::string, int> UniformCache;

		std::map<std::string, unsigned int> mUniforms;
		std::map<std::string, UniformTypes> mUniformsType;
		std::string mError = "";

	public:
		Shader(const std::string& filePath);
		Shader(const std::string& path, const std::string& content);

		bool Rebuild();

		void Bind() const;
		void Unbind() const;

		std::map<std::string, UniformTypes> GetUniforms()
		{
			return mUniformsType;
		}

		std::string GetError() const { return mError; }

		void SetUniforms(const std::vector<UniformVariable>& uniforms);

		void SetUniform(const std::string& name, float v0);
		void SetUniform(const std::string& name, float v0, float v1);
		void SetUniform(const std::string& name, float v0, float v1, float v3);
		void SetUniform(const std::string& name, float v0, float v1, float v3, float v4);
		void SetUniformUint(const std::string& name, uint32_t v0);
		void SetUniform(const std::string& name, int v0);
		void SetUniformv(const std::string& name, int size, int* value);
		void SetUniform(const std::string& name, Vector2 v0);
		void SetUniform(const std::string& name, Vector3 v0);
		void SetUniform(const std::string& name, Vector4 v0);
		void SetUniform(const std::string& name, Matrix3 v0);
		void SetUniform(const std::string& name, Matrix4 v0);
		void SetUniform(const std::string& name, int size, float* value);
		void SetUniform(const std::string& name, bool value);
		void SetUniform(const std::string& name, Texture* texture, int slot = 0);

		void SetUniform(uint32_t uniformSlot, int value);
		void SetUniform(uint32_t uniformSlot, Matrix4 value);

		int FindUniformLocation(std::string uniform);

	private:
		ShaderSource ParseShader(const std::string& filePath);
		unsigned int CreateProgram(ShaderSource source);
		unsigned int Compile(unsigned int type, ShaderSource source);
	};
}
