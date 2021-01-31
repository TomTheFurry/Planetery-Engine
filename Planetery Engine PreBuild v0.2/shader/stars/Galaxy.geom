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
uniform float cutout;

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
	a = projectionMatrix * a;
	a.z = a.z * (1/cutout);
	gl_Position = a;
	EmitVertex();
	
	fragPos = vec3(b);
	b = projectionMatrix * b;
	b.z = b.z * (1/cutout);
	gl_Position = b;
	EmitVertex();
	
	fragPos = vec3(c);
	c = projectionMatrix * c;
	c.z = c.z * (1/cutout);
	gl_Position = c;
	EmitVertex();
	
	fragPos = vec3(d);
	d = projectionMatrix * d;
	d.z = d.z * (1/cutout);
	gl_Position = d;
	EmitVertex();


	EndPrimitive();
}