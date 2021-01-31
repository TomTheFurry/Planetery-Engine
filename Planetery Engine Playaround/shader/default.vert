#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
out vec4 gl_Position;
out vec3 ourColor;
out vec2 texCoord;
uniform mat4 objectMatrix;
uniform mat4 coordMatrix;

void main()
{
    ourColor = aColor;
    texCoord = aTexCoord;
    gl_Position = coordMatrix*objectMatrix*vec4(aPos, 1.0);
}