#include "PCH.h"

#include "Application.h"
#include "RenderWindow.h"
#include "Scene.h"
#include "OnScreenLogWriter.h"

#include <Foundation/Configuration/Startup.h>

#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Logging/HTMLWriter.h>

#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>

Application::Application() :
   m_pHTMLLogWriter(nullptr),
   m_bRunning(true),
   m_pWindow(NULL),
   m_pScene(NULL)
{
}

Application::~Application()
{
}

void Application::AfterEngineInit()
{
  // start engine
  ezStartup::StartupEngine();

  // setups file system stuff
  SetupFileSystem();

  // load global config variables
  ezCVar::SetStorageFolder("CVars");
  ezCVar::LoadCVars();

  // setup log
  ezGlobalLog::AddLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);
  m_pHTMLLogWriter = EZ_DEFAULT_NEW(ezLogWriter::HTML);
  m_pHTMLLogWriter->BeginLog("log.html", "voxelsurfacerealtimeexp");
  ezGlobalLog::AddLogWriter(ezLoggingEvent::Handler(&ezLogWriter::HTML::LogMessageHandler, m_pHTMLLogWriter));
  

  // start window
  m_pWindow = EZ_DEFAULT_NEW(RenderWindowGL);

  // onscreen log
  m_pOnScreenLogWriter = EZ_DEFAULT_NEW(OnScreenLogWriter)(*m_pWindow);
  ezGlobalLog::AddLogWriter(ezLoggingEvent::Handler(&OnScreenLogWriter::LogMessageHandler, m_pOnScreenLogWriter));

  // setup input
  SetupInput();

  // load graphics stuff
  m_pScene = EZ_DEFAULT_NEW(Scene)(*m_pWindow);

  // reset time
  m_LastFrameTime = ezSystemTime::Now();
}

void Application::BeforeEngineShutdown()
{
  ezStartup::ShutdownEngine();

  EZ_DEFAULT_DELETE(m_pScene);

  ezGlobalLog::RemoveLogWriter(ezLoggingEvent::Handler(&OnScreenLogWriter::LogMessageHandler, m_pOnScreenLogWriter));
  EZ_DEFAULT_DELETE(m_pOnScreenLogWriter);

  EZ_DEFAULT_DELETE(m_pWindow);
    
  ezGlobalLog::RemoveLogWriter(ezLoggingEvent::Handler(&ezLogWriter::HTML::LogMessageHandler, m_pHTMLLogWriter));
  m_pHTMLLogWriter->EndLog();
  EZ_DEFAULT_DELETE(m_pHTMLLogWriter);
}

ezApplication::ApplicationExecution Application::Run()
{
  // timing
  ezTime now = ezSystemTime::Now();
  ezTime lastFrameDuration = now - m_LastFrameTime;
  m_LastFrameTime = now;

  // update
  UpdateInput(lastFrameDuration);
  m_pScene->Update(lastFrameDuration);
  if(m_pWindow->ProcessWindowMessages() != ezWindow::Continue)
    m_bRunning = false;
  m_pOnScreenLogWriter->Update(lastFrameDuration);

  // rendering
  RenderFrame(lastFrameDuration);


  return m_bRunning ? ezApplication::Continue : ezApplication::Quit;
}

void Application::RenderFrame(ezTime lastFrameDuration)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  m_pScene->Render(lastFrameDuration);
  m_pOnScreenLogWriter->Render();

  m_pWindow->SwapBuffers();
}

void Application::SetupFileSystem()
{
  ezStringBuilder applicationDir(ezOSFile::GetApplicationDirectory());

  ezOSFile::CreateDirectoryStructure(applicationDir.GetData());
  ezFileSystem::RegisterDataDirectoryFactory(ezDataDirectory::FolderType::Factory);

  ezFileSystem::AddDataDirectory(applicationDir.GetData(), ezFileSystem::AllowWrites, "general", "");
  
  ezStringBuilder shaderDir(applicationDir);
  shaderDir.AppendPath("..", "..", "voxelsurfacerealtimeexp", "shader"); // dev only!
  ezFileSystem::AddDataDirectory(shaderDir.GetData(), ezFileSystem::ReadOnly, "graphics", "");
}

EZ_APPLICATION_ENTRY_POINT(Application);