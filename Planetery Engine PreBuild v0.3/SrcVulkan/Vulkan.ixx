export module Vulkan;
export import: Enum;
export import: Lifetime;
export import: Device;
export import: Swapchain;
export import: Queue;
export import: Buffer;
export import: Image;
export import: Shader;
export import: Sync;
export import: Commend;
export import: Descriptor;
export import: Pipeline;
export import: Tick;

import: Declaration;

import std.core;
import Define;

export namespace vk {


	const uint* getLayerVersion(const char* name);		// may return nullptr
	const uint* getExtensionVersion(const char* name);	// may return nullptr
	bool requestLayer(const char* name, uint minVersion = 0);
	bool requestExtension(const char* name, uint minVersion = 0);

	void requestDeviceExtension(const char* name, bool optional = false);

	void init();  // request all needed extension/layers before call!
	bool drawFrame(ulint timeout = ulint(-1));  // Render Thread only
	void end();
	void setCallback(DeviceCallback dCallback);		 // Render Thread only
	void setCallback(SwapchainCallback scCallback);	 // Render Thread only
	void setCallback(FrameCallback fCallback);		 // Render Thread only

}
