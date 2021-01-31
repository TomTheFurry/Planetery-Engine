#version 460 core
in vec3 bColor;
out vec4 fragColor;
uniform mat4 colorMatrix;


void main() {
	fragColor = colorMatrix*vec4(bColor, 1.0);
}