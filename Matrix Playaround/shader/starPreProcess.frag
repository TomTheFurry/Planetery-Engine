#version 460 core
in vec3 bColor;
out vec4 fragColor;


void main() {
	fragColor = vec4(bColor, 1.0);
}