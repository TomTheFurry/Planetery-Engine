#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in uint aId;
layout (location = 3) in float aRadius;

out vec3 bColor;
out uint bId;
out float bRadius;
out vec4 bPos;

uniform mat4 coordMatrix;

void main()
{
	bColor = aColor;
	bId = aId;
	bRadius = aRadius;
	bPos = coordMatrix * vec4(aPos, 1.0);
}