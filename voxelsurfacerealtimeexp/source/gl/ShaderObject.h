#pragma once

namespace gl
{
  enum class ShaderVariableType
  {
    FLOAT​ = GL_FLOAT,
    FLOAT_VEC2​ = GL_FLOAT_VEC2,
    FLOAT_VEC3​ = GL_FLOAT_VEC3,
    FLOAT_VEC4​ = GL_FLOAT_VEC4,
    DOUBLE​ = GL_DOUBLE,
    DOUBLE_VEC2​ = GL_DOUBLE_VEC2,
    DOUBLE_VEC3​ = GL_DOUBLE_VEC3,
    DOUBLE_VEC4​ = GL_DOUBLE_VEC4,
    INT​ = GL_INT,
    INT_VEC2​ = GL_INT_VEC2,
    INT_VEC3​ = GL_INT_VEC3,
    INT_VEC4​ = GL_INT_VEC4,
    UNSIGNED_INT​ = GL_UNSIGNED_INT,
    UNSIGNED_INT_VEC2​ = GL_UNSIGNED_INT_VEC2,
    UNSIGNED_INT_VEC3​ = GL_UNSIGNED_INT_VEC3,
    UNSIGNED_INT_VEC4​ = GL_UNSIGNED_INT_VEC4,
    BOOL​ = GL_BOOL,
    BOOL_VEC2​ = GL_BOOL_VEC2,
    BOOL_VEC3​ = GL_BOOL_VEC3,
    BOOL_VEC4​ = GL_BOOL_VEC4,
    FLOAT_MAT2​ = GL_FLOAT_MAT2,
    FLOAT_MAT3​ = GL_FLOAT_MAT3,
    FLOAT_MAT4​ = GL_FLOAT_MAT4,
    FLOAT_MAT2x3​ = GL_FLOAT_MAT2x3,
    FLOAT_MAT2x4​ = GL_FLOAT_MAT2x4,
    FLOAT_MAT3x2​ = GL_FLOAT_MAT3x2,
    FLOAT_MAT3x4​ = GL_FLOAT_MAT3x4,
    FLOAT_MAT4x2​ = GL_FLOAT_MAT4x2,
    FLOAT_MAT4x3​ = GL_FLOAT_MAT4x3,
    DOUBLE_MAT2​ = GL_DOUBLE_MAT2,
    DOUBLE_MAT3​ = GL_DOUBLE_MAT3,
    DOUBLE_MAT4​ = GL_DOUBLE_MAT4,
    DOUBLE_MAT2x3​ = GL_DOUBLE_MAT2x3,
    DOUBLE_MAT2x4​ = GL_DOUBLE_MAT2x4,
    DOUBLE_MAT3x2​ = GL_DOUBLE_MAT3x2,
    DOUBLE_MAT3x4​ = GL_DOUBLE_MAT3x4,
    DOUBLE_MAT4x2​ = GL_DOUBLE_MAT4x2,
    DOUBLE_MAT4x3​ = GL_DOUBLE_MAT4x3,
    SAMPLER_1D​ = GL_SAMPLER_1D,
    SAMPLER_2D​ = GL_SAMPLER_2D,
    SAMPLER_3D​ = GL_SAMPLER_3D,
    SAMPLER_CUBE​ = GL_SAMPLER_CUBE,
    SAMPLER_1D_SHADOW​ = GL_SAMPLER_1D_SHADOW,
    SAMPLER_2D_SHADOW​ = GL_SAMPLER_2D_SHADOW,
    SAMPLER_1D_ARRAY​ = GL_SAMPLER_1D_ARRAY,
    SAMPLER_2D_ARRAY​ = GL_SAMPLER_2D_ARRAY,
    SAMPLER_1D_ARRAY_SHADOW​ = GL_SAMPLER_1D_ARRAY_SHADOW,
    SAMPLER_2D_ARRAY_SHADOW​ = GL_SAMPLER_2D_ARRAY_SHADOW,
    SAMPLER_2D_MULTISAMPLE​ = GL_SAMPLER_2D_MULTISAMPLE,
    SAMPLER_2D_MULTISAMPLE_ARRAY​ = GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
    SAMPLER_CUBE_SHADOW​ = GL_SAMPLER_CUBE_SHADOW,
    SAMPLER_BUFFER​ = GL_SAMPLER_BUFFER,
    SAMPLER_2D_RECT​ = GL_SAMPLER_2D_RECT,
    SAMPLER_2D_RECT_SHADOW​ = GL_SAMPLER_2D_RECT_SHADOW,
    INT_SAMPLER_1D​ = GL_INT_SAMPLER_1D,
    INT_SAMPLER_2D​ = GL_INT_SAMPLER_2D,
    INT_SAMPLER_3D​ = GL_INT_SAMPLER_3D,
    INT_SAMPLER_CUBE​ = GL_INT_SAMPLER_CUBE,
    INT_SAMPLER_1D_ARRAY​ = GL_INT_SAMPLER_1D_ARRAY,
    INT_SAMPLER_2D_ARRAY​ = GL_INT_SAMPLER_2D_ARRAY,
    INT_SAMPLER_2D_MULTISAMPLE​ = GL_INT_SAMPLER_2D_MULTISAMPLE,
    INT_SAMPLER_2D_MULTISAMPLE_ARRAY​ = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    INT_SAMPLER_BUFFER​ = GL_INT_SAMPLER_BUFFER,
    INT_SAMPLER_2D_RECT​ = GL_INT_SAMPLER_2D_RECT,
    UNSIGNED_INT_SAMPLER_1D​ = GL_UNSIGNED_INT_SAMPLER_1D,
    UNSIGNED_INT_SAMPLER_2D​ = GL_UNSIGNED_INT_SAMPLER_2D,
    UNSIGNED_INT_SAMPLER_3D​ = GL_UNSIGNED_INT_SAMPLER_3D,
    UNSIGNED_INT_SAMPLER_CUBE​ = GL_UNSIGNED_INT_SAMPLER_CUBE,
    UNSIGNED_INT_SAMPLER_1D_ARRAY​ = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,
    UNSIGNED_INT_SAMPLER_2D_ARRAY​ = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
    UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE​ = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
    UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY​ = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    UNSIGNED_INT_SAMPLER_BUFFER​ = GL_UNSIGNED_INT_SAMPLER_BUFFER,
    UNSIGNED_INT_SAMPLER_2D_RECT​ = GL_UNSIGNED_INT_SAMPLER_2D_RECT
  };

