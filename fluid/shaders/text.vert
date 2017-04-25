#version 410

uniform mat4 projection;

layout(location=0) in vec2 position;
layout(location=1) in vec2 vertex_uv;

out vec2 uv;

void main() {
	gl_Position = projection * vec4(position, 0.0f, 1.0f);
	uv = vertex_uv;
}