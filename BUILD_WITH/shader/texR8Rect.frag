#version 460 core
in vec2 texPos;
uniform sampler2D fontMap;
uniform vec4 textColor;
out vec4 fragColor;

void main()
{
	fragColor = textColor * vec4(1.,1.,1.,1.-texture(fontMap, texPos).r);
}