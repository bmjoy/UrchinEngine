#version 330
#extension GL_EXT_gpu_shader4 : enable

#define NEAR_PLANE 0
#define FAR_PLANE 1

in vec2 textCoordinates;

uniform sampler2DArray colorTex;
uniform float colorIntensity;
uniform float cameraPlanes[2];
uniform int layer;

out vec4 fragColor;

float linearizeDepth(float depthValue){
	float unmapDepthValue = depthValue * 2.0 - 1.0;
	return (2.0f * cameraPlanes[NEAR_PLANE]) / (cameraPlanes[FAR_PLANE] + cameraPlanes[NEAR_PLANE] - 
			unmapDepthValue * (cameraPlanes[FAR_PLANE] - cameraPlanes[NEAR_PLANE])); //[0.0=nearPlane, 1.0=far plane]
}

void main(){
	vec4 diffuse = texture2DArray(colorTex, vec3(textCoordinates, layer));

	vec4 finalDiffuse;
	if(#IS_DEFAULT_VALUE#){
		finalDiffuse = diffuse * colorIntensity;
	}else if (#IS_DEPTH_VALUE#){
		float linearizedDepthValue = linearizeDepth(diffuse.x) * colorIntensity;
		finalDiffuse = vec4(linearizedDepthValue, linearizedDepthValue, linearizedDepthValue, diffuse.w);
	}else if(#IS_GRAYSCALE_VALUE#){
		float value = diffuse.x * colorIntensity;
		finalDiffuse = vec4(value, value, value, diffuse.w);
	}else if(#IS_INVERSE_GRAYSCALE_VALUE#){
		float value = 1.0f - (diffuse.x * colorIntensity);
		finalDiffuse = vec4(value, value, value, diffuse.w);
	}
	
	fragColor = finalDiffuse;
}