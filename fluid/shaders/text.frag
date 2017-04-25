#version 410

uniform sampler2D sampler;
uniform vec4 color;

in vec2 uv;
out vec4 result_color;

void main() {
    vec4 test = texture(sampler, vec2(uv.x, uv.y));
	result_color = vec4(color.r, color.g, color.b, test.r * color.a);
}