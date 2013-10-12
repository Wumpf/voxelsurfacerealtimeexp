#pragma once

/// Easy to use wrapper for OpenGL shader.
class ShaderObject
{
public:
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

private:
	// Print information about the compiling step
	void PrintShaderInfoLog(GLuint shader, const ezString& sShaderName) const;
	// Print information about the linking step
	void PrintProgramInfoLog(GLuint program) const;

	GLuint m_Program;
  bool m_ContainsAssembledProgram;

  struct Shader
  {
    GLuint m_ShaderObject;
    ezString m_sOrigin;
    bool m_bLoaded;
  };

  Shader m_aShader[ShaderType::NUM_SHADER_TYPES];
};

