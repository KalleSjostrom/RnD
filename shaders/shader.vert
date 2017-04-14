#version 410

//@ uniform
//@ passthrough matrix4x4 view = look_at(V3(0.0f, 0.0f, 20.0f), V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
//@ passthrough matrix4x4 projection = perspective_fov(100, RES_WIDTH / RES_HEIGHT, 1.0f, 100.0f);
//@ value (projection * view).m
uniform mat4 model_view;

layout(location = 0) in vec2 positions;
layout(location = 1) in vec2 density_pressure;

out float density;

void main () {
	density = density_pressure.x;
	gl_Position = model_view * vec4(positions, 0.0f, 1.0f);
}
