#version 410

layout(location = 0) out vec4 color;

in VS_OUT                              
{                                          
    vec3    tc;                            
} fs_in;

uniform samplerCube tex_cubemap;


void main()
{
	color = texture(tex_cubemap, fs_in.tc);
}