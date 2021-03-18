#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "Define.h"

namespace vk {
	const uint* getLayerVersion(const char* name);		// may return nullptr
	const uint* getExtensionVersion(const char* name);	// may return nullptr
	bool requestLayer(const char* name, uint minVersion = 0);
	bool requestExtension(const char* name, uint minVersion = 0);
	
	void requestDeviceExtension(const char* name, bool optional = false);



	void init();  // request all needed extension/layers before call!
	void tick();
	void end();

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

