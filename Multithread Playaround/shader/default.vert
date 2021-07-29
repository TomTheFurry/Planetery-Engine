#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 bColor;
uniform mat4 viewportMatrix;
uniform mat4 projectMatrix;

void main()
{
    bColor = aColor;
    gl_Position = projectMatrix*viewportMatrix*vec4(aPos, 1.0);
}