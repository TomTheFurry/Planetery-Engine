#version 460 core
#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536
in vec2 bPos;

out vec4 fragColor;

uniform sampler2D screenTex;
uniform ivec2 windowSize;
uniform float brightness;



float gaussianFunction(float x, float m, float d) {
	return (1.0 / (d * sqrt(2.0 * M_PI))) * pow(M_E, -0.5 * pow((x - m) / d, 2.0));
}


void main() {

	vec2 pos = bPos/2.0 +vec2(0.5);
	vec3 sumColor = vec3(0.0);
	//fragColor = vec4(pos.xy,1.0,0.5);
	ivec2 ipos = ivec2(pos*vec2(windowSize));
	ivec2 lowLim = ipos - ivec2(50);
	ivec2 highLim = ipos + ivec2(50);
	if (lowLim.x<=0) lowLim.x = 1;
	if (lowLim.y<=0) lowLim.y = 1;
	if (highLim.x>windowSize.x) highLim.x = windowSize.x;
	if (highLim.y>windowSize.y) highLim.y = windowSize.y;


	int x,y;
	y = int(pos.y*float(windowSize.y));
	for (x = lowLim.x; x<highLim.x; x++) {
		ivec2 ipos = ivec2(x,y);
		float dist = distance(ipos, ivec2(pos*vec2(windowSize)));
		sumColor = sumColor + (texture(screenTex, vec2(ipos)/vec2(windowSize)).xyz - brightness) * gaussianFunction(dist, 0.0, 5)*3;
	}

	x = int(pos.x*float(windowSize.x));
	for (int y = lowLim.y; y<highLim.y; y++) {
		ivec2 ipos = ivec2(x,y);
		float dist = distance(ipos, ivec2(pos*vec2(windowSize)));
		sumColor = sumColor + (texture(screenTex, vec2(ipos)/vec2(windowSize)).xyz - brightness) * gaussianFunction(dist, 0.0, 5)*3;
	}

	fragColor = vec4(sumColor, 1.0);
}