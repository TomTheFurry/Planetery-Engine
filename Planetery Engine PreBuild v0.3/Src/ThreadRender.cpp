module;
#include "Marco.h"
// Image loading using stb. Remove this when not testing!!!
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
module ThreadRender;
import "GlfwModule.h";
import std.core;
import std.threading;
import Define;
import Util;
import Font;
import StringBox;
import ThreadEvents;
import Logger;
import Vulkan;


using namespace render;

// GL Callback

struct _Thread {
	std::mutex mxRenderHandles{};
	std::atomic<bool> renderHandlesEmpty = true;
	std::vector<RenderHandle*>* newRenderHanles =
	  new std::vector<RenderHandle*>{};
};

static std::thread* _thread = nullptr;
static std::atomic<State> _state = State::init;
static std::mutex mx{};
static std::condition_variable cv{};
static std::unordered_map<std::thread::id, _Thread> _threads{};
static std::vector<RenderHandle*> _renderJobs{};

// Thread
static GLFWwindow* _window = nullptr;

// Local Thread control (THIS THREAD ONLY)
void _selfRequestStop() {
	_state.store(State::requestStop, std::memory_order_relaxed);
}
void _selfRequestPause() {
	State st = _state.exchange(State::paused, std::memory_order_relaxed);
	if (st == State::requestStop) { _selfRequestStop(); }
}

// Vulkan
const float testVert[]{
  0.5f,
  0.5f,
  1.f,
  1.f,

  -0.5f,
  0.5f,
  0.f,
  1.f,

  0.5f,
  -0.5f,
  1.f,
  0.f,

  -0.5f,
  -0.5f,
  0.f,
  0.f,
};
const float testVert2[]{
  0.2f,
  0.2f,
  1.f,
  1.f,

  -0.2f,
  0.2f,
  0.f,
  1.f,

  0.2f,
  -0.2f,
  1.f,
  0.f,

  -0.2f,
  -0.2f,
  0.f,
  0.f,
};
const uint testInd[]{
  0,
  1,
  2,
  3,
};
static const glm::vec4 START_COLOR{0.f, 0.5f, 0.7f, 1.f};
static glm::vec4 currentColor{1.f, 0.9f, 1.f, 1.f};

// Depend on Device
static vk::Image* _imgTest = nullptr;
static vk::ImageView* _imgViewTest = nullptr;
static vk::ImageSampler* _imgSamplerBasic = nullptr;
static vk::VertexBuffer* _vertBuff = nullptr;
static vk::IndexBuffer* _indexBuff = nullptr;
static vk::ShaderCompiled* _vertShad = nullptr;
static vk::ShaderCompiled* _fragShad = nullptr;
static vk::VertexAttribute va{};

static vk::DescriptorLayout* _dsl = nullptr;

// Depend on Swapchain
static vk::DescriptorContainer* _dpc = nullptr;
static vk::RenderPass* _renderPass = nullptr;
static vk::ShaderPipeline* _pipeline = nullptr;

// Depend on Frame
struct FrameContainer {
	util::OptionalUniquePtr<vk::DescriptorSet> ds;
	util::OptionalUniquePtr<vk::UniformBuffer> ub;
	util::OptionalUniquePtr<vk::CommendBuffer> cb;
	util::OptionalUniquePtr<vk::ImageView> sv;
	util::OptionalUniquePtr<vk::FrameBuffer> fb;
};
static std::vector<FrameContainer> _frames{};

