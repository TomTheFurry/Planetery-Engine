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
	struct DeviceCallback {
		void (*onCreate)(LogicalDevice&) = nullptr;
		void (*onDestroy)(LogicalDevice&) = nullptr;
	};
	struct SwapchainCallback {
		void (*onCreate)(SwapChain&, bool) = nullptr;
		void (*onDestroy)(bool) = nullptr;
	};
	struct FrameCallback {
		void (*onDraw)(RenderTick&) = nullptr;
	};


	const uint* getLayerVersion(const char* name);		// may return nullptr
	const uint* getExtensionVersion(const char* name);	// may return nullptr
	bool requestLayer(const char* name, uint minVersion = 0);
	bool requestExtension(const char* name, uint minVersion = 0);

	void requestDeviceExtension(const char* name, bool optional = false);

	void init();  // request all needed extension/layers before call!
	bool drawFrame();	 // Render Thread only
	void checkStatus() noexcept(false);	 // throws OutdatedSwapchainException
	void end();
	void setCallback(DeviceCallback dCallback);	   // Render Thread only
	void setCallback(SwapchainCallback scCallback);  // Render Thread only
	void setCallback(FrameCallback fCallback);		 // Render Thread only

	void notifyOutdatedSwapchain();	 // Can be called by any frame

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

}
