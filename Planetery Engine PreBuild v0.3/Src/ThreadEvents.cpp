module;
#include "Marco.h"
#include "ConsoleFormat.h"
module ThreadEvents;
import "Assert.h";
import "GlfwModule.h";
import std.core;
import std.threading;
import Define;
import Logger;
import ThreadRender;
#ifdef USE_OPENGL
import GL;
#endif
#ifdef USE_VULKAN
import Vulkan;
#endif

#define WINDOW_MIN_WIDTH 400
#define WINDOW_MIN_HEIGHT 300
#define WINDOW_MAX_WIDTH GLFW_DONT_CARE
#define WINDOW_MAX_HEIGHT GLFW_DONT_CARE

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#	define SYS_DEFAULT_DPI 96
#else
#	error No default system dpi value is set! Please add it!
#endif



using namespace events;

constexpr uvec2 windowInitSize = {1520, 800};

struct _KeyEvent {
	KeyEventFunction func;
	int keyCode = KeyCode::unkown;
	KeyAction keyAction = KeyAction::null;
	KeyModFlags keyModFlags = KeyModFlags(KeyModFlag::null);
};
struct _KeyCallback {
	KeyEventFunction func;
	int keyCode = KeyCode::null;
	KeyAction keyAction = KeyAction::null;
	KeyModFlags keyModFlags = KeyModFlags(KeyModFlag::null);
	std::thread::id id;
};
namespace _flag {
	enum _Flag {
		windowResized = 1,
		windowMoved = 2,
		mouseMoved = 4,
	};
}
using _Flags = unsigned short;
struct _Thread {
	std::mutex mxEvents{};
	std::atomic<bool> EventsEmpty = true;
	std::vector<_KeyEvent>* events = new std::vector<_KeyEvent>{};
	std::mutex mxCallbacks{};
	std::atomic<bool> CallbacksEmpty = true;
	std::vector<_KeyCallback>* newCallbacks = new std::vector<_KeyCallback>{};
	std::atomic<_Flags> flags = _Flags(-1);
};

static std::atomic<State> _state{State::init};
static std::thread* _thread = nullptr;
static GLFWwindow* _window = nullptr;
static GLFWmonitor* _targetMonitor = nullptr;
static std::atomic<vec2> _pixelPerInch{vec2(0, 0)};
static std::atomic<uvec2> _framebufferSize{uvec2(0, 0)};
static std::atomic<uvec2> _windowSize{uvec2(0, 0)};
static std::atomic<ivec2> _windowPos{ivec2(0, 0)};
static std::atomic<uvec2> _mousePos{uvec2(0, 0)};
static FullScreenMode _fullScreenMode = FullScreenMode::windowed;
static std::atomic<FullScreenMode> _fullScreenModeTarget =
  FullScreenMode::windowed;
static std::atomic<bool> _fullScreenModeChangeRequest = false;
static uvec2 _windowedSize{uvec2(0, 0)};
static ivec2 _windowedPos{ivec2(0, 0)};
static std::exception_ptr _eptr{};	// BUG: err... atomic eptr????
static std::mutex mx{};
static std::condition_variable cv{};
static std::unordered_map<std::thread::id, _Thread> _threads{};
static std::unordered_map<std::thread::id, std::vector<_KeyEvent>>
  _eventsToAdd{};
static std::vector<_KeyCallback> _activeCallbacks{};
std::atomic<uint> ThreadEvents::counter{0};

