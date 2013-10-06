#include "PCH.h"

#include "Application.h"
#include "RenderWindow.h"
#include "gl/ShaderObject.h"

#include <Foundation/Configuration/Startup.h>

#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Logging/HTMLWriter.h>

#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>

Application::Application() :
   m_pHTMLLogWriter(nullptr),
   m_bRunning(true),
   m_pTestShader(NULL),
   m_pWindow(NULL)
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

  // setup log
  ezLog::AddLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);
  m_pHTMLLogWriter = EZ_DEFAULT_NEW(ezLogWriter::HTML);
  m_pHTMLLogWriter->BeginLog("log.html", "voxelsurfacerealtimeexp");
  ezLog::AddLogWriter(ezLog::Event::Handler(&ezLogWriter::HTML::LogMessageHandler, m_pHTMLLogWriter));
  
  // start window
  m_pWindow = EZ_DEFAULT_NEW(RenderWindowGL);

  // setup input
  SetupInput();

  // load graphics stuff
  LoadGraphicsResources();

  // reset time
  m_LastFrameTime = ezSystemTime::Now();
}

void Application::LoadGraphicsResources()
{
  // 
  m_pTestShader = EZ_DEFAULT_NEW(ShaderObject);
  m_pTestShader->AddShaderFromFile(ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_pTestShader->AddShaderFromFile(ShaderObject::ShaderType::FRAGMENT, "background.frag");
  m_pTestShader->CreateProgram();
}


void Application::BeforeEngineShutdown()
{
  ezStartup::ShutdownEngine();

  EZ_DEFAULT_DELETE(m_pTestShader);
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