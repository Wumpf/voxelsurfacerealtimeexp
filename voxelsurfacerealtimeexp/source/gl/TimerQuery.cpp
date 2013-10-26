#include "PCH.h"
#include "TimerQuery.h"

namespace gl
{

  TimerQuery::TimerQuery(void)
  {
    glGenQueries(1, &m_Query);
  }

  TimerQuery::~TimerQuery(void)
  {
    glDeleteQueries(1, &m_Query);
  }

  void TimerQuery::Start()
  {
    glBeginQuery(GL_TIME_ELAPSED, m_Query);
  }

  void TimerQuery::End()
  {
    glEndQuery(GL_TIME_ELAPSED);
  }

  ezTime TimerQuery::GetLastTimeElapsed()
  {
    GLuint time = 0;
    glGetQueryObjectuiv(m_Query, GL_QUERY_RESULT, &time);

    return ezTime::NanoSeconds(time);
  }
}
