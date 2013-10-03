#include "PCH.h"

#include "Application.h"
#include "RenderWindow.h"

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Logging/HTMLWriter.h>

Application::Application() :
   m_pHTMLLogWriter(nullptr),
   m_bRunning(true)
{
}

Application::~Application()
{
}

void Application::AfterEngineInit()
{
  // start engine
  ezStartup::StartupEngine();

  // setup log
  ezLog::AddLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);
  m_pHTMLLogWriter = EZ_DEFAULT_NEW(ezLogWriter::HTML);
  m_pHTMLLogWriter->BeginLog("log.html", "voxelsurfacerealtimeexp");
  ezLog::AddLogWriter(ezLog::Event::Handler(&ezLogWriter::HTML::LogMessageHandler, m_pHTMLLogWriter));
  
  // start window
  m_pWindow = EZ_DEFAULT_NEW(RenderWindowGL);
	m_pWindow->Initialize();

  // setup input
  SetupInput();

  // reset time
  m_LastFrameTime = ezSystemTime::Now();
}

void Application::BeforeEngineShutdown()
{
  ezStartup::ShutdownEngine();

  m_pWindow->DestroyGraphicsContext();
	m_pWindow->Destroy();
  EZ_DEFAULT_DELETE(m_pWindow);

  ezLog::RemoveLogWriter(ezLog::Event::Handler(&ezLogWriter::HTML::LogMessageHandler, m_pHTMLLogWriter));
  m_pHTMLLogWriter->EndLog();
  EZ_DEFAULT_DELETE(m_pHTMLLogWriter);
}

ezApplication::ApplicationExecution Application::Run()
{
  // timing
  ezTime now = ezSystemTime::Now();
  ezTime lastFrameDuration = now - m_LastFrameTime;
  m_LastFrameTime = now;

  UpdateInput(m_LastFrameTime);

  if(m_pWindow->ProcessWindowMessages() != ezWindow::Continue)
    m_bRunning = false;

  return m_bRunning ? ezApplication::Continue : ezApplication::Quit;
}

void Application::RenderFrame()
{

  m_pWindow->SwapBuffers();
}

EZ_APPLICATION_ENTRY_POINT(Application);