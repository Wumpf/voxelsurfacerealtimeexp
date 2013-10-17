#include "PCH.h"
#include "Scene.h"
#include "GlobalCVar.h"

#include "RenderWindow.h"

#include "gl\ScreenAlignedTriangle.h"
#include "gl\ShaderObject.h"
#include "gl\Font.h"

#include "Camera\FreeCamera.h"


Scene::Scene(const RenderWindowGL& renderWindow) :
  m_pPostEffectShader(EZ_DEFAULT_NEW_UNIQUE(gl::ShaderObject)),
  m_pComputeShaderTest(EZ_DEFAULT_NEW_UNIQUE(gl::ShaderObject)),
  m_pScreenAlignedTriangle(EZ_DEFAULT_NEW_UNIQUE(gl::ScreenAlignedTriangle)),
  
  m_pCamera(EZ_DEFAULT_NEW_UNIQUE(FreeCamera, ezMath::DegToRad(90.0f), static_cast<float>(GeneralConfig::g_ResolutionWidth.GetValue()) / GeneralConfig::g_ResolutionHeight.GetValue())),
  m_pFont(EZ_DEFAULT_NEW_UNIQUE(gl::Font, "Arial", 20, renderWindow.GetDeviceContext())) // cvar for resolution?
{
  EZ_LOG_BLOCK("Scene shader init");

  m_pPostEffectShader->AddShaderFromFile(gl::ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_pPostEffectShader->AddShaderFromFile(gl::ShaderObject::ShaderType::FRAGMENT, "background.frag");
  m_pPostEffectShader->CreateProgram();

  m_pComputeShaderTest->AddShaderFromFile(gl::ShaderObject::ShaderType::COMPUTE, "comptest.comp");
  m_pComputeShaderTest->CreateProgram();

  glGenBuffers(1, &m_TestBuffer);
  float fInitalData;
  glBindBuffer(GL_ARRAY_BUFFER, m_TestBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float), &fInitalData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


Scene::~Scene(void)
{
}


ezResult Scene::Update(ezTime lastFrameDuration)
{
  m_pCamera->Update(lastFrameDuration);

  return EZ_SUCCESS;
}

ezResult Scene::Render(ezTime lastFrameDuration)
{
  // render test
 // m_pPostEffectShader->Activate();
 // m_pScreenAlignedTriangle->display();

  // compute test
  m_pComputeShaderTest->Activate();
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_TestBuffer);
  glDispatchCompute(1, 1, 1);

  // read
  glFlush();
  float* data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
  float f = *data;
  EZ_ASSERT(f == 42, "Compute shader doesn't work as expected!");
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  
  // render some info text
    // performance
  ezStringBuilder fpsString;
  fpsString.AppendFormat("fps: %.2f (%.2f ms)", 1.0 / lastFrameDuration.GetSeconds(), lastFrameDuration.GetMilliSeconds());
  m_pFont->DrawString(fpsString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 100, 30.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
    // camera
 /* ezStringBuilder cameraString;
  cameraString.AppendFormat("CameraPos (%.1f, %.1f, %.1f)", m_pCamera->GetPosition().x, m_pCamera->GetPosition().y, m_pCamera->GetPosition().z);
  m_pFont->DrawString(cameraString.GetData(), ezVec2(10.0f, 60.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
  */
  return EZ_SUCCESS;
}
