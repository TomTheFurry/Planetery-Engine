#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;
out vec4 gl_Position;
out vec3 vecColor;
out vec2 texCoord;
out vec3 vecNormal;
out vec3 fragPos;
out vec3 lightPos;
uniform mat4 objectMatrix;
uniform mat4 coordMatrix;
uniform mat4 projectMatrix;
uniform mat4 normalMatrix;
uniform vec3 lightCoord;

void main()
{
    vecColor = aColor;
    texCoord = aTexCoord;
    vecNormal = vec3(coordMatrix*normalMatrix*vec4(aNormal,0.0));
    lightPos = vec3(coordMatrix*vec4(lightCoord, 1.0));
    fragPos = vec3(coordMatrix*objectMatrix*vec4(aPos, 1.0));
    gl_Position = projectMatrix*objectMatrix*vec4(aPos, 1.0);
}