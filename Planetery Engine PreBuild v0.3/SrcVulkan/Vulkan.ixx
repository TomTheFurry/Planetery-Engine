export module Vulkan;
export import: Enum;
export import: Device;
export import: Buffer;
export import: Image;
export import: Shader;
export import: Sync;
export import: Commend;
export import: Descriptor;
export import: Pipeline;
export import: Tick;

import: Internal;

import std.core;
import Define;

export namespace vk {
	struct SwapchainCallback {
		void (*onCreate)(bool) = nullptr;
		void (*onDestroy)(bool) = nullptr;
	};


	const uint* getLayerVersion(const char* name);		// may return nullptr
	const uint* getExtensionVersion(const char* name);	// may return nullptr
	bool requestLayer(const char* name, uint minVersion = 0);
	bool requestExtension(const char* name, uint minVersion = 0);

	void requestDeviceExtension(const char* name, bool optional = false);

	void init();  // request all needed extension/layers before call!
	template<typename Func> bool drawFrame(Func f);	 // Render Thread only
	void checkStatus() noexcept(false);	 // throws OutdatedSwapchainException
	void end(void (*cleanupFunc)());
	void setSwapchinCallback(SwapchainCallback callback); //Render Thread only

	void notifyOutdatedSwapchain();	 // Can be called by any frame
	void testSwitch();

	namespace device {
		class PhysicalDevice;
		class LogicalDevice;
		class Queue;
	}

	namespace object {
		class Buffer;
		class VertexBuffer;
		class FrameBuffer;
		class Program;
	}

	namespace operation {
		class Operator;
		class Commeend;
	}

	void _prepareFrame();
	void _sendFrame();
	void _resetOutdatedFrame();
	void _testDraw();

	//ExternTempTest
	void _drawSetup();
}


export template<typename Func> bool vk::drawFrame(Func func) {
	try {
		_prepareFrame();
		func();
		_sendFrame();
		return true;
	} catch (OutdatedSwapchainException) {
		_resetOutdatedFrame();
		return false;
	}
}

