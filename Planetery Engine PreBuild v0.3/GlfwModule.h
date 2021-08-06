#pragma once
#include "Marco.h"
#ifdef USE_VULKAN
#	define GLFW_INCLUDE_NONE
#	define GLFW_INCLUDE_VULKAN
#	include <glfw/glfw3.h>
#else
#	define GLFW_INCLUDE_NONE
#	include <glfw/glfw3.h>
#endif