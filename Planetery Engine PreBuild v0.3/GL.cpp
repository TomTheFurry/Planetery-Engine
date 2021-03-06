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

// Test flag if same
static_assert(GL_MAP_READ_BIT == bufferFlags::MappingRead,
  "ERROR! GL Enum is incorrect in header file!");
static_assert(GL_MAP_WRITE_BIT == bufferFlags::MappingWrite,
  "ERROR! GL Enum is incorrect in header file!");
static_assert(GL_MAP_PERSISTENT_BIT == bufferFlags::MappingPersistent,
  "ERROR! GL Enum is incorrect in header file!");
static_assert(GL_MAP_COHERENT_BIT == bufferFlags::MappingCoherent,
  "ERROR! GL Enum is incorrect in header file!");
static_assert(GL_DYNAMIC_STORAGE_BIT == bufferFlags::DynamicStorage,
  "ERROR! GL Enum is incorrect in header file!");
static_assert(GL_CLIENT_STORAGE_BIT == bufferFlags::ClientStorage,
  "ERROR! GL Enum is incorrect in header file!");

struct _state {
    uint fbo = 0;
    uint vao = 0;
    uint ssbo = 0;
    uint spo = 0;
    bool useDepth = false;
    bool useBlend = false;
    bool useCull = false;
    bool useStencil = false;
    bool useMultisample = true;
    GLEnum depthFunc = GL_LESS;
    std::pair<GLEnum, GLEnum> blendFunc = std::make_pair(GL_ONE, GL_ZERO);
    GLEnum cullFace = GL_BACK;
    uvec4 viewport = uvec4{0, 0, 1920, 1080};
    vec2 pixelPerInch = vec2{0, 0};
    std::vector<Texture*> texUnit{};
};

static _state state;
static uint _maxTexUnit = 0;

// Enum convertion
GLEnum _val(DataType d) {
    switch (d) {
    case gl::DataType::Byte: return GL_BYTE;
    case gl::DataType::UnsignedByte: return GL_UNSIGNED_BYTE;
    case gl::DataType::Short: return GL_SHORT;
    case gl::DataType::UnsignedShort: return GL_UNSIGNED_SHORT;
    case gl::DataType::Int: return GL_INT;
    case gl::DataType::UnsignedInt: return GL_UNSIGNED_INT;
    case gl::DataType::HalfFloat: return GL_HALF_FLOAT;
    case gl::DataType::Float: return GL_FLOAT;
    case gl::DataType::Double: return GL_DOUBLE;
    case gl::DataType::Fixed: return GL_FIXED;
    default: throw "Invalid gl::DataType Enum";
    }
}
GLEnum _val(ShaderType s) {
    switch (s) {
    case gl::ShaderType::Vertex: return GL_VERTEX_SHADER;
    case gl::ShaderType::Fragment: return GL_FRAGMENT_SHADER;
    case gl::ShaderType::Geometry: return GL_GEOMETRY_SHADER;
    case gl::ShaderType::TessControl: return GL_TESS_CONTROL_SHADER;
    case gl::ShaderType::TessEvaluation: return GL_TESS_EVALUATION_SHADER;
    default: throw "Invalid gl::ShaderType Enum";
    }
}
GLEnum _val(GeomType g) {
    switch (g) {
    case gl::GeomType::Points: return GL_POINTS;
    case gl::GeomType::Lines: return GL_LINES;
    case gl::GeomType::LineLoops: return GL_LINE_LOOP;
    case gl::GeomType::LineStrips: return GL_LINE_STRIP;
    case gl::GeomType::AdjacentLines: return GL_LINES_ADJACENCY;
    case gl::GeomType::AdjacentLineStrips: return GL_LINE_STRIP_ADJACENCY;
    case gl::GeomType::Triangles: return GL_TRIANGLES;
    case gl::GeomType::TriangleFans: return GL_TRIANGLE_FAN;
    case gl::GeomType::TriangleStrips: return GL_TRIANGLE_STRIP;
    case gl::GeomType::AdjacentTriangles: return GL_TRIANGLES_ADJACENCY;
    case gl::GeomType::AdjacentTriangleStrips:
        return GL_TRIANGLE_STRIP_ADJACENCY;
    case gl::GeomType::Patches: return GL_PATCHES;
    default: throw "Invalid gl::GeomType Enum";
    }
}
GLEnum _val(Texture::SamplingFilter s) {
    switch (s) {
    case gl::Texture::SamplingFilter::linear: return GL_LINEAR;
    case gl::Texture::SamplingFilter::nearest: return GL_NEAREST;
    case gl::Texture::SamplingFilter::nearestLODNearest:
        return GL_NEAREST_MIPMAP_NEAREST;
    case gl::Texture::SamplingFilter::linearLODNearest:
        return GL_LINEAR_MIPMAP_NEAREST;
    case gl::Texture::SamplingFilter::nearestLODLinear:
        return GL_NEAREST_MIPMAP_LINEAR;
    case gl::Texture::SamplingFilter::linearLODLinear:
        return GL_LINEAR_MIPMAP_LINEAR;
    default: throw "Invalid gl::Texture::SamplingFilter Enum";
    }
}
GLEnum _val(MapAccess m) {
    switch (m) {
    case gl::MapAccess::ReadOnly: return GL_READ_ONLY;
    case gl::MapAccess::WriteOnly: return GL_WRITE_ONLY;
    case gl::MapAccess::ReadWrite: return GL_READ_WRITE;
    default: throw "Invalid gl::MapAccess Enum";
    }
}



