#pragma once
class Scene
{
public:
  Scene();
  ~Scene();
  
  ezResult Update(ezTime lastFrameTime);
  ezResult Render(ezTime lastFrameTime);

private:
  ezUniquePtr<class ShaderObject> m_pPostEffectShader;
  ezUniquePtr<class ShaderObject> m_pComputeShaderTest;

  ezUniquePtr<class ScreenAlignedTriangle> m_pScreenAlignedTriangle;
 
  GLuint m_TestBuffer;
};

