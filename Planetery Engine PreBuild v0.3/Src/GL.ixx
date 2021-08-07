export module GL;
import "GladGlfwGlTempModule.h";
import std.core;
import Define;

export namespace gl {

	enum class DataType : GLenum {
		Byte = GL_BYTE,
		UnsignedByte = GL_UNSIGNED_BYTE,
		Short = GL_SHORT,
		UnsignedShort = GL_UNSIGNED_SHORT,
		Int = GL_INT,
		UnsignedInt = GL_UNSIGNED_INT,
		HalfFloat = GL_HALF_FLOAT,
		Float = GL_FLOAT,
		Double = GL_DOUBLE,
		Fixed = GL_FIXED, //Fixed 16.16 unit
		//Also has the (u)int-2-10-10-10-rev and 10f-11f-11f-11f-rev format unused
	};

	class Base
	{
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
	enum class ShaderType : GLenum {
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		TessControl = GL_TESS_CONTROL_SHADER,
		TessEvaluation = GL_TESS_EVALUATION_SHADER,
	};
	class Shader: virtual public Base
	{
	  public:
		void setSource(size_t arrayCount, const char* const* stringArray,
		  const int* lengthArray);
		bool compile();
		Shader(ShaderType type);
	  protected:
		~Shader();
	};
	class ShaderProgram: virtual public Base
	{
	  public:
		ShaderProgram();
		void attachShader(Shader* sd);
		bool linkProgram();
		void use();
		// call use() before calling the function below
		void setUniform(const std::string& valueNameP, bool value);
		void setUniform(const std::string& valueNameP, uint value);
		void setUniform(const std::string& valueNameP, uvec2 value);
		void setUniform(const std::string& valueNameP, uvec3 value);
		void setUniform(const std::string& valueNameP, uvec4 value);
		void setUniform(
		  const std::string& valueNameP, uint* value, uint length);
		void setUniform(
		  const std::string& valueNameP, uvec2* value, uint length);
		void setUniform(
		  const std::string& valueNameP, uvec3* value, uint length);
		void setUniform(
		  const std::string& valueNameP, uvec4* value, uint length);
		void setUniform(const std::string& valueNameP, int value);
		void setUniform(const std::string& valueNameP, ivec2 value);
		void setUniform(const std::string& valueNameP, ivec3 value);
		void setUniform(const std::string& valueNameP, ivec4 value);
		void setUniform(const std::string& valueNameP, int* value, uint length);
		void setUniform(
		  const std::string& valueNameP, ivec2* value, uint length);
		void setUniform(
		  const std::string& valueNameP, ivec3* value, uint length);
		void setUniform(
		  const std::string& valueNameP, ivec4* value, uint length);
		void setUniform(const std::string& valueNameP, float value);
		void setUniform(const std::string& valueNameP, vec2 value);
		void setUniform(const std::string& valueNameP, vec3 value);
		void setUniform(const std::string& valueNameP, vec4 value);
		void setUniform(
		  const std::string& valueNameP, float* value, uint length);
		void setUniform(
		  const std::string& valueNameP, vec2* value, uint length);
		void setUniform(
		  const std::string& valueNameP, vec3* value, uint length);
		void setUniform(
		  const std::string& valueNameP, vec4* value, uint length);
		void setUniform(const std::string& valueNameP, mat2 value);
		void setUniform(const std::string& valueNameP, mat3 value);
		void setUniform(const std::string& valueNameP, mat4 value);
	  protected:
		~ShaderProgram();
	  private:
		std::vector<Shader*> _sd;
	};
	extern ShaderProgram* makeShaderProgram(
	  const char* fileName, bool hasGeomShader);

