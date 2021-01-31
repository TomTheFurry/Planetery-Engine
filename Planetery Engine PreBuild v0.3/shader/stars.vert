#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aId;
layout (location = 3) in float aRadius;

out vec3 bColor;
out float bId;
out float bRadius;

uniform mat4 coordMatrix;

void main()
{
	bColor = aColor;
	bId = aId;
	bRadius = aRadius;
    gl_Position = coordMatrix * vec4(aPos, 1.0);
}