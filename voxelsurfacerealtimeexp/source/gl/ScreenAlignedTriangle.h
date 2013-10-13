#pragma once

namespace gl
{
  // class for rendering a screen aligned triangle
  class ScreenAlignedTriangle
  {
  public:
    ScreenAlignedTriangle();
    ~ScreenAlignedTriangle();

    void display();

  private:
    GLuint vbo;
    GLuint vao;
  };
}