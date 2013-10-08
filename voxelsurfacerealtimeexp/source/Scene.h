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
  ezUniquePtr<class ScreenAlignedTriangle> m_pScreenAlignedTriangle;
};

