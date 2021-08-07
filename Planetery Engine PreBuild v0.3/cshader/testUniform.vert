#version 460 core
layout (location = 0) in vec2 aPos;
layout(binding = 0) uniform UniformData{
	vec4 inColor;
} ubo;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
}