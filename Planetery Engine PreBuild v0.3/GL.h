#pragma once
#include "Define.h"
#include "DefineMath.h"
#include <unordered_map>
#include <map>
#include <bitset>
#include <type_traits>
#include <concepts>

namespace gl {
	typedef uint GLEnum;
	typedef uint GLflags;
	class Base;
	template<typename T>
	concept GLObject = std::is_base_of<Base, T>::value;
	template<GLObject T>
	class Ptr;
	class Base {
	protected:
		Base() = default;
		Base(Base& b) = delete;
		Base(Base&& b) = delete;
		uint _refCount = 1;
		uint _id = -1;
		void addLink();
		void release();
		virtual ~Base() = default;
		template<GLObject T>
		friend class Ptr;
	};
	enum class ShaderType {
		Vertex,
		Fragment,
		Geometry,
		TessControl,
		TessEvaluation
	};

	template<GLObject T>
	class Ptr {
		T* _ptr;
	public:
		Ptr() { _ptr = nullptr; }
		template<GLObject U>
		Ptr(const Ptr<U>& v) noexcept {
			_ptr = &*v;
			_ptr->addLink();
		}
		template<GLObject U>
		Ptr(Ptr<U>&& v) noexcept {
			_ptr = &*v; //wtf is this lol...
			v._ptr = nullptr;
		}
		Ptr(T* v) noexcept {
			_ptr = v;
		}
		uint id() const noexcept {
			return *this ? _ptr->_id : 0;
		}
		template<GLObject U>
		Ptr& operator=(const Ptr<U>& v) noexcept {
			if (*this) _ptr->release();
			_ptr = &*v;
			_ptr->addLink();
			return *this;
		}
		template<GLObject U>
		Ptr& operator=(Ptr<U>&& v) noexcept {
			if (*this) _ptr->release();
			_ptr = &*v;
			v._ptr = nullptr;
			return *this;
		}
		T* operator->() const noexcept {
			assert(_ptr!=nullptr);
			return _ptr;
		}
		T& operator*() const noexcept {
			return *_ptr;
		}
		operator bool() const noexcept {
			return _ptr!=nullptr;
		}
		bool operator==(const Ptr& v) const noexcept {
			return _ptr==v._ptr;
		}
		~Ptr() noexcept {
			if (*this) _ptr->release();
		}
	};

	class Shader : virtual public Base {
	public:
		void setSource(size_t arrayCount, const char*const* stringArray, const int* lengthArray);
		bool compile();
		Shader(ShaderType type);
	protected:
		~Shader();
	};
	class ShaderProgram : virtual public Base {
	public:
		ShaderProgram();
		void attachShader(Ptr<Shader> sd);
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
		std::vector<Ptr<Shader>> _sd;
	};
	extern Ptr<ShaderProgram> makeShaderProgram(const char* fileName, bool hasGeomShader);
	class BufferBase : virtual public Base {
	public:
		void setFormatAndData(size_t size, GLflags usageFlags, const void* data = nullptr);
		void editData(size_t size, const void* data, size_t atOffset = 0);
		void* map(GLEnum access);
		void* getMapPointer();
		void unmap();
		void reset();
		BufferBase();
	protected:
		~BufferBase();
	private:
		void* _mapPointer;
	};
	class VertexBuffer : virtual public BufferBase {
	public:
		VertexBuffer() = default;
	protected:
		~VertexBuffer() = default;
	};
	class IndiceBuffer : virtual public BufferBase {
	public:
		IndiceBuffer() = default;
	protected:
		~IndiceBuffer() = default;
	};
	class ShaderStorageBuffer : virtual public BufferBase {
	public:
		ShaderStorageBuffer() = default;
	protected:
		~ShaderStorageBuffer() = default;
	};

