#version 450


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in flat uint instanceId;
layout(location = 3) in flat uint textureId;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) buffer Block {
	vec4 mouse;
	vec2 windowSize;
	uint mouseButtons;
	uint selectionBufferLength;
	uint frameId;
	uint selectionMap[4096];
	uint selectionBuffer[256];
};

layout(binding = 2) uniform sampler2D textures[512];

void main() {
	vec2 adjustedFrag = floor(vec2(gl_FragCoord));
	if((mouseButtons & 2) == 2) {
		float lowestX = mouse.x < mouse.z ? mouse.x : mouse.z;
		float lowestY = mouse.y < mouse.w ? mouse.y : mouse.w;
		float highestX = mouse.x >= mouse.z ? mouse.x : mouse.z;
		float highestY = mouse.y >= mouse.w ? mouse.y : mouse.w;

		if(lowestX <= adjustedFrag.x && adjustedFrag.x <= highestX && lowestY <= adjustedFrag.y && adjustedFrag.y <= highestY) {
			if(selectionMap[instanceId] != frameId) {
				uint beforeSelected = atomicExchange(selectionMap[instanceId], frameId);
				if(beforeSelected != frameId) {
					uint index = atomicAdd(selectionBufferLength, 1);
					selectionBuffer[selectionBufferLength] = instanceId;
				}
			}
		}
	}
	if(selectionMap[instanceId] != 0) {
		outColor = (vec4(dot(fragNorm, vec3(0,0.7,0.7)) * fragColor * 0.2 + vec3(0.1, 0.1, 0.4), 1.0)* texture(textures[0], vec2(0,0))  + 0.2 ) / 1.2 ;
	} else {
		outColor = (vec4(dot(fragNorm, vec3(0,0.7,0.7)) * fragColor, 1.0)* texture(textures[0], vec2(0,0))  + 0.2 ) / 1.2 ;
	}
}
