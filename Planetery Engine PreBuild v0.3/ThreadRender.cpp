module;
#include "Marco.h"
#include "GLFW.h"
#include <glad/glad.h>
module ThreadRender;
import std.core;
import std.threading;
import Define;
import Util;
import Font;
import StringBox;
import ThreadEvents;
import Logger;

#ifdef USE_OPENGL
import GL;
#endif
#ifdef USE_VULKAN
import Vulkan;
#endif


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

// main for ThreadRender
static void _main() {
	try {
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
		vk::init();
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
		bool flips = false;
		std::chrono::steady_clock::time_point hotTickTimerA;
		std::chrono::steady_clock::time_point hotTickTimerB;
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

		while (_state.load(std::memory_order_relaxed)
			   != State::requestStop) {	 // does not have to instantly respond
			if (_state.load(std::memory_order_relaxed) == State::paused) {
				logger("Thread paused.");
				_state.wait(State::paused, std::memory_order_relaxed);
				logger("Thread resumed.");
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
#endif

#ifdef USE_VULKAN
				bool realFrame = vk::drawFrame([]() {
#endif
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
#ifdef USE_VULKAN
				});
				if (realFrame) {
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
					roller.next(hotSpan);
					if (nsDeltaPerSec >= NS_PER_S) {
						sec_count++;
						eventTickCount =
						  events::ThreadEvents::counter.exchange(0);
						logger("Average Tick speed: ", nanoSec(roller.get()),
						  " (", tickCount, "), Event tps: ", eventTickCount,
						  "\n");
#ifdef USE_VULKAN
						vk::testSwitch();
#endif
						nsDeltaPerSec -= NS_PER_S;
						tps = tickCount;
						// fpsBox.clear();
						// fpsBox << tps << "(" << eventTickCount << ")\n";
						tickCount = 0;
						flips = !flips;
					}
#ifdef USE_VULKAN
				}
#endif
#ifdef USE_OPENGL
				events::ThreadEvents::swapBuffer();	 // Will block and wait for
													 // screen updates (v-sync)
#endif
			}
		}
	} catch (const char* e) {
		logger("Uncaught Exception!! ", e);
		events::ThreadEvents::panic(std::current_exception());
	}  // catch (...) {
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
		vk::end([]() {});
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
	bool works = _state.compare_exchange_strong(result, State::normal,
	  std::memory_order_relaxed);
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
