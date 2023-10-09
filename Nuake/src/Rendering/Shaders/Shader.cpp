#include "src/Core/Core.h"
#include "Shader.h"
#include "src/Core/FileSystem.h"
#include "src/Core/Logger.h"

#include <sstream>
#include <glad/glad.h>


namespace Nuake
{
	// TODO: Register all uniform when creating shader.
	Shader::Shader(const std::string& filePath) 
	{
		Path = filePath;

		if (FileSystem::FileExists(filePath, true))
		{
			if (const auto& fileContent = FileSystem::ReadFile(filePath);
				!fileContent.empty())
			{
				Source = ParseShader(fileContent);
				ProgramId = CreateProgram(Source);
				return;
			}
		}

		Logger::Log("Shader source file not found: " + filePath, "shader", CRITICAL);
	}

	Shader::Shader(const std::string& filePath, const std::string& content)
	{
		Path = filePath;

		Source = ParseShader(content);
		ProgramId = CreateProgram(Source);
	}

	bool Shader::Rebuild()
	{
		if (!FileSystem::FileExists(Path, true))
		{
			Logger::Log("Failed to reload shader, file doesn't exists: " + Path, "shader", CRITICAL);
			return false;
		}

		ShaderSource newSource = ParseShader(FileSystem::ReadFile(this->Path));
		unsigned int newProgramId = CreateProgram(newSource);

		if(newProgramId == 0)
			return false;

		Source = newSource;
		ProgramId = newProgramId;

		return true;
	}

	// Bind the shader
	void Shader::Bind() const 
	{
		glUseProgram(ProgramId);
	}

	// unbind the shader
	void Shader::Unbind() const 
	{
		glUseProgram(0);
	}

	// Prase the shader file and create string source.
	ShaderSource Shader::ParseShader(const std::string& content) 
	{
		std::stringstream stream(content.c_str());

		enum class ShaderType 
		{
			NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2, COMPUTE = 3
		};

		ShaderType type = ShaderType::NONE;

		std::string line;
		std::stringstream ss[4];

		while (getline(stream, line))
		{
			// If the current lines contains #shader
			if (line.find("#shader") != std::string::npos)
			{
				// If the current line containes vertex
				if (line.find("vertex") != std::string::npos) 
				{
					// set mode to vertex
					type = ShaderType::VERTEX;
				}
				else if (line.find("fragment") != std::string::npos) 
				{
					// set mode to fragment
					type = ShaderType::FRAGMENT;
				}
				else if (line.find("geometry") != std::string::npos)
				{
					type = ShaderType::GEOMETRY;
				}
				else if (line.find("compute") != std::string::npos)
				{
					type = ShaderType::COMPUTE;
				}
			}
			else
			{
				ss[(int)type] << line << "\n";
			}
		}

		return { ss[0].str(), ss[1].str(), ss[2].str(), ss[3].str() };
	}

	// Parse both source and creates a shader program.
	// This returns the program id.
	unsigned int Shader::CreateProgram(ShaderSource source)
	{
		unsigned int program = glCreateProgram();

		unsigned int vs = Compile(GL_VERTEX_SHADER, source);
		unsigned int fs = Compile(GL_FRAGMENT_SHADER, source);
		unsigned int gs = 0;
		unsigned int cs = 0;

		if (vs == 0)
			return 0;

		if (fs == 0)
			return 0;

		if (Source.GeometryShader != "")
		{
			gs = Compile(GL_GEOMETRY_SHADER, source);
			glAttachShader(program, gs);
		}

		if (Source.ComputeShader != "")
		{
			cs = Compile(GL_COMPUTE_SHADER, source);
			glAttachShader(program, cs);
		}

		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(vs);
		glDeleteShader(fs);

		if (gs != 0)
			glDeleteShader(gs);
		if (cs != 0)
			glDeleteShader(cs);

		Source = {};

		return program;
	}

