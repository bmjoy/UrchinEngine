#version 330
#extension GL_EXT_gpu_shader4 : enable

//deferred textures:
//|-------------------------------------------------------------------|
//| Depth                                                             |
//|-------------------------------------------------------------------|
//| Diffuse red    | Diffuse green  | Diffuse blue   | EMPTY          |
//|-------------------------------------------------------------------|
//| Normal X       | Normal Y       | Normal Z       | Ambient factor |
//|-------------------------------------------------------------------|
in vec2 textCoordinates;
uniform sampler2D depthTex;
uniform sampler2D colorTex;
uniform sampler2D normalAndAmbientTex;
uniform mat4 mInverseViewProjection;

//lights and shadows:
struct StructLightInfo{
	bool isExist;
	bool produceShadow;
	bool hasParallelBeams;
	vec3 positionOrDirection;
	
	float exponentialAttenuation;
	vec3 lightAmbient;
	
	sampler2DArray shadowMapTex;

	mat4 mLightProjectionView[#NUMBER_SHADOW_MAPS#];
};
uniform StructLightInfo lightsInfo[#MAX_LIGHTS#];
uniform float depthSplitDistance[#NUMBER_SHADOW_MAPS#];
uniform vec4 globalAmbient;

layout (location = #OUTPUT_LOCATION#) out vec4 fragColor;

float linearStep(float min, float max, float v){
  	return clamp((v - min) / (max - min), 0.0f, 1.0f);  
} 

float computePercentLit(float shadowMapZ, vec2 moments){
    float shadowMapZBiased = shadowMapZ - 0.000015f;
    float isInHardShadow = float(shadowMapZBiased <= moments.x);
    
    float variance = moments.y - (moments.x*moments.x);
    float d = moments.x - shadowMapZBiased;
    float pMax = variance / (variance + d*d);
    
    pMax = linearStep(0.4f, 1.0f, pMax); //reduce light bleeding
    
    return max(isInHardShadow, pMax);
}

float computeShadowContribution(int lightIndex, inout vec4 texPosition, inout vec4 position){
	float shadowContribution = 1.0;
	
	if(lightsInfo[lightIndex].produceShadow){
		#LOOP1_START(#NUMBER_SHADOW_MAPS#)#
			if(texPosition.z < depthSplitDistance[#LOOP1_COUNTER#]){
				vec4 shadowCoord = (((lightsInfo[lightIndex].mLightProjectionView[#LOOP1_COUNTER#] * position) / 2.0) + 0.5);

				if(shadowCoord.s<=1.0 && shadowCoord.s>=0.0 && shadowCoord.t<=1.0 && shadowCoord.t>=0.0){
					//model has produceShadow flag to true
					vec2 moments = texture2DArray(lightsInfo[lightIndex].shadowMapTex, vec3(shadowCoord.st, #LOOP1_COUNTER#)).rg;

					shadowContribution = computePercentLit(shadowCoord.z, moments);
					
					#LOOP1_IF_LAST_ITERATION#
						//shadow attenuation before disappear
						float startAttenuation = depthSplitDistance[#NUMBER_SHADOW_MAPS#-1] * 0.9996;
						shadowContribution = min(1.0f, shadowContribution 
								+ max(0.0f, ((texPosition.z - startAttenuation)/(depthSplitDistance[#NUMBER_SHADOW_MAPS#-1] - startAttenuation))));
					#LOOP1_ENDIF_LAST_ITERATION#
				}

				#LOOP1_STOP#
			}
		#LOOP1_END#
	}
	
	return shadowContribution;
}

void main(){	
	vec4 diffuse = texture2D(colorTex, textCoordinates);
	vec4 normalAndAmbient = vec4(texture2D(normalAndAmbientTex, textCoordinates));
	float modelAmbientFactor = normalAndAmbient.a;
	
	if(modelAmbientFactor >= 0.999f){ //no lighting
		fragColor = diffuse;
		return;
	}
	
	vec4 texPosition = vec4(
		textCoordinates.s * 2.0f - 1.0f,
		textCoordinates.t * 2.0f - 1.0f,
		texture2D(depthTex, textCoordinates).r * 2.0f - 1.0f,
		1.0
	);
	vec4 position = mInverseViewProjection * texPosition;
	position /= position.w;
	vec3 normal = vec3(normalAndAmbient) * 2.0f - 1.0f;
	vec4 modelAmbient = diffuse * modelAmbientFactor;
	
	fragColor = globalAmbient * modelAmbient;
		
	for(int i=0; i<#MAX_LIGHTS#;++i){
		if(lightsInfo[i].isExist){
			vec3 vertexToLightNormalized;
			float lightAttenuation;
				
			if(lightsInfo[i].hasParallelBeams){ //sun light
				vec3 vertexToLight = -lightsInfo[i].positionOrDirection;
				vertexToLightNormalized = normalize(vertexToLight);
				lightAttenuation = 1.0f;
			}else{ //omnidirectional light
				vec3 vertexToLight = lightsInfo[i].positionOrDirection - vec3(position);
				float dist = length(vertexToLight);
				vertexToLightNormalized = normalize(vertexToLight);
				lightAttenuation = exp(-dist * lightsInfo[i].exponentialAttenuation);
			}
			
			float NdotL = max(dot(normal, vertexToLightNormalized), 0.0f);
			vec4 ambient = vec4(lightsInfo[i].lightAmbient, 0.0f) * modelAmbient;
			
			float percentLit = computeShadowContribution(i, texPosition, position);
			
			fragColor += lightAttenuation * (percentLit * (diffuse * NdotL) + ambient);
		}
	}
}
