#version 430

#include "constantbuffers.glsl"

struct Vertex
{
	vec3 Normal;
};
layout(location = 0) in Vertex In;

layout(location = 0, index = 0) out vec4 ps_out_fragColor;

void main()
{
	vec3 normal = normalize(In.Normal);

	float lighting = clamp(dot(normal, GlobalDirLightDirection), 0, 1);
	ps_out_fragColor.xyz = GlobalDirLightColor * lighting + GlobalAmbient;
	ps_out_fragColor.a = 1.0f;
}
