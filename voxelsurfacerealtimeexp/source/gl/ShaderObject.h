#pragma once

#include "ShaderDataMetaInfo.h"

namespace gl
{

  /// Easy to use wrapper for OpenGL shader.
  class ShaderObject
  {
  public:
    typedef ezMap<ezString, UniformVariableInfo> GlobalUniformInfos;
    typedef ezMap<ezString, ShaderStorageBufferMetaInfo> ShaderStorageInfos;
    typedef ezMap<ezString, UniformBufferMetaInfo> UniformBlockInfos;

    enum class ShaderType
    {
      VERTEX,
      FRAGMENT,
      EVALUATION,
      CONTROL,
      GEOMETRY,
      COMPUTE,

      NUM_SHADER_TYPES
    };

    ShaderObject();
    ~ShaderObject();

    ezResult AddShaderFromFile(ShaderType type, const ezString& sFilename);
    ezResult AddShaderFromSource(ShaderType type, const ezString& pSourceCode, const ezString& sOriginName);
    ezResult CreateProgram();

    /// Returns raw gl program identifier (you know what you're doing, right?)
    GLuint GetProgram() const;

    /// Makes program active
    /// You can only activate one program at a time
    /// \todo use new pipeline stage feature instead of glUseProgram - things like that: http://www.opengl.org/wiki/GLAPI/glBindProgramPipeline
    ///         -> then you also can acitvate multiple programs at a time because they only overwrite the stages they use
    void Activate() const;

    /// The set of active user-defined inputs to the first shader stage in this program. 
    /// 
    /// If the first stage is a Vertex Shader, then this is the list of active attributes.
    /// If the program only contains a Compute Shader, then there are no inputs.
    GLint GetTotalProgramInputCount() const   { return m_iTotalProgramInputCount; }

    /// The set of active user-defined outputs from the final shader stage in this program.
    /// 
    /// If the final stage is a Fragment Shader, then this represents the fragment outputs that get written to individual color buffers.
    /// If the program only contains a Compute Shader, then there are no outputs.
    GLint GetTotalProgramOutputCount() const  { return m_iTotalProgramOutputCount; }

    /// Returns infos about global uniforms
    const GlobalUniformInfos& GetGlobalUniformInfo() const    { return m_GlobalUniformInfo; }

    /// Returns infos about used uniform buffer definitions
    const UniformBlockInfos& GetUniformBufferInfo() const    { return m_UniformBlockInfos; }

    /// Returns infos about used shader storage buffer definitions
    const ShaderStorageInfos& GetShaderStorageBufferInfo() const    { return m_ShaderStorageInfos; }


  private:
    /// Print information about the compiling step
    static void PrintShaderInfoLog(GLuint shader, const ezString& sShaderName);
    /// Print information about the linking step
    static void PrintProgramInfoLog(GLuint program);

    /// queries uniform informations from the program
    void QueryProgramInformations();

    /// Intern helper function for gather general BufferInformations
    template<typename BufferVariableType>
    void QueryBlockInformations(ezMap<ezString, BufferInfo<BufferVariableType>>& BufferToFill, GLenum InterfaceName);

    // the program itself
    GLuint m_Program;
    bool m_ContainsAssembledProgram;

    // underlying shaders
    struct Shader
    {
      GLuint    iGLShaderObject;
      ezString  sOrigin;
      bool      bLoaded;
    };
    Shader m_aShader[ShaderType::NUM_SHADER_TYPES];

    // meta information
    GlobalUniformInfos m_GlobalUniformInfo;
    UniformBlockInfos  m_UniformBlockInfos;
    ShaderStorageInfos m_ShaderStorageInfos;

    // misc
    GLint m_iTotalProgramInputCount;  ///< \see GetTotalProgramInputCount
    GLint m_iTotalProgramOutputCount; ///< \see GetTotalProgramOutputCount

    // currently missing meta information
    // - transform feedback buffer
    // - transform feedback varying
    // - subroutines
    // - atomic counter buffer
  };

}