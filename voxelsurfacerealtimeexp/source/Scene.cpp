#include "PCH.h"
#include "Scene.h"
#include "GlobalCVar.h"

#include "RenderWindow.h"

#include "gl\ScreenAlignedTriangle.h"
#include "gl\Font.h"

#include "Camera\FreeCamera.h"


Scene::Scene(const RenderWindowGL& renderWindow) :
  m_pScreenAlignedTriangle(EZ_DEFAULT_NEW_UNIQUE(gl::ScreenAlignedTriangle)),
  
  m_pCamera(EZ_DEFAULT_NEW_UNIQUE(FreeCamera, ezMath::DegToRad(90.0f), static_cast<float>(GeneralConfig::g_ResolutionWidth.GetValue()) / GeneralConfig::g_ResolutionHeight.GetValue())),
  m_pFont(EZ_DEFAULT_NEW_UNIQUE(gl::Font, "Arial", 20, renderWindow.GetDeviceContext())) // cvar for resolution?
{
  EZ_LOG_BLOCK("Scene shader init");

  m_PostEffectShader.AddShaderFromFile(gl::ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_PostEffectShader.AddShaderFromFile(gl::ShaderObject::ShaderType::FRAGMENT, "background.frag");
  m_PostEffectShader.CreateProgram();

  m_ComputeShaderTest.AddShaderFromFile(gl::ShaderObject::ShaderType::COMPUTE, "comptest.comp");
  m_ComputeShaderTest.CreateProgram();

  m_TestUBO.Init(m_ComputeShaderTest.GetUniformBufferInfo()["TestUBO"], "TestUBO");
  m_TestUBO["Result"].Set(-123456.0f);

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
  m_ComputeShaderTest.BindUBO(m_TestUBO, "TestUBO");
  m_ComputeShaderTest.Activate();
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
  m_pFont->DrawString(fpsString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 100.0f, 30.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
    // camera
 /* ezStringBuilder cameraString;
  cameraString.AppendFormat("CameraPos (%.1f, %.1f, %.1f)", m_pCamera->GetPosition().x, m_pCamera->GetPosition().y, m_pCamera->GetPosition().z);
  m_pFont->DrawString(cameraString.GetData(), ezVec2(10.0f, 60.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
  */
  return EZ_SUCCESS;
}
