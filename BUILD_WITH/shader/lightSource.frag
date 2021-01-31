#version 460 core
in vec3 lightColor;
out vec4 fragColor;


void main() {
	fragColor = vec4(lightColor,1.0);
}