static void glfwErrorCallback(int errorCode, const char* text) {
	logger.newMessage("GLFWError");
	logger << format({BACKGROUND COLOR_RED}) << "GLFW ERROR:\n";
	logger(format({COLOR_RED}), "Error", format({COLOR_DEFAULT}),
	  " Code: ", format({BRIGHT COLOR_WHITE}), std::to_string(errorCode), "\n");
	logger(std::string(text), "\n");
	if (errorCode == 65543) {
		logger << format({BACKGROUND COLOR_RED, BRIGHT COLOR_WHITE})
			   << "This software requires OpenGL Version 4.6 or higher!\n";
	}
	logger.closeMessage("GLFWError");
}
static void window_size_callback(GLFWwindow* _, int width, int height) {
	_windowSize.store(uvec2(width, height), std::memory_order_release);
	for (auto& pair : _threads) {
		pair.second.flags.fetch_or(
		  _flag::windowResized, std::memory_order_relaxed);
	}
}
static void window_pos_callback(GLFWwindow* _, int x, int y) {
	_windowPos.store(ivec2(x, y), std::memory_order_release);
	for (auto& pair : _threads) {
		pair.second.flags.fetch_or(
		  _flag::windowMoved, std::memory_order_relaxed);
	}
}
static void framebuffer_size_callback(GLFWwindow* _, int width, int height) {
#ifdef USE_VULKAN
	vk::notifyOutdatedSwapchain();
#endif
	_framebufferSize.store(uvec2(width, height), std::memory_order_release);
	for (auto& pair : _threads) {
		pair.second.flags.fetch_or(
		  _flag::windowResized, std::memory_order_relaxed);
	}
}
static void mouse_pos_callback(GLFWwindow* _, double x, double y) {
	if (x < 0 || y < 0) return;
	_mousePos.store(uvec2{x, y}, std::memory_order_release);
	for (auto& pair : _threads) {
		pair.second.flags.fetch_or(
		  _flag::mouseMoved, std::memory_order_relaxed);
	}
}

static void key_callback(
  GLFWwindow* window, int key, int scancode, int action, int mods) {
	KeyAction k;
	switch (action) {
	case GLFW_RELEASE: k = KeyAction::release; break;
	case GLFW_PRESS: k = KeyAction::press; break;
	case GLFW_REPEAT: k = KeyAction::repeat; break;
	default: assert(false);	 // throw on debug mode
	}

	uint flags{(uint)mods};

	for (auto& c : _activeCallbacks) {
		if ((c.keyAction == KeyAction::null || c.keyAction == k)
			&& (c.keyCode == KeyCode::null || c.keyCode == key
				|| c.keyCode == -scancode)
			&& ((flags ^ (c.keyModFlags.get() >> 6) & c.keyModFlags.get())
				== 0)) {
			if (c.id == std::this_thread::get_id()) {
				c.func(key, k, flags << 6);
			} else {
				auto& v = _eventsToAdd[c.id];  // WILL modify/add entries to map
				v.emplace_back(_KeyEvent{c.func, key, k, flags << 6});
			}
		}
	}
}

inline void _unsetFullScreenMode(FullScreenMode v) {
	switch (v) {
	case FullScreenMode::windowed:
		_windowedSize = _windowSize.load(std::memory_order_relaxed);
		_windowedPos = _windowPos.load(std::memory_order_relaxed);
		break;
	case FullScreenMode::fullscreen: break;
	case FullScreenMode::windowedFullscreen:
		glfwSetWindowAttrib(_window, GLFW_DECORATED, GLFW_TRUE);
		break;
	default: assert(false);
	}
}
inline void _setFullScreenMode(FullScreenMode v) {
	auto vMode = glfwGetVideoMode(_targetMonitor);
	switch (v) {
	case FullScreenMode::windowed:
		glfwSetWindowMonitor(_window, NULL, _windowedPos.x, _windowedPos.y,
		  _windowedSize.x, _windowedSize.y, 0);
		break;
	case FullScreenMode::fullscreen:
		glfwSetWindowMonitor(_window, _targetMonitor, 0, 0, vMode->width,
		  vMode->height, vMode->refreshRate);
		break;
	case FullScreenMode::windowedFullscreen:
		glfwSetWindowAttrib(_window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowMonitor(
		  _window, NULL, 0, 0, vMode->width, vMode->height, vMode->refreshRate);
		break;
	default: assert(false);
	}
}

// main for the threadEvents
static void _main() {
	try {
		logger.newThread("ThreadEvent");
		_state.store(State::normal, std::memory_order_relaxed);
		_state.notify_all();
		glfwSetErrorCallback(glfwErrorCallback);
		glfwInit();

		// window settings
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);	// resizable?
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);	// visible on start?
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);	// show window border?
		glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);	// focus on start?
		glfwWindowHint(GLFW_AUTO_ICONIFY,
		  GLFW_TRUE);  // On fullscreen mode, iconify when lost focus?
		glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);	// Window always on top?
		glfwWindowHint(
		  GLFW_MAXIMIZED, GLFW_FALSE);	// Window maximized on start?
		glfwWindowHint(GLFW_CENTER_CURSOR,
		  GLFW_TRUE);  // On fullscreen mode, centre mouse on start?
		glfwWindowHint(GLFW_FOCUS_ON_SHOW,
		  GLFW_FALSE);	// When called show window, focus input?
		glfwWindowHint(GLFW_SCALE_TO_MONITOR,
		  GLFW_TRUE);  // Dot Per Inch messurement use monitor value instead of
					   // OS value?

