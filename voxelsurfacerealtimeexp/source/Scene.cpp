#include "PCH.h"
#include "Scene.h"
#include "GlobalCVar.h"

#include "RenderWindow.h"
#include "NoiseGenerator.h"

#include "gl\ScreenAlignedTriangle.h"
#include "gl\Font.h"
#include "gl\Texture3D.h"

#include "Camera\FreeCamera.h"

const ezUInt32 Scene::m_vVolumeWidth = 256;
const ezUInt32 Scene::m_vVolumeHeight = 64;
const ezUInt32 Scene::m_vVolumeDepth = 256;

Scene::Scene(const RenderWindowGL& renderWindow) :
  m_pScreenAlignedTriangle(EZ_DEFAULT_NEW_UNIQUE(gl::ScreenAlignedTriangle)),
  
  m_pCamera(EZ_DEFAULT_NEW_UNIQUE(FreeCamera, 120.0f, static_cast<float>(GeneralConfig::g_ResolutionWidth.GetValue()) / GeneralConfig::g_ResolutionHeight.GetValue())),
  m_pFont(EZ_DEFAULT_NEW_UNIQUE(gl::Font, "Arial Bold", 20, renderWindow.GetDeviceContext())) // cvar for resolution?
{
  EZ_LOG_BLOCK("Scene shader init");

  m_BackgroundShader.AddShaderFromFile(gl::ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_BackgroundShader.AddShaderFromFile(gl::ShaderObject::ShaderType::FRAGMENT, "background.frag");
  m_BackgroundShader.CreateProgram();

  m_DirectVolVisShader.AddShaderFromFile(gl::ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_DirectVolVisShader.AddShaderFromFile(gl::ShaderObject::ShaderType::FRAGMENT, "directvis.frag");
  m_DirectVolVisShader.CreateProgram();

  m_ComputeShaderTest.AddShaderFromFile(gl::ShaderObject::ShaderType::COMPUTE, "comptest.comp");
  m_ComputeShaderTest.CreateProgram();

  m_TestUBO.Init(m_ComputeShaderTest.GetUniformBufferInfo()["TestUBO"], "TestUBO");
  m_TestUBO["Result"].Set(-123456.0f);

  m_CameraUBO.Init(m_DirectVolVisShader.GetUniformBufferInfo()["Camera"], "Camera");

  glGenBuffers(1, &m_TestBuffer);
  float fInitalData;
  glBindBuffer(GL_ARRAY_BUFFER, m_TestBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float), &fInitalData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  CreateVolumeTexture();

  m_pCamera->SetPosition(ezVec3(64, 40, 64));
}


Scene::~Scene(void)
{
  EZ_DEFAULT_DELETE(m_pVolumeTexture);
}

void Scene::CreateVolumeTexture()
{
  m_pVolumeTexture = EZ_DEFAULT_NEW(gl::Texture3D)(m_vVolumeWidth, m_vVolumeHeight, m_vVolumeDepth, GL_RGBA8, 1);
  ezColor* volumeData = EZ_DEFAULT_NEW_RAW_BUFFER(ezColor, m_vVolumeWidth * m_vVolumeHeight * m_vVolumeDepth);

  NoiseGenerator noiseGen;
  ezUInt32 slicePitch = m_vVolumeWidth * m_vVolumeHeight;

  // todo optimize?
  #pragma omp parallel for // OpenMP parallel for loop
  for(ezInt32 z=0; z<m_vVolumeDepth; ++z) // needs to be signed for openmp
  {
    for(ezUInt32 y=0; y<m_vVolumeHeight; ++y)
    {
      for(ezUInt32 x=0; x<m_vVolumeWidth; ++x)
      {
        ezVec3 gradient;
        ezColor& current = volumeData[x + y * m_vVolumeWidth + z * slicePitch];
        current.w = noiseGen.GetValueNoise(ezVec3(static_cast<float>(x) / m_vVolumeWidth,static_cast<float>(y) / m_vVolumeHeight,static_cast<float>(z) / m_vVolumeDepth), 
                                                  0, 4, 0.5f, false, &gradient) * 0.5f + 0.5f;
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
  //m_CameraUBO["ViewProjection"].Set(m_pCamera->GetViewProjectionMatrix());
  ezMat4 inverseViewProjection = m_pCamera->GetViewProjectionMatrix();
  inverseViewProjection.Invert();
  m_CameraUBO["InverseViewProjection"].Set(inverseViewProjection);
  m_CameraUBO["CameraPosition"].Set(m_pCamera->GetPosition());

  return EZ_SUCCESS;
}

ezResult Scene::Render(ezTime lastFrameDuration)
{
  // render nice background
  m_BackgroundShader.Activate();
  m_BackgroundShader.BindUBO(m_CameraUBO, "Camera");
  m_pScreenAlignedTriangle->display();
  
  // 
  m_DirectVolVisShader.Activate();
  m_DirectVolVisShader.BindUBO(m_CameraUBO, "Camera");
  m_pScreenAlignedTriangle->display();
  
  // compute test
  m_ComputeShaderTest.Activate();
  m_ComputeShaderTest.BindUBO(m_TestUBO, "TestUBO");
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_TestBuffer);
  glDispatchCompute(1, 1, 1);

  // read
  glFlush();
  float* data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
  float f = *data;
  EZ_ASSERT(f == -123456.0f, "Compute shader doesn't work as expected!");
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  
  // render some info text
    // performance
  ezStringBuilder fpsString;
  fpsString.AppendFormat("fps: %.2f (%.2f ms)", 1.0 / lastFrameDuration.GetSeconds(), lastFrameDuration.GetMilliSeconds());
  m_pFont->DrawString(fpsString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 200.0f, 30.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
    // camera
 /* ezStringBuilder cameraString;
  cameraString.AppendFormat("CameraPos (%.1f, %.1f, %.1f)", m_pCamera->GetPosition().x, m_pCamera->GetPosition().y, m_pCamera->GetPosition().z);
  m_pFont->DrawString(cameraString.GetData(), ezVec2(10.0f, 60.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
  */
  return EZ_SUCCESS;
}
