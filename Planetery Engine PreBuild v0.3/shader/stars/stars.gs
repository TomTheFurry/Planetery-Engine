#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 bColor[];
in float bId[];
in float bRadius[];
in vec4 bPos[];

out vec3 cColor;
out float cId;
out float cRadius;
out vec3 fragPos;
out vec3 centre;

uniform mat4 projectionMatrix;

void main() {
	vec4 pos = bPos[0];
	cRadius = bRadius[0];
	float radius = cRadius;
	cColor = bColor[0];
	cId = bId[0];
	centre = vec3(pos);

	// a,b
	// c,d
	vec4 a = pos+vec4(-radius, radius,0.0f,0.0f);
	vec4 b = pos+vec4( radius, radius,0.0f,0.0f);
	vec4 c = pos+vec4( radius,-radius,0.0f,0.0f);
	vec4 d = pos+vec4(-radius,-radius,0.0f,0.0f);

	fragPos = vec3(a);
	gl_Position = projectionMatrix * a;
	EmitVertex();
	fragPos = vec3(b);
	gl_Position = projectionMatrix * b;
	EmitVertex();
	fragPos = vec3(c);
	gl_Position = projectionMatrix * c;
	EmitVertex();
	fragPos = vec3(d);
	gl_Position = projectionMatrix * d;
	EmitVertex();

	EndPrimitive();
}