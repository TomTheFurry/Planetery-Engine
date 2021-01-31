#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 bColor;
uniform mat4 viewportMatrix;
uniform mat4 projectMatrix;
uniform mat4 testMatrix;

void main()
{
    bColor = aColor;
    vec4 prePos = testMatrix*vec4(aPos, 1.0);
    vec3 gPos = vec3(prePos.xyz / prePos.w);
    gl_Position = projectMatrix*viewportMatrix*vec4(gPos, 1.0);
}