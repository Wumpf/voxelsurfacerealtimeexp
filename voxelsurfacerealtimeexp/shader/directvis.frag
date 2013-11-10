#version 430

#include "constantbuffers.glsl"
#include "helper.glsl"
#include "sparseVoxelOctree.glsl"

// input
in vec2 vs_out_texcoord;


// output
layout(location = 0, index = 0) out vec4 ps_out_fragColor;


void main()
{
	vec3 rayDirection = ComputeRayDirection(vs_out_texcoord, InverseViewProjection);

	// extremly primitive raymarching
	float t0, t1;
	if(!rayBoxIntersection(CameraPosition, rayDirection, vec3(0), vec3(VolumeWorldSize), t0, t1))
		discard;

	float start = max(5, min(t0, t1));
	float end = max(t0, t1);

    float nextStep;
	for(float t=start; t<end; t+=nextStep)
	{
		nextStep = t * 0.01f; // default step - this is min!

		vec3 samplePos = CameraPosition + rayDirection * t;
		vec3 interpFactor = fract(samplePos);


		uvec3 samplePosUInt = uvec3(samplePos);

		float density000 = SampleOctree(samplePosUInt + uvec3(0, 0, 0));
		float density100 = SampleOctree(samplePosUInt + uvec3(1, 0, 0));
		float density010 = SampleOctree(samplePosUInt + uvec3(0, 1, 0));
		float density110 = SampleOctree(samplePosUInt + uvec3(1, 1, 0));
		float density001 = SampleOctree(samplePosUInt + uvec3(0, 0, 1));
		float density101 = SampleOctree(samplePosUInt + uvec3(1, 0, 1));
		float density011 = SampleOctree(samplePosUInt + uvec3(0, 1, 1));
		float density111 = SampleOctree(samplePosUInt + uvec3(1, 1, 1));

		float density = mix(mix(mix(density000, density100, interpFactor.x), mix(density010, density110, interpFactor.x), interpFactor.y),
						    mix(mix(density001, density101, interpFactor.x), mix(density011, density111, interpFactor.x), interpFactor.y), interpFactor.z);

	//	vec4 vol = textureLod(VolumeTexture, samplePos / VolumeWorldSize, 0);
		if(density > 0.0f)
		{
			// compute normal by weighting the 8 
		/*	vec3 invertInterpFactor = vec3(1.0f) - interpFactor;
			vec3 normal;
			normal.x  = (density100 - density000) * max(invertInterpFactor.y, invertInterpFactor.z);
			normal.x += (density110 - density010) * max(      interpFactor.y, invertInterpFactor.z);
			normal.x += (density101 - density001) * max(invertInterpFactor.y,       interpFactor.z);
			normal.x += (density111 - density011) * max(      interpFactor.y,       interpFactor.z);

			normal.y  = (density010 - density000) * max(invertInterpFactor.x, invertInterpFactor.z);
			normal.y += (density110 - density100) * max(      interpFactor.x, invertInterpFactor.z);
			normal.y += (density011 - density001) * max(invertInterpFactor.x,       interpFactor.z);
			normal.y += (density111 - density101) * max(      interpFactor.x,       interpFactor.z);

			normal.z  = (density001 - density000) * max(invertInterpFactor.x, invertInterpFactor.y);
			normal.z += (density101 - density100) * max(      interpFactor.x, invertInterpFactor.y);
			normal.z += (density011 - density010) * max(invertInterpFactor.x,       interpFactor.y);
			normal.z += (density111 - density110) * max(      interpFactor.x,       interpFactor.y);


			normal = normalize(normal);
			float lighting = clamp(dot(normal, GlobalDirLightDirection), 0, 1);
			ps_out_fragColor.xyz = GlobalDirLightColor * lighting + GlobalAmbient;
			ps_out_fragColor.a = 1.0f;

	//		ps_out_fragColor.xyz = abs(rayDirection);//abs(normal);
*/

			// initial cell coloring
			uint currentBoxSize = InitialNodeSize;
			uvec3 initalNodePos = samplePosUInt / InitialNodeSize;
			uint initialNodeIndex = (initalNodePos.x + (initalNodePos.y + initalNodePos.z * NumInitialNodesPerAxis) * NumInitialNodesPerAxis);
			ps_out_fragColor.xyz = vec3(initalNodePos) / NumInitialNodesPerAxis;

			return;
		}


		/*else if(lastBoxSize > nextStep * 4)
		{
			vec3 boxMin = (uvec3(samplePos) / uvec3(lastBoxSize)) + vec3(lastBoxSize);
			vec3 boxMax = boxMin + vec3(lastBoxSize);
			rayBoxIntersection(samplePos, rayDirection, boxMin, boxMax, t0, t1);

			nextStep = max(min(t0, t1), nextStep);
		}*/
	}

	discard;
}