void vkDeviceCallbackOnCreate(vk::LogicalDevice& d) {
	using namespace vk;
	va.addAttributeByType<vec2>();
	va.addAttributeByType<vec2>();
	va.addBindingPoint();
	_vertBuff = new VertexBuffer(
	  d, sizeof(testVert), vk::MemoryFeature::IndirectWritable);
	_vertBuff->blockingIndirectWrite((void*)std::data(testVert));
	_indexBuff =
	  new IndexBuffer(d, sizeof(testInd), MemoryFeature::IndirectWritable);
	_indexBuff->blockingIndirectWrite((void*)std::data(testInd));

	// Make Image
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("cshader/test.png", &texWidth, &texHeight,
		  &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		if (!pixels) throw "TEST_VulkanStbImageLoadFailure";

		_imgTest = new Image(d, uvec3{texWidth, texHeight, 1}, 2,
		  VK_FORMAT_R8G8B8A8_SRGB, TextureUseType::ShaderSampling,
		  MemoryFeature::IndirectWritable);
		assert(_imgTest.texMemorySize == imageSize);
		_imgTest->blockingIndirectWrite(pixels);
		stbi_image_free(pixels);
		_imgTest->blockingTransformActiveUsage(
		  TextureActiveUseType::ReadOnlyShader);

		_imgViewTest = new ImageView(d, *_imgTest);
		_imgSamplerBasic =
		  new ImageSampler(d, SamplerFilter::Nearest, SamplerFilter::Linear);
	}

	_vertShad = new ShaderCompiled(
	  d, ShaderType::Vert, "cshader/testUniformAndTexture.vert.spv");
	_fragShad = new ShaderCompiled(
	  d, ShaderType::Frag, "cshader/testUniformAndTexture.frag.spv");

	std::vector<DescriptorLayoutBinding> dslb{};
	dslb.push_back(DescriptorLayoutBinding{0, DescriptorDataType::UniformBuffer,
	  1, Flags<ShaderType>(ShaderType::Vert) | ShaderType::Frag});
	dslb.push_back(DescriptorLayoutBinding{
	  1, DescriptorDataType::ImageAndSampler, 1, ShaderType::Frag});
	_dsl = new DescriptorLayout(d, dslb);
}
void vkDeviceCallbackOnDestroy(vk::LogicalDevice& d) {
	using namespace vk;
	if (_vertBuff != nullptr) delete _vertBuff;
	if (_indexBuff != nullptr) delete _indexBuff;
	if (_dsl != nullptr) delete _dsl;
	if (_imgTest != nullptr) delete _imgTest;
	if (_imgViewTest != nullptr) delete _imgViewTest;
	if (_imgSamplerBasic != nullptr) delete _imgSamplerBasic;
	if (_vertShad != nullptr) delete _vertShad;
	if (_fragShad != nullptr) delete _fragShad;
}
void vkSwapchainCallbackOnCreate(vk::Swapchain& sc, bool recreation) {
	using namespace vk;
	// logger("vkSwapchainCallbackOnCreate:", recreation ? " true" : " false");
	assert(frames.empty());
	_dpc =
	  new DescriptorContainer(sc.d, *_dsl, 16, DescriptorPoolType::Dynamic);
	// Make renderPass
	VkFormat swapchainFormat = sc.surfaceFormat.format;
	{
		RenderPass::Attachment swapchainAtm(swapchainFormat,
		  TextureActiveUseType::Undefined, AttachmentReadOp::Clear,
		  TextureActiveUseType::Present, AttachmentWriteOp::Write);
		RenderPass::SubPass subPass1({}, {}, {0}, {}, {}, {});
		RenderPass::SubPassDependency dependency1(uint(-1),
		  PipelineStage::OutputAttachmentColor, MemoryAccess::None, 0,
		  PipelineStage::OutputAttachmentColor,
		  MemoryAccess::AttachmentColorWrite, false);
		_renderPass =
		  new RenderPass(sc.d, {swapchainAtm}, {subPass1}, {dependency1});
	}
	// Setup frame containers
	_frames.resize(sc.getImageCount());

	// Make swapchain framebuffer
	// Make Pipeline
	_pipeline = new ShaderPipeline(sc.d, {_dsl}, *_renderPass, 0,
	  {ShaderPipeline::ShaderStage(*_vertShad, "main"),
		ShaderPipeline::ShaderStage(*_fragShad, "main")},
	  va, PrimitiveTopology::TriangleStrip, false,
	  {VkViewport{
		.x = 0,
		.y = 0,
		.width = (float)sc.pixelSize.x,
		.height = (float)sc.pixelSize.y,
		.minDepth = 0,
		.maxDepth = 1,
	  }},
	  {VkRect2D{VkOffset2D{0, 0}, VkExtent2D{sc.pixelSize.x, sc.pixelSize.y}}},
	  false, false, PolygonMode::Fill, CullMode::None,
	  FrontDirection::Clockwise, std::optional<ShaderPipeline::DepthBias>(),
	  1.f, 1, true, std::optional<ShaderPipeline::DepthStencilSettings>(),
	  LogicOperator::None, {ShaderPipeline::AttachmentBlending()}, vec4());
	assert(_pipeline->p != nullptr);
}
void vkSwapchainCallbackOnDestroy(vk::Swapchain& sc, bool recreation) {
	// logger("vkSwapchainCallbackOnDestroy:", recreation ? " true" : " false");
	if (_dpc != nullptr) delete _dpc;
	if (_pipeline != nullptr) delete _pipeline;
	if (_renderPass != nullptr) delete _renderPass;
}
static std::atomic<bool> isPausedOnMinimized{false};
void vkSwapchainCallbackOnSurfaceMinimized(vk::Swapchain&) {
	logger("Window Minimized. Enter sleep state in next cycle...\n");
	_selfRequestPause();
	isPausedOnMinimized.store(true, std::memory_order_relaxed);
}
void glfwVkFrameBufferResizeWakeupInlineCallback(events::WindowEventType) {
	if (isPausedOnMinimized.exchange(false, std::memory_order_relaxed)) {
		ThreadRender::unpause();
	}
}
void vkFrameCallbackOnCreate(vk::RenderTick& rt) {
	using namespace vk;
	uint frameId = rt.getImageIndex();
	// logger("vkFrameCallbackOnCreate:", frameId);
	auto& frame = _frames[frameId];
	// FIXME: RenderTick missing a getSwapchain() method!
	auto& sc = *rt.d.swapChain;

	frame.sv.make(sc.getChainImageView(frameId));

	std::vector<ImageView*> b;
	b.emplace_back(&*frame.sv);
	frame.fb.make(rt.d, *_renderPass, sc.pixelSize, b);

	frame.ub.make(rt.d, sizeof(START_COLOR), MemoryFeature::Mappable);
	frame.ub->directWrite(&START_COLOR);

	frame.ds.make(_dpc->allocNewSet());
	std::array<DescriptorSet::WriteData, 1> wd{
	  DescriptorSet::WriteData(&*frame.ub)};
	std::array<DescriptorSet::WriteData, 1> wd2{DescriptorSet::WriteData(
	  _imgViewTest, _imgSamplerBasic, TextureActiveUseType::ReadOnlyShader)};
	frame.ds->blockingWrite(0, DescriptorDataType::UniformBuffer, 1, 0, wd);
	frame.ds->blockingWrite(1, DescriptorDataType::ImageAndSampler, 1, 0, wd2);

	frame.cb.make(rt.d.getCommendPool(CommendPoolType::Default));
	frame.cb->startRecording(CommendBufferUsage::None);
	frame.cb->cmdBeginRender(*_renderPass, *frame.fb, vec4(1., 0., 0., 0.));
	frame.cb->cmdBind(*_pipeline);
	frame.cb->cmdBind(*frame.ds, *_pipeline);
	frame.cb->cmdBind(*_vertBuff);
	frame.cb->cmdBind(*_indexBuff);
	// frame.cb->cmdDraw((uint)std::size(testVert) / 2);
	frame.cb->cmdDrawIndexed(std::size(testInd));
	frame.cb->cmdEndRender();
	frame.cb->endRecording();
}
void vkFrameCallbackOnRender(vk::RenderTick& rt) {
	using namespace vk;
	uint frameId = rt.getImageIndex();
	// logger("vkFrameCallbackOnRender:", frameId);
	auto& frame = _frames[frameId];

	const float DELTA = 0.0001f;
	currentColor = util::transformHSV(currentColor, 0.1f, 1.f, 1.f);
	currentColor = glm::vec4(glm::normalize(glm::vec3{currentColor.x + DELTA,
							   currentColor.y + DELTA, currentColor.z + DELTA}),
	  1.f);
	frame.ub->directWrite(&currentColor);
	rt.addCmdStage(
	  *frame.cb, {}, {}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});
}
void vkFrameCallbackOnDestroy(vk::RenderTick& rt) {
	uint frameId = rt.getImageIndex();
	// logger("vkFrameCallbackOnDestroy:", frameId);
	auto& frame = _frames[frameId];
	frame.cb.reset();
	frame.ds.reset();
	frame.ub.reset();
	frame.fb.reset();
	frame.sv.reset();
}

