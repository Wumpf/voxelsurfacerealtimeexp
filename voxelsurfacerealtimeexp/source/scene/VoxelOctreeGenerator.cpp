#include "PCH.h"
#include "VoxelOctreeGenerator.h"
#include "math/NoiseGenerator.h"

VoxelOctreeGenerator::VoxelOctreeGenerator(ezUInt32 uiVolumeSize) :
  m_volumeSize(uiVolumeSize),
  // Now the Octree itself will be generated.
  // To reduce unnecessary traversal steps, there is an initial depth, that determines how many start nodes there are.
  m_initialDepth(4), // 3 means: 16x16x16 = 4096 start nodes -> 8*4 byte per node = kb "inital table"
  m_numInitialNodes(ezMath::Pow(8, m_initialDepth)),

  m_numInitialNodesPerAxis(ezMath::Pow(2, m_initialDepth)),
  m_numInitialNodesPerSlice(m_numInitialNodesPerAxis * m_numInitialNodesPerAxis),

  m_initialNodeSize(m_volumeSize / m_numInitialNodesPerAxis),
  m_maxDepth(static_cast<ezUInt32>(ezMath::Log2(static_cast<double>(m_volumeSize))))
{
    EZ_ASSERT(m_maxDepth > m_initialDepth, "Volume is too small!");
}


float* VoxelOctreeGenerator::GenerateRawVolumeFromNoise()
{
  float* pVolumeData = EZ_DEFAULT_NEW_RAW_BUFFER(float, m_volumeSize * m_volumeSize * m_volumeSize);
  NoiseGenerator noiseGen;
  ezUInt32 slicePitch = m_volumeSize * m_volumeSize;
  float mulitplier = 1.0f / static_cast<float>(m_volumeSize - 1);

#pragma omp parallel for // OpenMP parallel for loop.
  for(ezInt32 z=0; z<static_cast<ezInt32>(m_volumeSize); ++z) // Needs to be signed for OpenMP.
  {
    for(ezUInt32 y=0; y<m_volumeSize; ++y)
    {
      for(ezUInt32 x=0; x<m_volumeSize; ++x)
      {
        ezVec3 gradient;
        pVolumeData[x + y * m_volumeSize + z * slicePitch] = noiseGen.GetValueNoise(ezVec3(mulitplier*x, mulitplier*y, mulitplier*z), 0, 5, 0.8f, false, NULL);
      }
    }
  }

  return pVolumeData;
}

ezUInt32 VoxelOctreeGenerator::ComputeDataEntryRec(const ezVec3U32& vMin, const ezVec3U32& vMax, const std::function<float(ezVec3U32 vSamplingPos)>& samplingFunc)
{
  // Areas are always square, so check only one delta.
  // Data ending?
  if(vMax.x - vMin.x == 1)
  {
    float value = samplingFunc(vMin);
    return *reinterpret_cast<ezUInt32*>(&value);
  }

  // Check if there's any data, all empty or all filled!
  bool anyFilled = false;
  bool anyEmpty = false;
  ezVec3U32 samplingPos;
  for(samplingPos.z=vMin.z; samplingPos.z<vMax.z; ++samplingPos.z)
  {
    for(samplingPos.y=vMin.y; samplingPos.y<vMax.y; ++samplingPos.y)
    {
      for(samplingPos.x=vMin.x; samplingPos.x<vMax.x; ++samplingPos.x)
      {
        if(samplingFunc(samplingPos) < 0.0f)
          anyEmpty = true;
        else
          anyFilled = true;

        if(anyEmpty == anyFilled)
          samplingPos = vMax; // break
      }
    }
  }

  // Subdivide!
  if(anyEmpty == anyFilled)
  {
    ezVec3U32 vBoxMins[8];
    ezVec3U32 vBoxMaxs[8];
    ComputeSubdivionAreas(vMin, vMax, vBoxMins, vBoxMaxs);

    // The 8 data packages need to lie consistent in memory!
    ezUInt32 dataBlockStartPosition = m_sparseVoxelData.GetCount();
    m_sparseVoxelData.SetCount(dataBlockStartPosition + 8);
    for(ezUInt32 i=0; i<8; ++i)
      m_sparseVoxelData[dataBlockStartPosition + i] = ComputeDataEntryRec(vBoxMins[i], vBoxMaxs[i], samplingFunc);

    return dataBlockStartPosition; 
  }

  // Nothing? Return 0.
  else if(anyEmpty)
    return 0;

  // Completely filled? Return max.
  else
    return std::numeric_limits<ezUInt32>::max();
}

