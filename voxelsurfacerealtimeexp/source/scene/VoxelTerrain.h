#pragma once

#include "gl\ShaderObject.h"
#include "gl\UniformBuffer.h"

namespace gl
{
  class ScreenAlignedTriangle;
};

class VoxelTerrain
{
public:
  VoxelTerrain(const gl::ScreenAlignedTriangle* pScreenAlignedTriangle);
  ~VoxelTerrain();
  
  // you should put as much stuff between ComputeGeometryInfo and Draw as possible!
  void ComputeGeometryInfo();
  void Draw();
  void DrawReferenceRaycast();

  ezVec3 GetWorldSize() { return ezVec3(static_cast<float>(m_uiVolumeWidth), static_cast<float>(m_uiVolumeHeight), static_cast<float>(m_uiVolumeDepth)); }

  const gl::ShaderObject& GetShaderDirectVolVis() { return m_DirectVolVisShader; }
  const gl::ShaderObject& GetShaderVolumeRenderShader() { return m_VolumeRenderShader; }
  const gl::ShaderObject& GetShaderExtractGeometryInfo() { return m_ExtractGeometryInfoShader; }

private:
  void CreateVolumeTexture();

  const gl::ScreenAlignedTriangle* m_pScreenAlignedTriangle;

  gl::ShaderObject m_DirectVolVisShader;
  gl::ShaderObject m_ExtractGeometryInfoShader;
  gl::ShaderObject m_VolumeRenderShader;

  gl::UniformBuffer m_VolumeInfoUBO;

  GLuint m_VolumeIndirectDrawBuffer;
  GLuint m_GeometryInfoBuffer;
  GLuint m_GeometryInfoVA;

  static const ezUInt32 m_uiVolumeWidth;
  static const ezUInt32 m_uiVolumeHeight;
  static const ezUInt32 m_uiVolumeDepth;
  gl::Texture* m_pVolumeTexture;

  static const ezUInt32 m_GeometryBufferElementCount = 2097152; // 8mb buffer, hopefully sufficient (if not we're sooo doomed)
};

