#include "PCH.h"
#include "VoxelTerrain.h"

#include "math/NoiseGenerator.h"

#include "gl/ScreenAlignedTriangle.h"
#include "gl/resources/textures/Texture3D.h"
#include "gl/resources/textures/Texture2D.h"
#include "gl/GLUtils.h"

#include "..\config\GlobalCVar.h"

const ezUInt32 VoxelTerrain::m_uiVolumeWidth = 128;//256;
const ezUInt32 VoxelTerrain::m_uiVolumeHeight = 64;
const ezUInt32 VoxelTerrain::m_uiVolumeDepth = 128;//256;

namespace SceneConfig
{
  ezCVarFloat g_GradientDescendStepMultiplier("Gradient Descend Step Multiplier", 8.0f, ezCVarFlags::Save, "min=0.1 max=30.0 step=0.1 group=VolumeRendering");
  ezCVarInt g_GradientDescendStepCount("Gradient Descend Step Count", 12, ezCVarFlags::Save, "min=1 max=50 group=VolumeRendering");
  ezCVarBool g_UseAnisotropicFilter("Anisotropic Filter on/off", true, ezCVarFlags::Save, "group=VolumeRendering");
}

VoxelTerrain::VoxelTerrain(const std::shared_ptr<const gl::ScreenAlignedTriangle>& pScreenAlignedTriangle) :
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
  volumeInfoUBOusingShader.PushBack(&m_VolumeRenderShader);
  m_VolumeInfoUBO.Init(volumeInfoUBOusingShader, "VolumeDataInfo");

  m_VolumeInfoUBO["VolumeWorldSize"].Set(ezVec3(static_cast<float>(m_uiVolumeWidth), static_cast<float>(m_uiVolumeHeight), static_cast<float>(m_uiVolumeDepth)));
  m_VolumeInfoUBO["VolumePosToTexcoord"].Set(ezVec3(1.0f / static_cast<float>(m_uiVolumeWidth-1), 1.0f /static_cast<float>(m_uiVolumeHeight-1), 1.0f /static_cast<float>(m_uiVolumeDepth-1))); // minus one is very important! otherwise the fetching won't match exactly! texture with 256 -> 255 maps to 1

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

  // sampler
  {
    glGenSamplers(1, &m_VolumeSamplerObject);
    glSamplerParameteri(m_VolumeSamplerObject, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
    glSamplerParameteri(m_VolumeSamplerObject, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_VolumeSamplerObject, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  
    glSamplerParameteri(m_VolumeSamplerObject, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
    glSamplerParameteri(m_VolumeSamplerObject, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  // sampler
  {
    glGenSamplers(1, &m_TexturingSamplerObjectAnisotropic);
    glSamplerParameteri(m_TexturingSamplerObjectAnisotropic, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glSamplerParameteri(m_TexturingSamplerObjectAnisotropic, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_TexturingSamplerObjectAnisotropic, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  
    glSamplerParameteri(m_TexturingSamplerObjectAnisotropic, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glSamplerParameteri(m_TexturingSamplerObjectAnisotropic, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);

    glGenSamplers(1, &m_TexturingSamplerObjectTrilinear);
    glSamplerParameteri(m_TexturingSamplerObjectTrilinear, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glSamplerParameteri(m_TexturingSamplerObjectTrilinear, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_TexturingSamplerObjectTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  
    glSamplerParameteri(m_TexturingSamplerObjectTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glSamplerParameteri(m_TexturingSamplerObjectTrilinear, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
  }

  // load textures
  m_pTextureY.Swap(gl::Texture2D::LoadFromFile("grass.png"));
  m_pTextureXZ.Swap(gl::Texture2D::LoadFromFile("rock.png"));

  CreateVolumeTexture();
}

VoxelTerrain::~VoxelTerrain(void)
{
  EZ_DEFAULT_DELETE(m_pVolumeTexture);
  
  glDeleteVertexArrays(1, &m_GeometryInfoVA);
  glDeleteBuffers(1, &m_GeometryInfoBuffer);
  glDeleteBuffers(1, &m_VolumeIndirectDrawBuffer);
  glDeleteSamplers(1, &m_VolumeSamplerObject);
  glDeleteSamplers(1, &m_TexturingSamplerObjectAnisotropic);
  glDeleteSamplers(1, &m_TexturingSamplerObjectTrilinear);
}

void VoxelTerrain::CreateVolumeTexture()
{
  m_pVolumeTexture = EZ_DEFAULT_NEW(gl::Texture3D)(m_uiVolumeWidth, m_uiVolumeHeight, m_uiVolumeDepth, GL_RGBA8, 1); // tried to use GL_RGBA32F without changing anything else: saw nothing then
  ezColor* volumeData = EZ_DEFAULT_NEW_RAW_BUFFER(ezColor, m_uiVolumeWidth * m_uiVolumeHeight * m_uiVolumeDepth);

  NoiseGenerator noiseGen;
  ezUInt32 slicePitch = m_uiVolumeWidth * m_uiVolumeHeight;
  float mulitplier = 0.5f / static_cast<float>(ezMath::Max(m_uiVolumeWidth, m_uiVolumeHeight, m_uiVolumeDepth) - 1);

#pragma omp parallel for // OpenMP parallel for loop.
  for(ezInt32 z=0; z<m_uiVolumeDepth; ++z) // Needs to be signed for OpenMP.
  {
    for(ezUInt32 y=0; y<m_uiVolumeHeight; ++y)
    {
      for(ezUInt32 x=0; x<m_uiVolumeWidth; ++x)
      {
        ezVec3 gradient;
        ezColor& current = volumeData[x + y * m_uiVolumeWidth + z * slicePitch];
        current.a = noiseGen.GetValueNoise(ezVec3(mulitplier*x, mulitplier*y, mulitplier*z), 0, 5, 0.8f, false, &gradient);
        gradient.Normalize();
        current.SetRGB(gradient);
        current *= 0.5f;
        current += ezColor32f(0.5f,0.5f,0.5f,0.5f);
      }
    }
  }

  m_pVolumeTexture->SetData(0, volumeData);

  EZ_DEFAULT_DELETE_RAW_BUFFER(volumeData);
}

void VoxelTerrain::ComputeGeometryInfo()
{
  m_VolumeInfoUBO.BindBuffer(3);

  // need to clear the indirect draw buffer, otherwise we'll accumulate to much stuff...
  gl::DrawArraysIndirectCommand indirectCommandEmpty;
  indirectCommandEmpty.count = 6;
  indirectCommandEmpty.primCount = 1;
  indirectCommandEmpty.first = 0;
  indirectCommandEmpty.baseInstance = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_VolumeIndirectDrawBuffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gl::DrawArraysIndirectCommand), &indirectCommandEmpty);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // extract geometry info from volume
  m_pVolumeTexture->BindImage(0, gl::Texture::ImageAccess::READ);
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
  // todo? change only if necessary
  m_VolumeInfoUBO["GradientDescendStepMultiplier"].Set(SceneConfig::g_GradientDescendStepMultiplier);
  m_VolumeInfoUBO["GradientDescendStepCount"].Set(SceneConfig::g_GradientDescendStepCount);
  m_VolumeInfoUBO.BindBuffer(3);

  m_VolumeRenderShader.Activate();


  glBindSampler(0, m_VolumeSamplerObject);
  m_pVolumeTexture->Bind(0);

  GLint textureSampler = m_TexturingSamplerObjectAnisotropic;
  if(!SceneConfig::g_UseAnisotropicFilter)
    textureSampler = m_TexturingSamplerObjectTrilinear;
  glBindSampler(1, textureSampler);
  m_pTextureY->Bind(1);
  glBindSampler(2, textureSampler);
  m_pTextureXZ->Bind(2);

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
  glBindSampler(0, m_VolumeSamplerObject);
  m_pVolumeTexture->Bind(0);
  m_DirectVolVisShader.BindUBO(m_VolumeInfoUBO);
  m_pScreenAlignedTriangle->Draw();
}