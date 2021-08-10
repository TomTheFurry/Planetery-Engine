#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout (binding = 0) uniform geomUni {
	vec2 emSize;
}

struct glGlyph
{
	//uint gId;
	uvec2 origin; //top left offset of bitmap texture
	uvec2 size; //size of bitmap texture
	vec2 emBearing; //how much left then how much top
	vec2 emSize; //width and height
};

layout(std430, binding = 2) readonly buffer fontSet
{
	uint identifier;
	uint size;
	glGlyph glyphs[];
};

layout (location = 0) in vec2 posV[];
layout (location = 1) in uint gIdV[];

layout (location = 0) out vec2 texPosG;
layout (location = 1) flat out int debugFlag;

void main() {
	vec2 pos = posV[0];
	debugFlag = 0;
	glGlyph g;

	if (identifier!=43 || gIdV[0]>=size) { //used for debug output
		uvec2 _debugOut = uvec2(50);
		if (identifier!=43) {
			_debugOut.y = 100;
			debugFlag = 1;
		} else if (gIdV[0]>=size) {
			_debugOut.x = size;
			debugFlag = 2;
		}
		g = glGlyph( uvec2(0), uvec2(0), vec2(0), vec2(10));
	} else {
		g = glyphs[gIdV[0]];
		if (g.origin==uvec2(0) && g.size==uvec2(0))  {
			g.size = uvec2(50);
			debugFlag = 3;
		} else {
			debugFlag = 0;
		}
	}

	uint texminx = g.origin.x;
	uint texmaxx = g.origin.x+g.size.x;
	uint texminy = g.origin.y;
	uint texmaxy = g.origin.y+g.size.y;
	vec2 upperLeft = pos + vec2(-g.emBearing.x, g.emBearing.y) * emSize;
	vec2 bottomRight = upperLeft + vec2(g.emSize.x, -g.emSize.y) * emSize;

	//2 4
	//1 3

	texPosG = vec2(texminx, texmaxy);
	gl_Position = vec4(upperLeft.x, bottomRight.y, 0.0, 1.0);
	EmitVertex();
	texPosG = vec2(texminx, texminy);
	gl_Position = vec4(upperLeft.x, upperLeft.y, 0.0, 1.0);
	EmitVertex();
	texPosG = vec2(texmaxx, texmaxy);
	gl_Position = vec4(bottomRight.x, bottomRight.y, 0.0, 1.0);
	EmitVertex();
	texPosG = vec2(texmaxx, texminy);
	gl_Position = vec4(bottomRight.x, upperLeft.y, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}