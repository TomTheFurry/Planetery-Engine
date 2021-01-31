#version 460 core
in vec3 vecColor;
in vec2 texCoord;
in vec3 vecNormal;
in vec3 fragPos;
in vec3 lightPos;
out vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec3 lightColorA;
uniform vec3 lightColorD;
uniform vec3 lightColorS;
uniform float lightPower;


void main() {
	vec3 baseTexture = vec3(texture(texture0,texCoord));

	//Light Calculation
	float dist = distance(lightPos,fragPos);
	float power = pow((lightPower)/(lightPower+1),dist)/2;


	//Ambient Light
	vec3 ambientLight = lightColorA*(power);
	
	//Diffuse Light
	vec3 lightDir = normalize(lightPos-fragPos);
	vec4 normalTexture = texture(texture1,texCoord);
	vec3 diffuseColor = lightColorD*(max(dot(vecNormal, lightDir), 0.0) * power);

	//Specular Light
	float specularStrength = 2.0;
	vec3 viewDir = -normalize(fragPos);
	vec3 reflectDir = reflect(-lightDir, vecNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
	vec3 specular = specularStrength * spec * lightColorS * power;



	fragColor = vec4((ambientLight+diffuseColor + specular)*baseTexture,1.0);

}