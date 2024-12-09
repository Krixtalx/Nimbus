#version 450

layout (location = 0) in vec3 vPosition;

uniform mat4 mModelViewProj;

void main ()
{
	gl_Position = mModelViewProj * vec4(vPosition, 1.0);
}