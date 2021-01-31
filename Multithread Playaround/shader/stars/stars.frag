#version 460 core

in vec3 cColor;
in uint cId;
in float cRadius;
in vec3 fragPos;
in vec4 fp;
in vec3 centre;

float near = 0.0001;
float far = 1000.0;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float LinearizeDepth(float depth, float min, float max) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    z = ((2.0 * near * far) / (far + near - z * (far - near)))/far;
    return map(z,0.0,1.0,min,max);
}

void main() {
	float camDist = distance(vec3(0.0),centre);
	float distance = distance(centre,fragPos);
	if (distance>cRadius) discard;
	float p = (cRadius-distance)/cRadius;
	//float q = 1/pow(LinearizeDepth(gl_FragCoord.z,1.0,far/near),2);
	gl_FragColor = vec4(cColor*2.0*pow(p,2),p);
}