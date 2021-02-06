#include "Logger.h"

#include "GL.h"
#include <assert.h>
#include <vector>

#include <glad/glad.h>
#include <glfw/glfw3.h>

using namespace gl;

struct _state {
	uint fbo = 0;
	uint vao = 0;
	uint ssbo = 0;
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
	std::vector<Texture*> texUnit {};
};

static _state state;
static uint _maxTexUnit = 0;

//Base
void Base::addLink() { _refCount++; }
void Base::release() {
	_refCount--;
	if (_refCount==0)
		delete this;
}

//Shader
Shader::Shader(GLEnum type) { id = glCreateShader(type); }
void Shader::setSource(size_t arrayCount, const char*const* stringArray, const int* lengthArray) {
	glShaderSource(id, arrayCount, stringArray, lengthArray);
}
void Shader::compile() { glCompileShader(id); }
Shader::~Shader() { glDeleteShader(id); }

//ShaderProgram
ShaderProgram::ShaderProgram() { id = glCreateProgram(); }
void ShaderProgram::attachShader(Shader* sd) {
	glAttachShader(id, sd->id);
	sd->addLink();
	_sd.push_back(sd);
}
void ShaderProgram::linkProgram() {
	glLinkProgram(id);
}
ShaderProgram::~ShaderProgram() {
	glDeleteProgram(id);
}

//BufferBase
BufferBase::BufferBase() { _mapPointer = nullptr; glCreateBuffers(1, &id); }
void gl::BufferBase::setFormatAndData(size_t size, GLflags usageFlags, const void* data) {
	glNamedBufferStorage(id, size, data, usageFlags);
}
void BufferBase::editData(size_t size, const void* data, size_t atOffset) {
	glNamedBufferSubData(id, atOffset, size, data);
}
void* gl::BufferBase::map(GLEnum access) {
	return _mapPointer = glMapNamedBuffer(id, access);
}
void* gl::BufferBase::getMapPointer() {
	return _mapPointer;
}
void gl::BufferBase::unmap() {
	glUnmapNamedBuffer(id);
}
void gl::BufferBase::reset() {
	glInvalidateBufferData(id);
}
BufferBase::~BufferBase() {
	glDeleteBuffers(1, &id);
}

//VertexBuffer

//IndiceBuffer

//ShaderStorageBuffer

//VertexAttributeArray
VertexAttributeArray::VertexAttributeArray(IndiceBuffer* ib) {
	glCreateVertexArrays(1, &id);
	_ib = ib;
	if (ib!=nullptr) {
		glVertexArrayElementBuffer(id, ib->id);
		ib->addLink();
	}
}
void VertexAttributeArray::setAttribute(uint atbId, Attribute atb, GLEnum internalType) {
	glEnableVertexArrayAttrib(id, atbId);
	switch (internalType) {
	case GL_INT:
	case GL_UNSIGNED_INT:
		glVertexArrayAttribIFormat(id, atbId, atb.dataSize, atb.type, atb.offset);
		break;
	case GL_FLOAT:
		glVertexArrayAttribFormat(id, atbId, atb.dataSize, atb.type, atb.normalized, atb.offset);
		break;
	case GL_DOUBLE:
		glVertexArrayAttribLFormat(id, atbId, atb.dataSize, atb.type, atb.offset);
		break;
	default:
		throw;
	}
}
void VertexAttributeArray::setBufferBinding(uint bfbId, BufferBinding bfb) {
	auto it = _bfb.lower_bound(bfbId);
	if (it!=_bfb.end() && it->first==bfbId) {
		it->second->release();
		it->second = bfb.targetBuffer;
	} else {
		_bfb.emplace_hint(it, std::make_pair(bfbId, bfb.targetBuffer));
	}
	bfb.targetBuffer->addLink();
	glVertexArrayVertexBuffer(id, bfbId, bfb.targetBuffer->id, bfb.offset, bfb.stride);
}
void VertexAttributeArray::bindAttributeToBufferBinding(uint atbId, uint bfbId) {
	glVertexArrayAttribBinding(id, atbId, bfbId);
}
void VertexAttributeArray::bindIndiceBuffer(IndiceBuffer* ib) {
	glVertexArrayElementBuffer(id, ib->id);
}
VertexAttributeArray::~VertexAttributeArray() {
	for (auto& p : _bfb) {
		p.second->release();
	}
	if (_ib != nullptr) _ib->release();
}

//Texture2D
Texture2D::Texture2D() { glCreateTextures(GL_TEXTURE_2D, 1, &id); }
void Texture2D::setFormat(GLEnum internalFormat, size_t width, size_t height, uint levels) {
	glTextureStorage2D(id, levels, internalFormat, width, height);
}
void gl::Texture2D::setData(int x, int y, uint w, uint h, uint level, GLEnum dataFormat, GLEnum dataType, const void* data) {
	glTextureSubImage2D(id, level, x, y, w, h, dataFormat, dataType, data);
}
Texture2D* gl::Texture2D::cloneData(const Texture2D* source, uvec2 pos, uvec2 size, uint level) {
	glCopyImageSubData(source->id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, size.x, size.y, 1);
	return this;
}
Texture2D* gl::Texture2D::cloneData(const Texture2D* source, uvec2 pos, uvec2 size, uint level, uvec2 targetPos, uint targetLevel) {
	glCopyImageSubData(source->id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, id, GL_TEXTURE_2D, targetLevel, targetPos.x, targetPos.y, 0, size.x, size.y, 1);
	return this;
}
Texture2D::~Texture2D() { glDeleteTextures(1, &id); }

