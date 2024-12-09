#version 450

in vec3 color;
layout(location = 0) out vec4 fColor;


void main() {
	fColor = vec4(color, 0.35);
}