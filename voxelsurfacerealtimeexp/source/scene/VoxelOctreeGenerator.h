#pragma once

#include <functional  >

class VoxelOctreeGenerator
{
public:
  VoxelOctreeGenerator(ezUInt32 uiVolumeSize);

  /// \brief Creates raw volume data from 3d noise.
  /// 
  /// Output allocated with EZ_DEFAULT_NEW_RAW_BUFFER
  float* GenerateRawVolumeFromNoise();


  ezDynamicArray<ezUInt32>& CreateVoxelOctreeFromRawVolume(const std::function<float(ezVec3U32 vSamplingPos)>& samplingFunc);
 
private:
  ezUInt32 ComputeDataEntryRec(const ezVec3U32& vMin, const ezVec3U32& vMax, const std::function<float(ezVec3U32 vSamplingPos)>& samplingFunc);
  
  void ComputeSubdivionAreas(const ezVec3U32& vMin, const ezVec3U32& vMax, ezVec3U32 (&outMins)[8], ezVec3U32 (&outMaxs)[8]);

  ezDynamicArray<ezUInt32> m_sparseVoxelData;

  const ezUInt32 m_volumeSize;
};

