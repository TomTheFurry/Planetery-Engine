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
	vec3 pole = bPole[0];
	cRadius = radius;
	cColor = bColor[0];
	cType = bType[0];
	centre = vec3(pos);
	if (cRadius != 0.0f) {
		float offset = distance(projectionMatrix*pos,
			projectionMatrix*pos+vec4(radius,0.0f,0.0f,0.0f));

		vec3 crossA = normalize(cross(pole, vec3(0.0f,0.0f,-1.0f)));
		if (crossA.x + crossA.y + crossA.z == 0.0f) {
			crossA = normalize(cross(pole, vec3(-1.0f,0.0f,0.0f)));
		}
		vec3 crossB = normalize(cross(pole, crossA));
		cPole = pole;

		//float ax = dot(vec3(0.a0f,0.0f,1.0f)); //TODO);
		//float ay = dot(vec3(0.0f,0.0f,1.0f));

		//vec4 a = pos+vec4(-radius, radius,0.0f,0.0f);
		//vec4 b = pos+vec4( radius, radius,0.0f,0.0f);
		//vec4 c = pos+vec4(-radius,-radius,0.0f,0.0f);
		//vec4 d = pos+vec4( radius,-radius,0.0f,0.0f);

		crossA = crossA * radius;
		crossB = crossB * radius;

		vec4 a = pos+vec4((crossB-crossA),0.0f);
		vec4 b = pos+vec4((crossA+crossB),0.0f);
		vec4 c = pos+vec4((vec3(0.0f)-crossA-crossB),0.0f);
		vec4 d = pos+vec4((crossA-crossB),0.0f);

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
}