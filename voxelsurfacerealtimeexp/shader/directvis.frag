#version 330

// uniforms
layout(shared) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	mat4 InverseViewProjection;
	vec3 CameraPosition;
};

uniform sampler3D VolumeTexture;
const float IsoValue = 0.5;
const vec3 textureSize = vec3(128, 32, 128);

const vec3 LightDirection = vec3(0, -0.333, 0.333);

// input
in vec2 vs_out_texcoord;

// output
out vec4 ps_out_fragColor;

void main()
{
	// "picking" - compute raydirection
	vec2 deviceCor = 2.0 * vs_out_texcoord - 1.0;
	vec4 rayOrigin = vec4(deviceCor, -1, 1) * InverseViewProjection;
	rayOrigin.xyz /= rayOrigin.w;
	vec4 rayTarget = vec4(deviceCor, 0, 1) * InverseViewProjection;
	rayTarget.xyz /= rayTarget.w;
	vec3 rayDirection = normalize(rayTarget.xyz - rayOrigin.xyz);

	// extremly primitive raymarching
	
	for(float t=0.005f; t<1000; t*=1.005)
	{
		vec3 samplePos = CameraPosition + rayDirection * (t+1);
		if(all(greaterThan(samplePos, vec3(0.01,0.01,0.01))) && 
		   all(lessThan(samplePos, textureSize)) )
		{
			vec4 vol = textureLod(VolumeTexture, samplePos / textureSize, 0);
			if(vol.w < IsoValue)
			{
				vec3 normal = normalize(vol.xyz * 2 - 1);
				normal = normalize(normal);
				float lighting = clamp(dot(normal, -LightDirection), 0, 1) + 0.3;
				ps_out_fragColor = vec4(lighting);
			//	ps_out_fragColor.xyz = abs(normal);
				return;
			}
		}
	}

	discard;
}
