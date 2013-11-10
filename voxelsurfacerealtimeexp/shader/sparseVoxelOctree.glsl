layout(binding = 0) readonly buffer SparseVoxelOctree_type
{
	uint Data[];
} SparseVoxelOctree;


float SampleOctree(uvec3 volumePosition, uint smallestBoxSize = 0)
{
	// Read inital node
	uint currentBoxSize = InitialNodeSize;
	uvec3 initalNodePos = volumePosition / currentBoxSize;
	uint initialNodeIndex = (initalNodePos.x + (initalNodePos.y + initalNodePos.z * NumInitialNodesPerAxis) * NumInitialNodesPerAxis);
	uint currentNodeAdress = initialNodeIndex * 8;

	while(true)
	{
		vec3 inBoxCoord = mod(volumePosition, vec3(currentBoxSize));

		// Next box is half the size!
		currentBoxSize /= uint(2);

		// In which subbox is volumePosition? True means positive, false negative.
		uvec3 subBoxCord = uvec3(greaterThanEqual(inBoxCoord, uvec3(currentBoxSize)));
		// Compute to which node to jump
		currentNodeAdress += subBoxCord.x + subBoxCord.y * 2 + subBoxCord.z * 4;

		// Read current node data and decide what to do next.
		uint currentData = SparseVoxelOctree.Data[currentNodeAdress];
		// The end?
		if(currentBoxSize <= 1)
			return uintBitsToFloat(currentData);
	//	else if (currentBoxSize <= smallestBoxSize)
	//		return -1.0f;
		else
		{
			// Nothing in it?
			if(currentData == 0)
				return -1.0f;
			// Filled?
			if(currentData == uint(0xFFFFFFFF))
				return 1.0f;
			// It's a new adress! Jump!
			else
				currentNodeAdress = currentData;
		}
	}
}