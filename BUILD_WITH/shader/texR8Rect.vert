#version 460 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 size;
out vec2 aPos;
out vec2 aSize;


void main()
{
	aPos = pos;
	aSize = size;
}