#include "Logger.h"

#include "ThreadRender.h"

#include "ThreadEvents.h"

#include "RollingAverage.h"
#include "Font.h"

#include <thread>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <mutex>

#include <exception>

#include "GL.h"
#include <glad/glad.h>

using namespace render;

//GL Callback

static void APIENTRY glDebugOutput(
	GLenum source, GLenum type, uint id, GLenum severity,
	GLsizei length, const char* message, const void* userParam) {
	if (id == 131185 || id == 131204 || id == 131169 ) return; //7: deprecated behavior warning
	logger.newMessage("GLError");
	logger << "--------OPENGL ERROR--------\n";
	logger << "Error id " << std::to_string(id) << ": " << message << "\n";

	logger.newLayer();
	logger << "Source: ";
	switch (source) {
	case GL_DEBUG_SOURCE_API:             logger << "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   logger << "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: logger << "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     logger << "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     logger << "Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           logger << "Other"; break;
	default:
		logger << "Unknown (" << std::to_string(source) << ")"; break;
	}
	logger << "\n";
	logger.closeLayer();

	logger.newLayer();
	logger << "Type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               logger << "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: logger << "Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  logger << "Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         logger << "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         logger << "Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              logger << "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          logger << "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           logger << "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               logger << "Other"; break;
	default:
		logger << "Unknown (" << std::to_string(type) << ")"; break;
	}
	logger << "\n";
	logger.closeLayer();

	logger.newLayer();
	logger << "Severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         logger << "high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       logger << "medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          logger << "low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: logger << "notification"; break;
	default:
		logger << "Unknown (" << std::to_string(severity) << ")"; break;
	}
	logger << "\n";
	logger.closeLayer();
	logger.closeMessage("GLError");
}

struct _Thread {
	std::mutex mxRenderHandles{};
	std::atomic<bool> renderHandlesEmpty = true;
	std::vector<RenderHandle*>* newRenderHanles = new std::vector<RenderHandle*>{};
};

static std::thread* _thread = nullptr;
static std::atomic<State> _state = State::init;
static std::mutex mx{};
static std::condition_variable cv{};
static std::unordered_map<std::thread::id, _Thread> _threads{};
static std::vector<RenderHandle*> _renderJobs{};

//Thread
static const char* (*_callback)(void) = nullptr;

