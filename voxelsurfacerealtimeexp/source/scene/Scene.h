#pragma once

#include "gl\UniformBuffer.h"
#include "gl\ShaderObject.h"

// declarations
namespace gl
{
  class Font;
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

  void DrawInfos(ezTime &lastFrameDuration);

private:
  void CreateVolumeTexture();

  gl::UniformBuffer m_CameraUBO;
  gl::UniformBuffer m_TimeUBO;
  gl::UniformBuffer m_GlobalSceneInfo;

  class VoxelTerrain* m_pVoxelTerrain;
  class Background* m_pBackground;

  ezUniquePtr<gl::TimerQuery> m_ExtractGeometryTimer;
  ezUniquePtr<gl::TimerQuery> m_DrawTimer;

  ezUniquePtr<gl::ScreenAlignedTriangle> m_pScreenAlignedTriangle;
  ezUniquePtr<FreeCamera> m_pCamera;

  ezUniquePtr<gl::Font> m_pFont;
};

