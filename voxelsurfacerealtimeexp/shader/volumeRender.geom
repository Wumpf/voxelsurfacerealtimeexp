#version 430

struct VertexInOut
{
	vec3 Normal;
	vec3 WorldPos;
};

layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in VertexInOut In[];

layout(location = 0) out VertexInOut Out;

void main()
{	
	for(int i = 0; i < 3; ++i)
	{
		gl_Position = gl_in[i].gl_Position;
		Out.Normal = In[i].Normal;
		Out.WorldPos = In[i].WorldPos;
		EmitVertex();
	}
	EndPrimitive();
}

