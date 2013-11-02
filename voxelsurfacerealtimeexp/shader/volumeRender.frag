#version 430

#include "constantbuffers.glsl"

layout(binding = 1) uniform sampler2D TextureY;
layout(binding = 2) uniform sampler2D TextureXZ;

struct Vertex
{
	vec3 Normal;
	vec3 WorldPos;
};
layout(location = 0) in Vertex In;

layout(location = 0, index = 0) out vec4 ps_out_fragColor;

void main()
{
	vec3 normal = normalize(In.Normal);

	float lighting = clamp(dot(normal, GlobalDirLightDirection), 0, 1);
	
	// good old backlighting hack!
	vec3 backLightDir = GlobalDirLightDirection;
	backLightDir.xz = -backLightDir.xz;
	float ambientLightAmount = clamp(dot(normal, backLightDir), 0, 1) + 0.4f; // currently ambient is meant for adding - so we have to scale this a bit


	// texturing
	vec3 texcoord3D = In.WorldPos*0.1f;
	vec3 textureWeights = abs(normal);
	textureWeights.y *= 2;
	textureWeights /= textureWeights.x + textureWeights.y + textureWeights.z;
	
	vec3 textureY = texture(TextureY, texcoord3D.xz).xyz;
	vec3 textureZ = texture(TextureXZ, texcoord3D.xy).xyz;
	vec3 textureX = texture(TextureXZ, texcoord3D.zy).xyz;

	vec3 diffuseColor = textureX * textureWeights.x + textureY * textureWeights.y + textureZ * textureWeights.z;
	


	ps_out_fragColor.xyz = diffuseColor * (GlobalDirLightColor * lighting + GlobalAmbient * ambientLightAmount);
	ps_out_fragColor.a = 1.0f;
}
