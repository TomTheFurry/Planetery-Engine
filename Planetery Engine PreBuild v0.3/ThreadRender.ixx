export module ThreadRender;
import std.core;
import std.threading;
import Define;

export namespace render {
	enum class State {
		init,
		paused,
		normal,
		requestStop,
		complete
	};
	using RenderFunction = std::function<void()>;
	struct RenderHandle {
		RenderFunction func;
		std::atomic<bool> requestDelete = false;
	};
	class ThreadRender {
	public:
		//thread control
		static void start(const char* (*callback)(void));
		static void requestStop(); //stop thread (with cleanup)
		static void pause(); //pause thread (for maybe debug?)
		static void unpause(); //unpause thread
		//for owner
		static void join(); //join thread
		static void join(uint nsTimeout); //join thread

		//for all thread
		static State getState();
		static RenderHandle* newRenderHandle(RenderFunction func);

		//for render func in render handle

	};
}


