#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 bColor[];
in vec3 bPole[];
in uint bType[];
in float bRadius[];
in vec4 bPos[];

out vec3 cColor;
out vec3 cPole;
out uint cType;
out float cRadius;
out vec3 fragPos;
out vec4 fp;
out vec3 centre;

uniform mat4 projectionMatrix;

void main() {
	vec4 pos = bPos[0];
	float radius = bRadius[0];
	cRadius = radius;
	cColor = bColor[0];
	cPole = bPole[0];
	cType = bType[0];
	centre = vec3(pos);

	float offset = distance(projectionMatrix*pos,
		projectionMatrix*pos+vec4(radius,0.0f,0.0f,0.0f));
	// a,b
	// c,d
	vec4 a = pos+vec4(-radius, radius,0.0f,0.0f);
	vec4 b = pos+vec4( radius, radius,0.0f,0.0f);
	vec4 c = pos+vec4(-radius,-radius,0.0f,0.0f);
	vec4 d = pos+vec4( radius,-radius,0.0f,0.0f);

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