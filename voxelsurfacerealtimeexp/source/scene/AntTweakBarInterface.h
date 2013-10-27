#pragma once

#include <Foundation/Containers/List.h>
#include "GlobalEvents.h"

class AntTweakBarInterface
{
public:
  AntTweakBarInterface(void);
  ~AntTweakBarInterface(void);

  ezResult Init();
  void Draw();

private:
  void WindowMessageEventHandler(const GlobalEvents::Win32Message& message);

  struct CTwBar* m_pTweakBar;
  ezList<GlobalEvents::Win32Message> m_MessageQueue;
};

