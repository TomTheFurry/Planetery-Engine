#version 460 core
layout (location = 0) in uint gId;
layout (location = 1) in vec2 pos;
layout (location = 0) out vec2 posV;
layout (location = 1) out uint gIdV;

void main()
{
	posV = pos;
	gIdV = gId;
}