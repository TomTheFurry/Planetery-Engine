#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform vec2 scale;

struct glGlyph
{
	uvec2 origin; //top left offset of bitmap texture
	uvec2 size; //size of bitmap texture
	ivec2 bearing; //how much right than up
};

layout(std430) buffer fontSet
{
	uint identifier;
	uint size;
	glGlyph glyphs[];
};

in vec2 posV[];
in uint gIdV[];

out vec2 texPosG;
flat out int debugFlag;

void main() {
	vec2 pos = posV[0];
	debugFlag = 0;
	glGlyph g;

	if (identifier!=42 || gIdV[0]>=size) { //used for debug output
		uvec2 _debugOut = uvec2(50);
		if (identifier!=42) {
			_debugOut.y = 100;
			debugFlag = 1;
		} else if (gIdV[0]>=size) {
			_debugOut.x = size;
			debugFlag = 2;
		}
		g = glGlyph(uvec2(0), _debugOut, ivec2(0));
	} else {
		g = glyphs[gIdV[0]];
		debugFlag = 0;
	}

	int minx = g.bearing.x;
	int maxx = g.bearing.x+int(g.size.x);
	int miny = g.bearing.y-int(g.size.y);
	int maxy = g.bearing.y;

	uint texminx = g.origin.x;
	uint texmaxx = g.origin.x + g.size.x;
	uint texminy = g.origin.y;
	uint texmaxy = g.origin.y + g.size.y;

	//2 4
	//1 3
	//ivec4 rect2 = ivec4(-1,-1,1,1);


	texPosG = vec2(texminx, texmaxy);
	gl_Position = vec4(pos + vec2(minx,miny) * scale, 0.0, 1.0);
	EmitVertex();
	texPosG = vec2(texminx, texminy);
	gl_Position = vec4(pos + vec2(minx,maxy) * scale, 0.0, 1.0);
	EmitVertex();
	texPosG = vec2(texmaxx, texmaxy);
	gl_Position = vec4(pos + vec2(maxx,miny) * scale, 0.0, 1.0);
	EmitVertex();
	texPosG = vec2(texmaxx, texminy);
	gl_Position = vec4(pos + vec2(maxx,maxy) * scale, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}