#pragma once

#include "gl\UniformBuffer.h"
#include "gl\ShaderObject.h"

// declarations
namespace gl
{
  class Font;
  class ShaderObject;
  class Texture;
  class UniformBuffer;
  class ScreenAlignedTriangle;
  class TimerQuery;
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
  void CreateVolumeTexture();

  gl::ShaderObject m_BackgroundShader;
  gl::ShaderObject m_DirectVolVisShader;
  gl::ShaderObject m_extractgeometryShader;

  gl::UniformBuffer m_CameraUBO;
  gl::UniformBuffer m_TimeUBO;
  gl::UniformBuffer m_VolumeInfoUBO;

  static const ezUInt32 m_uiVolumeWidth;
  static const ezUInt32 m_uiVolumeHeight;
  static const ezUInt32 m_uiVolumeDepth;
  gl::Texture* m_pVolumeTexture;
  
  ezUniquePtr<gl::TimerQuery> m_glTimer;

  ezUniquePtr<gl::ScreenAlignedTriangle> m_pScreenAlignedTriangle;
  ezUniquePtr<FreeCamera> m_pCamera;

  ezUniquePtr<gl::Font> m_pFont;

  GLuint m_TestBuffer;
};