	// Compile a single shader and checks for error.
	unsigned int Shader::Compile(unsigned int type, ShaderSource source)
	{
		unsigned int id = glCreateShader(type);

		const char* src;
		if (type == GL_FRAGMENT_SHADER) src = source.FragmentShader.c_str();
		if (type == GL_VERTEX_SHADER)   src = source.VertexShader.c_str();
		if (type == GL_GEOMETRY_SHADER) src = source.GeometryShader.c_str();
		if (type == GL_COMPUTE_SHADER)  src = Source.ComputeShader.c_str();

		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		//TODO: Error handling
		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);

		// Shader error.
		if (result == GL_FALSE)
		{
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

			// We allocate the size of the array using 
			// m alloc. casted as a pointer.
			char* message = (char*)malloc(length * sizeof(char));

			glGetShaderInfoLog(id, length, &length, message);

			std::string stype;
			if (type == GL_FRAGMENT_SHADER) stype = "Fragment";
			if (type == GL_VERTEX_SHADER)  stype = "Vertex";
			if (type == GL_GEOMETRY_SHADER) stype = "Geometry";
			if (type == GL_COMPUTE_SHADER) stype = "Compute";

			std::cout << "Failed to compile " <<
				(type == GL_VERTEX_SHADER ? "vertex" : "Fragment") << " shader!" << std::endl;

			std::cout << message << std::endl;
			// Delete invalid shader
			glDeleteShader(id);

			delete[] message;

			return 0;
		}

		return id;
	}

	// Retrieve uniform address and registers it if not already registered.
	int Shader::FindUniformLocation(std::string uniform) 
	{
		if (UniformCache.find(uniform) == UniformCache.end())
		{
			int addr = glGetUniformLocation(ProgramId, uniform.c_str());

			if (addr == -1)
				return addr;
			else 
				UniformCache[uniform] = addr;

			return addr;
		}

		return UniformCache[uniform];
	}

	// Uniforms

	void Shader::SetUniformVec4(const std::string& name, Vector4 vec)
	{
		SetUniform4f(name, vec.x, vec.y, vec.z, vec.w);
	}

	void Shader::SetUniformVec3(const std::string& name, Vector3 vec)
	{
		SetUniform3f(name, vec.x, vec.y, vec.z);
	}

	void Shader::SetUniformVec2(const std::string& name, Vector2 vec)
	{
		SetUniform2f(name, vec.x, vec.y);
	}

	void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) 
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);

		if (addr != -1)
			glUniform4f(addr, v0, v1, v2, v3);
	}

	void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) 
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);

		if (addr != -1)
			glUniform3f(addr, v0, v1, v2);
	}

	void Shader::SetUniform2f(const std::string& name, float v0, float v1) 
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);

		if (addr != -1)
			glUniform2f(addr, v0, v1);
	}

	void Shader::SetUniform1i(const std::string& name, int v0)
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);

		if (addr != -1)
			SetUniform1i(addr, v0);
	}

	void Shader::SetUniform1i(uint32_t location, int v0)
	{
		glUniform1i(location, v0);
	}

	void Shader::SetUniform1iv(const std::string& name, int size, int* value)
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);
		if (addr != -1)
			glUniform1iv(addr, size, value);
	}

	void Shader::SetUniform1fv(const std::string& name, int size, float* value)
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);
		if (addr != -1)
			glUniform1fv(addr, size, value);
	}

	void Shader::SetUniformMat3f(const std::string& name, Matrix3 mat)
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);
		if(addr != -1)
			glUniformMatrix3fv(addr, 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::SetUniformMat4f(uint32_t location, const Matrix4& mat)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::SetUniformMat4f(const std::string& name, const Matrix4& mat)
	{
		int addr = FindUniformLocation(name);

		if (addr != -1)
		{
			SetUniformMat4f(addr, mat);
		}
	}

	void Shader::SetUniform1f(const std::string& name, float v0) 
	{
		int addr = FindUniformLocation(name);
		//ASSERT(addr != -1);

		if (addr != -1)
			glUniform1f(addr, v0);
	}


	void Shader::SetUniformTex(const std::string& name, Texture* texture, unsigned int slot)
	{
		//ASSERT(texture != nullptr);

		SetUniform1i(name, slot);
		texture->Bind(slot);
	}
}
