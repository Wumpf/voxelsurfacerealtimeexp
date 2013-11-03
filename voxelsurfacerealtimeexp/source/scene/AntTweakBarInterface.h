#pragma once

#include <Foundation/Containers/List.h>
#include "GlobalEvents.h"

class AntTweakBarInterface
{
public:
  AntTweakBarInterface(void);
  ~AntTweakBarInterface(void);

  ezResult Init();
  void Render(ezTime lastFrameDuration);

private:
  void WindowMessageEventHandler(const GlobalEvents::Win32Message& message);

  struct CTwBar* m_pTweakBar;
  ezList<GlobalEvents::Win32Message> m_MessageQueue;

  static const ezUInt32 m_maxStringLength = 256;
  char m_szFpsInfo[m_maxStringLength];
  char m_szFrameTimeInfo[m_maxStringLength];
};

