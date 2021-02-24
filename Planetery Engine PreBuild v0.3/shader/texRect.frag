#version 460 core
in vec2 texPos;
uniform sampler2D fontMap;
out vec4 fragColor;

void main()
{
	fragColor = texture(fontMap, texPos);
}