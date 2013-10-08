#include "PCH.h"
#include "Scene.h"
#include "gl\ScreenAlignedTriangle.h"
#include "gl\ShaderObject.h"


Scene::Scene() :
  m_pPostEffectShader(EZ_DEFAULT_NEW_UNIQUE(ShaderObject)),
  m_pScreenAlignedTriangle(EZ_DEFAULT_NEW_UNIQUE(ScreenAlignedTriangle))
{
  m_pPostEffectShader->AddShaderFromFile(ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_pPostEffectShader->AddShaderFromFile(ShaderObject::ShaderType::FRAGMENT, "background.frag");
  m_pPostEffectShader->CreateProgram();
}


Scene::~Scene(void)
{
}


ezResult Scene::Update(ezTime lastFrameTime)
{
  return EZ_SUCCESS;
}

ezResult Scene::Render(ezTime lastFrameTime)
{
  m_pPostEffectShader->UseProgram();
  m_pScreenAlignedTriangle->display();

  return EZ_SUCCESS;
}
