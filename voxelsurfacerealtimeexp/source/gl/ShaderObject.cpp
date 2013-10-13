#include "PCH.h"
#include "ShaderObject.h"
#include "GLUtils.h"

#include <GL/glew.h>

#include <Foundation/io/FileSystem/FileReader.h>
#include <Foundation/io/OSFile.h>
#include <Foundation/Basics/Types/ArrayPtr.h>

namespace gl
{

  ShaderObject::ShaderObject() :
    m_Program(0),
    m_ContainsAssembledProgram(false)
  {
    for(Shader& shader : m_aShader)
    {
      shader.iGLShaderObject = 0;
      shader.sOrigin = "none";
      shader.bLoaded = false;
    }
  }

  ShaderObject::~ShaderObject()
  {
    for(Shader& shader : m_aShader)
    {
      if(shader.bLoaded)
        glDeleteShader(shader.iGLShaderObject);
    }

    if(m_ContainsAssembledProgram)
      glDeleteProgram(m_Program);
  }

  ezResult ShaderObject::AddShaderFromFile(ShaderType Type, const ezString& sFilename)
  {
    // open file
    ezFileReader file;
    if(file.Open(sFilename.GetData(), ezFileMode::Read) != EZ_SUCCESS)
    {
      ezLog::Error("Unable to open shader file %s", sFilename.GetData());
      return EZ_FAILURE;
    }

    // read file
    ezUInt64 uiFileSize = file.GetFileSize();
    ezDynamicArray<char> pData;
    pData.SetCount(static_cast<ezUInt32>(uiFileSize)+1);
    ezUInt64 uiReadBytes = file.ReadBytes(static_cast<ezArrayPtr<char>>(pData).GetPtr(), uiFileSize);
    EZ_ASSERT(uiReadBytes == uiFileSize, "FileSize does not matches number of bytes read.");
    pData[static_cast<ezUInt32>(uiFileSize)] = '\0';
    file.Close();


    return AddShaderFromSource(Type, static_cast<ezArrayPtr<char>>(pData).GetPtr(), sFilename);
  }

  ezResult ShaderObject::AddShaderFromSource(ShaderType type, const ezString& pSourceCode, const ezString& sOriginName)
  {
    // create shader
    Shader& shader = m_aShader[static_cast<ezUInt32>(type)];
    EZ_ASSERT(!shader.bLoaded, "Shader already loaded!");
    shader.sOrigin = sOriginName;
    switch (type)
    {
    case ShaderObject::ShaderType::VERTEX:
      shader.iGLShaderObject = glCreateShader(GL_VERTEX_SHADER);
      break;
    case ShaderObject::ShaderType::FRAGMENT:
      shader.iGLShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
      break;
    case ShaderObject::ShaderType::EVALUATION:
      shader.iGLShaderObject = glCreateShader(GL_TESS_EVALUATION_SHADER);
      break;
    case ShaderObject::ShaderType::CONTROL:
      shader.iGLShaderObject = glCreateShader(GL_TESS_CONTROL_SHADER);
      break;
    case ShaderObject::ShaderType::GEOMETRY:
      shader.iGLShaderObject = glCreateShader(GL_GEOMETRY_SHADER);
      break;
    case ShaderObject::ShaderType::COMPUTE:
      shader.iGLShaderObject = glCreateShader(GL_COMPUTE_SHADER);
      break;

    default:
      EZ_ASSERT(false, "Unkown shader type");
      break;
    }


    // compile shader
    const char* pSourceRaw = pSourceCode.GetData();
    glShaderSource(shader.iGLShaderObject, 1, &pSourceRaw, NULL);	// attach shader code
    glCompileShader(shader.iGLShaderObject);								    // compile

    // check result
    ezResult result = gl::Utils::CheckError("glCompileShader");
    if(result == EZ_SUCCESS)
      shader.bLoaded = true;

    // log output
   // PrintShaderInfoLog(m_aShader[static_cast<ezUInt32>(type)].m_ShaderObject, sOriginName);					

    return result;
  }


  ezResult ShaderObject::CreateProgram()
  {
    EZ_ASSERT(!m_ContainsAssembledProgram, "ShaderObject contains already an assembled program.");

    // clear meta information
    m_iTotalProgramInputCount = 0;
    m_iTotalProgramOutputCount = 0;
    m_GlobalUniformInfo.Clear();
    m_UniformBlockInfos.Clear();
    m_ShaderStorageInfos.Clear();

    // Create shader program
    m_Program = glCreateProgram();	

    // attach programs
    int numAttachedShader = 0;
    for(Shader& shader : m_aShader)
    {
      if(shader.bLoaded)
      {
        glAttachShader(m_Program, shader.iGLShaderObject);
        ++numAttachedShader;
      }
    }
    EZ_ASSERT(numAttachedShader > 0, "Need at least one shader to link a gl program!");

    // Link program
    glLinkProgram(m_Program);

    // check
    if(gl::Utils::CheckError("CreateProgram") == EZ_FAILURE)
      return EZ_FAILURE;
    m_ContainsAssembledProgram = true;

    // debug output
    PrintProgramInfoLog(m_Program);

    // get informations about the program
    QueryProgramInformations();

    return EZ_SUCCESS;
  }

