#version 450


layout(location = 0) in vec4 fragColor;
layout(location = 1) in float lightAdjust;
layout(location = 2) in flat uint instanceId;
layout(location = 3) in flat uint textureId1;
layout(location = 4) in flat uint textureId2;
layout(location = 5) in vec2 uvcoords;
layout(location = 6) in float blendFactor;
layout(location = 7) in vec4 worldCoord;

layout(location = 0) out vec4 outColor;
layout(binding = 1) buffer InputIn {
	vec4 mouse;
	vec2 windowSize;
	uint mouseButtons;
};
layout(binding = 2) buffer InputOut {
	vec4 worldMouse;
	int worldMouseDepth;
	uint selectionBufferLength;
	uint selectionMap[65536];
	uint selectionBuffer[256];
};
layout(binding = 3) uniform sampler2D textures[16];

void main() {
	vec2 adjustedFrag = floor(vec2(gl_FragCoord));
	if( mouse.x == adjustedFrag.x && mouse.y == adjustedFrag.y) {
		int oldDepth = atomicMax(worldMouseDepth, int((1-gl_FragCoord.z) * 1000000));
		if(oldDepth <= gl_FragCoord.z) {
			worldMouse[0] = worldCoord[0];
			worldMouse[1] = worldCoord[1];
			worldMouse[2] = worldCoord[2];
		}
	}
	if((mouseButtons & 2) == 2) {
		float lowestX = mouse.x < mouse.z ? mouse.x : mouse.z;
		float lowestY = mouse.y < mouse.w ? mouse.y : mouse.w;
		float highestX = mouse.x >= mouse.z ? mouse.x : mouse.z;
		float highestY = mouse.y >= mouse.w ? mouse.y : mouse.w;

		if(lowestX <= adjustedFrag.x && adjustedFrag.x <= highestX && lowestY <= adjustedFrag.y && adjustedFrag.y <= highestY) {
			uint old = atomicExchange(selectionMap[instanceId], 1);
			if(old == 0) {
				uint index = atomicAdd(selectionBufferLength, 1);
				selectionBuffer[selectionBufferLength] = instanceId;
			}
		}
	}
	vec4 blendedTexture = texture(textures[textureId1], uvcoords) * blendFactor + texture(textures[textureId2], uvcoords) * (1-blendFactor);
	vec4 selectMask;
	if(selectionMap[instanceId] != 0) {
		selectMask = vec4(1,1,1,1);
	} else {
		selectMask = vec4(1,1,1,1);
	}
	outColor = fragColor * blendedTexture * vec4(vec3(lightAdjust), 1.0) * selectMask;
	if(outColor.a == 0) {
		discard;
	}
}
