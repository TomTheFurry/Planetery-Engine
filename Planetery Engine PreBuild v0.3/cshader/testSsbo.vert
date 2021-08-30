#version 460 core
#extension GL_KHR_vulkan_glsl : enable
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;

layout (location = 0) out vec2 texPos;
layout (std140, binding = 1) readonly buffer Ssbo{
	vec4 offsets[];
} ssbo;

void main()
{
	texPos = aUv;
    gl_Position = vec4(aPos, 0.0, 1.0) + ssbo.offsets[gl_VertexIndex];
}