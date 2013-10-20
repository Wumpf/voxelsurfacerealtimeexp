inline void ShaderVariable::Set(float f)            
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT, "Variable type does not match!");
  Set(&f, sizeof(f));
}
inline void ShaderVariable::Set(const ezVec2& v)    
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT_VEC2, "Variable type does not match!");
  Set(&v, sizeof(v));
}
inline void ShaderVariable::Set(const ezVec3& v)    
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT_VEC3, "Variable type does not match!");
  Set(&v, sizeof(v));
}
inline void ShaderVariable::Set(const ezVec4& v)    
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT_VEC4, "Variable type does not match!");
  Set(&v, sizeof(v));
}
inline void ShaderVariable::Set(const ezMat3& m)    
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT_MAT3, "Variable type does not match!");
  Set(&m, sizeof(m));
}
inline void ShaderVariable::Set(const ezMat4& m)    
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT_MAT4, "Variable type does not match!");
  Set(&m, sizeof(m));
}
inline void ShaderVariable::Set(double f)           
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::DOUBLE, "Variable type does not match!");
  Set(&f, sizeof(f));
}
inline void ShaderVariable::Set(const ezVec2d& v)   
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::DOUBLE_VEC2, "Variable type does not match!");
  Set(&v, sizeof(v));
}
inline void ShaderVariable::Set(const ezVec3d& v)   
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::DOUBLE_VEC3, "Variable type does not match!");
  Set(&v, sizeof(v));
}
inline void ShaderVariable::Set(const ezVec4d& v)   
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::DOUBLE_VEC4, "Variable type does not match!");
  Set(&v, sizeof(v));
}
inline void ShaderVariable::Set(const ezMat3d& m)   
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT_MAT3, "Variable type does not match!");
  Set(&m, sizeof(m));
}
inline void ShaderVariable::Set(const ezMat4d& m)   
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::FLOAT_MAT4, "Variable type does not match!");
  Set(&m, sizeof(m));
}
inline void ShaderVariable::Set(ezUInt32 ui)         
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::UNSIGNED_INT, "Variable type does not match!");
  Set(&ui, sizeof(ui));
}
inline void ShaderVariable::Set(const ezVec2U32& v) 
{
  EZ_ASSERT(m_pMetaInfo->Type == ShaderVariableType::UNSIGNED_INT_VEC2, "Variable type does not match!");
  Set(&v, sizeof(v));
}