void VoxelOctreeGenerator::ComputeSubdivionAreas(const ezVec3U32& vMin, const ezVec3U32& vMax, ezVec3U32 (&outMins)[8], ezVec3U32 (&outMaxs)[8])
{
  ezVec3U32 vCenter = vMin + vMax;
  vCenter.x /= 2;
  vCenter.y /= 2;
  vCenter.z /= 2;

  // this order is important for general orientation!
  
  // X- Y- Z-
  outMins[0] = ezVec3U32(   vMin.x,    vMin.y,    vMin.z);
  outMaxs[0] = ezVec3U32(vCenter.x, vCenter.y, vCenter.z);
  // X+ Y- Z-
  outMins[1] = ezVec3U32(vCenter.x,    vMin.y,    vMin.z);
  outMaxs[1] = ezVec3U32(   vMax.x, vCenter.y, vCenter.z);
  // X- Y+ Z-
  outMins[2] = ezVec3U32(   vMin.x, vCenter.y,    vMin.z);
  outMaxs[2] = ezVec3U32(vCenter.x,    vMax.y, vCenter.z);
  // X+ Y+ Z-
  outMins[3] = ezVec3U32(vCenter.x, vCenter.y,    vMin.z);
  outMaxs[3] = ezVec3U32(   vMax.x,    vMax.y, vCenter.z);

  // X- Y- Z+
  outMins[4] = ezVec3U32(   vMin.x,    vMin.y, vCenter.z);
  outMaxs[4] = ezVec3U32(vCenter.x, vCenter.y,    vMax.z);
  // X+ Y- Z+
  outMins[5] = ezVec3U32(vCenter.x,    vMin.y, vCenter.z);
  outMaxs[5] = ezVec3U32(   vMax.x, vCenter.y,    vMax.z);
  // X- Y+ Z+
  outMins[6] = ezVec3U32(   vMin.x, vCenter.y, vCenter.z);
  outMaxs[6] = ezVec3U32(vCenter.x,    vMax.y,    vMax.z);
  // X+ Y+ Z+
  outMins[7] = ezVec3U32(vCenter.x, vCenter.y, vCenter.z);
  outMaxs[7] = ezVec3U32(   vMax.x,    vMax.y,    vMax.z);

}

ezDynamicArray<ezUInt32>& VoxelOctreeGenerator::CreateVoxelOctreeFromRawVolume(const std::function<float(ezVec3U32 vSamplingPos)>& samplingFunc)
{
  // each node consists of 8 data packages (32bit) in a row
  m_sparseVoxelData.Clear();
  m_sparseVoxelData.Reserve(m_volumeSize * m_volumeSize * 3);
  // reserve data for initial nodes
  m_sparseVoxelData.SetCount(m_numInitialNodes * 8);


  // Start node should be indexed with: startNode = (x + (y + z * numInitialNodesPerAxis) * numInitialNodesPerAxis)
  // Keep in mind that every node consists of 8 "data packages" (either address, empty, full or data)

  for(ezUInt32 startNode=0; startNode < m_numInitialNodes; ++ startNode)
  {
    ezVec3U32 volumeMin;
    volumeMin.z = startNode / m_numInitialNodesPerSlice,
    volumeMin.y = (startNode - volumeMin.z * m_numInitialNodesPerSlice)  / m_numInitialNodesPerAxis,
    volumeMin.x = startNode % m_numInitialNodesPerAxis;
    volumeMin *= m_initialNodeSize;

    ezVec3U32 vBoxMins[8];
    ezVec3U32 vBoxMaxs[8];
    ComputeSubdivionAreas(volumeMin, volumeMin + ezVec3U32(m_initialNodeSize), vBoxMins, vBoxMaxs);
    for(ezUInt32 i=0; i<8; ++i)
      m_sparseVoxelData[startNode * 8 + i] = ComputeDataEntryRec(vBoxMins[i], vBoxMaxs[i], samplingFunc);
  }

  return m_sparseVoxelData;
}