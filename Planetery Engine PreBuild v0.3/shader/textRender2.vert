#version 460 core
layout (location = 0) in uint gId;
layout (location = 1) in vec2 pos;
out vec2 posV;
out uint gIdV;

void main()
{
	posV = pos;
	gIdV = gId;
}