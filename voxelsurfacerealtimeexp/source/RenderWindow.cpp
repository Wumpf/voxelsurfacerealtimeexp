#include "PCH.h"
#include "RenderWindow.h"
#include "gl/GLUtils.h"

#include <Core/Input/InputManager.h>
#include <InputWindows/InputDeviceWindows.h>


RenderWindowGL::RenderWindowGL() :
  ezWindow(),
  m_hDC(NULL), m_hRC(NULL)
{
  m_CreationDescription.m_Title = "avoxelsurfacerealtimeexp";
  m_CreationDescription.m_ClientAreaSize.width = 1280;
  m_CreationDescription.m_ClientAreaSize.height = 900;
  m_CreationDescription.m_bFullscreenWindow = false;

  Initialize();
  CreateGraphicsContext();
}


RenderWindowGL::~RenderWindowGL()
{
  DestroyGraphicsContext();
  Destroy();
}

void RenderWindowGL::OnWindowMessage(HWND hWnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
  ezInputDeviceWindows::GetDevice()->WindowMessage(hWnd, Msg, WParam, LParam);
}

void RenderWindowGL::DestroyGraphicsContext()
{
  if (m_hRC)
  {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(m_hRC);
    m_hRC = NULL;
  }

  if (m_hDC)
  {
    ReleaseDC(GetNativeWindowHandle(), m_hDC);
    m_hDC = NULL;
  }
}

void RenderWindowGL::CreateGraphicsContext()
{
  // init opengl device
  int iColorBits = 24;
  int iDepthBits = 24;
  int iBPC = 8;

  PIXELFORMATDESCRIPTOR pfd =
  {
    sizeof (PIXELFORMATDESCRIPTOR),
    1, // Version
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE, // Flags
    PFD_TYPE_RGBA, // Pixeltype
    iColorBits, // Color Bits
    iBPC, 0, iBPC, 0, iBPC, 0, iBPC, 0,// Red Bits / Red Shift, Green Bits / Shift, Blue Bits / Shift, Alpha Bits / Shift
    0, 0, 0, 0, 0, // Accum Bits (total), Accum Bits Red, Green, Blue, Alpha
    iDepthBits, 8, // Depth, Stencil Bits
    0, // Aux Buffers
    PFD_MAIN_PLANE, // Layer Type (ignored)
    0, 0, 0, 0 // ignored deprecated flags
  };

  HDC hDC = GetDC (GetNativeWindowHandle());

  if (hDC == NULL)
    return;

  int iPixelformat = ChoosePixelFormat (hDC, &pfd);
  if (iPixelformat == 0)
    return;

  if (!SetPixelFormat (hDC, iPixelformat, &pfd))
    return;

  HGLRC hRC = wglCreateContext (hDC);
  if (hRC == NULL)
    return;

  if (!wglMakeCurrent (hDC, hRC))
    return;

  m_hDC = hDC;
  m_hRC = hRC;


  // load using glew
  glewExperimental = TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK)
  {
    ezLog::FatalError("glewInit failed!");
  }

  // enable debug output
  GLUtils::ActivateDebugOutput();
}

void RenderWindowGL::SwapBuffers()
{
  ::SwapBuffers(m_hDC);
}
