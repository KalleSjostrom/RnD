#define RES_WIDTH 1024
#define RES_HEIGHT 768
#define RES_ASPECT ((float)RES_WIDTH/(float)RES_HEIGHT)

#define ROOT "../../fluid/"

#define SHADER_VERTEX_SOURCE ROOT"shaders/shader.vert"
#define SHADER_FRAGMENT_SOURCE ROOT"shaders/shader.frag"
#define GL_SHADER_BINARY ROOT"shaders/shader.bin"

#define COMPUTE_SHADER_SOURCE ROOT"shaders/compute_shader.cl"
#define COMPUTE_SHADER_BINARY ROOT"shaders/compute_shader.bin"

#define KERNEL_NAME "simulate"
