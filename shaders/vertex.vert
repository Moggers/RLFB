#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNorm;
layout(location = 3) in vec2 uvcoords;
layout(location = 4) in float blendFactor;

layout(location = 5) in mat4 instanceRotation;
layout(location = 9) in vec3 instancePosition;
layout(location = 10) in vec3 instanceScale;
layout(location = 11) in uint instanceId;
layout(location = 12) in uint textureId1;
layout(location = 13) in uint textureId2;
layout(location = 14) in uint billboard;

layout(set = 0, binding = 0) uniform Block {
	mat4 mvp;
};

layout(set = 0, binding = 1) buffer Block {
  vec4 mouse;
	vec2 windowSize;
  uint mouseButtons;
};

layout(set = 0, binding = 2) buffer Block {
	vec4 worldMouse;
	int worldMouseDepth;
	uint selectionBufferLength;
  uint selectionMap[65536];
  uint selectionBuffer[256];
};

layout(location = 0) out vec4 fragColor;
layout(location = 1) out float lightAdjust;
layout(location = 2) out flat uint outInstanceId;
layout(location = 3) out flat uint outTextureId1;
layout(location = 4) out flat uint outTextureId2;
layout(location = 5) out vec2 outUV;
layout(location = 6) out float outBlendFactor;
layout(location = 7) out vec4 worldCoord;

void main() {
		worldCoord = ((instanceRotation * vec4(inPosition * instanceScale,1) + vec4( instancePosition,1)));
		if(billboard == 1) {
			gl_Position = mvp * worldCoord;
		} else {
			gl_Position = mvp * worldCoord;
		}
    fragColor = inColor;
		lightAdjust = abs(dot(normalize(vec3(instanceRotation * vec4(inNorm, 1))), normalize(vec3(0, 1, 1))));
		outInstanceId = instanceId;
		outTextureId1 = textureId1;
		outTextureId2 = textureId2;
		outBlendFactor = blendFactor;
		outUV = uvcoords;
}