#ifdef USE_OPENGL
		// openGL settings
		glfwWindowHint(
		  GLFW_CLIENT_API, GLFW_OPENGL_API);  // Use what rendering api?
		glfwWindowHint(GLFW_OPENGL_PROFILE,
		  GLFW_OPENGL_CORE_PROFILE);  // use what type of openGL? full
									  // compitibility?
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
		  GLFW_TRUE);  // remove all deprecated functions in openGL?
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);	// api required version?
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);	// api required version?
		glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR,
		  GLFW_RELEASE_BEHAVIOR_NONE);	// Should openGL flush on buffer
										// release?
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,
		  IS_DEBUG_MODE ? GLFW_TRUE : GLFW_FALSE);	// is openGL in debug mode?
		glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS,
		  IS_DEBUG_MODE
			? GLFW_LOSE_CONTEXT_ON_RESET
			: GLFW_NO_ROBUSTNESS);	// should openGL care about out of bound
									// issue? what should it do?
		glfwWindowHint(GLFW_CONTEXT_NO_ERROR,
		  IS_DEBUG_MODE ? GLFW_FALSE
						: GLFW_TRUE);  // should openGL not check error?
#endif
#ifdef USE_VULKAN
		// Vulkan Renderer settings
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

		// Base render settings
		glfwWindowHint(GLFW_SAMPLES, 4);  // gl sub sampling?
		glfwWindowHint(GLFW_SRGB_CAPABLE,
		  GLFW_TRUE);  // OpenGL can use sRGB color instead of RGB?
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);  // Use double buffering?
		glfwWindowHint(GLFW_REFRESH_RATE,
		  GLFW_DONT_CARE);	// On fullscreen mode, what
							// refresh rate? (DONT_CARE = max)
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER,
		  GLFW_FALSE);	// Is the window transparent?

		bool useFullScreen = false;
		uint windowW = windowInitSize.x;
		uint windowH = windowInitSize.y;

		// make window. Finally
		_targetMonitor = glfwGetPrimaryMonitor();
		_window = glfwCreateWindow(windowW, windowH, "Project Planetery Engine",
		  useFullScreen ? glfwGetPrimaryMonitor() : NULL, NULL);
		if (_window == NULL) {
			logger(format({COLOR_RED}), "Failed to create GLFW windows.\n");
			glfwTerminate();
			throw "GLFW init failed";
		}
		{
			int x, y;
			glfwGetFramebufferSize(_window, &x, &y);
			_framebufferSize.store(uvec2{x, y});
			glfwGetWindowSize(_window, &x, &y);
			_windowSize.store(uvec2{x, y});
			glfwGetWindowPos(_window, &x, &y);
			_windowPos.store(ivec2{x, y});
			double dx, dy;
			glfwGetCursorPos(_window, &dx, &dy);
			_mousePos.store(uvec2{dx, dy});
			float fx, fy;
			glfwGetMonitorContentScale(_targetMonitor, &fx, &fy);
			_pixelPerInch.store(
			  vec2(SYS_DEFAULT_DPI) * vec2(fx, fy), std::memory_order_relaxed);
		}

		glfwSetFramebufferSizeCallback(
		  _window, framebuffer_size_callback);	// Main thread only
		// glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Main
		// thread only glfwSetCursorPosCallback(window, mouse_callback); //Main
		// thread only
		glfwSetCursorPosCallback(_window, mouse_pos_callback);
		glfwSetWindowSizeCallback(_window, window_size_callback);
		glfwSetWindowPosCallback(_window, window_pos_callback);
		glfwSetKeyCallback(_window, key_callback);
		glfwSetWindowSizeLimits(_window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT,
		  WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT);

		// start render thread
		render::ThreadRender::start(_window);

		while (
		  !(glfwWindowShouldClose(_window)
			|| _state.load(std::memory_order_relaxed) == State::requestStop
			|| render::ThreadRender::getState() == render::State::complete)) {
			ThreadEvents::counter++;

			// Process request
			if (_fullScreenModeChangeRequest.exchange(
				  false, std::memory_order_release)) {
				auto newMode =
				  _fullScreenModeTarget.load(std::memory_order_acquire);
				if (newMode != _fullScreenMode) {
					_unsetFullScreenMode(_fullScreenMode);
					_setFullScreenMode(newMode);
					_fullScreenMode = newMode;
				}
			}

			// Do event stuffs

			// First, add new events callback
			for (auto& p : _threads) {
				auto& d = p.second;

				// Since vector.size is not atomic/thread-safe, use additional
				// atomic<uint> for size, WHICH MAY NOT CORRESPOND WITH THE
				// ACTUAL SIZE.
				if (d.CallbacksEmpty.load(std::memory_order_relaxed) == false) {
					static std::vector<_KeyCallback> vec{};
					assert(vec.empty());
					{
						std::lock_guard _{d.mxCallbacks};
						d.CallbacksEmpty.store(
						  true, std::memory_order_relaxed);	 // Set the size
						d.newCallbacks->swap(vec);
					}
					_activeCallbacks.insert(
					  _activeCallbacks.end(), vec.begin(), vec.end());
					vec.clear();
				}
			}

			// Second,
			// TODO: delete events callback

			// Third, poll in events and add keyEvents entry into multimap
			// glfwPollEvents(); //WILL MAYBE BLOCK
			glfwWaitEvents();  // Will BLOCK, call glfwPostEmptyEvent() to wake
							   // thread

			// Forth, dispach events to each threads
			for (auto& pair : _eventsToAdd) {
				auto& vec = pair.second;
				auto& t = _threads[pair.first];	 // WILL add entries
				{
					std::lock_guard _{t.mxEvents};
					t.EventsEmpty.store(false, std::memory_order_relaxed);
					t.events->insert(t.events->end(), vec.begin(), vec.end());
				}
			}

			// Fifth, cleanup
			_eventsToAdd.clear();

			// Sixth, add sleep so that this doesn't eat up all runtime
			// TODO: Add sleep here
		}
		auto& eptr = _eptr;
		auto rThreadState = render::ThreadRender::getState();
		if (eptr) {
			logger("Critical unknown error in ThreadRender. Haiting...");
		} else if (rThreadState == render::State::complete) {
			logger("Unknown exit in ThreadRender. Haiting...");
		} else {
			logger("Window close request recived. Stopping render thread...");
		}
		render::ThreadRender::requestStop();
		render::ThreadRender::join(1000000000);
		logger("Render thread stopped.");
	} catch (...) {
		logger("Critical unknown error in ThreadEvents. Halting...\n");
		_state.store(State::requestStop, std::memory_order_relaxed);
		if (render::ThreadRender::getState() != render::State::init) {
			render::ThreadRender::requestStop();
			render::ThreadRender::join(1000000000);
			logger("Render thread stopped.");
		} else {
			logger("ThreadRender is not initiated.");
			// BUG: Not gonna work if threadRender is running but before init
			// set and threadEvents got error. However std::thread will also
			// halt all child thread if not poperly closed...
		}
	}
	glfwTerminate();
	logger.closeThread("ThreadEvent");
	_state.store(State::complete, std::memory_order_relaxed);
}

