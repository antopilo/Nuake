#pragma once
#include "Shader.h"
#include <sstream>
#include <GL\glew.h>

#define ASSERT(x) if (!(x)) assert(false)

// TODO: Register all uniform when creating shader.
Shader::Shader(const std::string& filePath) {
	Source = ParseShader(filePath);

	ProgramId = CreateProgram();
}

// Bind the shader
void Shader::Bind() const {
	glUseProgram(ProgramId);
}

// unbind the shader
void Shader::Unbind() const {
	glUseProgram(0);
}

// Prase the shader file and create string source.
ShaderSource Shader::ParseShader(const std::string& filePath) {
	std::ifstream stream(filePath);

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	ShaderType type = ShaderType::NONE;

	std::string line;
	std::stringstream ss[2];

	while (getline(stream, line))
	{
		// If the current lines contains #shader
		if (line.find("#shader") != std::string::npos)
		{
			// If the current line containes vertex
			if (line.find("vertex") != std::string::npos) {
				// set mode to vertex
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) {
				// set mode to fragment
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << "\n";
		}
	}

	return { ss[0].str(), ss[1].str() };
}

// Parse both source and creates a shader program.
// This returns the program id.
unsigned int Shader::CreateProgram() {
	unsigned int program = glCreateProgram();
	unsigned int vs = Compile(GL_VERTEX_SHADER);
	unsigned int fs = Compile(GL_FRAGMENT_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

// Compile a single shader and checks for error.
unsigned int Shader::Compile(unsigned int type)
{
	unsigned int id = glCreateShader(type);

	const char* src;
	if (type == GL_FRAGMENT_SHADER) src = Source.FragmentShader.c_str();
	if (type == GL_VERTEX_SHADER)   src = Source.VertexShader.c_str();

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

		std::cout << "Failed to compile " <<
			(type == GL_VERTEX_SHADER ? "vertex" : "Fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;

		// Delete invalid shader
		glDeleteShader(id);
		return 0;
	}

	return id;
}

// Retrieve uniform address and registers it if not already registered.
int Shader::FindUniformLocation(std::string uniform) {
	if (UniformCache.find(uniform) == UniformCache.end()) {
		int addr = glGetUniformLocation(ProgramId, uniform.c_str());

		if (addr == -1)
			std::cout << "Warning: uniform '" << uniform << "' doesn't exists!" << std::endl;
		else {
			std::cout << "Info: uniform '" << uniform << "' registered." << std::endl;
			UniformCache[uniform] = addr;
		}


		return addr;
	}

	return UniformCache[uniform];
}

// Uniforms
void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniform4f(addr, v0, v1, v2, v3);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniform3f(addr, v0, v1, v2);
}

void Shader::SetUniform2f(const std::string& name, float v0, float v1) {
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniform2f(addr, v0, v1);
}

void Shader::SetUniform1i(const std::string& name, int v0) {
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniform1i(addr, v0);
}

void Shader::SetUniform1iv(const std::string& name, int size, int* value)
{
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniform1iv(addr, size, value);
}

void Shader::SetUniformMat3f(const std::string& name, glm::mat3 mat)
{
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniformMatrix3fv(addr, 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetUniformMat4f(const std::string& name, glm::mat4 mat)
{
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniformMatrix4fv(addr, 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetUniform1f(const std::string& name, float v0) {
	int addr = FindUniformLocation(name);
	ASSERT(addr != -1);
	glUniform1f(addr, v0);
}

