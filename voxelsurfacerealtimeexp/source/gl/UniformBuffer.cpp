#include "PCH.h"
#include "UniformBuffer.h"
#include "GLUtils.h"

namespace gl
{

  UniformBuffer::UniformBuffer() :
    m_iBufferObject(9999),
    m_iBufferSizeBytes(0),
    m_Variables(),
    m_sBufferName(""),
    m_bDirty(false)
  {
  }


  UniformBuffer::~UniformBuffer(void)
  {
    glDeleteBuffers(1, &m_iBufferObject);
  }


  ezResult UniformBuffer::Init(ezUInt32 iBufferSizeBytes, const ezString& sBufferName)
  {
    m_sBufferName = sBufferName;
    m_iBufferSizeBytes = iBufferSizeBytes;

    glGenBuffers(1, &m_iBufferObject);
    glBindBuffer(GL_UNIFORM_BUFFER, m_iBufferObject);
    glBufferData(GL_UNIFORM_BUFFER, m_iBufferSizeBytes, NULL, GL_DYNAMIC_DRAW);

    return gl::Utils::CheckError("glBufferData");
  }

  ezResult UniformBuffer::Init(const gl::UniformBufferMetaInfo& MetaInfo, const ezString& sBufferName)
  {
    for(auto it = MetaInfo.Variables.GetIterator(); it.IsValid(); ++it)
      m_Variables.Insert(it.Key(), Variable(this, it.Value()));

    return Init(MetaInfo.iBufferDataSizeByte, sBufferName);
  }

  ezResult UniformBuffer::Variable::Set(const void* pData, ezUInt32 iSizeInBytes)
  {
    EZ_ASSERT(m_pUniformBuffer != NULL, "Uniform variable is not associated with a uniform buffer.");
    EZ_ASSERT(iSizeInBytes != 0, "Given size to set for uniform variable is 0.");
    EZ_ASSERT(pData != NULL, "Data to set for uniform variable is NULL.");

    /// ... TODO

    return EZ_SUCCESS;
  }

  UniformBuffer::Variable& UniformBuffer::operator[] (const ezString& sVariableName)             
  {
    EZ_ASSERT(m_Variables.Find(sVariableName).IsValid(), "There is no variable named %s in the uniform buffer \"%s\"", sVariableName, m_sBufferName);
    return m_Variables[sVariableName];
  }

}