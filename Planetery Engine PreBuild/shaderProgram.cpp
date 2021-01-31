#include "ShaderProgram.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>

using str = std::string;
using file = std::ifstream;
using strStream = std::stringstream;

ShaderProgram::ShaderProgram(str vertexPathP, str fragmentPathP)
{
	vPath = vertexPathP;
	fPath = fragmentPathP;
	loadFile();
}

void ShaderProgram::enable()
{
	glUseProgram(id);
}

void ShaderProgram::setBool(str valueNameP, bool value) const
{
	glUseProgram(id);
	glUniform1i(glGetUniformLocation(id, valueNameP.c_str()), (int)value);
}

void ShaderProgram::setInt(str valueNameP, int value) const
{
	glUseProgram(id);
	glUniform1i(glGetUniformLocation(id, valueNameP.c_str()), value);
}

void ShaderProgram::setFloat(str valueNameP, float value) const
{
	glUseProgram(id);
	glUniform1f(glGetUniformLocation(id, valueNameP.c_str()), value);
}

void ShaderProgram::setVec(str valueNameP, float x, float y, float z, float w) const
{
	glUseProgram(id);
	glUniform4f(glGetUniformLocation(id, valueNameP.c_str()), x,y,z,w);
}

ShaderProgram::~ShaderProgram()
{
	if (shaderInit)
	{
		//glDeleteProgram(id);
	}
}

void ShaderProgram::loadFile()
{
	//get code from path
	str vCode;
	str fCode;
	file vFile;
	file fFile;
	vFile.exceptions(file::failbit | file::badbit);
	fFile.exceptions(file::failbit | file::badbit);
	try
	{
		vFile.open(vPath);
		fFile.open(fPath);
		strStream vStream, fStream;
		vStream << vFile.rdbuf();
		fStream << fFile.rdbuf();
		vFile.close();
		fFile.close();
		vCode = vStream.str();
		fCode = fStream.str();
	}
	catch (file::failure e) {
		logger << "ERROR::SHADER::FILE_READING_FAILED" << "\n";
		if (shaderInit)
		{
			logger << "WARNING::SHADER::USING_PREVIOUS_VERSION" << "\n";
			return;
		}
		else
		{
			throw;
		}
	}

	//comple shaders
	unsigned int vertexId, fragmentId;
	int success = 0;
	char infoLog[512];

	const char* c_vCode = vCode.c_str();
	vertexId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexId, 1, &c_vCode, NULL);
	glCompileShader(vertexId);
	glGetShaderiv(vertexId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexId, 512, NULL, infoLog);
		logger << "ERROR::SHADER:VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
		if (shaderInit)
		{
			logger << "WARNING::SHADER::USING_PREVIOUS_VERSION" << "\n";
			glDeleteShader(vertexId);
			return;
		}
		else
		{
			throw;
		}
	}

	success = 0;
	const char* c_fCode = fCode.c_str();
	fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentId, 1, &c_fCode, NULL);
	glCompileShader(fragmentId);
	glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentId, 512, NULL, infoLog);
		logger << "ERROR::SHADER:FRAGMENT::COMPILATION_FAILED\n" << infoLog << "\n";
		if (shaderInit)
		{
			logger << "WARNING::SHADER::USING_PREVIOUS_VERSION" << "\n";
			glDeleteShader(fragmentId);
			return;
		}
		else
		{
			throw;
		}
	}

	success = 0;
	unsigned int tempId;
	tempId = glCreateProgram();
	glAttachShader(tempId, vertexId);
	glAttachShader(tempId, fragmentId);
	glLinkProgram(tempId);
	glGetProgramiv(tempId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(tempId, 512, NULL, infoLog);
		logger << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
		if (shaderInit)
		{
			logger << "WARNING::SHADER::USING_PREVIOUS_VERSION" << "\n";
			glDeleteShader(vertexId);
			glDeleteShader(fragmentId);
			glDeleteProgram(tempId);
			return;
		}
		else
		{
			throw;
		}
	}
	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);
	id = tempId;
	shaderInit = true;
	logger << "LOG::SHADER::PROGRAM::LOADING_SUCCESSFUL:" << std::to_string(id) << "\n";

}
