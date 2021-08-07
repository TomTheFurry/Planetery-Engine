#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;

layout (location = 0) out vec2 texPos;

void main()
{
	texPos = aUv;
    gl_Position = vec4(aPos, 0.0, 1.0);
}