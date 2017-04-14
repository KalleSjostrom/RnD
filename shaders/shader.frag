#version 410

in float density;

out vec4 color;

float remap(float t) {
	return t*2.0f - 1.0f;
}

void main () {
	vec2 test = vec2(remap(gl_PointCoord.s), remap(gl_PointCoord.t));
	float alpha = sqrt(test.x*test.x+test.y*test.y);

	vec3 bright = vec3(0.1176f, 0.6314f, 0.8431f) * 1.3f;
	vec3 dark = vec3(0.1137f, 0.6000f, 0.8039) * 0.8f;

	float t = (density-100) / 60.0f;

	vec3 blue = vec3(0, 0.2, 0.8);
	vec3 red = vec3(1, 0, 0);

	color = vec4(mix(blue, red, clamp(t, 0.0f, 1.0f)), (1-pow(alpha, 8.0f)) * 0.2f);
	// color = vec4(1, 1, 1, (1-pow(alpha, 8.0f)) * 0.2f);
}
/*
“const char* GL_type_to_string (GLenum type) {
  switch (type) {
    case GL_BOOL: return "bool";
    case GL_INT: return "int";
    case GL_FLOAT: return "float";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    case GL_SAMPLER_2D: return "sampler2D";
    case GL_SAMPLER_3D: return "sampler3D";
    case GL_SAMPLER_CUBE: return "samplerCube";
    case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
    default: break;
  }
  return "other";
}”

Excerpt From: Anton Gerdelan. “Anton's OpenGL 4 Tutorials.” iBooks.
“You can also "validate" a shader programme before using it. Only do this during development, because it is quite computationally expensive.”

Excerpt From: Anton Gerdelan. “Anton's OpenGL 4 Tutorials.” iBooks.

“Calling glUniform is quite expensive during run-time. Structure your programme so that glUniform is only called when the value needs to change. This might be the case every time that you draw a new object (e.g. its position might be different), but some uniforms may not change often (e.g. projection matrix).”

Excerpt From: Anton Gerdelan. “Anton's OpenGL 4 Tutorials.” iBooks.
*/