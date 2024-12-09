#version 450

layout(location = 0) out vec4 colorFragmento;

uniform vec3 color;
void main ()
{
	colorFragmento = vec4(color, 1.0f);
}