  /// Easy to use wrapper for OpenGL shader.
  class ShaderObject
  {
  public:
    template<typename VariableType>
    struct BufferInfo
    {
      ezString  sName;
      GLint     iBufferBinding;       ///< OpengGL buffer binding index
      ezInt32   iBufferDataSizeByte;  ///< Minimal buffer size in bytes

      /// Known contained variable information
      ezDynamicArray<VariableType> Variables;

      // possible, but currently missing:
      // - usage by shader stage (GL_REFERENCED_BY_..)
    };

    struct ShaderVariableInfoBase
    {
      ezString            sName;
      ShaderVariableType  Type;         ///< Type
      ezInt32             iBlockOffset; ///< Offset in corresponding buffer, -1 if there's no buffer

      ezInt32 iArrayElementCount;   ///< Number of array elements
      ezInt32 iArrayStride; ///< Stride between array elements in bytes
      
      ezInt32 iMatrixStride;  ///< stride between columns of a column-major matrix or rows of a row-major matrix
      bool    bRowMajor;

      // possible, but currently missing:
      // - usage by shader stage (GL_REFERENCED_BY_..)
    };

    struct UniformInfo : ShaderVariableInfoBase
    {
      GLint iLocation;      ///< OpenGL location, -1 if used in a buffer
      GLint iAtomicCounterbufferIndex; ///< index of active atomic counter buffer containing this variable​. -1 if this is no atomic counter buffer
    };

    struct BufferVariableInfo : ShaderVariableInfoBase
    {
      ezInt32 iTopLevelArraySize;
      ezInt32 iTopLevelArrayStride;
    };

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

  private:
    /// Print information about the compiling step
    static void PrintShaderInfoLog(GLuint shader, const ezString& sShaderName);
    /// Print information about the linking step
    static void PrintProgramInfoLog(GLuint program);

    /// queries uniform informations from the program
    void QueryProgramInformations();

    /// Intern helper function for gather general BufferInformations
    template<typename BufferVariableType>
    void QueryBlockInformations(ezDynamicArray<BufferInfo<BufferVariableType>>& BufferToFill, GLenum InterfaceName);

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
    ezDynamicArray<UniformInfo> m_GlobalUniformInfo;
    ezDynamicArray<BufferInfo<UniformInfo>>         m_UniformBlockInfos;
    ezDynamicArray<BufferInfo<BufferVariableInfo>>  m_ShaderStorageInfos;

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