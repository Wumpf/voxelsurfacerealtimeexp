#include "PCH.h"
#include "Scene.h"
#include "config/GlobalCVar.h"

#include "RenderWindow.h"

#include "gl/ScreenAlignedTriangle.h"
#include "gl/Font.h"
#include "gl/TimerQuery.h"
#include "gl/GLUtils.h"

#include "math/camera/FreeCamera.h"

#include "VoxelTerrain.h"
#include "Background.h"


GLuint vertexArray;

Scene::Scene(const RenderWindowGL& renderWindow) :
  m_pScreenAlignedTriangle(std::make_shared<gl::ScreenAlignedTriangle>()),
  
  m_pCamera(EZ_DEFAULT_NEW_UNIQUE(FreeCamera, 70.0f, static_cast<float>(GeneralConfig::g_ResolutionWidth.GetValue()) / GeneralConfig::g_ResolutionHeight.GetValue())),
  m_pFont(EZ_DEFAULT_NEW_UNIQUE(gl::Font, "Arial", 20, renderWindow.GetDeviceContext())),

  m_ExtractGeometryTimer(EZ_DEFAULT_NEW_UNIQUE(gl::TimerQuery)),
  m_DrawTimer(EZ_DEFAULT_NEW_UNIQUE(gl::TimerQuery))
{
  EZ_LOG_BLOCK("Scene init");

  m_pVoxelTerrain = EZ_DEFAULT_NEW(VoxelTerrain)(m_pScreenAlignedTriangle);
  m_pBackground = EZ_DEFAULT_NEW(Background)(m_pScreenAlignedTriangle);

  // global ubo init
  ezDynamicArray<const gl::ShaderObject*> cameraUBOusingShader;
  cameraUBOusingShader.PushBack(&m_pVoxelTerrain->GetShaderDirectVolVis());
  cameraUBOusingShader.PushBack(&m_pBackground->GetShader());
  cameraUBOusingShader.PushBack(&m_pVoxelTerrain->GetShaderVolumeRenderShader());
  m_CameraUBO.Init(cameraUBOusingShader, "Camera");

  ezDynamicArray<const gl::ShaderObject*> globalSceneInfoUBOusingShader;
  globalSceneInfoUBOusingShader.PushBack(&m_pVoxelTerrain->GetShaderDirectVolVis());
  globalSceneInfoUBOusingShader.PushBack(&m_pVoxelTerrain->GetShaderVolumeRenderShader());
  m_GlobalSceneInfo.Init(globalSceneInfoUBOusingShader, "GlobalSceneInfo");

/*    ezDynamicArray<const gl::ShaderObject*> timeUBOusingShader;
  cameraUBOusingShader.PushBack(&m_DirectVolVisShader);
  m_TimeUBO.Init(cameraUBOusingShader, "Time");
*/

  m_GlobalSceneInfo["GlobalDirLightDirection"].Set(ezVec3(1,-1.2,1).GetNormalized());
  m_GlobalSceneInfo["GlobalDirLightColor"].Set(ezVec3(0.98f, 0.98f, 0.8f));
  m_GlobalSceneInfo["GlobalAmbient"].Set(ezVec3(0.3f, 0.3f, 0.3f));

  ezVec3 vCameraPos = m_pVoxelTerrain->GetWorldSize();
  vCameraPos.x /=2;
  vCameraPos.z /=2;
  m_pCamera->SetPosition(vCameraPos);
}


Scene::~Scene(void)
{
  EZ_DEFAULT_DELETE(m_pVoxelTerrain);
  EZ_DEFAULT_DELETE(m_pBackground);
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
  // set global ubos to their global binding points
  m_CameraUBO.BindBuffer(0);
  m_TimeUBO.BindBuffer(1);
  m_GlobalSceneInfo.BindBuffer(2);

  m_ExtractGeometryTimer->Start();
  m_pVoxelTerrain->ComputeGeometryInfo();
  m_ExtractGeometryTimer->End();

  // no depth test needed so far
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // render nice background
  m_pBackground->Draw();

  // activate depth test


  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  // render processed data
  m_DrawTimer->Start();
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  m_pVoxelTerrain->Draw();
  
 // m_pVoxelTerrain->DrawReferenceRaycast();
   
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  m_DrawTimer->End();

  // disable depth test
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);


  // on screen info stuff
  DrawInfos(lastFrameDuration);

  return EZ_SUCCESS;
}

void Scene::DrawInfos(ezTime &lastFrameDuration)
{
  ezStringBuilder fpsString;
  fpsString.AppendFormat("fps: %.2f (%.2f ms)", 1.0 / lastFrameDuration.GetSeconds(), lastFrameDuration.GetMilliSeconds());
  m_pFont->DrawString(fpsString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 300.0f, 30.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));

  ezStringBuilder extractgeometryTimeString;
  extractgeometryTimeString.AppendFormat("extract geom data: %.4f ms", m_ExtractGeometryTimer->GetLastTimeElapsed().GetMilliSeconds());
  m_pFont->DrawString(extractgeometryTimeString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 300.0f, 50.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));

  ezStringBuilder drawTimeString;
  drawTimeString.AppendFormat("draw geom data: %.4f ms", m_DrawTimer->GetLastTimeElapsed().GetMilliSeconds());
  m_pFont->DrawString(drawTimeString.GetData(), ezVec2(GeneralConfig::g_ResolutionWidth - 300.0f, 70.0f).CompDiv(GeneralConfig::GetScreenResolutionF()));
}