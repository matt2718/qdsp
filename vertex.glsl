#version 330 core
layout (location = 0) in float xPos;
layout (location = 1) in float yPos;

uniform float xMin;
uniform float xMax;
uniform float yMin;
uniform float yMax;

void main() {
	float x = 2 * (xPos - xMin) / (xMax - xMin) - 1;
	float y = 2 * (yPos - yMin) / (yMax - yMin) - 1;
	gl_Position = vec4(x, y, 0.0, 1.0);
}
