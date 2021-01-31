#pragma once
#include "ShaderProgram.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utility.h"

using str = std::string;
using file = std::ifstream;
using strStream = std::stringstream;

ShaderProgram::ShaderProgram(str vertexPathP, str fragmentPathP)
{
	vPath = vertexPathP;
	fPath = fragmentPathP;
	hasGeoShader = false;
	loadFile();
}

ShaderProgram::ShaderProgram(str vertexPathP, str geometryPathP, str fragmentPathP)
{
	vPath = vertexPathP;
	gPath = geometryPathP;
	fPath = fragmentPathP;
	hasGeoShader = true;
	loadFile();
}

void ShaderProgram::enable()
{
	glUseProgram(id);
}

void ShaderProgram::setBool(str valueNameP, bool value) const
{
	glUniform1i(glGetUniformLocation(id, valueNameP.c_str()), (int)value);
}

void ShaderProgram::setInt(str valueNameP, int value) const
{
	glUniform1i(glGetUniformLocation(id, valueNameP.c_str()), value);
}

void ShaderProgram::setFloat(str valueNameP, float value) const
{
	glUniform1f(glGetUniformLocation(id, valueNameP.c_str()), value);
}

void ShaderProgram::setVec3(str valueNameP, glm::vec3 value) const
{
	glUniform3f(glGetUniformLocation(id, valueNameP.c_str()),value.x,value.y,value.z);
}

void ShaderProgram::setVec4(str valueNameP, glm::vec4 value) const
{
	glUniform4f(glGetUniformLocation(id, valueNameP.c_str()), value.x, value.y, value.z, value.w);
}

void ShaderProgram::setMat3(str valueNameP, glm::mat3 value) const
{
	glUniformMatrix3fv(glGetUniformLocation(id, valueNameP.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setMat4(str valueNameP, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, valueNameP.c_str()), 1, GL_FALSE, glm::value_ptr(value));
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
	str gCode;
	str fCode;
	file vFile;
	file gFile;
	file fFile;
	vFile.exceptions(file::failbit | file::badbit);
	gFile.exceptions(file::failbit | file::badbit);
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
		if (hasGeoShader) {
			gFile.open(gPath);
			strStream gStream;
			gStream << gFile.rdbuf();
			gFile.close();
			gCode = gStream.str();
		}
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
	unsigned int vertexId, geometryId, fragmentId;
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

	if (hasGeoShader) {
		success = 0;
		const char* c_gCode = gCode.c_str();
		geometryId = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryId, 1, &c_gCode, NULL);
		glCompileShader(geometryId);
		glGetShaderiv(geometryId, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometryId, 512, NULL, infoLog);
			logger << "ERROR::SHADER:GEOMETRY::COMPILATION_FAILED\n" << infoLog << "\n";
			if (shaderInit)
			{
				logger << "WARNING::SHADER::USING_PREVIOUS_VERSION" << "\n";
				glDeleteShader(geometryId);
				return;
			}
			else
			{
				throw;
			}
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


	//Link shaders
	success = 0;
	unsigned int tempId;
	tempId = glCreateProgram();
	glAttachShader(tempId, vertexId);
	if (hasGeoShader) glAttachShader(tempId, geometryId);
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
			if (hasGeoShader) glDeleteShader(geometryId);
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
	if (hasGeoShader) glDeleteShader(geometryId);
	glDeleteShader(fragmentId);
	id = tempId;
	shaderInit = true;
	logger << "LOG::SHADER::PROGRAM::LOADING_SUCCESSFUL:" << std::to_string(id) << "\n";

}