	namespace bufferFlags {
		enum bufferFlags : GLenum {
			None = 0,
			MappingRead = GL_MAP_READ_BIT,
			MappingWrite = GL_MAP_WRITE_BIT,
			MappingPersistent = GL_MAP_PERSISTENT_BIT,
			MappingCoherent = GL_MAP_COHERENT_BIT,
			DynamicStorage = GL_DYNAMIC_STORAGE_BIT,
			ClientStorage = GL_CLIENT_STORAGE_BIT
		};
	}
	enum class MapAccess : GLenum {
		ReadOnly = GL_READ_ONLY,
		WriteOnly = GL_WRITE_ONLY,
		ReadWrite = GL_READ_WRITE,
	};

	class BufferBase: virtual public Base
	{
	  public:
		BufferBase();
		void setFormatAndData(size_t size, GLenum usageFlags, const void* data = nullptr);
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
	class VertexBuffer: virtual public BufferBase
	{};
	class IndiceBuffer: virtual public BufferBase
	{};
	class ShaderStorageBuffer: virtual public BufferBase
	{};

	class VertexAttributeArray: virtual public Base
	{
	  public:
		VertexAttributeArray(IndiceBuffer* ib = nullptr);
		// internalType only accepts: Int, UnsignedInt, Float, Double
		void setAttribute(uint atbId, int dataSize, DataType inputType,
		  uint offset, DataType internalType, bool normalized = false);
		void setBufferBinding(uint bfbId, VertexBuffer* targetBuffer,
		  uint stride,
		  int offset = 0);	// bfb.targetBuffer MUST be a valid Vertex Buffer!
		void bindAttributeToBufferBinding(uint atbId, uint bfbId);
		void bindIndiceBuffer(IndiceBuffer* ib);
	  protected:
		std::map<uint, VertexBuffer*> _bfb;
		IndiceBuffer* _ib;
		~VertexAttributeArray();
	};

	class Texture : virtual public Base {
	public:
		enum class SamplingFilter : GLenum {
			linear = GL_LINEAR,
			nearest = GL_NEAREST,
			nearestLODNearest = GL_NEAREST_MIPMAP_NEAREST,
			linearLODNearest = GL_LINEAR_MIPMAP_NEAREST,
			nearestLODLinear = GL_NEAREST_MIPMAP_LINEAR,
			linearLODLinear = GL_LINEAR_MIPMAP_LINEAR,
		};
		void setTextureSamplingFilter(
		  SamplingFilter maximize, SamplingFilter minimize);
	};

	class Texture2D: virtual public Texture
	{
	  public:
		Texture2D();
		void setFormat(
		  GLenum internalFormat, size_t width, size_t height, uint levels);
		void setData(int x, int y, uint w, uint h, uint level,
		  GLenum dataFormat, DataType dataType, const void* data);
		Texture2D* cloneData(
		  const Texture2D* source, uvec2 pos, uvec2 size, uint level);
		Texture2D* cloneData(const Texture2D* source, uvec2 pos, uvec2 size,
		  uint level, uvec2 targetPos, uint targetLevel);
	  protected:
		~Texture2D();
	};

	class RenderBuffer: virtual public Base
	{
	  public:
		RenderBuffer();
		void setFormat(GLenum internalFormat, size_t width, size_t height);
	  protected:
		~RenderBuffer();
	};
	class FrameBuffer: virtual public Base
	{
	  public:
		FrameBuffer();
		void attach(RenderBuffer* rb, GLenum attachmentPoint);
		void attach(Texture* tx, GLenum attachmentPoint, int level);
	  protected:
		~FrameBuffer();
	  private:
		// std::vector<Base*> _rb;
	};

	enum class GeomType : GLenum {
		Points = GL_POINTS,
		Lines = GL_LINES,
		LineLoops = GL_LINE_LOOP,
		LineStrips = GL_LINE_STRIP,
		AdjacentLines = GL_LINES_ADJACENCY,
		AdjacentLineStrips = GL_LINE_STRIP_ADJACENCY,
		Triangles = GL_TRIANGLES,
		TriangleFans = GL_TRIANGLE_FAN,
		TriangleStrips = GL_TRIANGLE_STRIP,
		AdjacentTriangles = GL_TRIANGLES_ADJACENCY,
		AdjacentTriangleStrips = GL_TRIANGLE_STRIP_ADJACENCY,
		Patches = GL_PATCHES,
	};

