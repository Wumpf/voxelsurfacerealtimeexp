namespace GLUtils
{
  /// classes of Debug message severity
  enum class DebugMessageSeverity
  {
    LOW,
    MEDIUM,
    HIGH
  };

  /// Activates the Opengl 4.3 debug output
  void ActivateDebugOutput(DebugMessageSeverity minMessageSeverity = DebugMessageSeverity::MEDIUM);

  /// checks if there is any OpenGL error
  ezResult CheckError(const ezString& sTitle);
}