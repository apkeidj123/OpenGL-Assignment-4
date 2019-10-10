#version 410 core

uniform mat4 mvp;

layout(location = 0) in vec4 Position;

void main(void)
{

	gl_Position = mvp * Position;
}