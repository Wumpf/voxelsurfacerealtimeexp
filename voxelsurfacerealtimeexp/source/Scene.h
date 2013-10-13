#pragma once

// declarations
namespace gl
{
  class Font;
  class ShaderObject;
  class ScreenAlignedTriangle;
};
class FreeCamera;

/// The scene where all the rendering stuff happens
class Scene
{
public:
  Scene(const class RenderWindowGL& renderWindow);
  ~Scene();
  
  ezResult Update(ezTime lastFrameDuration);
  ezResult Render(ezTime lastFrameDuration);

private:
  ezUniquePtr<gl::ShaderObject> m_pPostEffectShader;
  ezUniquePtr<gl::ShaderObject> m_pComputeShaderTest;

  ezUniquePtr<gl::ScreenAlignedTriangle> m_pScreenAlignedTriangle;
  ezUniquePtr<FreeCamera> m_pCamera;

  ezUniquePtr<gl::Font> m_pFont;

  GLuint m_TestBuffer;
};