	class VertexAttributeArray : virtual public Base {
		struct Attribute {
			int dataSize;
			GLEnum type;
			uint offset;
			bool normalized = false;
		};
		struct BufferBinding {
			Ptr<VertexBuffer> targetBuffer;
			size_t stride;
			int offset = 0; //in bytes
		};
	public:
		void setAttribute(uint atbId, Attribute atb, GLEnum internalType);
		void setBufferBinding(uint bfbId, BufferBinding bfb); // bfb.targetBuffer MUST be a valid Vertex Buffer!
		void bindAttributeToBufferBinding(uint atbId, uint bfbId);
		void bindIndiceBuffer(Ptr<IndiceBuffer> ib);
		VertexAttributeArray(Ptr<IndiceBuffer> ib = {});
	protected:
		~VertexAttributeArray();
	private:
		std::map<uint, BufferBinding> _bfb;
		Ptr<IndiceBuffer> _ib;
	};

	class Texture : virtual public Base {
	public:
		Texture() = default;
	protected:
		~Texture() = default;
	};

	class Texture2D : virtual public Texture {
	public:
		void setFormat(GLEnum internalFormat, size_t width, size_t height, uint levels);
		void setData(int x, int y, uint w, uint h, uint level, GLEnum dataFormat, GLEnum dataType, const void* data);
		//Bypass std::is_base_of<A,B> not accepting incomplete class (self class)
		template <typename T>
		void cloneData(const Ptr<T>& source, uvec2 pos, uvec2 size, uint level) {
			static_assert(std::is_same<T, Texture2D>, "This function requires const Ptr<Texture2D>&");
			_cloneData(*source, pos, size, level);
		};
		//Bypass std::is_base_of<A,B> not accepting incomplete class (self class)
		template <typename T>
		void cloneData(const Ptr<T>& source, uvec2 pos, uvec2 size, uint level, uvec2 targetPos, uint targetLevel) {
			static_assert(std::is_same<T, Texture2D>, "This function requires const Ptr<Texture2D>&");
			_cloneData(*source, pos, size, level, targetPos, targetLevel);
		};
		Texture2D();
	protected:
		~Texture2D();
		void _cloneData(const Texture2D& source, uvec2 pos, uvec2 size, uint level);
		void _cloneData(const Texture2D& source, uvec2 pos, uvec2 size, uint level, uvec2 targetPos, uint targetLevel);
	};

	class RenderBuffer : virtual public Base {
	public:
		void setFormat(GLEnum internalFormat, size_t width, size_t height);
		RenderBuffer();
	protected:
		~RenderBuffer();
	};
	class FrameBuffer : virtual public Base {
	public:
		void attach(Ptr<RenderBuffer> rb, GLEnum attachmentPoint);
		void attach(Ptr<Texture> tx, GLEnum attachmentPoint, int level);
		FrameBuffer();
	protected:
		~FrameBuffer();
	private:
		std::vector<Ptr<Base>> _rb;
	};

	class RenderTarget {
	public:
		RenderTarget();
		RenderTarget(RenderTarget& r) = delete;
		RenderTarget(RenderTarget&& r) = delete;
		Ptr<FrameBuffer> fbo;
		Ptr<VertexAttributeArray> vao;
		Ptr<ShaderStorageBuffer> ssbo;
		Ptr<ShaderProgram> spo;
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
		std::vector<Ptr<Texture>> texUnit{};
		void bind(Ptr<FrameBuffer> fb);
		void bind(Ptr<VertexAttributeArray> va);
		void bind(Ptr<ShaderStorageBuffer> ssb);
		void bind(Ptr<Texture> tx, uint targetUnit);
		void bind(Ptr<ShaderProgram> sp);
		void setViewport(uint x, uint y, uint width, uint height);
		void drawArrays(GLEnum mode, uint first, size_t count);
		void drawArraysInstanced(GLEnum mode, uint first, size_t count, size_t instanceCount);
		void drawElements(GLEnum mode, size_t count, GLEnum type, const void* indices = nullptr);
		void drawElementsInstanced(GLEnum mode, size_t count, GLEnum type, size_t instanceCount, const void* indices = nullptr);
		void activateFrameBuffer();
		template <typename T>
		T normalizePos(T v) {
			return (v/T{viewport.z,viewport.w})*T { 2 }-T{1};
		}
		template <typename T>
		T normalizeLength(T v) {
			return (v/T{viewport.z,viewport.w})*T { 2 };
		}

		~RenderTarget();
	private:
		void _use();
	};

	extern void init();
	extern void end();
	extern uint getMaxTextureSize();

	extern RenderTarget* target;

	template <GLObject T>
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