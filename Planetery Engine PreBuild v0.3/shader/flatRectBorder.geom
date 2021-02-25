#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 10) out;

in vec2 aPos[];
in vec2 aSize[];

uniform vec2 rectBorderWidth;

void main()
{
	vec2 pos = aPos[0];
	vec2 size = aSize[0];
	vec2 w = rectBorderWidth;
	
	gl_Position = vec4(pos.xy,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.xy+w.xy,0.,1.);
	EmitVertex();

	gl_Position = vec4(pos.x,pos.y+size.y,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.x+w.x,pos.y+size.y-w.y,0.,1.);
	EmitVertex();

	gl_Position = vec4(pos.x+size.x,pos.y+size.y,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.x+size.x-w.x,pos.y+size.y-w.y,0.,1.);
	EmitVertex();

	gl_Position = vec4(pos.x+size.x,pos.y,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.x+size.x-w.x,pos.y+w.y,0.,1.);
	EmitVertex();

	//End rect
	gl_Position = vec4(pos.xy,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.xy+w.xy,0.,1.);
	EmitVertex();
}