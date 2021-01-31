#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aPole;
layout (location = 3) in uint aType;
layout (location = 4) in float aRadius;

out vec3 bColor;

uniform mat4 viewportMatrix;
uniform mat4 projectionMatrix;
uniform float cutout;

uniform float near;
uniform float far;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
	bColor = aColor;
	vec4 pos = projectionMatrix * viewportMatrix * vec4(aPos, 1.0);
	float z = LinearizeDepth(pos.z);
	gl_Position = pos;
}