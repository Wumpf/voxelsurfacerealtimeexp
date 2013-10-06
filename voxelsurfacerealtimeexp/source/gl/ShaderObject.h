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
	GLuint GetProgram() const;
	void UseProgram() const;

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

