#include "PCH.h"
#include "OnScreenLogWriter.h"

#include "RenderWindow.h"
#include "gl/Font.h"
#include "GlobalCVar.h"

const ezVec2 OnScreenLogWriter::m_vScreenPos(10.0f, 20.0f);
const float OnScreenLogWriter::m_fFadeSpeed = 0.25f;

OnScreenLogWriter::OnScreenLogWriter(const RenderWindowGL& renderWindow) :
  m_pFont(EZ_DEFAULT_NEW_UNIQUE(gl::Font, "Courier New", 16, renderWindow.GetDeviceContext()))
{
}


OnScreenLogWriter::~OnScreenLogWriter(void)
{
}

void OnScreenLogWriter::LogMessageHandler(const ezLoggingEventData& eventData)
{
  // ignore groups
  if(eventData.m_EventType == ezLogMsgType::BeginGroup)
  {
    m_sCurrentGroup = eventData.m_szText;
    return;
  }
  if(eventData.m_EventType == ezLogMsgType::EndGroup)
  {
    m_sCurrentGroup = "";
    return;
  }

  ezStringBuilder logtext;
  if(m_sCurrentGroup != "")
  {
    logtext.Append("[");
    logtext.Append(m_sCurrentGroup.GetData());
    logtext.Append("] ");
  }

  //for(ezUInt32 i=0; i<eventData.m_uiIndentation; ++i)
  //  logtext.Append(" - ");
  logtext.Append(eventData.m_szText);

  LogEntry newEntry;
  newEntry.text = logtext.GetData();
  switch(eventData.m_EventType)
  {
  case ezLogMsgType::InfoMsg:
    newEntry.color = ezColor::Grey;
    break;
  case ezLogMsgType::SeriousWarningMsg:
  case ezLogMsgType::WarningMsg:
    newEntry.color = ezColor(1.0f, 0.7f, 0.1f);
    break;
  case ezLogMsgType::ErrorMsg:
    newEntry.color = ezColor::Red;
    break;
  case ezLogMsgType::SuccessMsg:
    newEntry.color = ezColor::Green;
    break;
  default:
    newEntry.color = ezColor::White;
    break;
  }
  if(!m_MessageBuffer.CanAppend())
  {
    m_fOldestItemFade = 1.0f;
    m_MessageBuffer.PopFront();
  }
  m_MessageBuffer.PushBack(newEntry);
}

ezResult OnScreenLogWriter::Update( ezTime lastFrameDuration )
{
  if(!m_MessageBuffer.IsEmpty())
  {
    m_fOldestItemFade -= static_cast<float>(lastFrameDuration.GetSeconds() * m_fFadeSpeed);
    if(m_fOldestItemFade < 0)
    {
      m_fOldestItemFade = 1.0f;
      m_MessageBuffer.PopFront();
    }
  }

  return EZ_SUCCESS;
}

ezResult OnScreenLogWriter::Render()
{
  ezVec2 vCurScreenPos = m_vScreenPos;
  for(int i=m_MessageBuffer.GetCount()-1; i>=0; --i)
  {
    ezColor color = m_MessageBuffer[i].color;
    if(i == 0)
      color.A() = m_fOldestItemFade;
    m_pFont->DrawString(m_MessageBuffer[i].text, vCurScreenPos.CompDiv(GeneralConfig::GetScreenResolutionF()), color);
    vCurScreenPos.y += 15.0f;
  }

  return EZ_SUCCESS;
}