void ThreadEvents::start() {
	if (_thread != nullptr)
		throw "ThreadEvents.start() called when thread already exist!";
	try {
		_thread = new std::thread(_main);
	} catch (std::system_error e) {
		logger("Errror! ThreadEvents could not be started!\n", e.what());
		// doErrorHandling
		throw e;
	}
}
void ThreadEvents::requestStop() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.load(std::memory_order_relaxed);
	if (st == State::normal)
		_state.store(State::requestStop, std::memory_order_relaxed);
	else if (st == State::paused) {
		_state.store(State::requestStop, std::memory_order_relaxed);
		_state.notify_all();
	}
}
void ThreadEvents::pause() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.load(std::memory_order_relaxed);
	if (st == State::normal)
		_state.store(State::paused, std::memory_order_relaxed);
}
void ThreadEvents::unpause() {
	_state.wait(State::init, std::memory_order_relaxed);
	State st = _state.load(std::memory_order_relaxed);
	if (st == State::paused) {
		_state.store(State::normal, std::memory_order_relaxed);
		_state.notify_all();
	}
}
void ThreadEvents::join() {
	_thread->join();
	delete _thread;
}

void ThreadEvents::join(uint nsTimeout) {
	std::unique_lock _{mx};
	bool success = cv.wait_for(_, std::chrono::nanoseconds(nsTimeout), []() {
		return _state.load(std::memory_order_relaxed) == State::complete;
	});
	if (success) {
		_thread->join();
		delete _thread;
	} else {
		logger("ThreadEvents join request timed out. Inore thread. (Will cause "
			   "memory leaks)\n");
	}
}
void ThreadEvents::panic(std::exception_ptr eptr) {
	_eptr = eptr;
	requestStop();
}
void ThreadEvents::log(std::exception_ptr eptr) {
	// do something???
}

