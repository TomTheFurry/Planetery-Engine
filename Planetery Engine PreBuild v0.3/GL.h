#pragma once
#include "Define.h"
#include "DefineMath.h"
#include <unordered_map>
#include <map>
#include <bitset>

namespace gl {
	typedef uint GLEnum;
	typedef uint GLflags;

	enum class DataType {
		Byte,
		UnsignedByte,
		Short,
		UnsignedShort,
		Int,
		UnsignedInt,
		HalfFloat,
		Float,
		Double,
		Fixed //Fixed 16.16 unit
		//Also has the (u)int-2-10-10-10-rev and 10f-11f-11f-11f-rev format unused
	};

	class Base {
	public:
		Base() = default;
		Base(Base& b) = delete;
		Base(Base&& b) = delete;
		uint id = -1;
		void addLink();
		void release();
		virtual ~Base() = default;
	private:
		uint _refCount = 1;
	};
	enum class ShaderType {
		Vertex,
		Fragment,
		Geometry,
		TessControl,
		TessEvaluation
	};
	class Shader : virtual public Base {
	public:
		void setSource(size_t arrayCount, const char* const* stringArray, const int* lengthArray);
		bool compile();
		Shader(ShaderType type);
	protected:
		~Shader();
	};
	class ShaderProgram : virtual public Base {
	public:
		ShaderProgram();
		void attachShader(Shader* sd);
		bool linkProgram();
		void use();
		//call use() before calling the function below
		void setUniform(const std::string& valueNameP, bool value);
		void setUniform(const std::string& valueNameP, uint value);
		void setUniform(const std::string& valueNameP, uvec2 value);
		void setUniform(const std::string& valueNameP, uvec3 value);
		void setUniform(const std::string& valueNameP, uvec4 value);
		void setUniform(const std::string& valueNameP, uint* value, uint length);
		void setUniform(const std::string& valueNameP, uvec2* value, uint length);
		void setUniform(const std::string& valueNameP, uvec3* value, uint length);
		void setUniform(const std::string& valueNameP, uvec4* value, uint length);
		void setUniform(const std::string& valueNameP, int value);
		void setUniform(const std::string& valueNameP, ivec2 value);
		void setUniform(const std::string& valueNameP, ivec3 value);
		void setUniform(const std::string& valueNameP, ivec4 value);
		void setUniform(const std::string& valueNameP, int* value, uint length);
		void setUniform(const std::string& valueNameP, ivec2* value, uint length);
		void setUniform(const std::string& valueNameP, ivec3* value, uint length);
		void setUniform(const std::string& valueNameP, ivec4* value, uint length);
		void setUniform(const std::string& valueNameP, float value);
		void setUniform(const std::string& valueNameP, vec2 value);
		void setUniform(const std::string& valueNameP, vec3 value);
		void setUniform(const std::string& valueNameP, vec4 value);
		void setUniform(const std::string& valueNameP, float* value, uint length);
		void setUniform(const std::string& valueNameP, vec2* value, uint length);
		void setUniform(const std::string& valueNameP, vec3* value, uint length);
		void setUniform(const std::string& valueNameP, vec4* value, uint length);
		void setUniform(const std::string& valueNameP, mat2 value);
		void setUniform(const std::string& valueNameP, mat3 value);
		void setUniform(const std::string& valueNameP, mat4 value);
	protected:
		~ShaderProgram();
	private:
		std::vector<Shader*> _sd;
	};
	extern ShaderProgram* makeShaderProgram(const char* fileName, bool hasGeomShader);

	namespace bufferFlags {
		enum bufferFlags {
			None = 0,
			MappingRead = 1,
			MappingWrite = 2,
			MappingPersistent = 64,
			MappingCoherent = 128,
			DynamicStorage = 256,
			ClientStorage = 512
		};
	}
	enum class MapAccess {
		ReadOnly,
		WriteOnly,
		ReadWrite,
	};

	class BufferBase : virtual public Base {
	public:
		BufferBase();
		void setFormatAndData(size_t size, GLflags usageFlags, const void* data = nullptr);
		void editData(size_t size, const void* data, size_t atOffset = 0);
		void* map(MapAccess access);
		void* getMapPointer();
		void unmap();
		void reset();
		size_t getSize();
	protected:
		void* _mapPointer;
		size_t _size;
		~BufferBase();
	};
	class VertexBuffer : virtual public BufferBase {};
	class IndiceBuffer : virtual public BufferBase {};
	class ShaderStorageBuffer : virtual public BufferBase {};

	class VertexAttributeArray : virtual public Base {
	public:
		VertexAttributeArray(IndiceBuffer* ib = nullptr);
		//internalType only accepts: Int, UnsignedInt, Float, Double
		void setAttribute(uint atbId, int dataSize, DataType inputType, uint offset, DataType internalType, bool normalized = false);
		void setBufferBinding(uint bfbId, VertexBuffer* targetBuffer, uint stride, int offset = 0); // bfb.targetBuffer MUST be a valid Vertex Buffer!
		void bindAttributeToBufferBinding(uint atbId, uint bfbId);
		void bindIndiceBuffer(IndiceBuffer* ib);
	protected:
		std::map<uint, VertexBuffer*> _bfb;
		IndiceBuffer* _ib;
		~VertexAttributeArray();
	};

	class Texture : virtual public Base {
	public:
		enum class SamplingFilter {
			linear,
			nearest,
			nearestLODNearest,
			linearLODNearest,
			nearestLODLinear,
			linearLODLinear,
		};
		void setTextureSamplingFilter(SamplingFilter maximize, SamplingFilter minimize);
	};

