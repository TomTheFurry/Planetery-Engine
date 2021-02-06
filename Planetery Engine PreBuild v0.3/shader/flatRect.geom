#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec2 aPos[];
in vec2 aSize[];

out vec2 texPos;

void main()
{
	texPos = vec2(-1.,-1.);
	gl_Position = aPos[0];
	EmitVertex();
	texPos = vec2(-1.,1.);
	gl_Position = vec2(aPos[0].x,aPos[0].y+aSize[0].y);
	EmitVertex();
	texPos = vec2(1.,-1.);
	gl_Position = vec2(aPos[0].x+aSize[0].x,aPos[0].y);
	EmitVertex();
	texPos = vec2(-1.,-1.);
	gl_Position = aPos[0]+aSize[0];
	EmitVertex();
}