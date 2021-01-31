#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 bColor[];
in float bId[];
in float bRadius[];

out vec3 cColor;
out float cId;
out float cRadius;
out vec3 fragPos;
out vec3 centre;

uniform mat4 projectionMatrix;

void main() {
	vec4 pos = gl_Position[0];

	cColor = bColor[0];
	cId = bId[0];

	// a,b
	// c,d
	vec4 a = pos+vec4(-bRadius,+bRadius,0.0f);
	vec4 b = pos+vec4(+bRadius,+bRadius,0.0f);
	vec4 c = pos+vec4(+bRadius,-bRadius,0.0f);
	vec4 d = pos+vec4(-bRadius,-bRadius,0.0f);



	gl_Position = projectionMatrix * a;
}