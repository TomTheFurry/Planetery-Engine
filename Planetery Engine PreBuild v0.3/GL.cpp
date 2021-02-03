#include "Logger.h"

#include "GL.h"
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

using namespace gl;

struct _state {
	Ptr<FrameBuffer> fbo{};
	Ptr<VertexAttributeArray> vao{};
	Ptr<ShaderStorageBuffer> ssbo{};
	Ptr<ShaderProgram> spo{};
	std::vector<Ptr<Texture>> texUnit{};
	bool useDepth = false;
	bool useBlend = false;
	bool useCull = false;
	bool useStencil = false;
	bool useMultisample = true;
	GLEnum depthFunc = GL_LESS;
	std::pair<GLEnum, GLEnum> blendFunc = std::make_pair(GL_ONE, GL_ZERO);
	GLEnum cullFace = GL_BACK;
	uvec4 viewport = uvec4{0,0,1920,1080};
	vec2 pixelPerInch = vec2{0,0};
};

static _state state;
static uint _maxTexUnit = 0;

//Base
void Base::addLink() { _refCount++; }
void Base::release() {
	_refCount--;
	if (_refCount==0) delete this;
}

//Shader
Shader::Shader(ShaderType type) {
	switch (type) {
	case gl::ShaderType::Vertex:
		_id = glCreateShader(GL_VERTEX_SHADER);
		break;
	case gl::ShaderType::Fragment:
		_id = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	case gl::ShaderType::Geometry:
		_id = glCreateShader(GL_GEOMETRY_SHADER);
		break;
	case gl::ShaderType::TessControl:
		_id = glCreateShader(GL_TESS_CONTROL_SHADER);
		break;
	case gl::ShaderType::TessEvaluation:
		_id = glCreateShader(GL_TESS_EVALUATION_SHADER);
		break;
	default:
		throw;
	}
}
void Shader::setSource(size_t arrayCount, const char*const* stringArray, const int* lengthArray) {
	glShaderSource(_id, arrayCount, stringArray, lengthArray);
}
bool Shader::compile() {
	glCompileShader(_id);
	int stat;
	glGetShaderiv(_id, GL_COMPILE_STATUS, &stat);
	return stat==GL_TRUE;
}
Shader::~Shader() { glDeleteShader(_id); }

//ShaderProgram
ShaderProgram::ShaderProgram() { _id = glCreateProgram(); }
void ShaderProgram::attachShader(Ptr<Shader> sd) {
	glAttachShader(_id, sd.id());
	_sd.emplace_back(sd);
}
bool ShaderProgram::linkProgram() {
	glLinkProgram(_id);
	int stat;
	glGetProgramiv(_id, GL_LINK_STATUS, &stat);
	return stat==GL_TRUE;
}

void gl::ShaderProgram::use() {
	if (state.spo.id()!=_id) {
		glUseProgram(_id);
		state.spo = this;
	}
}

