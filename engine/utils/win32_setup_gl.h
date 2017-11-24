#include "engine/include/SDL_opengl_glext.h"

#define GET_GL_FUNCTION_SDL(name_capitalized, name) name = (PFN ## name_capitalized ## PROC) wglGetProcAddress(#name);
#define DECLARE_GL_FUNCTION_SDL(name_capitalized, name) static PFN ## name_capitalized ## PROC name

DECLARE_GL_FUNCTION_SDL(GLCREATESHADER, glCreateShader);
DECLARE_GL_FUNCTION_SDL(GLSHADERSOURCE, glShaderSource);
DECLARE_GL_FUNCTION_SDL(GLCOMPILESHADER, glCompileShader);
DECLARE_GL_FUNCTION_SDL(GLGETSHADERINFOLOG, glGetShaderInfoLog);
DECLARE_GL_FUNCTION_SDL(GLCREATEPROGRAM, glCreateProgram);
DECLARE_GL_FUNCTION_SDL(GLATTACHSHADER, glAttachShader);
DECLARE_GL_FUNCTION_SDL(GLPROGRAMPARAMETERI, glProgramParameteri);
DECLARE_GL_FUNCTION_SDL(GLLINKPROGRAM, glLinkProgram);
DECLARE_GL_FUNCTION_SDL(GLGETPROGRAMIV, glGetProgramiv);
DECLARE_GL_FUNCTION_SDL(GLGETPROGRAMINFOLOG, glGetProgramInfoLog);
DECLARE_GL_FUNCTION_SDL(GLVALIDATEPROGRAM, glValidateProgram);
DECLARE_GL_FUNCTION_SDL(GLGETPROGRAMBINARY, glGetProgramBinary);
DECLARE_GL_FUNCTION_SDL(GLUNIFORMMATRIX4FV, glUniformMatrix4fv);
DECLARE_GL_FUNCTION_SDL(GLGENBUFFERS, glGenBuffers);
DECLARE_GL_FUNCTION_SDL(GLBINDBUFFER, glBindBuffer);
DECLARE_GL_FUNCTION_SDL(GLBUFFERDATA, glBufferData);
DECLARE_GL_FUNCTION_SDL(GLMAPBUFFER, glMapBuffer);
DECLARE_GL_FUNCTION_SDL(GLUNMAPBUFFER, glUnmapBuffer);
DECLARE_GL_FUNCTION_SDL(GLGENVERTEXARRAYS, glGenVertexArrays);
DECLARE_GL_FUNCTION_SDL(GLBINDVERTEXARRAY, glBindVertexArray);
DECLARE_GL_FUNCTION_SDL(GLGETATTRIBLOCATION, glGetAttribLocation);
DECLARE_GL_FUNCTION_SDL(GLVERTEXATTRIBPOINTER, glVertexAttribPointer);
DECLARE_GL_FUNCTION_SDL(GLENABLEVERTEXATTRIBARRAY, glEnableVertexAttribArray);
DECLARE_GL_FUNCTION_SDL(GLUSEPROGRAM, glUseProgram);
DECLARE_GL_FUNCTION_SDL(GLGETUNIFORMLOCATION, glGetUniformLocation);
DECLARE_GL_FUNCTION_SDL(GLGENFRAMEBUFFERS, glGenFramebuffers);
DECLARE_GL_FUNCTION_SDL(GLBINDFRAMEBUFFER, glBindFramebuffer);
DECLARE_GL_FUNCTION_SDL(GLFRAMEBUFFERTEXTURE2D, glFramebufferTexture2D);
DECLARE_GL_FUNCTION_SDL(GLDRAWBUFFERS, glDrawBuffers);
DECLARE_GL_FUNCTION_SDL(GLCHECKFRAMEBUFFERSTATUS, glCheckFramebufferStatus);
DECLARE_GL_FUNCTION_SDL(GLTEXIMAGE2DMULTISAMPLE, glTexImage2DMultisample);
DECLARE_GL_FUNCTION_SDL(GLBLITFRAMEBUFFER, glBlitFramebuffer);
DECLARE_GL_FUNCTION_SDL(GLGENERATEMIPMAP, glGenerateMipmap);

DECLARE_GL_FUNCTION_SDL(GLUNIFORM1I,  glUniform1i);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM1F,  glUniform1f);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM1FV, glUniform1fv);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM2F,  glUniform2f);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM2FV, glUniform2fv);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM3F,  glUniform3f);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM3FV, glUniform3fv);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM4F,  glUniform4f);
DECLARE_GL_FUNCTION_SDL(GLUNIFORM4FV, glUniform4fv);

DECLARE_GL_FUNCTION_SDL(GLACTIVETEXTURE, glActiveTexture);
DECLARE_GL_FUNCTION_SDL(GLBLENDEQUATIONSEPARATE, glBlendEquationSeparate);
DECLARE_GL_FUNCTION_SDL(GLBLENDFUNCSEPARATE, glBlendFuncSeparate);

