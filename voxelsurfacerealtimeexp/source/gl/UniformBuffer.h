#pragma once

#include "ShaderDataMetaInfo.h"

namespace gl
{
  class UniformBuffer
  {
  public:
    UniformBuffer();
    ~UniformBuffer();

    ezResult Init(ezUInt32 iBufferSizeBytes, const ezString& sBufferName);
    ezResult Init(const gl::UniformBufferMetaInfo& MetaInfo, const ezString& sBufferName);

    class Variable
    {
    public:
      Variable() : m_pUniformBuffer(NULL) {}

      ezResult Set(const void* pData, ezUInt32 iSizeInBytes);

      ezResult Set(float f)           { m_MetaInfo.Type == ShaderVariableType::FLOAT ? Set(&f, sizeof(float)) : EZ_FAILURE; }
      ezResult Set(const ezVec2& v)   { m_MetaInfo.Type == ShaderVariableType::FLOAT_VEC2 ? Set(&v, sizeof(ezVec2)) : EZ_FAILURE; }
      ezResult Set(const ezVec3& v)   { m_MetaInfo.Type == ShaderVariableType::FLOAT_VEC3 ? Set(&v, sizeof(ezVec3)) : EZ_FAILURE; }
      ezResult Set(const ezVec4& v)   { m_MetaInfo.Type == ShaderVariableType::FLOAT_VEC4 ? Set(&v, sizeof(ezVec4)) : EZ_FAILURE; }
      ezResult Set(const ezMat3& m)   { m_MetaInfo.Type == ShaderVariableType::FLOAT_MAT3 ? Set(&m, sizeof(ezMat3)) : EZ_FAILURE; }
      ezResult Set(const ezMat4& m)   { m_MetaInfo.Type == ShaderVariableType::FLOAT_MAT4 ? Set(&m, sizeof(ezMat4)) : EZ_FAILURE; }

      ezResult Set(double f)          { m_MetaInfo.Type == ShaderVariableType::DOUBLE ? Set(&f, sizeof(double)) : EZ_FAILURE; }
      ezResult Set(const ezVec2d& v);
      ezResult Set(const ezVec3d& v);
      ezResult Set(const ezVec4d& v);
      ezResult Set(const ezMat3d& m);
      ezResult Set(const ezMat4d& m);

      ezResult Set(ezUInt32 f);
      ezResult Set(const ezVec2U32& v);

      // add more type implementations here if necessary
      
      const gl::UniformVariableInfo& GetMetaInfo() const { return m_MetaInfo; }

    private:
      friend UniformBuffer;

      Variable(UniformBuffer* pUniformBuffer, const gl::UniformVariableInfo& MetaInfo) : 
        m_pUniformBuffer(pUniformBuffer), m_MetaInfo(MetaInfo) {}

      UniformBuffer* m_pUniformBuffer;
      gl::UniformVariableInfo m_MetaInfo;
    };

    bool ContainsVariable(const ezString& sVariableName) const       { return m_Variables.Find(sVariableName).IsValid(); }
    Variable& operator[] (const ezString& sVariableName);

  private:
    friend Variable;

    GLuint      m_iBufferObject;
    ezUInt32    m_iBufferSizeBytes;
    ezString    m_sBufferName;

    bool m_bDirty;

    /// meta information
    ezMap<ezString, Variable> m_Variables;  // \todo no ezHashTable possible?
  };

}