//main for ThreadRender
static void _main() {
	try {
		logger.newThread("ThreadRender");
		_state.store(State::normal, std::memory_order_relaxed);
		_state.notify_all();

		logger.newLayer();
		logger("Thread created\n");
		logger("Initing glad....\n");
		logger.closeLayer();

		if (!gladLoadGLLoader((GLADloadproc)_callback())) //Context Current Needed
		{
			logger("Failed to initialize GLAD\n");

			throw "GLAD init failed";
		}
		logger("glad init success.\n Now init GL graphics...\n");

		uvec2 windowSize{events::ThreadEvents::getFramebufferSize()};
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_MULTISAMPLE);
		if (IS_DEBUG_MODE) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
		gl::init();
		gl::target->setViewport(0, 0, windowSize.x, windowSize.y);

		//Shader setup
		logger("GL graphics init done.\n Now init fonts...\n");
		font::init();
		logger("Font init done. Now loading shader...\n");
		//ShaderProgram::initClass();

		//Gen Model and Renderer
		logger("Loading renderer...\n");
		//global->r = new RenderProgram();
		//global->r->objectMode();
		logger("All init done. Ready to sync with other threads.\n");

		//auto roller = RollingAverage<long long, long long, 60>();
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
		std::string strBuf{' '};
		std::wstring wstrBuf{L' '};
		uint sec_count = 0;
		uint eventTickCount = 0;
		while (_state.load(std::memory_order_relaxed) != State::requestStop) { //does not have to instantly respond
			if (_state.load(std::memory_order_relaxed) == State::paused) {
				logger("Thread paused.");
				_state.wait(State::paused, std::memory_order_relaxed);
				logger("Thread resumed.");
				tickCount = 0;
				nsDeltaPerSec = 0;
				tickTimerB = std::chrono::high_resolution_clock::now();
			} else {
				hotTickTimerB = std::chrono::high_resolution_clock::now();
				//poll event flags
				events::ThreadEvents::pollFlags();

				//collect new rander handles
				for (auto& pair : _threads) {
					auto& h = pair.second;
					if (!h.renderHandlesEmpty.exchange(true, std::memory_order_relaxed)) {
						static std::vector<RenderHandle*>* vec = new std::vector<RenderHandle*>{};
						{
							std::lock_guard _{h.mxRenderHandles};
							std::swap(vec, h.newRenderHanles);
						}
						_renderJobs.insert(_renderJobs.end(), vec->begin(), vec->end());
						vec->clear();
					}
				}

				//setup render stuff
				if (events::ThreadEvents::isWindowResized()) {
					auto v = events::ThreadEvents::getFramebufferSize();
					gl::target->setViewport(0, 0, v.x, v.y);
					gl::target->pixelPerInch = events::ThreadEvents::getPixelPerInch();
				}
				if (events::ThreadEvents::isWindowMoved()) {
					gl::target->pixelPerInch = events::ThreadEvents::getPixelPerInch();
				}
				
				gl::target->activateFrameBuffer();
				if (flips) { glClearColor(1.0f, 0.0f, 1.0f, 1.0f); } else { glClearColor(1.0f, 1.0f, 1.0f, 1.0f); }
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//do jobs
				for (auto& h : _renderJobs) {
					if (h!=nullptr) {
						if (h->requestDelete) {
							delete h;
							h = nullptr;
						} else {
							h->func();
						}
					}
				}

				//FontManager::renderString("01234567890ABCDEFG#......,,,,,,llllll  MMMMMM !!!!!!~~~~~~", vec2(-0.8, 0.9), vec2(1.f)/(vec2(windowSize)*.75f));
				font::drawString(std::to_string(tps)+"("+std::to_string(eventTickCount)+")", 72.f, vec2(-0.8, 0.6));
				font::drawString(wstrBuf, 11.f, vec2(-0.9, 0.85), 1.8f);
				//FontManager::renderString(wstrBuf, vec2(-0.9, 0.0), vec2(1.f)/(vec2(windowSize)*2.f));
				//FontManager::renderString(L"\uFFFD", vec2(-0.9, 0.9), vec2(1.f)/(vec2(windowSize)*1.f));

				for (uint i = 0; i<5; i++) {
					wstrBuf += wchar_t(wstrBuf.back()+1);
				}

				if (sec_count >= 10) {
					//while (true) {} //Lock the thread up after 30 sec for testing
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
					eventTickCount = events::ThreadEvents::counter.exchange(0);
					logger("Average Tick speed: ", nanoSec(roller.get()), " (", tickCount, "), Event tps: ", eventTickCount, "\n");
					nsDeltaPerSec -= NS_PER_S;
					tps = tickCount;
					tickCount = 0;
					flips = !flips;

				}
				events::ThreadEvents::swapBuffer(); //Will block and wait for screen updates (v-sync)
			}
		}
	} catch (...) {
		events::ThreadEvents::panic(std::current_exception());
	}
	logger("Thread ended.\n");
	try {
		font::close();
		gl::end();
	} catch (...) {
		events::ThreadEvents::panic(std::current_exception());
	}
	logger.closeThread("ThreadRender");
	mx.lock();
	_state.store(State::complete, std::memory_order_relaxed);
	mx.unlock();
	cv.notify_all();
}

void ThreadRender::start(const char* (*callback)(void)) {
	_callback = callback;
	_thread = new std::thread(_main);
}

void ThreadRender::requestStop() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.load(std::memory_order_relaxed);
	if (st == State::normal)
		_state.store(State::requestStop, std::memory_order_relaxed);
	else if (st == State::paused) {
		_state.store(State::requestStop, std::memory_order_relaxed);
		_state.notify_all();
	}
}

void ThreadRender::pause() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.load(std::memory_order_relaxed);
	if (st == State::normal)
		_state.store(State::paused, std::memory_order_relaxed);
}

void ThreadRender::unpause() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.load(std::memory_order_relaxed);
	if (st == State::paused) {
		_state.store(State::normal, std::memory_order_relaxed);
		_state.notify_all();
	}
}

void ThreadRender::join() {
	_thread->join();
	delete _thread;
}

void render::ThreadRender::join(uint nsTimeout) {
	std::unique_lock _{mx};
	bool success = cv.wait_for(_, std::chrono::nanoseconds(nsTimeout), []() {return _state.load(std::memory_order_relaxed)==State::complete; });
	if (success) {
		_thread->join();
		delete _thread;
	} else {
		logger("ThreadRender join request timed out. Inore thread. (Will cause memory leaks)\n");
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

