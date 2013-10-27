#pragma once

#include "gl/ShaderObject.h"

namespace gl
{
   class ScreenAlignedTriangle;
};

class Background
{
public:
  Background(const gl::ScreenAlignedTriangle* pScreenAlignedTriangle);
  ~Background();

  void Draw();

  const gl::ShaderObject& GetShader() { return m_BackgroundShader; }

private:
  const gl::ScreenAlignedTriangle* m_pScreenAlignedTriangle; // todo: sharedptr
  gl::ShaderObject m_BackgroundShader;
};