	class RenderTarget
	{
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
		GLenum depthFunc;
		std::pair<GLenum, GLenum> blendFunc;
		GLenum cullFace;
		uvec4 viewport;
		vec2 pixelPerInch;
		std::vector<Texture*> texUnit{};
		void bind(FrameBuffer* fb);
		void bind(VertexAttributeArray* va);
		void bind(ShaderStorageBuffer* ssb);
		void bind(ShaderProgram* ssb);
		void bind(Texture* tx, uint targetUnit);
		void setViewport(uint x, uint y, uint width, uint height);
		void clearColor(vec4 color, bool clearDepth = true);
		void drawArrays(GeomType geomType, uint first, size_t count);
		void drawArraysInstanced(
		  GeomType geomType, uint first, size_t count, size_t instanceCount);
		void drawElements(GeomType geomType, size_t count,
		  DataType indicesDataType, const void* indices = nullptr);
		void drawElementsInstanced(GeomType geomType, size_t count,
		  DataType indicesDataType, size_t instanceCount,
		  const void* indices = nullptr);
		void activateFrameBuffer();
		template<typename T> T normalizePos(T v) {
			return (v / T{viewport.z, viewport.w}) * T{2} - T{1};
		}
		template<typename T> T normalizeLength(T v) {
			return (v / T{viewport.z, viewport.w}) * T{2};
		}

		static RenderTarget* swapTarget(RenderTarget* target);
		~RenderTarget();
	  private:
		void _use();
	};

	void init();
	void end();
	[[nodiscard]] uint getMaxTextureSize();

	struct GLRect {
		vec2 pos;
		vec2 size;
	};
	[[nodiscard]] extern VertexBuffer* drawTexRectangle(
	  Texture2D* tex, GLRect rect);
	[[nodiscard]] extern VertexBuffer* drawTexR8Rectangle(
	  Texture2D* tex, GLRect rect, vec4 color);
	[[nodiscard]] extern VertexBuffer* drawRectangles(
	  std::vector<GLRect> rects, vec4 color);
	[[nodiscard]] extern VertexBuffer* drawRectanglesBorder(
	  std::vector<GLRect> rects, vec2 borderWidth, vec4 borderColor);
	[[nodiscard]] extern VertexBuffer* drawRectanglesFilledBorder(
	  std::vector<GLRect> rects, vec4 color, vec2 borderWidth,
	  vec4 borderColor);
	[[nodiscard]] extern VertexBuffer* drawLineStrip(
	  std::vector<vec2> lines, vec4 color, float width);
	extern void drawTexRectangle(Texture2D* tex, VertexBuffer* rectBuffer);
	extern void drawTexR8Rectangle(
	  Texture2D* tex, VertexBuffer* rectBuffer, vec4 color);
	extern void drawRectangles(VertexBuffer* rectBuffer, vec4 color);
	extern void drawRectanglesBorder(
	  VertexBuffer* rectBuffer, vec2 borderWidth, vec4 borderColor);
	extern void drawRectanglesFilledBorder(
	  VertexBuffer* rectBuffer, vec4 color, vec2 borderWidth, vec4 borderColor);
	extern void drawLineStrip(
	  VertexBuffer* rectBuffer, vec4 color, float width);
	extern RenderTarget* target;

	template<typename T> static void bind(T* t) { target->bind(t); }
}

export template<typename T> class Swapper
{
	T _b;
	T& _t;
  public:
	Swapper(T& loc, const T& v): _b(v), _t(loc) { std::swap(loc, _b); }
	Swapper(T& loc, T&& v): _b(std::move(v)), _t(loc) { std::swap(loc, _b); }
	Swapper(const Swapper& s) = delete;
	Swapper(Swapper&& s) = delete;
	~Swapper() { std::swap(_t, _b); }
};