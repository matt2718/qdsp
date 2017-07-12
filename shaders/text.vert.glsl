#version 330 core

layout (location = 0) in vec2 start;
layout (location = 1) in vec2 offset;
layout (location = 2) in vec2 st;

out vec2 texCoord;

uniform vec2 pixDims;
uniform vec2 charDims;

void main() {
	texCoord = st;
	vec2 sizeFrac = charDims / pixDims;
	// shift up & right by 1px to avoid interfering with grid lines
	vec2 pixShift = vec2(1.0, 1.0)/pixDims;
	gl_Position = vec4(start + sizeFrac * offset + pixShift, 0.75, 1.0);
}
