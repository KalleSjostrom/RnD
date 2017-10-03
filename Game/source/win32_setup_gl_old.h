#define GET_GL_FUNCTION(name) name = (type_##name*) wglGetProcAddress(#name);
#define DECLARE_GL_FUNCTION(name) static type_##name *name;

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7

#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA

#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB8_ALPHA8                   0x8C43

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_GEOMETRY_SHADER                0x8DD9

#define GL_FRAMEBUFFER                    0x8D40
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_DEPTH_COMPONENT32F             0x8CAC

#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C

#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_MAX_SAMPLES                    0x8D57
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F

// NOTE(casey): Windows-specific
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH          0x8741

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

typedef void WINAPI type_glBindframebuffer(GLenum target, GLuint framebuffer);
typedef void WINAPI type_glGenFramebuffers(GLsizei n, GLuint *framebuffers);
typedef void WINAPI type_glAttachShader(GLuint program, GLuint shader);
typedef void WINAPI type_glCompileShader(GLuint shader);
typedef GLuint WINAPI type_glCreateProgram(void);
typedef GLuint WINAPI type_glCreateShader(GLenum type);
typedef void WINAPI type_glLinkProgram(GLuint program);
typedef void WINAPI type_glShaderSource(GLuint shader, GLsizei count, const GLchar **string, GLint *length);
typedef void WINAPI type_glUseProgram(GLuint program);
typedef void WINAPI type_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI type_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI type_glValidateProgram(GLuint program);
typedef void WINAPI type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void WINAPI type_glProgramParameteri(GLuint program, GLenum pname, GLint value);
typedef GLint WINAPI type_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void WINAPI type_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI type_glUniform1i(GLint location, GLint v0);

typedef void WINAPI type_glUniform1f(GLint location, GLfloat v0);
typedef void WINAPI type_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glEnableVertexAttribArray(GLuint index);
typedef void WINAPI type_glDisableVertexAttribArray(GLuint index);
typedef GLint WINAPI type_glGetAttribLocation(GLuint program, const GLchar *name);
typedef void WINAPI type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void WINAPI type_glBindVertexArray(GLuint array);
typedef void WINAPI type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void WINAPI type_glBindBuffer (GLenum target, GLuint buffer);
typedef void WINAPI type_glGenBuffers (GLsizei n, GLuint *buffers);
typedef void WINAPI type_glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void WINAPI type_glActiveTexture (GLenum texture);
typedef void WINAPI type_glDeleteProgram (GLuint program);
typedef void WINAPI type_glDeleteShader (GLuint shader);
typedef void WINAPI type_glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers);
typedef void WINAPI type_glDrawBuffers (GLsizei n, const GLenum *bufs);
typedef void WINAPI type_glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void WINAPI type_glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);

DECLARE_GL_FUNCTION(glBindframebuffer);
DECLARE_GL_FUNCTION(glGenFramebuffers);
DECLARE_GL_FUNCTION(glAttachShader);
DECLARE_GL_FUNCTION(glCompileShader);
DECLARE_GL_FUNCTION(glCreateProgram);
DECLARE_GL_FUNCTION(glCreateShader);
DECLARE_GL_FUNCTION(glLinkProgram);
DECLARE_GL_FUNCTION(glShaderSource);
DECLARE_GL_FUNCTION(glUseProgram);
DECLARE_GL_FUNCTION(glGetProgramInfoLog);
DECLARE_GL_FUNCTION(glGetShaderInfoLog);
DECLARE_GL_FUNCTION(glValidateProgram);
DECLARE_GL_FUNCTION(glGetProgramiv);
DECLARE_GL_FUNCTION(glProgramParameteri);
DECLARE_GL_FUNCTION(glGetUniformLocation);
DECLARE_GL_FUNCTION(glUniform4fv);
DECLARE_GL_FUNCTION(glUniformMatrix4fv);
DECLARE_GL_FUNCTION(glUniform1i);

DECLARE_GL_FUNCTION(glUniform1f);
DECLARE_GL_FUNCTION(glUniform2fv);
DECLARE_GL_FUNCTION(glUniform3fv);
DECLARE_GL_FUNCTION(glEnableVertexAttribArray);
DECLARE_GL_FUNCTION(glDisableVertexAttribArray);
DECLARE_GL_FUNCTION(glGetAttribLocation);
DECLARE_GL_FUNCTION(glVertexAttribPointer);
DECLARE_GL_FUNCTION(glBindVertexArray);
DECLARE_GL_FUNCTION(glGenVertexArrays);
DECLARE_GL_FUNCTION(glBindBuffer);
DECLARE_GL_FUNCTION(glGenBuffers);
DECLARE_GL_FUNCTION(glBufferData);
DECLARE_GL_FUNCTION(glActiveTexture);
DECLARE_GL_FUNCTION(glDeleteProgram);
DECLARE_GL_FUNCTION(glDeleteShader);
DECLARE_GL_FUNCTION(glDeleteFramebuffers);
DECLARE_GL_FUNCTION(glDrawBuffers);
DECLARE_GL_FUNCTION(glTexImage3D);
DECLARE_GL_FUNCTION(glTexSubImage3D);

void setup_gl() {
	GET_GL_FUNCTION(glBindframebuffer);
	GET_GL_FUNCTION(glGenFramebuffers);
	GET_GL_FUNCTION(glAttachShader);
	GET_GL_FUNCTION(glCompileShader);
	GET_GL_FUNCTION(glCreateProgram);
	GET_GL_FUNCTION(glCreateShader);
	GET_GL_FUNCTION(glLinkProgram);
	GET_GL_FUNCTION(glShaderSource);
	GET_GL_FUNCTION(glUseProgram);
	GET_GL_FUNCTION(glGetProgramInfoLog);
	GET_GL_FUNCTION(glGetShaderInfoLog);
	GET_GL_FUNCTION(glValidateProgram);
	GET_GL_FUNCTION(glGetProgramiv);
	GET_GL_FUNCTION(glProgramParameteri);
	GET_GL_FUNCTION(glGetUniformLocation);
	GET_GL_FUNCTION(glUniform4fv);
	GET_GL_FUNCTION(glUniformMatrix4fv);
	GET_GL_FUNCTION(glUniform1i);

	GET_GL_FUNCTION(glUniform1f);
	GET_GL_FUNCTION(glUniform2fv);
	GET_GL_FUNCTION(glUniform3fv);
	GET_GL_FUNCTION(glEnableVertexAttribArray);
	GET_GL_FUNCTION(glDisableVertexAttribArray);
	GET_GL_FUNCTION(glGetAttribLocation);
	GET_GL_FUNCTION(glVertexAttribPointer);
	GET_GL_FUNCTION(glBindVertexArray);
	GET_GL_FUNCTION(glGenVertexArrays);
	GET_GL_FUNCTION(glBindBuffer);
	GET_GL_FUNCTION(glGenBuffers);
	GET_GL_FUNCTION(glBufferData);
	GET_GL_FUNCTION(glActiveTexture);
	GET_GL_FUNCTION(glDeleteProgram);
	GET_GL_FUNCTION(glDeleteShader);
	GET_GL_FUNCTION(glDeleteFramebuffers);
	GET_GL_FUNCTION(glDrawBuffers);
	GET_GL_FUNCTION(glTexImage3D);
	GET_GL_FUNCTION(glTexSubImage3D);
}