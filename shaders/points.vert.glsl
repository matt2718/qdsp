#version 330 core

uniform float xMin;
uniform float xMax;
uniform float yMin;
uniform float yMax;

uniform bool useCustom;
uniform int defaultColor;
uniform int pointSize;

layout (location = 0) in float xPos;
layout (location = 1) in float yPos;
layout (location = 2) in int customColor;

out vec3 myColor;

void main() {
	float x = 2 * (xPos - xMin) / (xMax - xMin) - 1;
	float y = 2 * (yPos - yMin) / (yMax - yMin) - 1;
	gl_Position = vec4(x, y, 0.0, 1.0);

	gl_PointSize = pointSize;

	int rgb = useCustom ? customColor : defaultColor;
	myColor = vec3((0xff & (rgb >> 16)) / 255.0,
	               (0xff & (rgb >> 8)) / 255.0,
	               (0xff & rgb) / 255.0);
}
