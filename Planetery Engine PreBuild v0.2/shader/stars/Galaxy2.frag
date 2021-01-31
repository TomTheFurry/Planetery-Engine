#version 460 core

in vec3 bColor;

void main() {
	gl_FragColor = vec4(bColor,1.0);
}