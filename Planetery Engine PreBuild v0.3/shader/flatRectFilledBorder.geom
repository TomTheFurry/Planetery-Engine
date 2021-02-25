#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 14) out;

in vec2 aPos[];
in vec2 aSize[];
out vec4 gColor;

uniform vec4 rectColor;
uniform vec4 rectBorderColor;
uniform vec2 rectBorderWidth;

void main()
{
	vec2 pos = aPos[0];
	vec2 size = aSize[0];
	vec2 w = rectBorderWidth;
	
	//Start rect
	gColor = rectBorderColor;
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

	gl_Position = vec4(pos.xy,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.xy+w.xy,0.,1.);
	EmitVertex();

	//Start fill, degenerative triangle on first one
	gColor = rectColor;
	gl_Position = vec4(pos.xy+w.xy,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.x+w.x,pos.y+size.y-w.y,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.x+size.x-w.x,pos.y+w.y,0.,1.);
	EmitVertex();
	gl_Position = vec4(pos.x+size.x-w.x,pos.y+size.y-w.y,0.,1.);
	EmitVertex();


}