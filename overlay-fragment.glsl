#version 330 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D myTex;

void main() {
	FragColor = vec4(texture(myTex, texCoord).xyz, 0.5);
}
