#include "PCH.h"
#include "VoxelTerrain.h"

#include "math/NoiseGenerator.h"

#include "gl/ScreenAlignedTriangle.h"
#include "gl/Texture3D.h"
#include "gl/GLUtils.h"

const ezUInt32 VoxelTerrain::m_uiVolumeWidth = 256;
const ezUInt32 VoxelTerrain::m_uiVolumeHeight = 64;
const ezUInt32 VoxelTerrain::m_uiVolumeDepth = 256;

VoxelTerrain::VoxelTerrain(const gl::ScreenAlignedTriangle* pScreenAlignedTriangle) :
  m_pScreenAlignedTriangle(pScreenAlignedTriangle)
{
  // shader init
  m_DirectVolVisShader.AddShaderFromFile(gl::ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_DirectVolVisShader.AddShaderFromFile(gl::ShaderObject::ShaderType::FRAGMENT, "directvis.frag");
  m_DirectVolVisShader.CreateProgram();

  m_VolumeRenderShader.AddShaderFromFile(gl::ShaderObject::ShaderType::VERTEX, "volumeRender.vert");
  m_VolumeRenderShader.AddShaderFromFile(gl::ShaderObject::ShaderType::CONTROL, "volumeRender.cont");
  m_VolumeRenderShader.AddShaderFromFile(gl::ShaderObject::ShaderType::EVALUATION, "volumeRender.eval");
  m_VolumeRenderShader.AddShaderFromFile(gl::ShaderObject::ShaderType::GEOMETRY, "volumeRender.geom");
  m_VolumeRenderShader.AddShaderFromFile(gl::ShaderObject::ShaderType::FRAGMENT, "volumeRender.frag");
  m_VolumeRenderShader.CreateProgram();

  m_ExtractGeometryInfoShader.AddShaderFromFile(gl::ShaderObject::ShaderType::COMPUTE, "extractgeometry.comp");
  m_ExtractGeometryInfoShader.CreateProgram();


  // ubo init
  ezDynamicArray<const gl::ShaderObject*> volumeInfoUBOusingShader;
  volumeInfoUBOusingShader.PushBack(&m_DirectVolVisShader);
  m_VolumeInfoUBO.Init(volumeInfoUBOusingShader, "VolumeDataInfo");

  m_VolumeInfoUBO["VolumeWorldSize"].Set(ezVec3(static_cast<float>(m_uiVolumeWidth), static_cast<float>(m_uiVolumeHeight), static_cast<float>(m_uiVolumeDepth)));  // add size scaling here if necessar

  // geometry info buffer
  {
    glGenBuffers(1, &m_GeometryInfoBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_GeometryInfoBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ezUInt32) * m_GeometryBufferElementCount, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }


  // indirect draw buffer
  {
    glGenBuffers(1, &m_VolumeIndirectDrawBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_VolumeIndirectDrawBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl::DrawArraysIndirectCommand), NULL, GL_DYNAMIC_DRAW); // modified every frame to clear it
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }


  // vertex array
  {
    glGenVertexArrays(1, &m_GeometryInfoVA);
    glBindVertexArray(m_GeometryInfoVA);
    glBindBuffer(GL_ARRAY_BUFFER, m_GeometryInfoBuffer);
    glVertexAttribIPointer(0, 4, GL_UNSIGNED_BYTE, sizeof(ezUInt32), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
  }


  CreateVolumeTexture();
}


VoxelTerrain::~VoxelTerrain(void)
{
  EZ_DEFAULT_DELETE(m_pVolumeTexture);
  
  glDeleteVertexArrays(1, &m_GeometryInfoVA);
  glDeleteBuffers(1, &m_GeometryInfoBuffer);
  glDeleteBuffers(1, &m_VolumeIndirectDrawBuffer);
}

void VoxelTerrain::CreateVolumeTexture()
{
  m_pVolumeTexture = EZ_DEFAULT_NEW(gl::Texture3D)(m_uiVolumeWidth, m_uiVolumeHeight, m_uiVolumeDepth, GL_RGBA8, 1);
  ezColor* volumeData = EZ_DEFAULT_NEW_RAW_BUFFER(ezColor, m_uiVolumeWidth * m_uiVolumeHeight * m_uiVolumeDepth);

  NoiseGenerator noiseGen;
  ezUInt32 slicePitch = m_uiVolumeWidth * m_uiVolumeHeight;
  float mulitplier = 1.0f / static_cast<float>(std::max(std::max(m_uiVolumeWidth, m_uiVolumeHeight), m_uiVolumeDepth));

#pragma omp parallel for // OpenMP parallel for loop
  for(ezInt32 z=0; z<m_uiVolumeDepth; ++z) // needs to be signed for openmp
  {
    for(ezUInt32 y=0; y<m_uiVolumeHeight; ++y)
    {
      for(ezUInt32 x=0; x<m_uiVolumeWidth; ++x)
      {
        ezVec3 gradient;
        ezColor& current = volumeData[x + y * m_uiVolumeWidth + z * slicePitch];
        current.w = noiseGen.GetValueNoise(ezVec3(mulitplier*x, mulitplier*y, mulitplier*z), 0, 4, 0.5f, false, &gradient) * 0.5f + 0.5f;
        gradient.Normalize();
        gradient = gradient * 0.5f;
        current.x = gradient.x + 0.5f;
        current.y = gradient.y + 0.5f;
        current.z = gradient.z + 0.5f;
      }
    }
  }

  m_pVolumeTexture->SetData(0, volumeData);

  EZ_DEFAULT_DELETE_RAW_BUFFER(volumeData);
}

void VoxelTerrain::ComputeGeometryInfo()
{
  // need to clear the indirect draw buffer, otherwise we'll accumlate to much stuff...
  gl::DrawArraysIndirectCommand indirectCommandEmpty;
  indirectCommandEmpty.count = 6;
  indirectCommandEmpty.primCount = 1;
  indirectCommandEmpty.first = 0;
  indirectCommandEmpty.baseInstance = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_VolumeIndirectDrawBuffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gl::DrawArraysIndirectCommand), &indirectCommandEmpty);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // extract geometry info from volume
  m_ExtractGeometryInfoShader.BindImage(*m_pVolumeTexture, gl::Texture::ImageAccess::READ, "VolumeTexture");
  m_ExtractGeometryInfoShader.Activate();
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_GeometryInfoBuffer);
  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_VolumeIndirectDrawBuffer);  // bind indirect draw buffer as atomic counter to count how many quads should be rendered
  glDispatchCompute(m_uiVolumeWidth / 8, m_uiVolumeHeight / 8, m_uiVolumeDepth / 8);
  gl::Utils::CheckError("glDispatchCompute");
  // unbinds
  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

  
   // read back (debug usage)
 /* glFlush();
  glBindBuffer(GL_ARRAY_BUFFER, m_VolumeIndirectDrawBuffer);
  gl::DrawArraysIndirectCommand* data2 = (gl::DrawArraysIndirectCommand*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
  gl::DrawArraysIndirectCommand f = *data2;
  glUnmapBuffer(GL_ARRAY_BUFFER); */
}

void VoxelTerrain::Draw()
{
  m_VolumeRenderShader.Activate();
  glBindVertexArray(m_GeometryInfoVA);
  glPatchParameteri(GL_PATCH_VERTICES, 1);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_VolumeIndirectDrawBuffer);
  glDrawArraysIndirect(GL_PATCHES, NULL);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0); // unbind
}

void VoxelTerrain::DrawReferenceRaycast()
{
  // "reference renderer" direct volume visualization
  m_DirectVolVisShader.Activate();
  m_DirectVolVisShader.BindTexture(*m_pVolumeTexture, "VolumeTexture");
  m_DirectVolVisShader.BindUBO(m_VolumeInfoUBO);
  m_pScreenAlignedTriangle->Draw();
}