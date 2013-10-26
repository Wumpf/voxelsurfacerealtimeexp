#include "PCH.h"
#include "Scene.h"
#include "GlobalCVar.h"

#include "RenderWindow.h"
#include "math/NoiseGenerator.h"

#include "gl/ScreenAlignedTriangle.h"
#include "gl/Font.h"
#include "gl/Texture3D.h"
#include "gl/TimerQuery.h"
#include "gl/GLUtils.h"

#include "math/camera/FreeCamera.h"

const ezUInt32 Scene::m_uiVolumeWidth = 256;
const ezUInt32 Scene::m_uiVolumeHeight = 64;
const ezUInt32 Scene::m_uiVolumeDepth = 256;

GLuint vertexArray;

Scene::Scene(const RenderWindowGL& renderWindow) :
  m_pScreenAlignedTriangle(EZ_DEFAULT_NEW_UNIQUE(gl::ScreenAlignedTriangle)),
  
  m_pCamera(EZ_DEFAULT_NEW_UNIQUE(FreeCamera, 70.0f, static_cast<float>(GeneralConfig::g_ResolutionWidth.GetValue()) / GeneralConfig::g_ResolutionHeight.GetValue())),
  m_pFont(EZ_DEFAULT_NEW_UNIQUE(gl::Font, "Arial", 20, renderWindow.GetDeviceContext())),
  m_glTimer(EZ_DEFAULT_NEW_UNIQUE(gl::TimerQuery))
{
  EZ_LOG_BLOCK("Scene shader init");

  // shader init
  {
    m_BackgroundShader.AddShaderFromFile(gl::ShaderObject::ShaderType::VERTEX, "screenTri.vert");
    m_BackgroundShader.AddShaderFromFile(gl::ShaderObject::ShaderType::FRAGMENT, "background.frag");
    m_BackgroundShader.CreateProgram();

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
  }

  // ubo init
  {
    ezDynamicArray<const gl::ShaderObject*> cameraUBOusingShader;
    cameraUBOusingShader.PushBack(&m_DirectVolVisShader);
    cameraUBOusingShader.PushBack(&m_BackgroundShader);
    cameraUBOusingShader.PushBack(&m_VolumeRenderShader);
    m_CameraUBO.Init(cameraUBOusingShader, "Camera");

/*    ezDynamicArray<const gl::ShaderObject*> timeUBOusingShader;
    cameraUBOusingShader.PushBack(&m_DirectVolVisShader);
    m_TimeUBO.Init(cameraUBOusingShader, "Time");
*/
    ezDynamicArray<const gl::ShaderObject*> volumeInfoUBOusingShader;
    volumeInfoUBOusingShader.PushBack(&m_DirectVolVisShader);
    m_VolumeInfoUBO.Init(volumeInfoUBOusingShader, "VolumeDataInfo");

    m_VolumeInfoUBO["VolumeWorldSize"].Set(ezVec3(static_cast<float>(m_uiVolumeWidth), static_cast<float>(m_uiVolumeHeight), static_cast<float>(m_uiVolumeDepth)));  // add size scaling here if necessar
 }

  {
    glGenBuffers(1, &m_GeometryInfoBuffer);
    float fInitalData;
    glBindBuffer(GL_ARRAY_BUFFER, m_GeometryInfoBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ezUInt32) * m_GeometryBufferElementCount, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  CreateVolumeTexture();

  // indirect draw buffer
  {
    glGenBuffers(1, &m_VolumeIndirectDrawBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_VolumeIndirectDrawBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl::DrawArraysIndirectCommand), NULL, GL_DYNAMIC_DRAW); // modified every frame to clear it
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  m_pCamera->SetPosition(ezVec3(static_cast<float>(m_uiVolumeWidth/2), static_cast<float>(m_uiVolumeHeight), static_cast<float>(m_uiVolumeDepth/2)));


  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_GeometryInfoBuffer);
    glVertexAttribIPointer(0, 4, GL_UNSIGNED_BYTE, sizeof(ezUInt32), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(0);
  glBindVertexArray(0);

}


Scene::~Scene(void)
{
  EZ_DEFAULT_DELETE(m_pVolumeTexture);

  glDeleteVertexArrays(1, &m_GeometryInfoBuffer);
  glDeleteBuffers(1, &vertexArray);
  glDeleteBuffers(1, &m_VolumeIndirectDrawBuffer);
}

void Scene::CreateVolumeTexture()
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


ezResult Scene::Update(ezTime lastFrameDuration)
{
  m_pCamera->Update(lastFrameDuration);

  //m_CameraUBO["ViewMatrix"].Set(m_pCamera->GetViewMatrix());
  m_CameraUBO["ViewProjection"].Set(m_pCamera->GetViewProjectionMatrix());
  ezMat4 inverseViewProjection = m_pCamera->GetViewProjectionMatrix();
  inverseViewProjection.Invert();
  m_CameraUBO["InverseViewProjection"].Set(inverseViewProjection);
  m_CameraUBO["CameraPosition"].Set(m_pCamera->GetPosition());
  

  //m_TimeUBO["CurrentTime"].Set();
  //m_TimeUBO["LastFrameDuration"].Set(lastFrameDuration);

  return EZ_SUCCESS;
}

ezResult Scene::Render(ezTime lastFrameDuration)
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
  m_glTimer->Start();
  glDispatchCompute(m_uiVolumeWidth / 8, m_uiVolumeHeight / 8, m_uiVolumeDepth / 8);
  m_glTimer->End();
  gl::Utils::CheckError("glDispatchCompute");
    // unbinds
  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
 
  // render nice background
  m_BackgroundShader.Activate();
  m_BackgroundShader.BindUBO(m_CameraUBO);
  m_pScreenAlignedTriangle->display();
  
  // "reference renderer" direct volume visualization
 /* m_DirectVolVisShader.Activate();
  m_DirectVolVisShader.BindTexture(*m_pVolumeTexture, "VolumeTexture");
  m_DirectVolVisShader.BindUBO(m_CameraUBO);
  m_DirectVolVisShader.BindUBO(m_VolumeInfoUBO);
  m_pScreenAlignedTriangle->display();*/

  // render processed data
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  m_VolumeRenderShader.BindUBO(m_CameraUBO);
  m_VolumeRenderShader.Activate();
  glBindVertexArray(vertexArray);
  glPatchParameteri(GL_PATCH_VERTICES, 1);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_VolumeIndirectDrawBuffer);
  glDrawArraysIndirect(GL_PATCHES, NULL);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0); // unbind
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  
  // read back (debug usage)
 /* glFlush();
  glBindBuffer(GL_ARRAY_BUFFER, m_VolumeIndirectDrawBuffer);
  gl::DrawArraysIndirectCommand* data2 = (gl::DrawArraysIndirectCommand*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
  gl::DrawArraysIndirectCommand f = *data2;
  glUnmapBuffer(GL_ARRAY_BUFFER); */

  // render some info text
    // performance
  ezStringBuilder fpsString;
  fpsString.AppendFormat("fps: %.2f (%.2f ms)", 1.0 / lastFrameDuration.GetSeconds(), lastFrameDuration.GetMilliSeconds());
  m_pFont->DrawString(fpsString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 250.0f, 30.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));

  ezStringBuilder extractgeometryTimeString;
  extractgeometryTimeString.AppendFormat("extractgeometry shader: %.4f ms", m_glTimer->GetLastTimeElapsed().GetMilliSeconds());
  m_pFont->DrawString(extractgeometryTimeString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 250.0f, 50.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));

    // camera
 /* ezStringBuilder cameraString;
  cameraString.AppendFormat("CameraPos (%.1f, %.1f, %.1f)", m_pCamera->GetPosition().x, m_pCamera->GetPosition().y, m_pCamera->GetPosition().z);
  m_pFont->DrawString(cameraString.GetData(), ezVec2(10.0f, 60.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
  */
  return EZ_SUCCESS;
}