void vulkanSetup() {
	using namespace vk;
	events::ThreadEvents::addInlineWindowEventCallback(
	  &glfwVkFrameBufferResizeWakeupInlineCallback,
	  events::WindowEventType::FrameBufferResize);

	{
		DeviceCallback dc{
		  &vkDeviceCallbackOnCreate, &vkDeviceCallbackOnDestroy};
		vk::setCallback(dc);
		SwapchainCallback sc{&vkSwapchainCallbackOnCreate,
		  &vkSwapchainCallbackOnDestroy,
		  &vkSwapchainCallbackOnSurfaceMinimized};
		vk::setCallback(sc);
		FrameCallback fc{&vkFrameCallbackOnCreate, &vkFrameCallbackOnRender,
		  &vkFrameCallbackOnDestroy};
		vk::setCallback(fc);
	}
	vk::init();
	logger("VK graphics init done.\n Now init shaders...\n");
}
static bool useA = true;
void vkTestSwitch() {
	if (useA) {
		_vertBuff->blockingIndirectWrite((void*)std::data(testVert2));
	} else {
		_vertBuff->blockingIndirectWrite((void*)std::data(testVert));
	}
	useA = !useA;
}




// main for ThreadRender
static void _main() {
	/*try*/ {
		logger.newThread("ThreadRender");
		_state.store(State::normal, std::memory_order_relaxed);
		_state.notify_all();

		logger.newLayer();
		logger("Thread created\n");
		logger("Initing graphic API....\n");
		logger.closeLayer();

#ifdef USE_OPENGL
		glfwMakeContextCurrent(_window);
		uvec2 windowSize{events::ThreadEvents::getFramebufferSize()};
		gl::init();
		gl::target->setViewport(0, 0, windowSize.x, windowSize.y);
		logger("GL graphics init done.\n Now init fonts...\n");
#endif
#ifdef USE_VULKAN
		// glfwMakeContextCurrent(_window); Vulkan does not have a context obj
		vulkanSetup();
		logger("VK graphics init done.\n Now init fonts...\n");
#endif
		// Shader setup
		// font::init();
		logger("Font init done. Now loading shader...\n");
		// ShaderProgram::initClass();

		// Gen Model and Renderer
		logger("Loading renderer...\n");
		// global->r = new RenderProgram();
		// global->r->objectMode();
		logger("All init done. Ready to sync with other threads.\n");

		// auto roller = RollingAverage<long long, long long, 60>();
		uint tps = 0;
		ulint nsDeltaPerSec = 0;
		uint tickCount = 0;
		RollingAverage<lint, lint, 60> roller{};
		RollingAverage<lint, lint, 60> hotRoller{};
		bool flips = false;
		std::chrono::steady_clock::time_point tickTimerA;
		std::chrono::steady_clock::time_point tickTimerB;
		tickTimerB = std::chrono::high_resolution_clock::now();
		uint sec_count = 0;
		uint eventTickCount = 0;
		// StringBox fpsBox{};
		// fpsBox.str("FPS Counter");
		// fpsBox.pos = vec2{-0.9, 0.5};
		// fpsBox.setTextSize(72.f);
		// fpsBox.setSize(vec2{0.3, 0.2});
		uint failCount = 0;
		std::chrono::steady_clock::time_point hotTickTimerA;
		std::chrono::steady_clock::time_point hotTickTimerB;

		while (_state.load(std::memory_order_relaxed)
			   != State::requestStop) {	 // does not have to instantly respond
			if (_state.load(std::memory_order_relaxed) == State::paused) {
				logger("Thread paused.\n");
				_state.wait(State::paused, std::memory_order_relaxed);
				logger("Thread resumed.\n");
				tickCount = 0;
				nsDeltaPerSec = 0;
				tickTimerB = std::chrono::high_resolution_clock::now();
			} else {
				hotTickTimerB = std::chrono::high_resolution_clock::now();
				// poll event flags
				events::ThreadEvents::pollFlags();

				// collect new rander handles
				for (auto& pair : _threads) {
					auto& h = pair.second;
					if (!h.renderHandlesEmpty.exchange(
						  true, std::memory_order_relaxed)) {
						static std::vector<RenderHandle*>* vec =
						  new std::vector<RenderHandle*>{};
						{
							std::lock_guard _{h.mxRenderHandles};
							std::swap(vec, h.newRenderHanles);
						}
						_renderJobs.insert(
						  _renderJobs.end(), vec->begin(), vec->end());
						vec->clear();
					}
				}

				// setup render stuff
#ifdef USE_OPENGL
				if (events::ThreadEvents::isWindowResized()) {
					auto v = events::ThreadEvents::getFramebufferSize();
					gl::target->setViewport(0, 0, v.x, v.y);
					gl::target->pixelPerInch =
					  events::ThreadEvents::getPixelPerInch();
					// fpsBox.notifyPPIChanged();
				}
				if (events::ThreadEvents::isWindowMoved()) {
					gl::target->pixelPerInch =
					  events::ThreadEvents::getPixelPerInch();
					// fpsBox.notifyPPIChanged();
				}
				gl::target->clearColor(vec4(1.0f, 0.8f, 0.6f, 1.0f));
				// do jobs
				for (auto& h : _renderJobs) {
					if (h != nullptr) {
						if (h->requestDelete) {
							delete h;
							h = nullptr;
						} else {
							h->func();
						}
					}
				}
#endif

#ifdef USE_VULKAN
				if (!vk::drawFrame(false)) {
					failCount++;
					continue;
				}
#endif

				// fpsBox.render();

				if (sec_count >= 10) {
					// testText.str("Dummy test:\n");
					// while (true) {} //Lock the thread up after 30 sec for
					// testing
				}
				tickCount++;
				tickTimerA = std::chrono::high_resolution_clock::now();
				lint nsDelta = (tickTimerA - tickTimerB).count();
				std::swap(tickTimerA, tickTimerB);
				nsDeltaPerSec += nsDelta;

				hotTickTimerA = std::chrono::high_resolution_clock::now();
				auto hotSpan = (hotTickTimerA - hotTickTimerB).count();
				hotRoller.next(hotSpan);
				roller.next(nsDelta);
				if (nsDeltaPerSec >= NS_PER_S) {
					sec_count++;
					eventTickCount = events::ThreadEvents::counter.exchange(0);
					logger("Average Tick span: ", nanoSec(hotRoller.get()), "/",
					  nanoSec(roller.get()), " (Tick:", tickCount,
#ifdef USE_VULKAN
					  "(FalseTick:", failCount, ")",
#endif
					  "), Event tps: ", eventTickCount, "\n");
#ifdef USE_VULKAN
					vkTestSwitch();
#endif
					nsDeltaPerSec -= NS_PER_S;
					tps = tickCount;
					// fpsBox.clear();
					// fpsBox << tps << "(" << eventTickCount << ")\n";
					tickCount = 0;
					failCount = 0;
					flips = !flips;
				}
#ifdef USE_OPENGL
				events::ThreadEvents::swapBuffer();	 // Will block and wait for
													 // screen updates (v-sync)
#endif
			}
		}
	}  // catch (const char* e) {
	//	logger("Uncaught Exception!! ", e);
	//	events::ThreadEvents::panic(std::current_exception());
	//}  // catch (...) {
	//	logger("Uncaught Unknown Exception!!");
	//	events::ThreadEvents::panic(std::current_exception());
	//}
	logger("Thread stopping...\n");
	try {
		// font::close();
#ifdef USE_OPENGL
		gl::end();
#endif
#ifdef USE_VULKAN
		vk::end();
#endif
		logger("Thread normally stopped.\n");
	} catch (...) { events::ThreadEvents::panic(std::current_exception()); }
	logger.closeThread("ThreadRender");
	mx.lock();
	_state.store(State::complete, std::memory_order_relaxed);
	mx.unlock();
	cv.notify_all();
}