// Base
void Base::addLink() { _refCount++; }
void Base::release() {
    _refCount--;
    if (_refCount == 0) delete this;
}

// Shader
Shader::Shader(ShaderType type) { id = glCreateShader(_val(type)); }
void Shader::setSource(
  size_t arrayCount, const char* const* stringArray, const int* lengthArray) {
    glShaderSource(id, arrayCount, stringArray, lengthArray);
}
bool Shader::compile() {
    glCompileShader(id);
    int stat;
    glGetShaderiv(id, GL_COMPILE_STATUS, &stat);
    return stat == GL_TRUE;
}
Shader::~Shader() { glDeleteShader(id); }

// ShaderProgram
ShaderProgram::ShaderProgram() { id = glCreateProgram(); }
void ShaderProgram::attachShader(Shader* sd) {
    glAttachShader(id, sd->id);
    sd->addLink();
    _sd.emplace_back(sd);
}
bool ShaderProgram::linkProgram() {
    glLinkProgram(id);
    int stat;
    glGetProgramiv(id, GL_LINK_STATUS, &stat);
    return stat == GL_TRUE;
}

void gl::ShaderProgram::use() {
    if (state.spo != id) {
        glUseProgram(id);
        state.spo = id;
    }
}

namespace gl {
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, bool value) {
        glUniform1ui(glGetUniformLocation(id, valueNameP.c_str()), (uint)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uint value) {
        glUniform1ui(glGetUniformLocation(id, valueNameP.c_str()), value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uvec2 value) {
        glUniform2ui(
          glGetUniformLocation(id, valueNameP.c_str()), value.x, value.y);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uvec3 value) {
        glUniform3ui(glGetUniformLocation(id, valueNameP.c_str()), value.x,
          value.y, value.z);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uvec4 value) {
        glUniform4ui(glGetUniformLocation(id, valueNameP.c_str()), value.x,
          value.y, value.z, value.w);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uint* value, uint length) {
        glUniform1uiv(
          glGetUniformLocation(id, valueNameP.c_str()), length, value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uvec2* value, uint length) {
        glUniform2uiv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (uint*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uvec3* value, uint length) {
        glUniform3uiv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (uint*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, uvec4* value, uint length) {
        glUniform4uiv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (uint*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, int value) {
        glUniform1i(glGetUniformLocation(id, valueNameP.c_str()), value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, ivec2 value) {
        glUniform2i(
          glGetUniformLocation(id, valueNameP.c_str()), value.x, value.y);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, ivec3 value) {
        glUniform3i(glGetUniformLocation(id, valueNameP.c_str()), value.x,
          value.y, value.z);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, ivec4 value) {
        glUniform4i(glGetUniformLocation(id, valueNameP.c_str()), value.x,
          value.y, value.z, value.w);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, int* value, uint length) {
        glUniform1iv(
          glGetUniformLocation(id, valueNameP.c_str()), length, value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, ivec2* value, uint length) {
        glUniform2iv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (int*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, ivec3* value, uint length) {
        glUniform3iv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (int*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, ivec4* value, uint length) {
        glUniform4iv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (int*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, float value) {
        glUniform1f(glGetUniformLocation(id, valueNameP.c_str()), value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, vec2 value) {
        glUniform2f(
          glGetUniformLocation(id, valueNameP.c_str()), value.x, value.y);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, vec3 value) {
        glUniform3f(glGetUniformLocation(id, valueNameP.c_str()), value.x,
          value.y, value.z);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, vec4 value) {
        glUniform4f(glGetUniformLocation(id, valueNameP.c_str()), value.x,
          value.y, value.z, value.w);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, float* value, uint length) {
        glUniform1fv(
          glGetUniformLocation(id, valueNameP.c_str()), length, value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, vec2* value, uint length) {
        glUniform2fv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (float*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, vec3* value, uint length) {
        glUniform3fv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (float*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, vec4* value, uint length) {
        glUniform4fv(
          glGetUniformLocation(id, valueNameP.c_str()), length, (float*)value);
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, mat2 value) {
        glUniformMatrix2fv(glGetUniformLocation(id, valueNameP.c_str()), 1,
          GL_FALSE, glm::value_ptr(value));
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, mat3 value) {
        glUniformMatrix3fv(glGetUniformLocation(id, valueNameP.c_str()), 1,
          GL_FALSE, glm::value_ptr(value));
    }
    void gl::ShaderProgram::setUniform(
      const std::string& valueNameP, mat4 value) {
        glUniformMatrix4fv(glGetUniformLocation(id, valueNameP.c_str()), 1,
          GL_FALSE, glm::value_ptr(value));
    }
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(id); }

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

ShaderProgram* gl::makeShaderProgram(const char* fileName, bool hasGeomShader) {
    logger.newMessage();
    logger << "GL: Loading shader " << fileName << "...\n";

    auto* program(new ShaderProgram());
    {
        auto* vertShader(new Shader(ShaderType::Vertex));
        try {
            auto vectStr = _tryRead(fileName, ".vert");
            const char* c = vectStr.c_str();
            const int s = vectStr.size();
            vertShader->setSource(1, &c, &s);
            if (!vertShader->compile()) {
                logger.newLayer();
                logger << "Error! Shader load failed! Cannot compile "
                       << SHADER_PATH << fileName << ".vert. Returning!\n";
                GLint maxLength = 0;
                glGetShaderiv(vertShader->id, GL_INFO_LOG_LENGTH, &maxLength);
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(
                  vertShader->id, maxLength, &maxLength, &errorLog[0]);
                logger << "Additional error message:\n";
                logger.newLayer();
                logger << errorLog.data();
                logger.closeLayer();
                logger.closeLayer();
                logger.closeMessage();
                vertShader->release();
                program->release();
                return {};
            }
        } catch (std::ifstream::failure e) {
            logger("Error! Shader loading failed! Cannot load file ",
              SHADER_PATH, fileName, ".vert. Returning!\n");
            logger.closeMessage();
            vertShader->release();
            program->release();
            return {};
        }
        program->attachShader(vertShader);
        vertShader->release();
    }
    {
        auto* fragShader(new Shader(ShaderType::Fragment));
        try {
            auto fragStr = _tryRead(fileName, ".frag");
            const char* c = fragStr.c_str();
            const int s = fragStr.size();
            fragShader->setSource(1, &c, &s);
            if (!fragShader->compile()) {
                logger.newLayer();
                logger << "Error! Shader load failed! Cannot compile "
                       << SHADER_PATH << fileName << ".frag. Returning!\n";
                GLint maxLength = 0;
                glGetShaderiv(fragShader->id, GL_INFO_LOG_LENGTH, &maxLength);
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(
                  fragShader->id, maxLength, &maxLength, &errorLog[0]);
                logger << "Additional error message:\n";
                logger.newLayer();
                logger << errorLog.data();
                logger.closeLayer();
                logger.closeLayer();
                logger.closeMessage();
                fragShader->release();
                program->release();
                return {};
            }
        } catch (std::ifstream::failure e) {
            logger("Error! Shader loading failed! Cannot load file ",
              SHADER_PATH, fileName, ".frag. Returning!\n");
            logger.closeMessage();
            fragShader->release();
            program->release();
            return {};
        }
        program->attachShader(fragShader);
        fragShader->release();
    }
    if (hasGeomShader) {
        auto* geomShader(new Shader(ShaderType::Geometry));
        try {
            auto geomStr = _tryRead(fileName, ".geom");
            const char* c = geomStr.c_str();
            const int s = geomStr.size();
            geomShader->setSource(1, &c, &s);
            if (!geomShader->compile()) {
                logger.newLayer();
                logger << "Error! Shader load failed! Cannot compile "
                       << SHADER_PATH << fileName << ".geom. Returning!\n";
                GLint maxLength = 0;
                glGetShaderiv(geomShader->id, GL_INFO_LOG_LENGTH, &maxLength);
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(
                  geomShader->id, maxLength, &maxLength, &errorLog[0]);
                logger << "Additional error message:\n";
                logger.newLayer();
                logger << errorLog.data();
                logger.closeLayer();
                logger.closeLayer();
                logger.closeMessage();
                geomShader->release();
                program->release();
                return {};
            }
        } catch (std::ifstream::failure e) {
            logger("Error! Shader loading failed! Cannot load file ",
              SHADER_PATH, fileName, ".geom. Returning!\n");
            logger.closeMessage();
            geomShader->release();
            program->release();
            return {};
        }
        program->attachShader(geomShader);
        geomShader->release();
    }
    if (!program->linkProgram()) {
        logger.newLayer();
        logger << "Error! Shader loading failed! Cannot link " << fileName
               << " shaders together. Returning!\n";
        GLint maxLength = 0;
        glGetProgramiv(program->id, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program->id, maxLength, &maxLength, &infoLog[0]);
        logger << "Additional error message:\n";
        logger.newLayer();
        logger << infoLog.data();
        logger.closeLayer();
        logger.closeLayer();
        logger.closeMessage();
        program->release();
        return {};
    }
    logger.closeMessage();
    return program;
}

// BufferBase
BufferBase::BufferBase() {
    _mapPointer = nullptr;
    _size = 0;
    glCreateBuffers(1, &id);
}
void gl::BufferBase::setFormatAndData(
  size_t size, GLflags usageFlags, const void* data) {
    _size = size;
    glNamedBufferStorage(id, size, data, usageFlags);
}
void BufferBase::editData(size_t size, const void* data, size_t atOffset) {
    glNamedBufferSubData(id, atOffset, size, data);
}
void* gl::BufferBase::map(MapAccess access) {
    return _mapPointer = glMapNamedBuffer(id, _val(access));
}
void* gl::BufferBase::getMapPointer() { return _mapPointer; }
void gl::BufferBase::unmap() { glUnmapNamedBuffer(id); }
void gl::BufferBase::reset() { glInvalidateBufferData(id); }
size_t gl::BufferBase::getSize() { return _size; }
BufferBase::~BufferBase() { glDeleteBuffers(1, &id); }

// VertexBuffer

// IndiceBuffer

// ShaderStorageBuffer

// VertexAttributeArray
VertexAttributeArray::VertexAttributeArray(IndiceBuffer* ib) {
    glCreateVertexArrays(1, &id);
    _ib = ib;
    if (ib != nullptr) {
        glVertexArrayElementBuffer(id, ib->id);
        ib->addLink();
    }
}
void VertexAttributeArray::setAttribute(uint atbId, int dataSize, DataType type,
  uint offset, DataType internalType, bool normalized) {
    glEnableVertexArrayAttrib(id, atbId);
    switch (internalType) {
    case DataType::Int:
    case DataType::UnsignedInt:
        glVertexArrayAttribIFormat(id, atbId, dataSize, _val(type), offset);
        break;
    case DataType::Float:
        glVertexArrayAttribFormat(
          id, atbId, dataSize, _val(type), normalized, offset);
        break;
    case DataType::Double:
        glVertexArrayAttribLFormat(id, atbId, dataSize, _val(type), offset);
        break;
    default: throw;
    }
}
void VertexAttributeArray::setBufferBinding(
  uint bfbId, VertexBuffer* targetBuffer, uint stride, int offset) {
    auto it = _bfb.lower_bound(bfbId);
    if (it != _bfb.end() && it->first == bfbId) {
        it->second->release();
        it->second = targetBuffer;
    } else {
        _bfb.emplace_hint(it, std::make_pair(bfbId, targetBuffer));
    }
    targetBuffer->addLink();
    glVertexArrayVertexBuffer(id, bfbId, targetBuffer->id, offset, stride);
}
void VertexAttributeArray::bindAttributeToBufferBinding(
  uint atbId, uint bfbId) {
    glVertexArrayAttribBinding(id, atbId, bfbId);
}
void VertexAttributeArray::bindIndiceBuffer(IndiceBuffer* ib) {
    glVertexArrayElementBuffer(id, ib->id);
}
VertexAttributeArray::~VertexAttributeArray() {
    for (auto& p : _bfb) { p.second->release(); }
    if (_ib != nullptr) _ib->release();
    glDeleteVertexArrays(1, &id);
}

// Texture
void gl::Texture::setTextureSamplingFilter(
  SamplingFilter maximize, SamplingFilter minimize) {
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, _val(maximize));
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, _val(minimize));
}

// Texture2D
Texture2D::Texture2D() { glCreateTextures(GL_TEXTURE_2D, 1, &id); }
void Texture2D::setFormat(
  GLEnum internalFormat, size_t width, size_t height, uint levels) {
    glTextureStorage2D(id, levels, internalFormat, width, height);
}
void gl::Texture2D::setData(int x, int y, uint w, uint h, uint level,
  GLEnum dataFormat, DataType dataType, const void* data) {
    glTextureSubImage2D(
      id, level, x, y, w, h, dataFormat, _val(dataType), data);
}
Texture2D* gl::Texture2D::cloneData(
  const Texture2D* source, uvec2 pos, uvec2 size, uint level) {
    glCopyImageSubData(source->id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, id,
      GL_TEXTURE_2D, level, pos.x, pos.y, 0, size.x, size.y, 1);
    return this;
}
Texture2D* gl::Texture2D::cloneData(const Texture2D* source, uvec2 pos,
  uvec2 size, uint level, uvec2 targetPos, uint targetLevel) {
    glCopyImageSubData(source->id, GL_TEXTURE_2D, level, pos.x, pos.y, 0, id,
      GL_TEXTURE_2D, targetLevel, targetPos.x, targetPos.y, 0, size.x, size.y,
      1);
    return this;
}
Texture2D::~Texture2D() { glDeleteTextures(1, &id); }

// RenderBuffer
RenderBuffer::RenderBuffer() { glCreateRenderbuffers(1, &id); }
void gl::RenderBuffer::setFormat(
  GLEnum internalFormat, size_t width, size_t height) {
    glNamedRenderbufferStorage(id, internalFormat, width, height);
}
RenderBuffer::~RenderBuffer() { glDeleteRenderbuffers(1, &id); }

// FrameBuffer
FrameBuffer::FrameBuffer() { glCreateFramebuffers(1, &id); }
void FrameBuffer::attach(RenderBuffer* rb, GLEnum attachmentPoint) {
    glNamedFramebufferRenderbuffer(
      id, attachmentPoint, GL_RENDERBUFFER, rb->id);
    // rb->addLink();
    //_rb.push_back(rb);
}
void FrameBuffer::attach(Texture* tx, GLEnum attachmentPoint, int level) {
    glNamedFramebufferTexture(id, attachmentPoint, tx->id, level);
    // tx->addLink();
    //_rb.push_back(tx);
}
FrameBuffer::~FrameBuffer() {
    // for (auto b : _rb) b->release();
    glDeleteFramebuffers(1, &id);
}


// RenderTarget
RenderTarget::RenderTarget() {
    fbo = nullptr;
    vao = nullptr;
    ssbo = nullptr;
    spo = nullptr;
    useDepth = false;
    useBlend = false;
    useCull = false;
    useStencil = false;
    useMultisample = true;
    depthFunc = GL_LESS;
    blendFunc = std::make_pair(GL_ONE, GL_ZERO);
    cullFace = GL_BACK;
    texUnit.reserve(_maxTexUnit);
    for (uint i = 0; i < _maxTexUnit; i++) texUnit.push_back(nullptr);
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
void RenderTarget::bind(ShaderProgram* sp) {
    if (spo != nullptr) spo->release();
    spo = sp;
    spo->addLink();
}
void gl::RenderTarget::bind(Texture* tx, uint i) {
    if (i >= _maxTexUnit) throw "Max texture unit reached!";
    if (texUnit[i] != nullptr) texUnit[i]->release();
    texUnit[i] = tx;
    tx->addLink();
}

void gl::RenderTarget::setViewport(uint x, uint y, uint width, uint height) {
    viewport = uvec4{x, y, width, height};
}

void RenderTarget::drawArrays(GeomType mode, uint first, size_t count) {
    _use();
    glDrawArrays(_val(mode), first, count);
}
void RenderTarget::drawArraysInstanced(
  GeomType mode, uint first, size_t count, size_t instanceCount) {
    _use();
    glDrawArraysInstanced(_val(mode), first, count, instanceCount);
}
void RenderTarget::drawElements(
  GeomType mode, size_t count, DataType type, const void* indices) {
    _use();
    glDrawElements(_val(mode), count, _val(type), indices);
}
void RenderTarget::drawElementsInstanced(GeomType mode, size_t count,
  DataType type, size_t instanceCount, const void* indices) {
    _use();
    glDrawElementsInstanced(
      _val(mode), count, _val(type), indices, instanceCount);
}
void gl::RenderTarget::activateFrameBuffer() {
    if (state.fbo != (fbo == nullptr ? 0 : fbo->id))
        glBindFramebuffer(GL_FRAMEBUFFER, (fbo == nullptr ? 0 : fbo->id));
}
RenderTarget* gl::RenderTarget::swapTarget(RenderTarget* target) {
    std::swap(target, gl::target);
    return target;
}
RenderTarget::~RenderTarget() {
    if (fbo != nullptr) fbo->release();
    if (vao != nullptr) vao->release();
    if (ssbo != nullptr) ssbo->release();
    for (auto ptr : texUnit)
        if (ptr != nullptr) ptr->release();
}

//----Hashtag Macro HELL!!! HAHAHAHAHAHAHAHAHAHA~~~
#define _M_BOOL(a, b)                     \
    if (state.##a != ##a) {               \
        ##a ? glEnable(b) : glDisable(b); \
        state.##a = ##a;                  \
    }
#define _M_SET(a, b)        \
    if (state.##a != ##a) { \
        b;                  \
        state.##a = ##a;    \
    }
#define _M_SETID(a, b, c)           \
    if (##a != nullptr) {           \
        if (state.##a != ##a->id) { \
            b;                      \
            state.##a = ##a->id;    \
        }                           \
    } else if (state.##a != 0) {    \
        c;                          \
        state.##a = 0;              \
    }

void RenderTarget::_use() {
    _M_BOOL(useDepth, GL_DEPTH_TEST);
    _M_BOOL(useBlend, GL_BLEND);
    _M_BOOL(useCull, GL_CULL_FACE);
    _M_BOOL(useStencil, GL_STENCIL_TEST);
    _M_BOOL(useMultisample, GL_MULTISAMPLE);
    if (useDepth) _M_SET(depthFunc, glDepthFunc(depthFunc));
    if (useBlend)
        _M_SET(blendFunc, glBlendFunc(blendFunc.first, blendFunc.second));
    if (useCull) _M_SET(cullFace, glCullFace(cullFace));
    // TODO: add stencil func and stuff...
    _M_SETID(fbo, glBindFramebuffer(GL_FRAMEBUFFER, fbo->id),
      glBindFramebuffer(GL_FRAMEBUFFER, 0));
    _M_SETID(vao, glBindVertexArray(vao->id), glBindVertexArray(0));
    _M_SETID(ssbo, glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo->id),
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
    _M_SETID(spo, glUseProgram(spo->id), glUseProgram(0));
    for (uint i = 0; i < texUnit.size(); i++) {
        if (texUnit[i] != nullptr && texUnit[i] != state.texUnit[i]) {
            glBindTextureUnit(i, texUnit[i]->id);
            state.texUnit[i] = texUnit[i];
            // Currently not unbinding texture unit... Not sure if I should
        }
    }
    if (viewport != state.viewport) {
        glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        state.viewport = viewport;
    }
}





static VertexAttributeArray* _rectVao = nullptr;
static VertexAttributeArray* _lineVao = nullptr;

//----Hashtag Macro HELL version 2 !!! HAHAHAHAHAHAHAHAHAHA~~~
#define _M_BOOL_S(a, b)     state.##a ? glEnable(b) : glDisable(b)
#define _M_SETID_S(a, b, c) state.##a != 0 ? b : c;

void gl::init() {
    logger("GL Interface init.\n");
    int __maxTexU;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &__maxTexU);
    _maxTexUnit = uint(__maxTexU);
    state.texUnit.reserve(_maxTexUnit);
    for (uint i = 0; i < _maxTexUnit; i++) state.texUnit.push_back(nullptr);
    gl::target = new RenderTarget();
    assert(gl::target != nullptr);
    _M_BOOL_S(useDepth, GL_DEPTH_TEST);
    _M_BOOL_S(useBlend, GL_BLEND);
    _M_BOOL_S(useCull, GL_CULL_FACE);
    _M_BOOL_S(useStencil, GL_STENCIL_TEST);
    _M_BOOL_S(useMultisample, GL_MULTISAMPLE);
    glDepthFunc(state.depthFunc);
    glBlendFunc(state.blendFunc.first, state.blendFunc.second);
    glCullFace(state.cullFace);
    // TODO: add stencil func and stuff...
    _M_SETID_S(fbo, glBindFramebuffer(GL_FRAMEBUFFER, state.fbo),
      glBindFramebuffer(GL_FRAMEBUFFER, 0));
    _M_SETID_S(vao, glBindVertexArray(state.vao), glBindVertexArray(0));
    _M_SETID_S(ssbo, glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, state.ssbo),
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
    glViewport(
      state.viewport.x, state.viewport.y, state.viewport.z, state.viewport.w);
    // Should I unbind all texture units?
    _rectVao = new VertexAttributeArray();
    _rectVao->setAttribute(0, 2, DataType::Float, 0, DataType::Float);  // pos
    _rectVao->setAttribute(
      1, 2, DataType::Float, sizeof(vec2), DataType::Float);  // size
    _rectVao->bindAttributeToBufferBinding(0, 0);
    _rectVao->bindAttributeToBufferBinding(1, 0);
    _lineVao = new VertexAttributeArray();
    _lineVao->setAttribute(0, 2, DataType::Float, 0, DataType::Float);  // pos
    _lineVao->bindAttributeToBufferBinding(0, 0);
}

void gl::end() {
    logger("GL Interface end.\n");
    if (target == nullptr) throw;
    if (target->fbo != nullptr)
        logger("WARNING!!! gl end() called when target is not the default "
               "original target! May point to memory leak! (Note that this "
               "catch is not always successful)\n");
    delete target;
}

uint gl::getMaxTextureSize() {
    int size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
    return (uint)size;
}





// Helper Renderer
[[nodiscard]] VertexBuffer* gl::drawTexRectangle(Texture2D* tex, GLRect rect) {
    VertexBuffer* rectBuffer = new VertexBuffer();
    rectBuffer->setFormatAndData(sizeof(rect), bufferFlags::None, &rect);
    drawTexRectangle(tex, rectBuffer);
    return rectBuffer;
}
[[nodiscard]] VertexBuffer* gl::drawTexR8Rectangle(
  Texture2D* tex, GLRect rect, vec4 color) {
    VertexBuffer* rectBuffer = new VertexBuffer();
    rectBuffer->setFormatAndData(sizeof(rect), bufferFlags::None, &rect);
    drawTexR8Rectangle(tex, rectBuffer, color);
    return rectBuffer;
}
[[nodiscard]] VertexBuffer* gl::drawRectangles(
  std::vector<GLRect> rects, vec4 color) {
    VertexBuffer* rectBuffer = new VertexBuffer();
    rectBuffer->setFormatAndData(
      rects.size() * sizeof(GLRect), bufferFlags::None, rects.data());
    drawRectangles(rectBuffer, color);
    return rectBuffer;
}
[[nodiscard]] VertexBuffer* gl::drawRectanglesBorder(
  std::vector<GLRect> rects, vec2 borderWidth, vec4 borderColor) {
    VertexBuffer* rectBuffer = new VertexBuffer();
    rectBuffer->setFormatAndData(
      rects.size() * sizeof(GLRect), bufferFlags::None, rects.data());
    drawRectanglesBorder(rectBuffer, borderWidth, borderColor);
    return rectBuffer;
}
[[nodiscard]] VertexBuffer* gl::drawRectanglesFilledBorder(
  std::vector<GLRect> rects, vec4 color, vec2 borderWidth, vec4 borderColor) {
    VertexBuffer* rectBuffer = new VertexBuffer();
    rectBuffer->setFormatAndData(
      rects.size() * sizeof(GLRect), bufferFlags::None, rects.data());
    drawRectanglesFilledBorder(rectBuffer, color, borderWidth, borderColor);
    return rectBuffer;
}
[[nodiscard]] VertexBuffer* gl::drawLineStrip(
  std::vector<vec2> lines, vec4 color, float width) {
    VertexBuffer* rectBuffer = new VertexBuffer();
    rectBuffer->setFormatAndData(
      (lines.size() + 2) * sizeof(vec2), bufferFlags::None, lines.data());
    drawLineStrip(rectBuffer, color, width);
    return rectBuffer;
}

static ShaderProgram* _texRectShader = nullptr;
void gl::drawTexRectangle(Texture2D* tex, VertexBuffer* rectBuffer) {
    if (_texRectShader == nullptr) {
        _texRectShader = makeShaderProgram("texRect", true);
    }
    _rectVao->setBufferBinding(0, rectBuffer, sizeof(GLRect));
    {
        Swapper _(target->vao, _rectVao);
        Swapper __(target->texUnit[0], (Texture*)tex);
        Swapper ___(target->spo, _texRectShader);
        target->drawArrays(GeomType::Points, 0, 1);
    }
}
static ShaderProgram* _texR8RectShader = nullptr;
void gl::drawTexR8Rectangle(
  Texture2D* tex, VertexBuffer* rectBuffer, vec4 color) {
    if (_texR8RectShader == nullptr) {
        _texR8RectShader = makeShaderProgram("texR8Rect", true);
    }
    _rectVao->setBufferBinding(0, rectBuffer, sizeof(GLRect));
    {
        Swapper _(target->vao, _rectVao);
        Swapper __(target->texUnit[0], (Texture*)tex);
        Swapper ___(target->spo, _texR8RectShader);
        Swapper ____(target->useBlend, true);
        Swapper _____{
          target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
        target->spo->use();
        target->spo->setUniform("textColor", color);
        target->drawArrays(GeomType::Points, 0, 1);
    }
}
static ShaderProgram* _flatRectShader = nullptr;
void gl::drawRectangles(VertexBuffer* rectBuffer, vec4 color) {
    if (_flatRectShader == nullptr) {
        _flatRectShader = makeShaderProgram("flatRect", true);
    }
    _rectVao->setBufferBinding(0, rectBuffer, sizeof(GLRect));
    {
        Swapper _(target->vao, _rectVao);
        Swapper __(target->spo, _flatRectShader);
        Swapper ___(target->useBlend, true);
        Swapper ____{target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
        target->spo->use();
        target->spo->setUniform("rectColor", color);
        target->drawArrays(
          GeomType::Points, 0, rectBuffer->getSize() / sizeof(GLRect));
    }
}
static ShaderProgram* _flatRectBorderShader = nullptr;
void gl::drawRectanglesBorder(
  VertexBuffer* rectBuffer, vec2 borderWidth, vec4 borderColor) {
    if (_flatRectBorderShader == nullptr) {
        _flatRectBorderShader = makeShaderProgram("flatRectBorder", true);
    }
    _rectVao->setBufferBinding(0, rectBuffer, sizeof(GLRect));
    {
        Swapper _(target->vao, _rectVao);
        Swapper __(target->spo, _flatRectBorderShader);
        Swapper ___(target->useBlend, true);
        Swapper ____{target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
        target->spo->use();
        target->spo->setUniform("rectBorderColor", borderColor);
        target->spo->setUniform("rectBorderWidth", borderWidth);
        target->drawArrays(
          GeomType::Points, 0, rectBuffer->getSize() / sizeof(GLRect));
    }
}
static ShaderProgram* _flatRectFilledBorderShader = nullptr;
void gl::drawRectanglesFilledBorder(
  VertexBuffer* rectBuffer, vec4 color, vec2 borderWidth, vec4 borderColor) {
    if (_flatRectFilledBorderShader == nullptr) {
        _flatRectFilledBorderShader =
          makeShaderProgram("flatRectFilledBorder", true);
    }
    _rectVao->setBufferBinding(0, rectBuffer, sizeof(GLRect));
    {
        Swapper _(target->vao, _rectVao);
        Swapper __(target->spo, _flatRectFilledBorderShader);
        Swapper ___(target->useBlend, true);
        Swapper ____{target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
        target->spo->use();
        target->spo->setUniform("rectColor", color);
        target->spo->setUniform("rectBorderColor", borderColor);
        target->spo->setUniform("rectBorderWidth", borderWidth);
        target->drawArrays(
          GeomType::Points, 0, rectBuffer->getSize() / sizeof(GLRect));
    }
}
static ShaderProgram* _lineStripShader = nullptr;
void gl::drawLineStrip(VertexBuffer* lineBuffer, vec4 color, float width) {
    if (_lineStripShader == nullptr) {
        _lineStripShader = makeShaderProgram("lineStrip", true);
    }
    _lineVao->setBufferBinding(0, lineBuffer, sizeof(vec2));
    {
        Swapper _(target->vao, _rectVao);
        Swapper __(target->spo, _lineStripShader);
        Swapper ___(target->useBlend, true);
        Swapper ____{target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
        target->spo->use();
        target->spo->setUniform("lineColor", color);
        target->spo->setUniform("lineWidth", width);
        target->drawArrays(GeomType::AdjacentLineStrips, 0,
          lineBuffer->getSize() / sizeof(vec2));
    }
}




RenderTarget* gl::target = nullptr;
