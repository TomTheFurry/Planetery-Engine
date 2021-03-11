#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "Define.h"

namespace vk {
	const uint* getLayerVersion(const char* name);		// may resturn nullptr
	const uint* getExtensionVersion(const char* name);	// may return nullptr
	bool requestLayer(const char* name, uint minVersion = 0);
	bool requestExtension(const char* name, uint minVersion = 0);
	void init();  // request all needed extension/layers before call!
	void end();
}