void setup_gl() {
	GET_GL_FUNCTION_SDL(GLCREATESHADER, glCreateShader);
	GET_GL_FUNCTION_SDL(GLSHADERSOURCE, glShaderSource);
	GET_GL_FUNCTION_SDL(GLCOMPILESHADER, glCompileShader);
	GET_GL_FUNCTION_SDL(GLGETSHADERINFOLOG, glGetShaderInfoLog);
	GET_GL_FUNCTION_SDL(GLCREATEPROGRAM, glCreateProgram);
	GET_GL_FUNCTION_SDL(GLATTACHSHADER, glAttachShader);
	GET_GL_FUNCTION_SDL(GLPROGRAMPARAMETERI, glProgramParameteri);
	GET_GL_FUNCTION_SDL(GLLINKPROGRAM, glLinkProgram);
	GET_GL_FUNCTION_SDL(GLGETPROGRAMIV, glGetProgramiv);
	GET_GL_FUNCTION_SDL(GLGETPROGRAMINFOLOG, glGetProgramInfoLog);
	GET_GL_FUNCTION_SDL(GLVALIDATEPROGRAM, glValidateProgram);
	GET_GL_FUNCTION_SDL(GLGETPROGRAMBINARY, glGetProgramBinary);
	GET_GL_FUNCTION_SDL(GLUNIFORMMATRIX4FV, glUniformMatrix4fv);
	GET_GL_FUNCTION_SDL(GLGENBUFFERS, glGenBuffers);
	GET_GL_FUNCTION_SDL(GLBINDBUFFER, glBindBuffer);
	GET_GL_FUNCTION_SDL(GLBUFFERDATA, glBufferData);
	GET_GL_FUNCTION_SDL(GLMAPBUFFER, glMapBuffer);
	GET_GL_FUNCTION_SDL(GLUNMAPBUFFER, glUnmapBuffer);
	GET_GL_FUNCTION_SDL(GLGENVERTEXARRAYS, glGenVertexArrays);
	GET_GL_FUNCTION_SDL(GLBINDVERTEXARRAY, glBindVertexArray);
	GET_GL_FUNCTION_SDL(GLGETATTRIBLOCATION, glGetAttribLocation);
	GET_GL_FUNCTION_SDL(GLVERTEXATTRIBPOINTER, glVertexAttribPointer);
	GET_GL_FUNCTION_SDL(GLENABLEVERTEXATTRIBARRAY, glEnableVertexAttribArray);
	GET_GL_FUNCTION_SDL(GLUSEPROGRAM, glUseProgram);
	GET_GL_FUNCTION_SDL(GLGETUNIFORMLOCATION, glGetUniformLocation);
	GET_GL_FUNCTION_SDL(GLGENFRAMEBUFFERS, glGenFramebuffers);
	GET_GL_FUNCTION_SDL(GLBINDFRAMEBUFFER, glBindFramebuffer);
	GET_GL_FUNCTION_SDL(GLFRAMEBUFFERTEXTURE2D, glFramebufferTexture2D);
	GET_GL_FUNCTION_SDL(GLDRAWBUFFERS, glDrawBuffers);
	GET_GL_FUNCTION_SDL(GLCHECKFRAMEBUFFERSTATUS, glCheckFramebufferStatus);
	GET_GL_FUNCTION_SDL(GLTEXIMAGE2DMULTISAMPLE, glTexImage2DMultisample);
	GET_GL_FUNCTION_SDL(GLBLITFRAMEBUFFER, glBlitFramebuffer);
	GET_GL_FUNCTION_SDL(GLGENERATEMIPMAP, glGenerateMipmap);

	GET_GL_FUNCTION_SDL(GLUNIFORM1I, glUniform1i);
	GET_GL_FUNCTION_SDL(GLUNIFORM1F, glUniform1f);
	GET_GL_FUNCTION_SDL(GLUNIFORM1FV, glUniform1fv);
	GET_GL_FUNCTION_SDL(GLUNIFORM2F, glUniform2f);
	GET_GL_FUNCTION_SDL(GLUNIFORM2FV, glUniform2fv);
	GET_GL_FUNCTION_SDL(GLUNIFORM3F, glUniform3f);
	GET_GL_FUNCTION_SDL(GLUNIFORM3FV, glUniform3fv);
	GET_GL_FUNCTION_SDL(GLUNIFORM4F, glUniform4f);
	GET_GL_FUNCTION_SDL(GLUNIFORM4FV, glUniform4fv);
	GET_GL_FUNCTION_SDL(GLACTIVETEXTURE, glActiveTexture);
	GET_GL_FUNCTION_SDL(GLBLENDEQUATIONSEPARATE, glBlendEquationSeparate);
	GET_GL_FUNCTION_SDL(GLBLENDFUNCSEPARATE, glBlendFuncSeparate);
}