  void ShaderObject::QueryProgramInformations()
  {
    // query basic uniform & shader storage block infos
    QueryBlockInformations(m_UniformBlockInfos, GL_UNIFORM_BLOCK);
    QueryBlockInformations(m_ShaderStorageInfos, GL_SHADER_STORAGE_BLOCK);

    // informations about uniforms ...
    GLint iTotalNumUniforms = 0;
    glGetProgramInterfaceiv(m_Program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &iTotalNumUniforms);
    const GLuint iNumQueriedUniformProps = 10;
    const GLenum pQueriedUniformProps[] = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_ATOMIC_COUNTER_BUFFER_INDEX, GL_LOCATION  };
    GLint pRawUniformBlockInfoData[iNumQueriedUniformProps];
    for(int iBlock=0; iBlock<iTotalNumUniforms; ++iBlock)
    {
      // general data
      glGetProgramResourceiv(m_Program, GL_UNIFORM, iBlock, iNumQueriedUniformProps, pQueriedUniformProps, iNumQueriedUniformProps, NULL, pRawUniformBlockInfoData);
      UniformInfo uniformInfo;
      uniformInfo.Type = static_cast<gl::ShaderVariableType>(pRawUniformBlockInfoData[1]);
      uniformInfo.iArrayElementCount = static_cast<ezInt32>(pRawUniformBlockInfoData[2]);
      uniformInfo.iBlockOffset = static_cast<ezInt32>(pRawUniformBlockInfoData[3]);
      uniformInfo.iArrayStride = static_cast<ezInt32>(pRawUniformBlockInfoData[5]) * 4;
      uniformInfo.iMatrixStride = static_cast<ezInt32>(pRawUniformBlockInfoData[6]);
      uniformInfo.bRowMajor = pRawUniformBlockInfoData[7] > 0;
      uniformInfo.iAtomicCounterbufferIndex = pRawUniformBlockInfoData[8];
      uniformInfo.iLocation = pRawUniformBlockInfoData[9];

      // name
      GLint iActualNameLength = 0;
      ezHybridArray<char, 64> RawName;
      RawName.SetCount(pRawUniformBlockInfoData[0] + 1);
      glGetProgramResourceName(m_Program, GL_UNIFORM, iBlock, RawName.GetCount(), &iActualNameLength, static_cast<ezArrayPtr<char>>(RawName).GetPtr());
      RawName[iActualNameLength] = '\0';
      uniformInfo.sName = static_cast<ezArrayPtr<char>>(RawName).GetPtr();

      // where to store:
      if(pRawUniformBlockInfoData[4] < 0)
        m_GlobalUniformInfo.PushBack(uniformInfo);
      else
        m_UniformBlockInfos[pRawUniformBlockInfoData[4]].Variables.PushBack(uniformInfo);
    }

