#pragma once

namespace gl
{
  namespace Utils
  {
    /// classes of Debug message severity
    enum class DebugMessageSeverity
    {
      LOW,
      MEDIUM,
      HIGH
    };

    /// Activates the OpenGL 4.3 debug output
    void ActivateDebugOutput(DebugMessageSeverity minMessageSeverity = DebugMessageSeverity::MEDIUM);

    /// checks if there is any OpenGL error
    ezResult CheckError(const ezString& sTitle);
  }
}