	class Texture2D : virtual public Texture {
	public:
		Texture2D();
		void setFormat(GLEnum internalFormat, size_t width, size_t height, uint levels);
		void setData(int x, int y, uint w, uint h, uint level, GLEnum dataFormat, DataType dataType, const void* data);
		Texture2D* cloneData(const Texture2D* source, uvec2 pos, uvec2 size, uint level);
		Texture2D* cloneData(const Texture2D* source, uvec2 pos, uvec2 size, uint level, uvec2 targetPos, uint targetLevel);
	protected:
		~Texture2D();
	};

	class RenderBuffer : virtual public Base {
	public:
		RenderBuffer();
		void setFormat(GLEnum internalFormat, size_t width, size_t height);
	protected:
		~RenderBuffer();
	};
	class FrameBuffer : virtual public Base {
	public:
		FrameBuffer();
		void attach(RenderBuffer* rb, GLEnum attachmentPoint);
		void attach(Texture* tx, GLEnum attachmentPoint, int level);
	protected:
		~FrameBuffer();
	private:
		//std::vector<Base*> _rb;
	};

	enum class GeomType {
		Points,
		Lines,
		LineLoops,
		LineStrips,
		AdjacentLines,
		AdjacentLineStrips,
		Triangles,
		TriangleFans,
		TriangleStrips,
		AdjacentTriangles,
		AdjacentTriangleStrips,
		Patches
	};

	class RenderTarget {
	public:
		RenderTarget();
		RenderTarget(RenderTarget& r) = delete;
		RenderTarget(RenderTarget&& r) = delete;
		FrameBuffer* fbo;
		VertexAttributeArray* vao;
		ShaderStorageBuffer* ssbo;
		ShaderProgram* spo;
		bool useDepth;
		bool useBlend;
		bool useCull;
		bool useStencil;
		bool useMultisample;
		GLEnum depthFunc;
		std::pair<GLEnum, GLEnum> blendFunc;
		GLEnum cullFace;
		uvec4 viewport;
		vec2 pixelPerInch;
		std::vector<Texture*> texUnit{};
		void bind(FrameBuffer* fb);
		void bind(VertexAttributeArray* va);
		void bind(ShaderStorageBuffer* ssb);
		void bind(ShaderProgram* ssb);
		void bind(Texture* tx, uint targetUnit);
		void setViewport(uint x, uint y, uint width, uint height);
		void drawArrays(GeomType geomType, uint first, size_t count);
		void drawArraysInstanced(GeomType geomType, uint first, size_t count, size_t instanceCount);
		void drawElements(GeomType geomType, size_t count, DataType indicesDataType, const void* indices = nullptr);
		void drawElementsInstanced(GeomType geomType, size_t count, DataType indicesDataType, size_t instanceCount, const void* indices = nullptr);
		void activateFrameBuffer();
		template <typename T>
		T normalizePos(T v) {
			return (v/T{viewport.z,viewport.w})*T{2}-T{1};
		}
		template <typename T>
		T normalizeLength(T v) {
			return (v/T{viewport.z,viewport.w})*T{2};
		}

		static RenderTarget* swapTarget(RenderTarget* target);
		~RenderTarget();
	private:
		void _use();
	};

	extern void init();
	extern void end();
	extern [[nodiscard]] uint getMaxTextureSize();

	struct GLRect {
		vec2 pos;
		vec2 size;
	};
	[[nodiscard]] extern VertexBuffer* drawTexRectangle(Texture2D* tex, GLRect rect);
	[[nodiscard]] extern VertexBuffer* drawTexR8Rectangle(Texture2D* tex, GLRect rect, vec4 color);
	[[nodiscard]] extern VertexBuffer* drawRectangles(std::vector<GLRect> rects, vec4 color);
	[[nodiscard]] extern VertexBuffer* drawRectanglesBorder(std::vector<GLRect> rects, vec2 borderWidth, vec4 borderColor);
	[[nodiscard]] extern VertexBuffer* drawRectanglesFilledBorder(std::vector<GLRect> rects, vec4 color, vec2 borderWidth, vec4 borderColor);
	[[nodiscard]] extern VertexBuffer* drawLineStrip(std::vector<vec2> lines, vec4 color, float width);
	extern void drawTexRectangle(Texture2D* tex, VertexBuffer* rectBuffer);
	extern void drawTexR8Rectangle(Texture2D* tex, VertexBuffer* rectBuffer, vec4 color);
	extern void drawRectangles(VertexBuffer* rectBuffer, vec4 color);
	extern void drawRectanglesBorder(VertexBuffer* rectBuffer, vec2 borderWidth, vec4 borderColor);
	extern void drawRectanglesFilledBorder(VertexBuffer* rectBuffer, vec4 color, vec2 borderWidth, vec4 borderColor);
	extern void drawLineStrip(VertexBuffer* rectBuffer, vec4 color, float width);
	extern RenderTarget* target;

	template <typename T>
	static void bind(T* t) { target->bind(t); }
}

template <typename T>
class Swapper {
	T _b;
	T& _t;
public:
	Swapper(T& loc, const T& v) :
		_b(v), _t(loc) {
		std::swap(loc, _b);
	}
	Swapper(T& loc, T&& v) :
		_b(std::move(v)), _t(loc) {
		std::swap(loc, _b);
	}
	Swapper(const Swapper& s) = delete;
	Swapper(Swapper&& s) = delete;
	~Swapper() {
		std::swap(_t, _b);
	}
};