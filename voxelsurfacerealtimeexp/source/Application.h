#pragma once

#include <Core/Application/Application.h>

namespace ezLogWriter
{
  class HTML;
}

class Application : public ezApplication
{
public:
  Application();
  ~Application();

  virtual void AfterEngineInit() EZ_OVERRIDE;
  virtual void BeforeEngineShutdown() EZ_OVERRIDE;
  virtual ezApplication::ApplicationExecution Run() EZ_OVERRIDE;

private:
  void LoadGraphicsResources();

  void SetupFileSystem();
  void SetupInput();
  
  void UpdateInput(ezTime lastFrameDuration);
  void RenderFrame();

  class RenderWindowGL* m_pWindow;
  class ShaderObject* m_pTestShader;


  ezTime m_LastFrameTime;
  bool m_bRunning;

  ezLogWriter::HTML* m_pHTMLLogWriter;
};
