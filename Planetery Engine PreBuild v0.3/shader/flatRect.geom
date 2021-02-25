#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec2 aPos[];
in vec2 aSize[];

void main()
{
	gl_Position = vec4(aPos[0].xy,0.,1.);
	EmitVertex();
	gl_Position = vec4(aPos[0].x,aPos[0].y+aSize[0].y,0.,1.);
	EmitVertex();
	gl_Position = vec4(aPos[0].x+aSize[0].x,aPos[0].y,0.,1.);
	EmitVertex();
	gl_Position = vec4(aPos[0].xy+aSize[0].xy,0.,1.);
	EmitVertex();
}