#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec4 gl_Position;
out vec3 bColor;
out vec3 viewPos;
uniform mat4 viewportMatrix;
uniform mat4 projectMatrix;
uniform mat4 objectMatrix;

void main()
{
    bColor = aColor;
	viewPos = (objectMatrix*vec4(aPos, 1.0)).xyz;
    gl_Position = projectMatrix*viewportMatrix*objectMatrix*vec4(aPos, 1.0);
}