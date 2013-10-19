#include "PCH.h"
#include "UniformBuffer.h"
#include "GLUtils.h"

namespace gl
{

  UniformBuffer::UniformBuffer() :
    m_BufferObject(9999),
    m_uiBufferSizeBytes(0),
    m_Variables(),
    m_sBufferName(""),
    m_pBufferData(NULL)
  {
  }

  UniformBuffer::~UniformBuffer(void)
  {
    EZ_DEFAULT_DELETE(m_pBufferData);
    glDeleteBuffers(1, &m_BufferObject);
  }

  ezResult UniformBuffer::Init(ezUInt32 uiBufferSizeBytes, const ezString& sBufferName)
  {
    m_sBufferName = sBufferName;
    m_uiBufferSizeBytes = uiBufferSizeBytes;

    EZ_DEFAULT_DELETE(m_pBufferData);
    m_pBufferData = EZ_DEFAULT_NEW_RAW_BUFFER(ezInt8, uiBufferSizeBytes);
    m_uiBufferDirtyRangeEnd = m_uiBufferDirtyRangeStart = 0;

    glGenBuffers(1, &m_BufferObject);
    glBindBuffer(GL_UNIFORM_BUFFER, m_BufferObject);
    glBufferData(GL_UNIFORM_BUFFER, m_uiBufferSizeBytes, NULL, GL_DYNAMIC_DRAW);

    return gl::Utils::CheckError("glBufferData");
  }

  ezResult UniformBuffer::Init(const gl::UniformBufferMetaInfo& MetaInfo, const ezString& sBufferName)
  {
    for(auto it = MetaInfo.Variables.GetIterator(); it.IsValid(); ++it)
      m_Variables.Insert(it.Key(), Variable(&it.Value(), this));

    return Init(MetaInfo.iBufferDataSizeByte, sBufferName);
  }

  ezResult UniformBuffer::UpdateGPUData()
  {
    if(m_uiBufferDirtyRangeEnd <= m_uiBufferDirtyRangeStart)
      return EZ_SUCCESS;

    glBindBuffer(GL_UNIFORM_BUFFER, m_BufferObject);
    glBufferSubData(GL_UNIFORM_BUFFER, m_uiBufferDirtyRangeStart, m_uiBufferDirtyRangeEnd - m_uiBufferDirtyRangeStart, m_pBufferData);

    m_uiBufferDirtyRangeEnd = std::numeric_limits<ezUInt32>::min();
    m_uiBufferDirtyRangeStart = std::numeric_limits<ezUInt32>::max();

    return gl::Utils::CheckError("glBufferSubData");
  }

  ezResult UniformBuffer::BindBuffer(GLuint locationIndex)
  {
    if(UpdateGPUData() == EZ_FAILURE)
      return EZ_FAILURE;

    glBindBufferBase(GL_UNIFORM_BUFFER, locationIndex, m_BufferObject);

    return gl::Utils::CheckError("glBindBufferBase");
  }

}