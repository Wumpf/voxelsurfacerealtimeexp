#include "PCH.h"
#include "AntTweakBarInterface.h"

#include "config/GlobalCVar.h"

#include <AntTweakBar.h>


static TwType TwTypeFromCVarType(ezCVarType::Enum type)
{
  TwType twType = TW_TYPE_UNDEF;
  switch (type)
  {
  case ezCVarType::Bool:
    twType = TW_TYPE_BOOLCPP;
    break;
  case ezCVarType::Int:
    twType = TW_TYPE_INT32;
    break;
  case ezCVarType::String:
    twType = TW_TYPE_CDSTRING;
    break;
  case ezCVarType::Float:
    twType = TW_TYPE_FLOAT;
    break;
  }
  return twType;
}

template<typename Type, ezCVarType::Enum CVarType>
static void SetCVarValue(ezTypedCVar<Type, CVarType>& cvar, const void *value)
{
  cvar = *reinterpret_cast<const Type*>(value);
}

template<typename Type, ezCVarType::Enum CVarType>
static void GetCVarValue(ezTypedCVar<Type, CVarType>& cvar, void *value)
{
  *reinterpret_cast<Type*>(value) = cvar;
}

#define ADD_CVAR_TO_TWEAKBAR_RW(cvar) \
  do { \
    TwSetVarCallback setFkt = [](const void *value, void *clientData) { \
      SetCVarValue(cvar, value); \
    }; \
    TwGetVarCallback getFkt = [](void *value, void *clientData) { \
      GetCVarValue(cvar, value); \
    }; \
    TwAddVarCB(m_pTweakBar, cvar.GetName(), TwTypeFromCVarType(cvar.GetType()), setFkt, getFkt, NULL, cvar.GetDescription()); \
    const char* errorDesc = TwGetLastError(); \
    if(errorDesc != NULL) ezLog::SeriousWarning(errorDesc); \
  } while(false)

#define ADD_CVAR_TO_TWEAKBAR_RO(cvar) \
  do { \
    TwGetVarCallback getFkt = [](void *value, void *clientData) { \
      GetCVarValue(cvar, value); \
    }; \
    TwAddVarCB(m_pTweakBar, cvar.GetName(), TwTypeFromCVarType(cvar.GetType()), NULL, getFkt, NULL, cvar.GetDescription()); \
    const char* errorDesc = TwGetLastError(); \
    if(errorDesc != NULL) ezLog::SeriousWarning(errorDesc); \
  } while(false)


AntTweakBarInterface::AntTweakBarInterface(void) :
   m_pTweakBar(NULL)
{
  m_szFpsInfo[0] = '\0';
  m_szFrameTimeInfo[0] = '\0';
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
  ezLogBlock("AntTweakBarInit");

  if (!TwInit(TW_OPENGL, NULL))
  {
    ezLog::Error("AntTweakBar initialization failed: %s\n", TwGetLastError());
    return EZ_FAILURE;
  }

  TwWindowSize(GeneralConfig::g_ResolutionWidth.GetValue(), GeneralConfig::g_ResolutionHeight.GetValue());

  // Create a tweak bar
  m_pTweakBar = TwNewBar("TweakBar");

  static const ezSizeU32 tweakBarSize(330, 300);
  ezStringBuilder stringBuilder;
  stringBuilder.Format(" TweakBar size='%i %i' ", tweakBarSize.width, tweakBarSize.height);
  TwDefine(stringBuilder.GetData());
  stringBuilder.Format(" TweakBar position='%i %i' ", GeneralConfig::g_ResolutionWidth - tweakBarSize.width - 10, 10);
  TwDefine(stringBuilder.GetData());

  TwDefine(" TweakBar refresh=0.2 ");
  //TwDefine(" TweakBar alpha=200 ");

  // general info
  TwAddVarRO(m_pTweakBar, "Frames per Seconds", TW_TYPE_CSSTRING(m_maxStringLength), m_szFpsInfo, "group='General Performance'");
  TwAddVarRO(m_pTweakBar, "Frame Time (ms)", TW_TYPE_CSSTRING(m_maxStringLength), m_szFrameTimeInfo, "group='General Performance'");
  
  

  // volume
  ADD_CVAR_TO_TWEAKBAR_RO(SceneConfig::Status::g_VolumePrepareTime);
  ADD_CVAR_TO_TWEAKBAR_RO(SceneConfig::Status::g_VolumeDrawTime);
  TwAddSeparator(m_pTweakBar, NULL, "group=VolumeRendering");
  ADD_CVAR_TO_TWEAKBAR_RW(SceneConfig::g_Wireframe);
  ADD_CVAR_TO_TWEAKBAR_RW(SceneConfig::g_UseReferenceVis);
  ADD_CVAR_TO_TWEAKBAR_RW(SceneConfig::g_UseAnisotropicFilter);
  ADD_CVAR_TO_TWEAKBAR_RW(SceneConfig::g_GradientDescendStepMultiplier);
  ADD_CVAR_TO_TWEAKBAR_RW(SceneConfig::g_GradientDescendStepCount);
  

  // register eventhandler
  GlobalEvents::g_pWindowMessage->AddEventHandler(ezEvent<const GlobalEvents::Win32Message&>::Handler(&AntTweakBarInterface::WindowMessageEventHandler, this));

  return EZ_SUCCESS;
}

void AntTweakBarInterface::WindowMessageEventHandler(const GlobalEvents::Win32Message& message)
{
  // puffer message to avoid recursive calling 
  m_MessageQueue.PushBack(message);
}

void AntTweakBarInterface::Render(ezTime lastFrameDuration)
{
  // unwind message buffer - this avoid recursive calls
  while(!m_MessageQueue.IsEmpty())
  {
    GlobalEvents::Win32Message message = m_MessageQueue.PeekFront();
    TwEventWin(message.wnd, message.msg, message.wParam, message.lParam);
    m_MessageQueue.PopFront();
  }

  sprintf_s(m_szFpsInfo, "%.2f fps", 1.0f / lastFrameDuration.GetSeconds());
  sprintf_s(m_szFrameTimeInfo, "%.2f ms", lastFrameDuration.GetMilliSeconds());

  TwDraw();
}