void gl::ShaderProgram::setUniform(const std::string& valueNameP, bool value) {
	glUniform1ui(glGetUniformLocation(_id, valueNameP.c_str()), (uint)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uint value) {
	glUniform1ui(glGetUniformLocation(_id, valueNameP.c_str()), value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uvec2 value) {
	glUniform2ui(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uvec3 value) {
	glUniform3ui(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y, value.z);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uvec4 value) {
	glUniform4ui(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y, value.z, value.w);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uint* value, uint length) {
	glUniform1uiv(glGetUniformLocation(_id, valueNameP.c_str()), length, value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uvec2* value, uint length) {
	glUniform2uiv(glGetUniformLocation(_id, valueNameP.c_str()), length, (uint*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uvec3* value, uint length) {
	glUniform3uiv(glGetUniformLocation(_id, valueNameP.c_str()), length, (uint*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, uvec4* value, uint length) {
	glUniform4uiv(glGetUniformLocation(_id, valueNameP.c_str()), length, (uint*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, int value) {
	glUniform1i(glGetUniformLocation(_id, valueNameP.c_str()), value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, ivec2 value) {
	glUniform2i(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, ivec3 value) {
	glUniform3i(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y, value.z);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, ivec4 value) {
	glUniform4i(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y, value.z, value.w);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, int* value, uint length) {
	glUniform1iv(glGetUniformLocation(_id, valueNameP.c_str()), length, value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, ivec2* value, uint length) {
	glUniform2iv(glGetUniformLocation(_id, valueNameP.c_str()), length, (int*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, ivec3* value, uint length) {
	glUniform3iv(glGetUniformLocation(_id, valueNameP.c_str()), length, (int*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, ivec4* value, uint length) {
	glUniform4iv(glGetUniformLocation(_id, valueNameP.c_str()), length, (int*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, float value) {
	glUniform1f(glGetUniformLocation(_id, valueNameP.c_str()), value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, vec2 value) {
	glUniform2f(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, vec3 value) {
	glUniform3f(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y, value.z);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, vec4 value) {
	glUniform4f(glGetUniformLocation(_id, valueNameP.c_str()), value.x, value.y, value.z, value.w);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, float* value, uint length) {
	glUniform1fv(glGetUniformLocation(_id, valueNameP.c_str()), length, value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, vec2* value, uint length) {
	glUniform2fv(glGetUniformLocation(_id, valueNameP.c_str()), length, (float*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, vec3* value, uint length) {
	glUniform3fv(glGetUniformLocation(_id, valueNameP.c_str()), length, (float*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, vec4* value, uint length) {
	glUniform4fv(glGetUniformLocation(_id, valueNameP.c_str()), length, (float*)value);
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, mat2 value) {
	glUniformMatrix2fv(glGetUniformLocation(_id, valueNameP.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, mat3 value) {
	glUniformMatrix3fv(glGetUniformLocation(_id, valueNameP.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void gl::ShaderProgram::setUniform(const std::string& valueNameP, mat4 value) {
	glUniformMatrix4fv(glGetUniformLocation(_id, valueNameP.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(_id);
}

constexpr const char* SHADER_PATH = "shader/";
inline std::string _tryRead(const char* fileName, const char* pointName) {
	std::string result;
	std::ifstream file;
	std::stringstream strStream;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	file.open(SHADER_PATH + (fileName + std::string(pointName)));
	strStream << file.rdbuf();
	file.close();
	result = strStream.str();
	return result;
}

Ptr<ShaderProgram> gl::makeShaderProgram(const char* fileName, bool hasGeomShader) {
	logger.newMessage();
	logger << "GL: Loading shader " << fileName << "...\n";

	Ptr program(new ShaderProgram());
	{
		Ptr vertShader(new Shader(ShaderType::Vertex));
		try {
			auto vectStr = _tryRead(fileName, ".vert");
			const char* c = vectStr.c_str();
			const int s = vectStr.size();
			vertShader->setSource(1, &c, &s);
			if (!vertShader->compile()) {
				logger("Error! Shader load failed! Cannot compile ", SHADER_PATH, fileName, ".vert. Returning!\n");
				logger.closeMessage();
				return {};
			}
		} catch (std::ifstream::failure e) {
			logger("Error! Shader loading failed! Cannot load file ", SHADER_PATH, fileName, ".vert. Returning!\n");
			logger.closeMessage();
			return {};
		}
		program->attachShader(vertShader);
	}
	{
		Ptr fragShader(new Shader(ShaderType::Fragment));
		try {
			auto fragStr = _tryRead(fileName, ".frag");
			const char* c = fragStr.c_str();
			const int s = fragStr.size();
			fragShader->setSource(1, &c, &s);
			if (!fragShader->compile()) {
				logger("Error! Shader load failed! Cannot compile ", SHADER_PATH, fileName, ".frag. Returning!\n");
				logger.closeMessage();
				return {};
			}
		} catch (std::ifstream::failure e) {
			logger("Error! Shader loading failed! Cannot load file ", SHADER_PATH, fileName, ".frag. Returning!\n");
			logger.closeMessage();
			return {};
		}
		program->attachShader(fragShader);
	}
	if (hasGeomShader) {
		Ptr geomShader(new Shader(ShaderType::Geometry));
		try {
			auto geomStr = _tryRead(fileName, ".geom");
			const char* c = geomStr.c_str();
			const int s = geomStr.size();
			geomShader->setSource(1, &c, &s);
			if (!geomShader->compile()) {
				logger("Error! Shader load failed! Cannot compile ", SHADER_PATH, fileName, ".geom. Returning!\n");
				logger.closeMessage();
				return {};
			}
		} catch (std::ifstream::failure e) {
			logger("Error! Shader loading failed! Cannot load file ", SHADER_PATH, fileName, ".geom. Returning!\n");
			logger.closeMessage();
			return {};
		}
		program->attachShader(geomShader);
	}
	if (!program->linkProgram()) {
		logger("Error! Shader loading failed! Cannot link ", fileName, " shaders together. Returning!\n");
		logger.closeMessage();
		return {};
	}
	return program;
}

//BufferBase
BufferBase::BufferBase() { _mapPointer = nullptr; glCreateBuffers(1, &_id); }
void gl::BufferBase::setFormatAndData(size_t size, GLflags usageFlags, const void* data) {
	glNamedBufferStorage(_id, size, data, usageFlags);
}
void BufferBase::editData(size_t size, const void* data, size_t atOffset) {
	glNamedBufferSubData(_id, atOffset, size, data);
}
void* gl::BufferBase::map(GLEnum access) {
	return _mapPointer = glMapNamedBuffer(_id, access);
}
void* gl::BufferBase::getMapPointer() {
	return _mapPointer;
}
void gl::BufferBase::unmap() {
	glUnmapNamedBuffer(_id);
}
void gl::BufferBase::reset() {
	glInvalidateBufferData(_id);
}
BufferBase::~BufferBase() {
	glDeleteBuffers(1, &_id);
}

//VertexBuffer

//IndiceBuffer

//ShaderStorageBuffer

//VertexAttributeArray
VertexAttributeArray::VertexAttributeArray(Ptr<IndiceBuffer> ib) :
	_ib{ib} {
	glCreateVertexArrays(1, &_id);
	if (_ib) {
		glVertexArrayElementBuffer(_id, ib.id());
	}
}
void VertexAttributeArray::setAttribute(uint atbId, Attribute atb, GLEnum internalType) {
	glEnableVertexArrayAttrib(_id, atbId);
	switch (internalType) {
	case GL_INT:
	case GL_UNSIGNED_INT:
		glVertexArrayAttribIFormat(_id, atbId, atb.dataSize, atb.type, atb.offset);
		break;
	case GL_FLOAT:
		glVertexArrayAttribFormat(_id, atbId, atb.dataSize, atb.type, atb.normalized, atb.offset);
		break;
	case GL_DOUBLE:
		glVertexArrayAttribLFormat(_id, atbId, atb.dataSize, atb.type, atb.offset);
		break;
	default:
		throw;
	}
}
void VertexAttributeArray::setBufferBinding(uint bfbId, BufferBinding bfb) {
	_bfb.insert_or_assign(bfbId, bfb);
	glVertexArrayVertexBuffer(_id, bfbId, bfb.targetBuffer.id(), bfb.offset, bfb.stride);
}
void VertexAttributeArray::bindAttributeToBufferBinding(uint atbId, uint bfbId) {
	glVertexArrayAttribBinding(_id, atbId, bfbId);
}
void VertexAttributeArray::bindIndiceBuffer(Ptr<IndiceBuffer> ib) {
	_ib = ib;
	glVertexArrayElementBuffer(_id, _ib.id());
}
VertexAttributeArray::~VertexAttributeArray() = default;

//Texture2D
Texture2D::Texture2D() { glCreateTextures(GL_TEXTURE_2D, 1, &_id); }
void Texture2D::setFormat(GLEnum internalFormat, size_t width, size_t height, uint levels) {
	glTextureStorage2D(_id, levels, internalFormat, width, height);
}
void gl::Texture2D::setData(int x, int y, uint w, uint h, uint level, GLEnum dataFormat, GLEnum dataType, const void* data) {
	glTextureSubImage2D(_id, level, x, y, w, h, dataFormat, dataType, data);
}
void gl::Texture2D::_cloneData(const Texture2D& source, uvec2 pos, uvec2 size, uint level) {
	glCopyImageSubData(source._id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, _id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, size.x, size.y, 1);
}
void gl::Texture2D::_cloneData(const Texture2D& source, uvec2 pos, uvec2 size, uint level, uvec2 targetPos, uint targetLevel) {
	glCopyImageSubData(source._id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, _id, GL_TEXTURE_2D, targetLevel, targetPos.x, targetPos.y, 0, size.x, size.y, 1);
}
Texture2D::~Texture2D() { glDeleteTextures(1, &_id); }

//RenderBuffer
RenderBuffer::RenderBuffer() { glCreateRenderbuffers(1, &_id); }
void gl::RenderBuffer::setFormat(GLEnum internalFormat, size_t width, size_t height) {
	glNamedRenderbufferStorage(_id, internalFormat, width, height);
}
RenderBuffer::~RenderBuffer() { glDeleteRenderbuffers(1, &_id); }

//FrameBuffer
FrameBuffer::FrameBuffer() { glCreateFramebuffers(1, &_id); }
void FrameBuffer::attach(Ptr<RenderBuffer> rb, GLEnum attachmentPoint) {
	glNamedFramebufferRenderbuffer(_id, attachmentPoint, GL_RENDERBUFFER, rb.id());
	_rb.emplace_back(rb);
}
void FrameBuffer::attach(Ptr<Texture> tx, GLEnum attachmentPoint, int level) {
	glNamedFramebufferTexture(_id, attachmentPoint, tx.id(), level);
	_rb.emplace_back(tx);
}
FrameBuffer::~FrameBuffer() = default;


//RenderTarget
RenderTarget::RenderTarget() {
	useDepth = false;
	useBlend = false;
	useCull = false;
	useStencil = false;
	useMultisample = true;
	depthFunc = GL_LESS;
	blendFunc = std::make_pair(GL_ONE, GL_ZERO);
	cullFace = GL_BACK;
	texUnit.reserve(_maxTexUnit);
	for (uint i = 0; i<_maxTexUnit; i++) texUnit.emplace_back();
	viewport = uvec4(0, 0, 1920, 1080);
	pixelPerInch = vec2(0, 0);
}
void RenderTarget::bind(Ptr<FrameBuffer> fb) { fbo = fb; }
void RenderTarget::bind(Ptr<VertexAttributeArray> va) { vao = va; }
void RenderTarget::bind(Ptr<ShaderStorageBuffer> ssb) { ssbo = ssb; }
void RenderTarget::bind(Ptr<Texture> tx, uint i) {
	if (i>=_maxTexUnit) throw "Max texture unit reached!";
	texUnit[i] = tx;
}
void RenderTarget::bind(Ptr<ShaderProgram> sp) { spo = sp; }


void gl::RenderTarget::setViewport(uint x, uint y, uint width, uint height) {
	viewport = uvec4{x,y,width,height};
}

void RenderTarget::drawArrays(GLEnum mode, uint first, size_t count) {
	_use();
	glDrawArrays(mode, first, count);
}
void RenderTarget::drawArraysInstanced(GLEnum mode, uint first, size_t count, size_t instanceCount) {
	_use();
	glDrawArraysInstanced(mode, first, count, instanceCount);
}
void RenderTarget::drawElements(GLEnum mode, size_t count, GLEnum type, const void* indices) {
	_use();
	glDrawElements(mode, count, type, indices);
}
void RenderTarget::drawElementsInstanced(GLEnum mode, size_t count, GLEnum type, size_t instanceCount, const void* indices) {
	_use();
	glDrawElementsInstanced(mode, count, type, indices, instanceCount);
}
void gl::RenderTarget::activateFrameBuffer() {
	if (state.fbo!=fbo)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo.id());
}
RenderTarget::~RenderTarget() = default;

//----Hashtag Macro HELL!!! HAHAHAHAHAHAHAHAHAHA~~~
#define _M_BOOL(a,b)\
if (state.##a != ##a) {\
	##a ? glEnable(b) : glDisable(b);\
	state.##a = ##a;\
}
#define _M_SET(a,b)\
if (state.##a != ##a) {\
	b;\
	state.##a = ##a;\
}

void RenderTarget::_use() {
	_M_BOOL(useDepth, GL_DEPTH_TEST);
	_M_BOOL(useBlend, GL_BLEND);
	_M_BOOL(useCull, GL_CULL_FACE);
	_M_BOOL(useStencil, GL_STENCIL_TEST);
	_M_BOOL(useMultisample, GL_MULTISAMPLE);
	if (useDepth)
		_M_SET(depthFunc, glDepthFunc(depthFunc));
	if (useBlend)
		_M_SET(blendFunc, glBlendFunc(blendFunc.first, blendFunc.second));
	if (useCull)
		_M_SET(cullFace, glCullFace(cullFace));
	//TODO: add stencil func and stuff...
	_M_SET(fbo, glBindFramebuffer(GL_FRAMEBUFFER, fbo.id()));
	_M_SET(vao, glBindVertexArray(vao.id()));
	_M_SET(ssbo, glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo.id()));
	_M_SET(spo, glUseProgram(spo.id()));
	for (uint i = 0; i < texUnit.size(); i++) {
		if (texUnit[i] && texUnit[i]!=state.texUnit[i]) {
			glBindTextureUnit(i, texUnit[i].id());
			state.texUnit[i] = texUnit[i];
		}
	}
	if (viewport != state.viewport) {
		glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
		state.viewport = viewport;
	}
}

//----Hashtag Macro HELL version 2 !!! HAHAHAHAHAHAHAHAHAHA~~~
#define _M_BOOL_S(a,b)\
	state.##a ? glEnable(b) : glDisable(b)


void gl::init() {
	logger("GL Interface init.\n");
	int __maxTexU;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &__maxTexU);
	_maxTexUnit = uint(__maxTexU);
	state.texUnit.reserve(_maxTexUnit);
	for (uint i = 0; i<_maxTexUnit; i++) state.texUnit.emplace_back();
	gl::target = new RenderTarget();
	assert(gl::target!=nullptr);
	_M_BOOL_S(useDepth, GL_DEPTH_TEST);
	_M_BOOL_S(useBlend, GL_BLEND);
	_M_BOOL_S(useCull, GL_CULL_FACE);
	_M_BOOL_S(useStencil, GL_STENCIL_TEST);
	_M_BOOL_S(useMultisample, GL_MULTISAMPLE);
	glDepthFunc(state.depthFunc);
	glBlendFunc(state.blendFunc.first, state.blendFunc.second);
	glCullFace(state.cullFace);
	//TODO: add stencil func and stuff...
	glBindFramebuffer(GL_FRAMEBUFFER, state.fbo.id());
	glBindVertexArray(state.vao.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, state.ssbo.id());
	glUseProgram(state.spo.id());
	glViewport(state.viewport.x, state.viewport.y, state.viewport.z, state.viewport.w);
	//Should I unbind all texture units?
}

void gl::end() {
	logger("GL Interface end.\n");
	if (target==nullptr) throw;
	if (target->fbo) logger("WARNING!!! gl end() called when target is not the default original target! May point to memory leak! (Note that this catch is not always successful)\n");
	delete target;
}

uint gl::getMaxTextureSize() {
	int size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	return (uint)size;
}

RenderTarget* gl::target = nullptr;