//RenderBuffer
RenderBuffer::RenderBuffer() { glCreateRenderbuffers(1, &id); }
void gl::RenderBuffer::setFormat(GLEnum internalFormat, size_t width, size_t height) {
	glNamedRenderbufferStorage(id, internalFormat, width, height);
}
RenderBuffer::~RenderBuffer() { glDeleteRenderbuffers(1, &id); }

//FrameBuffer
FrameBuffer::FrameBuffer() { glCreateFramebuffers(1, &id); }
void FrameBuffer::attach(RenderBuffer* rb, GLEnum attachmentPoint) {
	glNamedFramebufferRenderbuffer(id, attachmentPoint, GL_RENDERBUFFER, rb->id);
	rb->addLink();
	_rb.push_back(rb);
}
void FrameBuffer::attach(Texture* tx, GLEnum attachmentPoint, int level) {
	glNamedFramebufferTexture(id, attachmentPoint, tx->id, level);
	tx->addLink();
	_rb.push_back(tx);
}
FrameBuffer::~FrameBuffer() {
	for (auto b : _rb) b->release();
}


//RenderTarget
RenderTarget::RenderTarget() {
	fbo = nullptr;
	vao = nullptr;
	ssbo = nullptr;
	useDepth = false;
	useBlend = false;
	useCull = false;
	useStencil = false;
	useMultisample = true;
	depthFunc = GL_LESS;
	blendFunc = std::make_pair(GL_ONE, GL_ZERO);
	cullFace = GL_BACK;
	texUnit.reserve(_maxTexUnit);
	for (uint i = 0; i<_maxTexUnit; i++) texUnit.push_back(nullptr);
	viewport = uvec4(0, 0, 1920, 1080);
	pixelPerInch = vec2(0, 0);
}
void RenderTarget::bind(FrameBuffer* fb) {
	if (fbo != nullptr) fbo->release();
	fbo = fb;
	fbo->addLink();
}
void RenderTarget::bind(VertexAttributeArray* va) {
	if (vao != nullptr) vao->release();
	vao = va;
	vao->addLink();
}
void RenderTarget::bind(ShaderStorageBuffer* ssb) {
	if (ssbo != nullptr) ssbo->release();
	ssbo = ssb;
	ssbo->addLink();
}
void gl::RenderTarget::bind(Texture* tx, uint i) {
	if (i>=_maxTexUnit) throw "Max texture unit reached!";
	if (texUnit[i] != nullptr) texUnit[i]->release();
	texUnit[i] = tx;
	tx->addLink();
}

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
	if (state.fbo!=(fbo==nullptr ? 0 : fbo->id))
		glBindFramebuffer(GL_FRAMEBUFFER, (fbo==nullptr ? 0 : fbo->id));
}
RenderTarget* gl::RenderTarget::swapTarget(RenderTarget* target) {
	std::swap(target, gl::target);
	return target;
}
RenderTarget::~RenderTarget() {
	if (fbo!=nullptr) fbo->release();
	if (vao!=nullptr) vao->release();
	if (ssbo!=nullptr) ssbo->release();
	for (auto ptr : texUnit) if (ptr!=nullptr) ptr->release();
}

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
#define _M_SETID(a,b,c)\
if (##a!=nullptr) {\
	if (state.##a != ##a->id) b;\
} else if (state.##a != 0) {\
	c;\
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
	_M_SETID(fbo, glBindFramebuffer(GL_FRAMEBUFFER, fbo->id),
		glBindFramebuffer(GL_FRAMEBUFFER, 0));
	_M_SETID(vao, glBindVertexArray(vao->id),
		glBindVertexArray(0));
	_M_SETID(ssbo, {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo->id);
		}, {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		}
	);
	for (uint i = 0; i < texUnit.size(); i++) {
		if (texUnit[i]!=nullptr && texUnit[i]!=state.texUnit[i]) {
			glBindTextureUnit(i, texUnit[i]->id);
			state.texUnit[i] = texUnit[i];
			//Currently not unbinding texture unit... Not sure if I should
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
#define _M_SETID_S(a,b,c)\
	state.##a!=0 ? b : c;

void gl::init() {
	logger("GL Interface init.\n");
	int __maxTexU;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &__maxTexU);
	_maxTexUnit = uint(__maxTexU);
	state.texUnit.reserve(_maxTexUnit);
	for (uint i = 0; i<_maxTexUnit; i++) state.texUnit.push_back(nullptr);
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
	_M_SETID_S(fbo, glBindFramebuffer(GL_FRAMEBUFFER, state.fbo),
		glBindFramebuffer(GL_FRAMEBUFFER, 0));
	_M_SETID_S(vao, glBindVertexArray(state.vao),
		glBindVertexArray(0));
	_M_SETID_S(ssbo, glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, state.ssbo),
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
	glViewport(state.viewport.x, state.viewport.y, state.viewport.z, state.viewport.w);
	//Should I unbind all texture units?
}

void gl::end() {
	logger("GL Interface end.\n");
	if (target==nullptr) throw;
	if (target->fbo!=nullptr) logger("WARNING!!! gl end() called when target is not the default original target! May point to memory leak! (Note that this catch is not always successful)\n");
	delete target;
}

uint gl::getMaxTextureSize() {
	int size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	return (uint)size;
}

RenderTarget* gl::target = nullptr;
