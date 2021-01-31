#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <Vector>
#include <list>
#include <chrono>


#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "Global.h"
#include "ShaderProgram.h"
#include "RenderProgram.h"

#include "RollingAverage.h"

void APIENTRY glDebugOutput(
	GLenum source, GLenum type,
	uint id, GLenum severity,
	GLsizei length, const char* message,
	const void* userParam)
{
	if (id == 131185 || id == 131204 || id == 131169 || id == 7) return; //7: deprecated behavior warning
	logger << "\n--------OPENGL ERROR--------\n";
	logger << "Error id " << std::to_string(id) << ": " << message << "\n";

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             logger << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   logger << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: logger << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     logger << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     logger << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           logger << "Source: Other"; break;
	default:
		logger << "Source: Unknown (" << std::to_string(source) << ")"; break;
	}
	logger << "\n";

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               logger << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: logger << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  logger << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         logger << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         logger << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              logger << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          logger << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           logger << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               logger << "Type: Other"; break;
	default:
		logger << "Type: Unknown (" << std::to_string(type) << ")"; break;
	}
	logger << "\n";

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         logger << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       logger << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          logger << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: logger << "Severity: notification"; break;
	default:
		logger << "Severity: Unknown (" << std::to_string(severity) << ")"; break;
	}
	logger << "\n\n";

}

using RenderFunction = void(*)(void* const, ulint);
using InitFunction = RenderFunction(*)(void* const);
using DestFunction = void(*)(void* const);
struct InitCallback {
	inline InitCallback(InitFunction function, void* const object) {
		f = function;
		o = object;
	}
	InitFunction f;
	void* o;
};

struct RenderCallback {
	inline RenderCallback(RenderFunction function, void* const object) {
		f = function;
		o = object;
	}
	RenderFunction f;
	void* o;
};

struct DestCallback {
	inline DestCallback(DestFunction function, void* const object) {
		f = function;
		o = object;
	}
	DestFunction f;
	void* o;
};


class RenderThread : public std::thread //Multithreading is a b**ch!!
{
public:
	static inline RenderThread* get() {
		return _singleton;
	}
	static inline void create(GLFWwindow* w) {
		_singleton = new RenderThread(w);
	}
	static inline void setWindownHint() {
		//glfwWindowHint(GLFW_SAMPLES, 32);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	//NOT INSTANT
	static inline void start() {
		get()->_state.store(1, std::memory_order_release); //make sure the _state is stored before wake
		std::unique_lock<std::mutex> lockTool(get()->stateLock); //make sure the thread is not currently checking state
		get()->pauseCv.notify_all();
		//lockTool destructed on leave
	};
	//NOT INSTANT
	static inline void pause() {
		get()->_state.store(0, std::memory_order_relaxed); //don't care if the pause happens a bit late (~1 tick late)
		//don't need to wake up the thread. (You want the thread to sleep right?)
	}
	//NOT INSTANT
	static inline void stop() {
		get()->_state.store(2, std::memory_order_release); //make sure the _state is stored before wake
		std::unique_lock<std::mutex> lockTool(get()->stateLock); //make sure the thread is not currently checking state
		get()->pauseCv.notify_all();
		//lockTool destructed on leave
	}
	static inline void addInitQueue(InitCallback c) {
		get()->bufferLock.lock(); //makes sure thread isn't swapping buffer
		get()->initQueue->push_back(c);
		get()->bufferLock.unlock();
	}
	static inline void addDestQueue(DestCallback c) {
		get()->bufferLock.lock(); //makes sure thread isn't swapping buffer
		get()->destQueue->push_back(c);
		get()->bufferLock.unlock();
	}
private:
	static void _callback() {
		_singleton->_main();
	}

	void _main() {
		logger << "RenderThread: Thread created\n";
		glfwMakeContextCurrent(window);
		glViewport(0, 0, global->windowSize.x, global->windowSize.y);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		//Shader setup
		logger << "RenderThread: GL init done. Started to load shader..." << "\n";
		ShaderProgram::initClass();

		//Gen Model and Renderer
		logger << "RenderThread: Started to load renderer..." << "\n";
		global->r = new RenderProgram();
		global->r->objectMode();
		logger << "RenderThread: All init done.\n";

		auto roller = RollingAverage<long long, long long, 60>();
		ulint nsDeltaPerSec = 0;
		uint tickCount = 0;
		ulint nsDelta = 0;
		std::chrono::steady_clock::time_point hotTickTimerA;
		std::chrono::steady_clock::time_point hotTickTimerB;
		std::chrono::steady_clock::time_point tickTimerA;
		std::chrono::steady_clock::time_point tickTimerB;
		tickTimerB = std::chrono::high_resolution_clock::now();

		while (_state.load(std::memory_order_relaxed) < 2) { //does not have to instantly respond
			while (_state.load(std::memory_order_relaxed) == 0) { //does not have to instantly respond
				auto lockTool = std::unique_lock<std::mutex>(stateLock); //for condition var
				pauseCv.wait(lockTool, [this]() {
					return _state.load(std::memory_order_acquire) > 0;
					}
				);
			}
			hotTickTimerB = std::chrono::high_resolution_clock::now();
			bufferLock.lock(); //makes sure no one is adding new object to buffer
			auto bInitQueue = initQueue;
			auto bDestQueue = destQueue;
			initQueue = new std::vector<InitCallback>();
			destQueue = new std::vector<DestCallback>();
			bufferLock.unlock(); //buffer swap done. release the lock

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (auto& ic : *bInitQueue) {
				renderQueue.push_back(RenderCallback(ic.f(ic.o), ic.o));
			}
			delete bInitQueue;
			for (auto& dc : *bDestQueue) {
				renderQueue.erase(std::remove_if(renderQueue.begin(), renderQueue.end(),
					[dc](RenderCallback c) {return dc.o == c.o; }));
				dc.f(dc.o);
			}
			delete bDestQueue;

			//TODO: Add priority for rendering


			for (auto rc : renderQueue) {
				rc.f(rc.o, nsDelta);
			}

			tickCount++;
			tickTimerA = std::chrono::high_resolution_clock::now();
			nsDelta = (tickTimerA - tickTimerB).count();
			std::swap(tickTimerA, tickTimerB);
			nsDeltaPerSec += nsDelta;

			hotTickTimerA = std::chrono::high_resolution_clock::now();
			auto hotSpan = (hotTickTimerA - hotTickTimerB).count();
			roller.next(hotSpan);

			if (nsDeltaPerSec >= NS_PER_S) {
				logger << "RenderThread: Average Tick speed: " << nanoSec(roller.get()) <<
					", " << tickCount << "\n";
				nsDeltaPerSec -= NS_PER_S;
				tickCount = 0;
			}

			glfwSwapBuffers(window); //Will block and wait for screen updates (v-sync)
		}
		logger << "RenderThread: Thread ended\n";
	}
	
	std::atomic<uint> _state; // 0: pause, 1: run, 2: kill
	GLFWwindow* window;
	std::mutex stateLock;
	std::condition_variable pauseCv;

	RenderThread(GLFWwindow* w) : std::thread(_callback) {
		window = w;
		initQueue = new std::vector<InitCallback>();
		destQueue = new std::vector<DestCallback>();
		_state.store(0, std::memory_order_relaxed);
	};
	std::mutex bufferLock;
	std::vector<InitCallback>* initQueue;
	std::list<RenderCallback> renderQueue;
	std::vector<DestCallback>* destQueue;
	static RenderThread* _singleton;

};
RenderThread* RenderThread::_singleton = nullptr;

