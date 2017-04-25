#version 410

uniform mat4 model_view;

layout(location = 0) in vec3 positions;

void main () {
	gl_Position = model_view * vec4(positions, 1.0f);
}
