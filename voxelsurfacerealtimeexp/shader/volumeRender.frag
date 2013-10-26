#version 430

struct vertex
{
	vec4 Color;
};

layout(location = 0) in vertex In;
layout(location = 0, index = 0) out vec4 FragColor;

void main()
{
	FragColor = In.Color;
}
