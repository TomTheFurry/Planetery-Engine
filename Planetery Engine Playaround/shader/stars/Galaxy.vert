#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aPole;
layout (location = 3) in uint aType;
layout (location = 4) in float aRadius;

out vec3 bColor;
out vec3 bPole;
out uint bType;
out float bRadius;
out vec4 bPos;

uniform mat4 viewportMatrix;

void main()
{
	bColor = aColor;
	bPole = aPole;
	bType = aType;
	bRadius = aRadius;
	bPos = viewportMatrix * vec4(aPos, 1.0);
}