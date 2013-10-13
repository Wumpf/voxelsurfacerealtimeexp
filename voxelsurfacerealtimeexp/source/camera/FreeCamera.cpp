#include "PCH.h"
#include "FreeCamera.h"
#include "InputConfig.h"

FreeCamera::FreeCamera(float fov, float aspectRatio) :
  Camera(fov, aspectRatio),
  m_fMouseX(0.0f), m_fMouseY(0.0f)
{
}

FreeCamera::~FreeCamera()
{
}

void FreeCamera::Update(ezTime lastFrameDuration)
{
  float fMouseXDelta, fMouseYDelta;
  ezInputManager::GetInputActionState(InputConfig::g_szSetName_Camera, InputConfig::g_szAction_CameraRotateAxisX, &fMouseXDelta);
  ezInputManager::GetInputActionState(InputConfig::g_szSetName_Camera, InputConfig::g_szAction_CameraRotateAxisY, &fMouseYDelta);
  m_fMouseX += fMouseXDelta;
  m_fMouseY += fMouseYDelta;

  m_ViewDir.x = sinf(m_fMouseX) * cosf(m_fMouseY);
  m_ViewDir.y = sinf(m_fMouseY);
  m_ViewDir.z = cosf(m_fMouseX) * cosf(m_fMouseY);
  
  float fForward, fBackward, fLeft, fRight;
  ezInputManager::GetInputActionState(InputConfig::g_szSetName_Camera, InputConfig::g_szAction_CameraForward, &fForward);
  ezInputManager::GetInputActionState(InputConfig::g_szSetName_Camera, InputConfig::g_szAction_CameraBackward, &fBackward);
  ezInputManager::GetInputActionState(InputConfig::g_szSetName_Camera, InputConfig::g_szAction_CameraLeft, &fLeft);
  ezInputManager::GetInputActionState(InputConfig::g_szSetName_Camera, InputConfig::g_szAction_CameraRight, &fRight);

  m_vPosition += (fForward - fBackward) * m_ViewDir;
  m_vPosition += (fRight - fLeft) * m_ViewDir.Cross(m_vUp);

  UpdateMatrices();
}