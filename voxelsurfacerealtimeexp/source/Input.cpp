#include "PCH.h"
#include "Application.h"

#include <Core/Input/InputManager.h>

void Application::SetupInput()
{
  ezInputActionConfig inputConfig;
  inputConfig.m_sInputSlotTrigger[0] = ezInputSlot_KeyEscape;
  ezInputManager::SetInputActionConfig("Default", "CloseApp", inputConfig, false);
}

void Application::UpdateInput(ezTime lastFrameDuration)
{
  ezInputManager::Update(lastFrameDuration.GetSeconds());
  
  if (ezInputManager::GetInputActionState("Default", "CloseApp") == ezKeyState::Pressed)
    m_bRunning = false;
}