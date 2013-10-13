#pragma once

#include <Core/Input/InputManager.h>

namespace InputConfig
{
  extern const char* g_szSetName_MiscInput;
  extern const char* g_szAction_CloseApplication;
  
  extern const char* g_szSetName_Camera;
  extern const char* g_szAction_CameraForward;
  extern const char* g_szAction_CameraBackward;
  extern const char* g_szAction_CameraLeft;
  extern const char* g_szAction_CameraRight;
  extern const char* g_szAction_CameraRotateAxisX;
  extern const char* g_szAction_CameraRotateAxisY;

  extern const float g_fCameraMoveScale;
  extern const float g_fCameraRotationScale;
}