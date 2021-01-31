#version 460 core
layout (location = 0) in vec2 pos;
layout (location = 1) in uint gId;
out vec2 posV;
out uint gIdV;

void main()
{
	posV = pos;
	gIdV = gId;
}