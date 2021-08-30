#version 460 core
layout(location = 0) in vec2 texPos;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform UniformData{
	vec4 inColor;
} ubo;
layout(binding = 0) uniform sampler2D texSampler;

void main() {
	fragColor = ubo.inColor * texture(texSampler,texPos);
}