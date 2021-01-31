#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;
out vec4 gl_Position;
out vec3 lightColor;
uniform mat4 objectMatrix;
uniform mat4 coordMatrix;
uniform mat4 projectMatrix;

void main()
{
    lightColor = aColor;
    gl_Position = projectMatrix*objectMatrix*vec4(aPos, 1.0);
}