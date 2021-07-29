#version 460 core
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform UniformData{
	vec4 inColor;
} ubo;

void main() {
	fragColor = ubo.inColor;
}