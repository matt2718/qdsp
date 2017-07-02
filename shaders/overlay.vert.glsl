#version 330 core
layout (location = 0) in vec3 pos;

out vec2 texCoord;

uniform vec2 pixDims;
uniform vec2 imgDims;

void main() {
	texCoord = pos.xy;
	
	vec4 scale = vec4(2 * imgDims / pixDims, 1, 1);
	vec4 anchor = vec4(-1, 1, 0, 0);
	gl_Position = anchor + scale * vec4(pos.x, -pos.y, pos.z, 1);
}
