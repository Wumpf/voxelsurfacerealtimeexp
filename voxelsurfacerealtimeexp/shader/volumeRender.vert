#version 430

layout(location = 0) in uvec4 vs_in_quaddata;
layout(location = 0) out uvec4 vs_out_quaddata;

void main()
{	
	vs_out_quaddata = vs_in_quaddata;
	vs_out_quaddata.w *= 4; // expand index
}
