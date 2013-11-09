#pragma once

#include "gl/ShaderObject.h"
#include "gl/resources/UniformBuffer.h"

namespace gl
{
  class ScreenAlignedTriangle;
  class Texture2D;
};

class VoxelTerrain
{
public:
  VoxelTerrain(const std::shared_ptr<const gl::ScreenAlignedTriangle>& pScreenAlignedTriangle);
  ~VoxelTerrain();
  
  // you should put as much stuff between ComputeGeometryInfo and Draw as possible!
  void ComputeGeometryInfo();
  void Draw();
  void DrawReferenceRaycast();

  ezVec3 GetWorldSize() { return ezVec3(static_cast<float>(m_volumeSize)); }

  const gl::ShaderObject& GetShaderDirectVolVis() { return m_DirectVolVisShader; }
  const gl::ShaderObject& GetShaderVolumeRenderShader() { return m_VolumeRenderShader; }
  const gl::ShaderObject& GetShaderExtractGeometryInfo() { return m_ExtractGeometryInfoShader; }

private:
  void CreateSparseVoxelOctree();

  std::shared_ptr<const gl::ScreenAlignedTriangle> m_pScreenAlignedTriangle;

  gl::ShaderObject m_DirectVolVisShader;
  gl::ShaderObject m_ExtractGeometryInfoShader;
  gl::ShaderObject m_VolumeRenderShader;
  gl::ShaderObject m_ComputeNormalsShader;

  gl::UniformBuffer m_VolumeInfoUBO;

  GLuint m_VolumeIndirectDrawBuffer;
  GLuint m_GeometryInfoBuffer;
  GLuint m_GeometryInfoVA;

  GLuint m_VolumeSamplerObject;
  GLuint m_TexturingSamplerObjectAnisotropic;
  GLuint m_TexturingSamplerObjectTrilinear;

  GLuint m_SparseVoxelOctree;

  static const ezUInt32 m_volumeSize;

  ezUniquePtr<gl::Texture2D> m_pTextureY;
  ezUniquePtr<gl::Texture2D> m_pTextureXZ;

  static const ezUInt32 m_GeometryBufferElementCount = 1048576; // 4mb buffer, hopefully sufficient 
};

