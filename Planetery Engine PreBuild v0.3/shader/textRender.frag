#version 460 core
in vec2 texPosG;
flat in int debugFlag;
out vec4 fragColor;
uniform sampler2D fontMap;
uniform vec4 texColor;

void main() {
	if (debugFlag==1) {
		fragColor = vec4(1.0,0.0,0.0,1.0);
	} else if (debugFlag==2) {
		fragColor = vec4(0.0,1.0,0.0,1.0);
	} else {
		vec2 texSize = textureSize(fontMap,0);
		float s = texture(fontMap, texPosG/texSize).r;
		fragColor = texColor * vec4(1.0,1.0,1.0,s);
	}
}