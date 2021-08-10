#version 460 core
layout (location = 0) in vec2 texPosG;
layout (location = 1) flat in int debugFlag;
layout (location = 0) out vec4 fragColor;
layout (binding = 1) uniform fragUni {
	sampler2D fontMap;
	vec4 texColor;
}

void main() {
	if (debugFlag==1) {
		fragColor = vec4(1.0,0.0,0.0,1.0);
	} else if (debugFlag==2) {
		fragColor = vec4(0.0,1.0,0.0,1.0);
	} else if (debugFlag==3) {
		fragColor = vec4(0.0,0.0,1.0,1.0);
	} else {
		vec2 texSize = textureSize(fontMap,0);
		float s = texture(fontMap, texPosG/texSize).r;
		fragColor = texColor * vec4(1.0,1.0,1.0,s);
	}
}