void ThreadRender::start(GLFWwindow* window) {
	_window = window;
	_thread = new std::thread(_main);
}

void ThreadRender::requestStop() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.exchange(State::requestStop, std::memory_order_relaxed);
	if (st == State::paused) {
		_state.notify_all();
	} else if (st == State::complete) {
		_state.store(State::complete, std::memory_order_relaxed);
	}
}

void ThreadRender::pause() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.exchange(State::paused, std::memory_order_relaxed);
	if (st == State::requestStop) {
		requestStop();
		return;
	}
	if (st == State::complete) {
		_state.store(State::complete, std::memory_order_relaxed);
	}
}

void ThreadRender::unpause() {
	_state.wait(State::init, std::memory_order_relaxed);
	State result = State::paused;
	bool works = _state.compare_exchange_strong(
	  result, State::normal, std::memory_order_relaxed);
	if (works) _state.notify_all();
}

void ThreadRender::join() {
	if (_state.load(std::memory_order_relaxed) != State::complete)
		_thread->join();
	delete _thread;
}

void render::ThreadRender::join(uint nsTimeout) {
	std::unique_lock _{mx};
	bool success = cv.wait_for(_, std::chrono::nanoseconds(nsTimeout), []() {
		return _state.load(std::memory_order_relaxed) == State::complete;
	});
	if (success) {
		_thread->join();
		delete _thread;
	} else {
		logger("ThreadRender join request timed out. Inore thread. (Will cause "
			   "memory leaks)\n");
	}
}

State ThreadRender::getState() {
	return _state.load(std::memory_order_relaxed);
}

RenderHandle* render::ThreadRender::newRenderHandle(RenderFunction func) {
	auto& t = _threads[std::this_thread::get_id()];
	auto ptr = new RenderHandle{func};
	{
		std::lock_guard _{t.mxRenderHandles};
		t.newRenderHanles->push_back(ptr);
	}
	t.renderHandlesEmpty.store(false, std::memory_order_relaxed);
	return ptr;
}
