#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 yPos;

void main() {
	gl_Position = vec4(pos.x, yPos.y, pos.z, 1.0);
}
