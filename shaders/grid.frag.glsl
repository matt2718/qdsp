#version 330 core

out vec4 FragColor;

uniform vec4 xColor;
uniform vec4 yColor;
uniform int useY;

void main() {
	FragColor = (useY != 0) ? yColor : xColor;
}
