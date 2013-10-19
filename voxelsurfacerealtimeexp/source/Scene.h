#pragma once

#include "gl\UniformBuffer.h"
#include "gl\ShaderObject.h"

// declarations
namespace gl
{
  class Font;
  class ShaderObject;
  class UniformBuffer;
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
  gl::ShaderObject m_PostEffectShader;
  gl::ShaderObject m_ComputeShaderTest;

  gl::UniformBuffer m_TestUBO;

  ezUniquePtr<gl::ScreenAlignedTriangle> m_pScreenAlignedTriangle;
  ezUniquePtr<FreeCamera> m_pCamera;

  ezUniquePtr<gl::Font> m_pFont;

  GLuint m_TestBuffer;
};