    // informations about shader storage variables 
    GLint iTotalNumStorages = 0;
    glGetProgramInterfaceiv(m_Program, GL_BUFFER_VARIABLE, GL_ACTIVE_RESOURCES, &iTotalNumStorages);
    const GLuint iNumQueriedStorageProps = 10;
    const GLenum pQueriedStorageProps[] = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE };
    GLint pRawStorageBlockInfoData[iNumQueriedStorageProps];
    for(int iBlock=0; iBlock<iTotalNumStorages; ++iBlock)
    {
      // general data
      glGetProgramResourceiv(m_Program, GL_BUFFER_VARIABLE, iBlock, iNumQueriedStorageProps, pQueriedStorageProps, iNumQueriedStorageProps, NULL, pRawStorageBlockInfoData);
      BufferVariableInfo storageInfo;
      storageInfo.Type = static_cast<gl::ShaderVariableType>(pRawStorageBlockInfoData[1]);
      storageInfo.iArrayElementCount = static_cast<ezInt32>(pRawStorageBlockInfoData[2]);
      storageInfo.iBlockOffset = static_cast<ezInt32>(pRawStorageBlockInfoData[3]);
      storageInfo.iArrayStride = static_cast<ezInt32>(pRawStorageBlockInfoData[5]);
      storageInfo.iMatrixStride = static_cast<ezInt32>(pRawStorageBlockInfoData[6]);
      storageInfo.bRowMajor = pRawStorageBlockInfoData[7] > 0;
      storageInfo.iTopLevelArraySize = pRawStorageBlockInfoData[8];
      storageInfo.iTopLevelArrayStride = pRawStorageBlockInfoData[9];

      // name
      GLint iActualNameLength = 0;
      ezHybridArray<char, 64> RawName;
      RawName.SetCount(pRawStorageBlockInfoData[0] + 1);
      glGetProgramResourceName(m_Program, GL_BUFFER_VARIABLE, iBlock, RawName.GetCount(), &iActualNameLength, static_cast<ezArrayPtr<char>>(RawName).GetPtr());
      RawName[iActualNameLength] = '\0';
      storageInfo.sName = static_cast<ezArrayPtr<char>>(RawName).GetPtr();

      // where to store:
       m_ShaderStorageInfos[pRawStorageBlockInfoData[4]].Variables.PushBack(storageInfo);
    }

    // other informations
    glGetProgramInterfaceiv(m_Program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &m_iTotalProgramInputCount);
    glGetProgramInterfaceiv(m_Program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &m_iTotalProgramOutputCount);
  }

  template<typename BufferVariableType>
  void ShaderObject::QueryBlockInformations(ezDynamicArray<BufferInfo<BufferVariableType>>& BufferToFill, GLenum InterfaceName)
  {
    BufferToFill.Clear();

    GLint iTotalNumBlocks = 0;
    glGetProgramInterfaceiv(m_Program, InterfaceName, GL_ACTIVE_RESOURCES, &iTotalNumBlocks);
    BufferToFill.SetCount(iTotalNumBlocks);

    // gather infos about uniform blocks
    const GLuint iNumQueriedBlockProps = 4;
    const GLenum pQueriedBlockProps[] = { GL_NAME_LENGTH, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES };
    GLint pRawUniformBlockInfoData[iNumQueriedBlockProps];
    for(int iBlock=0; iBlock<iTotalNumBlocks; ++iBlock)
    {
      // general data
      glGetProgramResourceiv(m_Program, InterfaceName, iBlock, iNumQueriedBlockProps, pQueriedBlockProps, iNumQueriedBlockProps, NULL, pRawUniformBlockInfoData);
      BufferToFill[iBlock].iBufferBinding = pRawUniformBlockInfoData[1];
      BufferToFill[iBlock].iBufferDataSizeByte = pRawUniformBlockInfoData[2] * sizeof(float);
      BufferToFill[iBlock].Variables.Reserve(pRawUniformBlockInfoData[3]);

      // name
      GLint iActualNameLength = 0;
      ezHybridArray<char, 64> RawName;
      RawName.SetCount(pRawUniformBlockInfoData[0] + 1);
      glGetProgramResourceName(m_Program, InterfaceName, iBlock, RawName.GetCount(), &iActualNameLength, static_cast<ezArrayPtr<char>>(RawName).GetPtr());
      RawName[iActualNameLength] = '\0';
      BufferToFill[iBlock].sName = static_cast<ezArrayPtr<char>>(RawName).GetPtr();
    }
  }

  GLuint ShaderObject::GetProgram() const
  {
    EZ_ASSERT(m_ContainsAssembledProgram, "No shader program ready yet. Call CreateProgram first!");
    return m_Program;
  }

  void ShaderObject::Activate() const 	
  {
    EZ_ASSERT(m_ContainsAssembledProgram, "No shader program ready yet. Call CreateProgram first!");
    glUseProgram(m_Program);
  }

  void ShaderObject::PrintShaderInfoLog(GLuint Shader, const ezString& sShaderName)
  {
#ifdef EZ_COMPILE_FOR_DEVELOPMENT
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;

    glGetShaderiv(Shader, GL_INFO_LOG_LENGTH,&infologLength);		
    ezArrayPtr<char> pInfoLog = EZ_DEFAULT_NEW_ARRAY(char, infologLength);
    glGetShaderInfoLog(Shader, infologLength, &charsWritten, pInfoLog.GetPtr());
    pInfoLog[charsWritten] = '\0';
    if(strlen(pInfoLog.GetPtr()) > 0)
    {
      ezLog::Error("Shader %s compiled. Output:", sShaderName.GetData());
      ezLog::Error(pInfoLog.GetPtr());
    }
    else
      ezLog::Dev("Shader %s compiled successfully", sShaderName.GetData());

    EZ_DEFAULT_DELETE_ARRAY(pInfoLog);
#endif
  }

  // Print information about the linking step
  void ShaderObject::PrintProgramInfoLog(GLuint Program)
  {
#ifdef EZ_COMPILE_FOR_DEVELOPMENT
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;

    glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &infologLength);		
    ezArrayPtr<char> pInfoLog = EZ_DEFAULT_NEW_ARRAY(char, infologLength);
    glGetProgramInfoLog(Program, infologLength, &charsWritten, pInfoLog.GetPtr());
    pInfoLog[charsWritten] = '\0';

    if(strlen(pInfoLog.GetPtr()) > 0)
    {
      ezLog::Error("Compiling program. Output:");
      ezLog::Error(pInfoLog.GetPtr());
    }
    else
      ezLog::Dev("Compiled program successfully");

    EZ_DEFAULT_DELETE_ARRAY(pInfoLog);
#endif
  }

}