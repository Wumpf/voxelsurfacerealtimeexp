namespace GLUtils
{
  /// Activates the Opengl 4.3 debug output
  void ActivateDebugOutput();

  /// checks if there is any OpenGL error
  ezResult CheckError(const ezString& sTitle);
}