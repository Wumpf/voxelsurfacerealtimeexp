#include "PCH.h"
#include "AntTweakBarInterface.h"

#include "config/GlobalCVar.h"

#include <AntTweakBar.h>

AntTweakBarInterface::AntTweakBarInterface(void) :
   m_pTweakBar(NULL)
{
}

namespace TWCallbacks
{
  static void TW_CALL SetWireframe(const void *value, void *clientData)
  { 
    SceneConfig::g_Wireframe = *reinterpret_cast<const bool*>(value);  // for instance
  }
  static void TW_CALL GetWireframe(void *value, void *clientData)
  { 
    *reinterpret_cast<bool*>(value) = SceneConfig::g_Wireframe;  // for instance
  }
  static void TW_CALL SetLegacyVolVis(const void *value, void *clientData)
  { 
    SceneConfig::g_UseReferenceVis = *reinterpret_cast<const bool*>(value);  // for instance
  }
  static void TW_CALL GetLegacyVolVis(void *value, void *clientData)
  { 
    *reinterpret_cast<bool*>(value) = SceneConfig::g_UseReferenceVis;  // for instance
  }
}

AntTweakBarInterface::~AntTweakBarInterface(void)
{
  if(!m_pTweakBar)
  {
    TwTerminate();
    GlobalEvents::g_pWindowMessage->RemoveEventHandler(ezEvent<const GlobalEvents::Win32Message&>::Handler(&AntTweakBarInterface::WindowMessageEventHandler, this));
  }
}

ezResult AntTweakBarInterface::Init()
{
  if (!TwInit(TW_OPENGL, NULL))
  {
    ezLog::Error("AntTweakBar initialization failed: %s\n", TwGetLastError());
    return EZ_FAILURE;
  }

  TwWindowSize(GeneralConfig::g_ResolutionWidth.GetValue(), GeneralConfig::g_ResolutionHeight.GetValue());

  // Create a tweak bar
  m_pTweakBar = TwNewBar("TweakBar");
  TwDefine(" TweakBar size='250 100' ");
  TwDefine(" TweakBar position='10 150' ");

  TwAddVarCB(m_pTweakBar, "Wireframe", TW_TYPE_BOOLCPP, &TWCallbacks::SetWireframe, &TWCallbacks::GetWireframe, NULL, NULL);
  TwAddVarCB(m_pTweakBar, "Legacy Direct VolVis", TW_TYPE_BOOLCPP, &TWCallbacks::SetLegacyVolVis, &TWCallbacks::GetLegacyVolVis, NULL, NULL);

  // register eventhandler
  GlobalEvents::g_pWindowMessage->AddEventHandler(ezEvent<const GlobalEvents::Win32Message&>::Handler(&AntTweakBarInterface::WindowMessageEventHandler, this));

  return EZ_SUCCESS;
}

void AntTweakBarInterface::WindowMessageEventHandler(const GlobalEvents::Win32Message& message)
{
  // puffer message to avoid recursive calling 
  m_MessageQueue.PushBack(message);
}

void AntTweakBarInterface::Draw()
{
  // unwind message buffer - this avoid recursive calls
  while(!m_MessageQueue.IsEmpty())
  {
    GlobalEvents::Win32Message message = m_MessageQueue.PeekFront();
    TwEventWin(message.wnd, message.msg, message.wParam, message.lParam);
    m_MessageQueue.PopFront();
  }

  TwDraw();
}