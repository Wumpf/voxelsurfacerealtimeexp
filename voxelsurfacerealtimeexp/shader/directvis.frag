#version 430

#include "constantbuffers.glsl"
#include "helper.glsl"

layout(binding = 0) uniform sampler3D VolumeTexture;
const vec3 LightDirection = vec3(0, -0.333, 0.333);

// input
in vec2 vs_out_texcoord;

// output
layout(location = 0, index = 0) out vec4 ps_out_fragColor;

void main()
{
	vec3 rayDirection = ComputeRayDirection(vs_out_texcoord, InverseViewProjection);

	// extremly primitive raymarching
	
  vec3 boundsUpper = -(VolumeWorldSize - CameraPosition) / rayDirection;
  vec3 boundsLower = -(vec3(0.01,0.01,0.01) - CameraPosition) / rayDirection;
  vec3 boundsMax = max(boundsUpper, boundsLower);

  float end = min(max(max(boundsMax.x, boundsMax.y), boundsMax.z), 1000);
    
	for(float t=4; t<end; t*=1.005)
	{
		vec3 samplePos = CameraPosition + rayDirection * t;
		if(all(greaterThan(samplePos, vec3(0.01,0.01,0.01))) && 
		   all(lessThan(samplePos, VolumeWorldSize)) )
		{
			vec4 vol = textureLod(VolumeTexture, samplePos / VolumeWorldSize, 0);
			if(vol.w > IsoValue)
			{
				vec3 normal = normalize(vol.xyz * 2 - 1);
				normal = normalize(normal);
				float lighting = clamp(dot(normal, -LightDirection), 0, 1) + 0.3;
				ps_out_fragColor = vec4(lighting);

		//		ps_out_fragColor.xyz = abs(rayDirection);//abs(normal);
				return;
			}
		}
	}

	discard;
}
