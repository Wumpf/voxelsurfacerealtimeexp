#include "PCH.h"
#include "ShaderObject.h"
#include "GLUtils.h"
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
      shader.m_ShaderObject = 0;
      shader.m_sOrigin = "none";
      shader.m_bLoaded = false;
    }
  }

  ShaderObject::~ShaderObject()
  {
    for(Shader& shader : m_aShader)
    {
      if(shader.m_bLoaded)
        glDeleteShader(shader.m_ShaderObject);
    }

    if(m_ContainsAssembledProgram)
      glDeleteProgram(m_Program);
  }

  ezResult ShaderObject::AddShaderFromFile(ShaderType type, const ezString& sFilename)
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


    return AddShaderFromSource(type, static_cast<ezArrayPtr<char>>(pData).GetPtr(), sFilename);
  }

  ezResult ShaderObject::AddShaderFromSource(ShaderType type, const ezString& pSourceCode, const ezString& sOriginName)
  {
    // create shader
    Shader& shader = m_aShader[static_cast<ezUInt32>(type)];
    EZ_ASSERT(!shader.m_bLoaded, "Shader already loaded!");
    shader.m_sOrigin = sOriginName;
    switch (type)
    {
    case ShaderObject::ShaderType::VERTEX:
      shader.m_ShaderObject = glCreateShader(GL_VERTEX_SHADER);
      break;
    case ShaderObject::ShaderType::FRAGMENT:
      shader.m_ShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
      break;
    case ShaderObject::ShaderType::EVALUATION:
      shader.m_ShaderObject = glCreateShader(GL_TESS_EVALUATION_SHADER);
      break;
    case ShaderObject::ShaderType::CONTROL:
      shader.m_ShaderObject = glCreateShader(GL_TESS_CONTROL_SHADER);
      break;
    case ShaderObject::ShaderType::GEOMETRY:
      shader.m_ShaderObject = glCreateShader(GL_GEOMETRY_SHADER);
      break;
    case ShaderObject::ShaderType::COMPUTE:
      shader.m_ShaderObject = glCreateShader(GL_COMPUTE_SHADER);
      break;

    default:
      EZ_ASSERT(false, "Unkown shader type");
      break;
    }


    // compile shader
    const char* pSourceRaw = pSourceCode.GetData();
    glShaderSource(shader.m_ShaderObject, 1, &pSourceRaw, NULL);	// attach shader code
    glCompileShader(shader.m_ShaderObject);								    // compile

    // check result
    ezResult result = gl::Utils::CheckError("glCompileShader");
    if(result == EZ_SUCCESS)
      shader.m_bLoaded = true;

    // log output
    PrintShaderInfoLog(m_aShader[static_cast<ezUInt32>(type)].m_ShaderObject, sOriginName);					

    return result;
  }


  ezResult ShaderObject::CreateProgram()
  {
    EZ_ASSERT(!m_ContainsAssembledProgram, "ShaderObject contains already an assembled program.");

    // Create shader program
    m_Program = glCreateProgram();	

    // attach programs
    int numAttachedShader = 0;
    for(Shader& shader : m_aShader)
    {
      if(shader.m_bLoaded)
      {
        glAttachShader(m_Program, shader.m_ShaderObject);
        ++numAttachedShader;
      }
    }
    EZ_ASSERT(numAttachedShader > 0, "Need at least one shader to link a gl program!");

    // Link program
    glLinkProgram(m_Program);

    // debug output
    PrintProgramInfoLog(m_Program);

    // check
    if(gl::Utils::CheckError("CreateProgram") == EZ_FAILURE)
      return EZ_FAILURE;
    m_ContainsAssembledProgram = true;

    // get uniform information
    // todo: store anything you can get
    // -> debugging
    // -> automatic attaching of stuff
    // http://www.opengl.org/wiki/Program_Introspection#Uniforms_and_blocks


    /* int iMaxUniformIndex;
    glGetProgramiv(m_Program, GL_ACTIVE_UNIFORMS?, &iMaxUniformIndex);
    ezDynamicArray<
    glGetUniformIndices?(m_Program?, iMaxUniformIndex, const char ** uniformNames?, GLuint *uniformIndices?);
    */

    return EZ_SUCCESS;
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

  void ShaderObject::PrintShaderInfoLog(GLuint shader, const ezString& sShaderName) const
  {
#ifdef EZ_COMPILE_FOR_DEVELOPMENT
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);		
    ezArrayPtr<char> pInfoLog = EZ_DEFAULT_NEW_ARRAY(char, infologLength);
    glGetShaderInfoLog(shader, infologLength, &charsWritten, pInfoLog.GetPtr());
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
  void ShaderObject::PrintProgramInfoLog(GLuint program) const
  {
#ifdef EZ_COMPILE_FOR_DEVELOPMENT
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);		
    ezArrayPtr<char> pInfoLog = EZ_DEFAULT_NEW_ARRAY(char, infologLength);
    glGetProgramInfoLog(program, infologLength, &charsWritten, pInfoLog.GetPtr());
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