#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "Define.h"

namespace vk {
	
	struct Extension {
		std::string name;
		uint version;
	};

	const std::vector<Extension>& getExtensions();
	void init();
	void end();
}