void ThreadEvents::swapBuffer() { glfwSwapBuffers(_window); }

void* events::ThreadEvents::getGLFWWindow() { return _window; }

void ThreadEvents::setWindowedInline() {
	if (_fullScreenMode != FullScreenMode::windowed) {
		_unsetFullScreenMode(_fullScreenMode);
		_setFullScreenMode(FullScreenMode::windowed);
		_fullScreenMode = FullScreenMode::windowed;
	}
}
void ThreadEvents::setFullScreenInline() {
	if (_targetMonitor != NULL
		&& _fullScreenMode
			 != FullScreenMode::fullscreen) {  // use NULL here as
											   // glfw returns null
		_unsetFullScreenMode(_fullScreenMode);
		_setFullScreenMode(FullScreenMode::fullscreen);
		_fullScreenMode = FullScreenMode::fullscreen;
	} else if (_targetMonitor == NULL
			   && _fullScreenMode
					!= FullScreenMode::windowedFullscreen) {  // use NULL here
															  // as glfw returns
															  // null
		_unsetFullScreenMode(_fullScreenMode);
		_setFullScreenMode(FullScreenMode::windowedFullscreen);
		_fullScreenMode = FullScreenMode::windowedFullscreen;
	}
}
void ThreadEvents::setWindowedFullScreenInline() {
	if (_fullScreenMode != FullScreenMode::windowedFullscreen) {
		_unsetFullScreenMode(_fullScreenMode);
		_setFullScreenMode(FullScreenMode::windowedFullscreen);
		_fullScreenMode = FullScreenMode::windowedFullscreen;
	}
}

