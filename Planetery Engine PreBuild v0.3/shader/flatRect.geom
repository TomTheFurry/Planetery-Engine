#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec2 aPos[];
in vec2 aSize[];

out vec2 texPos;

void main()
{
	texPos = vec2(0.,0.);
	gl_Position = vec4(aPos[0].xy,0.,1.);
	EmitVertex();
	texPos = vec2(0.,1.);
	gl_Position = vec4(aPos[0].x,aPos[0].y+aSize[0].y,0.,1.);
	EmitVertex();
	texPos = vec2(1.,0.);
	gl_Position = vec4(aPos[0].x+aSize[0].x,aPos[0].y,0.,1.);
	EmitVertex();
	texPos = vec2(1.,1.);
	gl_Position = vec4(aPos[0].xy+aSize[0].xy,0.,1.);
	EmitVertex();
}