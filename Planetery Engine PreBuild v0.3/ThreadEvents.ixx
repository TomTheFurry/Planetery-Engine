export module ThreadEvents;
import Define;
import std.core;

export namespace events {
	enum class State {
		init,
		paused,
		normal,
		requestStop,
		complete
	};
	namespace KeyCode {
		enum KeyCode {
			unkown = -1, //TAG: Hey old me... unk'n'own!!!
			null = 0,
			space = 32,
			apostrophe = 39,
			comma = 44,
			minus,
			period,
			slash,
			num0 = 48,
			num1,
			num2,
			num3,
			num4,
			num5,
			num6,
			num7,
			num8,
			num9,
			semicolon = 59,
			equal = 61,
			a = 65,
			b,
			c,
			d,
			e,
			f,
			g,
			h,
			i,
			j,
			k,
			l,
			m,
			n,
			o,
			p,
			q,
			r,
			s,
			t,
			u,
			v,
			w,
			x,
			y,
			z,
			leftBracket = 91,
			backSlash,
			rightBracket,
			graveAccent = 96,
			world1 = 161,  //?? WTF is this key?
			world2,
			escape = 256,
			enter,
			tab,
			backspace,
			insert,
			deleteKey,
			arrowRight = 262,
			arrowLeft,
			arrowDown,
			arrowUp,
			pageUp = 266,
			pageDown,
			home,
			end,
			capsLock = 280,
			scrollLock,
			numLock,
			printScreen,
			pause,
			F1 = 290,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			F13,
			F14,
			F15,
			F16,
			F17,
			F18,
			F19,
			F20,
			F21,
			F22,
			F23,
			F24,
			F25,
			numpad0 = 320,
			numpad1,
			numpad2,
			numpad3,
			numpad4,
			numpad5,
			numpad6,
			numpad7,
			numpad8,
			numpad9,
			numpadDecimal = 330,
			numpadDivide,
			numpadMultiply,
			numpadSubtract,
			numpadAdd,
			numpadEnter,
			numpadEqual,
			leftShift = 340,
			leftControl,
			leftAlt,
			leftSuper,
			rightShift = 344,
			rightControl,
			rightAlt,
			rightSuper,
			menu = 348
		};
	}
	enum class KeyAction { null = -1, release = 0, press = 1, repeat = 2 };
	namespace KeyModFlag {
		enum KeyModFlag {
			null = 0,
			matchShift = 1,
			matchControl = 2,
			matchAlt = 4,
			matchSuper = 8,
			matchCapsLock = 16,
			matchNumLock = 32,
			shift = 64,
			control = 128,
			alt = 256,
			super = 512,
			capsLock = 1024,
			numLock = 2048
		};
	}
	class KeyModFlags
	{
		uint _flag;
	  public:
		KeyModFlags(uint flags): _flag(std::move(flags)) {}
		bool get(uint flag) { return bool(_flag & flag); }
		void set(uint flag) { _flag |= flag; }
		void unset(uint flag) { _flag &= ~flag; }
		uint get() { return _flag; }
	};
	using KeyEventFunction = std::function<void(int, KeyAction, KeyModFlags)>;
	enum class FullScreenMode { windowed, fullscreen, windowedFullscreen };
	class ThreadEvents
	{
	  public:
		static std::atomic<uint> counter;
		// called ONLY by main thread
		static void start();			   // split thread
		static void requestStop();		   // stop thread (with cleanup)
		static void pause();			   // pause thread (for maybe debug?)
		static void unpause();			   // unpause thread
		static void join();				   // join thread
		static void join(uint nsTimeout);  // join thread

		// called ONLY by child thread
		static void panic(std::exception_ptr eptr);	 // report critical
													 // exception
		static void log(
		  std::exception_ptr eptr);	 // report non critical exception

		// called ONLY by render thread
		static void swapBuffer();
		// HACK: not sure who should own the glfwWindow object.
		// Don't wanna show the glfwWindow in header file, so... return void*
		static void* getGLFWWindow();

		// called ONLY by event thread (inline function)
		static void setWindowedInline();
		static void setFullScreenInline();
		static void setWindowedFullScreenInline();
		static FullScreenMode getFullScreenModeInline();

		// for all thread
		// getter
		static State getState();

		static void pollFlags();  // update flag for value changes

		static bool isWindowResized();
		static uvec2 getWindowSize();
		static uvec2 getFramebufferSize();

		static bool isWindowMoved();
		static ivec2 getWindowPos();

		// effected by both windowMoved & windowResized
		static vec2 getPixelPerInch();

		static bool isMouseMoved();
		static uvec2 getMousePos();

		// settings
		static void closeWindow();
		static void setWindowed();
		static void setFullScreen();
		static void setWindowedFullscreen();

		// window key event (NON BLOCKING, so the actual binding and unbinding
		// WILL be delayed)
		[[nodiscard]] static void addKeyEventCallback(KeyEventFunction f,
		  int keyCode = KeyCode::null, KeyAction keyAction = KeyAction::null,
		  KeyModFlags keyModFlags = KeyModFlags(
			KeyModFlag::null));	 // Unchangable
		[[nodiscard]] static void addInlineKeyEventCallback(KeyEventFunction f,
		  int keyCode = KeyCode::null, KeyAction keyAction = KeyAction::null,
		  KeyModFlags keyModFlags = KeyModFlags(
			KeyModFlag::null));	 // Unchangable

		static bool pollEvents();  // return true if there are events
	};
}