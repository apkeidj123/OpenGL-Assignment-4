#version 410
#define _CRT_SECURE_NO_WARNINGS
layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform mat4 shadow_matrix;

in VertexData
{
	vec4 shadow_coord;
    vec3 N; // eye space normal
	vec3 N2;
    vec3 L; // eye space light vector
    vec3 V; // eye view vector
	vec3 V2;
    vec2 texcoord;
} vertexData;

uniform sampler2D tex;
uniform samplerCube tex_cubemap;
uniform sampler2DShadow shadow_tex;

vec3 diffuse_albedo = vec3(0.35);
vec3 specular_albedo = vec3(0.7);
float specular_power = 200.0;
uniform int shadow_index;
vec3 quad_color = vec3(0.41, 0.36, 0.37);
vec3 shadow_color = vec3(0.64, 0.57, 0.49);

void main()
{
	//blinn phong
	vec3 N = normalize(vertexData.N);
	vec3 L = normalize(vertexData.L);
	vec3 V = normalize(vertexData.V);
	vec3 H = normalize(L + V);

	vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
	vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;
	vec4 bcolor = vec4(diffuse + specular, 1.0);

	//environment map
	vec3 r = reflect(vertexData.V2, normalize(vertexData.N2));
	vec4 ecolor = texture(tex_cubemap, r) ;//*vec4(0.95, 0.80, 0.45, 1.0);

    //vec3 texColor = texture(tex,vertexData.texcoord).rgb;
    //fragColor = vec4(texColor, 1.0);
	//fragColor =  0.65 * bcolor + 0.35 * ecolor;
	//fragColor += 0.35 * ecolor;

	//shadow
	float shadow_factor = textureProj(shadow_tex, vertexData.shadow_coord);
	//float shadow_factor = 0.3;
	//fragColor *= shadow_factor;

	vec4 becolor = 0.65 * bcolor + 0.35 * ecolor;

	//fragColor = becolor ;

	///*
	if (shadow_index == 0)
	{
		fragColor =  becolor ;
		//fragColor = becolor * shadow_factor;
	}
	else if (shadow_index == 1) 
	{
		if (shadow_factor > 0.5) 
		{
			fragColor = vec4(quad_color, 1.0);
		}
		else 
		{
			fragColor = vec4(shadow_color, 1.0);
		}	
	}
	
	//*/
	

}