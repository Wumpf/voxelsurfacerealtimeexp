#include "PCH.h"
#include "Scene.h"
#include "gl\ScreenAlignedTriangle.h"
#include "gl\ShaderObject.h"


Scene::Scene() :
  m_pPostEffectShader(EZ_DEFAULT_NEW_UNIQUE(ShaderObject)),
  m_pComputeShaderTest(EZ_DEFAULT_NEW_UNIQUE(ShaderObject)),
  m_pScreenAlignedTriangle(EZ_DEFAULT_NEW_UNIQUE(ScreenAlignedTriangle))
{
  ezLogBlock("Scene shader init");

  m_pPostEffectShader->AddShaderFromFile(ShaderObject::ShaderType::VERTEX, "screenTri.vert");
  m_pPostEffectShader->AddShaderFromFile(ShaderObject::ShaderType::FRAGMENT, "background.frag");
  m_pPostEffectShader->CreateProgram();

  m_pComputeShaderTest->AddShaderFromFile(ShaderObject::ShaderType::COMPUTE, "comptest.comp");
  m_pComputeShaderTest->CreateProgram();

  glGenBuffers(1, &m_TestBuffer);
  float fInitalData;
  glBindBuffer(GL_ARRAY_BUFFER, m_TestBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float), &fInitalData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
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
  // render test
  m_pPostEffectShader->Activate();
  m_pScreenAlignedTriangle->display();

  // compute test
  m_pComputeShaderTest->Activate();
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_TestBuffer);
  glDispatchCompute(1, 1, 1);

  // read
  glFlush();
  float* data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
  float f = *data;
  EZ_ASSERT(f == 42, "Compute shader doesn't work as expected!");
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  

  return EZ_SUCCESS;
}