FullScreenMode ThreadEvents::getFullScreenModeInline() {
	return _fullScreenMode;
}

State ThreadEvents::getState() {
	return _state.load(std::memory_order_relaxed);
}

thread_local _Flags _loadedFlag = 0;

void ThreadEvents::pollFlags() {
	_loadedFlag = _threads[std::this_thread::get_id()].flags.exchange(0);
}
bool ThreadEvents::isWindowResized() {
	return _loadedFlag & _flag::windowResized;
}
uvec2 ThreadEvents::getWindowSize() {
	return _windowSize.load(std::memory_order_relaxed);
}
uvec2 ThreadEvents::getFramebufferSize() {
	return _framebufferSize.load(std::memory_order_relaxed);
}
bool ThreadEvents::isWindowMoved() { return _loadedFlag & _flag::windowMoved; }
ivec2 ThreadEvents::getWindowPos() {
	return _windowPos.load(std::memory_order_relaxed);
}
vec2 ThreadEvents::getPixelPerInch() {
	return _pixelPerInch.load(std::memory_order_relaxed);
}
bool ThreadEvents::isMouseMoved() { return _loadedFlag & _flag::mouseMoved; }
uvec2 ThreadEvents::getMousePos() {
	return _mousePos.load(std::memory_order_relaxed);
}

void events::ThreadEvents::closeWindow() {
	glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

void events::ThreadEvents::setWindowed() {
	_fullScreenModeTarget.store(
	  FullScreenMode::windowed, std::memory_order_release);
	_fullScreenModeChangeRequest.store(true, std::memory_order_relaxed);
}
void events::ThreadEvents::setFullScreen() {
	_fullScreenModeTarget.store(
	  FullScreenMode::fullscreen, std::memory_order_release);
	_fullScreenModeChangeRequest.store(true, std::memory_order_relaxed);
}
void events::ThreadEvents::setWindowedFullscreen() {
	_fullScreenModeTarget.store(
	  FullScreenMode::windowedFullscreen, std::memory_order_release);
	_fullScreenModeChangeRequest.store(true, std::memory_order_relaxed);
}

void events::ThreadEvents::addKeyEventCallback(KeyEventFunction f, int keyCode,
  KeyAction keyAction, KeyModFlags keyModFlags) {
	auto& t = _threads[std::this_thread::get_id()];	 // WILL add entries
	{
		std::lock_guard _{t.mxCallbacks};
		t.newCallbacks->emplace_back(_KeyCallback{
		  f, keyCode, keyAction, keyModFlags, std::this_thread::get_id()});
	}
	t.CallbacksEmpty.store(false, std::memory_order_relaxed);
}
void events::ThreadEvents::addInlineKeyEventCallback(KeyEventFunction f,
  int keyCode, KeyAction keyAction, KeyModFlags keyModFlags) {
	auto& t = _threads[std::this_thread::get_id()];	 // WILL add entries
	{
		std::lock_guard _{t.mxCallbacks};
		t.newCallbacks->emplace_back(
		  _KeyCallback{f, keyCode, keyAction, keyModFlags, _thread->get_id()});
	}
	t.CallbacksEmpty.store(false, std::memory_order_relaxed);
}

bool events::ThreadEvents::pollEvents() {
	auto t = _threads.find(std::this_thread::get_id());
	if (t == _threads.end()
		|| t->second.EventsEmpty.load(std::memory_order_relaxed))
		return false;
	static std::vector<_KeyEvent>* events = new std::vector<_KeyEvent>{};
	{
		std::lock_guard _{t->second.mxEvents};
		std::swap(events, t->second.events);
		t->second.EventsEmpty.store(true, std::memory_order_relaxed);
	}
	for (auto& e : *events) { e.func(e.keyCode, e.keyAction, e.keyModFlags); }
	return true;
}
