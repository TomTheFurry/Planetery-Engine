#pragma once
#include "Define.h"
#include "DefineMath.h"
#include <unordered_map>
#include <map>
#include <bitset>

namespace gl {
	typedef uint GLEnum;
	typedef uint GLflags;

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
	class Shader : virtual public Base {
	public:
		Shader(GLEnum type);
		void setSource(size_t arrayCount, const char*const* stringArray, const int* lengthArray);
		void compile();
	protected:
		~Shader();
	};
	class ShaderProgram : virtual public Base {
	public:
		ShaderProgram();
		void attachShader(Shader* sd);
		void linkProgram();
	protected:
		~ShaderProgram();
	private:
		std::vector<Shader*> _sd;
	};

	class BufferBase : virtual public Base {
	public:
		BufferBase();
		void setFormatAndData(size_t size, GLflags usageFlags, const void* data = nullptr);
		void editData(size_t size, const void* data, size_t atOffset = 0);
		void* map(GLEnum access);
		void* getMapPointer();
		void unmap();
		void reset();
	protected:
		void* _mapPointer;
		~BufferBase();
	};
	class VertexBuffer : virtual public BufferBase {};
	class IndiceBuffer : virtual public BufferBase {};
	class ShaderStorageBuffer : virtual public BufferBase {};

	class VertexAttributeArray : virtual public Base {
		struct Attribute {
			int dataSize;
			GLEnum type;
			uint offset;
			bool normalized = false;
		};
		struct BufferBinding {
			VertexBuffer* targetBuffer;
			size_t stride;
			int offset = 0; //in bytes
		};
	public:
		VertexAttributeArray(IndiceBuffer* ib = nullptr);
		void setAttribute(uint atbId, Attribute atb, GLEnum internalType);
		void setBufferBinding(uint bfbId, BufferBinding bfb); // bfb.targetBuffer MUST be a valid Vertex Buffer!
		void bindAttributeToBufferBinding(uint atbId, uint bfbId);
		void bindIndiceBuffer(IndiceBuffer* ib);
	protected:
		std::map<uint, VertexBuffer*> _bfb;
		IndiceBuffer* _ib;
		~VertexAttributeArray();
	};

	class Texture : virtual public Base {};

	class Texture2D : virtual public Texture {
	public:
		Texture2D();
		void setFormat(GLEnum internalFormat, size_t width, size_t height, uint levels);
		void setData(int x, int y, uint w, uint h, uint level, GLEnum dataFormat, GLEnum dataType, const void* data);
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
		std::vector<Base*> _rb;
	};

	class RenderTarget {
	public:
		RenderTarget();
		RenderTarget(RenderTarget& r) = delete;
		RenderTarget(RenderTarget&& r) = delete;
		FrameBuffer* fbo;
		VertexAttributeArray* vao;
		ShaderStorageBuffer* ssbo;
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
		void bind(Texture* tx, uint targetUnit);
		void setViewport(uint x, uint y, uint width, uint height);
		void drawArrays(GLEnum mode, uint first, size_t count);
		void drawArraysInstanced(GLEnum mode, uint first, size_t count, size_t instanceCount);
		void drawElements(GLEnum mode, size_t count, GLEnum type, const void* indices = nullptr);
		void drawElementsInstanced(GLEnum mode, size_t count, GLEnum type, size_t instanceCount, const void* indices = nullptr);
		void activateFrameBuffer();
		template <typename T>
		T normalize(T v) {
			return v/T{viewport.z,viewport.w}*T{2}-T{1};
		}

		static RenderTarget* swapTarget(RenderTarget* target);
		~RenderTarget();
	private:
		void _use();
	};

	extern void init();
	extern void end();
	extern uint getMaxTextureSize();

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