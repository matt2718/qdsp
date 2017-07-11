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
	gl_Position = vec4(start + sizeFrac * offset, 0.75, 1.0);
}
