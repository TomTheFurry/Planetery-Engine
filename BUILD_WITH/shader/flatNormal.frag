#version 460 core
in vec3 bColor;
in vec3 viewPos;
out vec4 fragColor;
uniform mat4 colorMatrix;

void main() {

	//Ambient Light
	vec3 ambientLight = vec3(0.05);
	
	//Diffuse Light
	vec3 lightDir = normalize(vec3(1.0,2.0,3.0));
    vec3 xTangent = dFdx( viewPos );
    vec3 yTangent = dFdy( viewPos );
    vec3 faceNormal = normalize( cross( xTangent, yTangent ) );
	vec3 diffuseColor = vec3(1.0)*(max(dot(faceNormal, lightDir), 0.0) * 0.9);

	fragColor = vec4((ambientLight+diffuseColor)*bColor,1.0);

}