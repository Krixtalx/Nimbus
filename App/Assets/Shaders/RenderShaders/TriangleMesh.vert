#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextCoord;

uniform mat4 mModelView;
uniform mat4 mModelViewProj;
out vec3 position;
out vec3 normal;
out vec2 textCoord;

void main ()
{
	position = (mModelView * vec4(vPosition, 1.0f)).xyz;
	normal = (mModelView * vec4(vNormal, .0f)).xyz;
	textCoord = vTextCoord;

	gl_Position = mModelViewProj * vec4(vPosition, 1.0);
}