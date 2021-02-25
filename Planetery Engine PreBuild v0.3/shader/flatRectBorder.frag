#version 460 core
uniform vec4 rectBorderColor;
out vec4 fragColor;

void main()
{
	fragColor = rectBorderColor;
}