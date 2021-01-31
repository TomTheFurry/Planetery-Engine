#version 460 core
in vec3 ourColor;
in vec2 texCoord;
out vec4 fragColor;
uniform vec3 uniColor;
uniform sampler2D texture0;
uniform sampler2D texture1;


void main() {
	//fragColor = vec4(ourColor.x+uniColor.x,ourColor.y+uniColor.y,ourColor.z+uniColor.z,uniColor.a);
	fragColor = texture(texture0,texCoord);
	fragColor = fragColor + (texture(texture1,texCoord)*texture(texture1,texCoord).a);
}