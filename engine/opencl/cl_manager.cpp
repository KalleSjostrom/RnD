#include "engine/utils/logger.h"
#include "cl_errors.cpp"

struct ClInfo {
	cl_context context;
	cl_command_queue command_queue;
	cl_device_id device;
};

static void _print_cl_device_info(u32 num_devices, cl_device_id *devices) {
	static const u32 buffer_size = 128;
	char buffer[buffer_size];
	for (u32 i = 0; i < num_devices; i++) {
		clGetDeviceInfo(devices[i], CL_DEVICE_NAME, buffer_size, buffer, 0);
		LOG_INFO("ClManager", "Device %s supports ", buffer);

		clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, buffer_size, buffer, 0);
		LOG_INFO("ClManager", "%s\n", buffer);

		clGetDeviceInfo(devices[i], CL_DEVICE_EXTENSIONS, buffer_size, buffer, 0);
		LOG_INFO("ClManager", "Device extensions: %s\n", buffer);

		clGetDeviceInfo(devices[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE, buffer_size, buffer, 0);
		LOG_INFO("ClManager", "Device max mem alloc size: %s\n", buffer);

		// CL_DEVICE_MAX_WORK_ITEM_SIZES
	}
}

// 384 CUDA cores
ClInfo init_opencl(MemoryArena &arena) {
	TempAllocator ta(&arena);

	cl_int errcode_ret;

	cl_uint num_platforms = 0;
	errcode_ret = clGetPlatformIDs(0, 0, &num_platforms);
	CL_CHECK_ERRORCODE(clGetPlatformIDs, errcode_ret);

	cl_platform_id *platforms = PUSH_STRUCTS(arena, num_platforms, cl_platform_id);
	errcode_ret = clGetPlatformIDs(num_platforms, platforms, 0);
	CL_CHECK_ERRORCODE(clGetPlatformIDs, errcode_ret);

	cl_uint num_devices = 0;
	errcode_ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, 0, &num_devices);
	CL_CHECK_ERRORCODE(clGetDeviceIDs, errcode_ret);

	cl_device_id *devices = PUSH_STRUCTS(arena, num_devices, cl_device_id);
	errcode_ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_devices, devices, 0);
	CL_CHECK_ERRORCODE(clGetDeviceIDs, errcode_ret);

	_print_cl_device_info(num_devices, devices);
#ifdef OS_WINDOWS
	cl_context_properties properties[] =
	{
		CL_GL_CONTEXT_KHR,   (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR,      (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], // OpenCL platform object
		0
	};
#else
	CGLContextObj gl_context = CGLGetCurrentContext(); // GL Context
	CGLShareGroupObj share_group = CGLGetShareGroup(gl_context); // Share Group
	cl_context_properties properties[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
		(cl_context_properties) share_group,
		0
	};
#endif

	// NOTE(kalle): I only care about the first GPU for now since I'm doing one share group with one context.
	num_devices = 1;
	int device_index = DEVICE_INDEX;

	cl_context context = clCreateContext(properties, num_devices, devices + device_index, 0, 0, &errcode_ret);
	CL_CHECK_ERRORCODE(clCreateContext, errcode_ret);

	// If the CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE property of a command-queue is not set, the commands enqueued to a command-queue execute in order. For example, if an application calls clEnqueueNDRangeKernel to execute kernel A followed by a clEnqueueNDRangeKernel to execute kernel B, the application can assume that kernel A finishes first and then kernel B is executed. If the memory objects output by kernel A are inputs to kernel B then kernel B will see the correct data in memory objects produced by execution of kernel A. If the CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE property of a commandqueue is set, then there is no guarantee that kernel A will finish before kernel B starts execution.
#ifdef OS_WINDOWS
	cl_queue_properties *queue_properties = 0; // { };
	cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, devices[device_index], queue_properties, &errcode_ret);
#else
	// cl_queue_properties_APPLE *queue_properties = 0; // { };
	// cl_command_queue_properties
	cl_command_queue command_queue = clCreateCommandQueue(context, devices[device_index], 0, &errcode_ret);
	// cl_command_queue command_queue = clCreateCommandQueueWithPropertiesAPPLE(context, devices[device_index], queue_properties, &errcode_ret);
#endif

	CL_CHECK_ERRORCODE(clCreateCommandQueueWithProperties, errcode_ret);

	ClInfo info = { };
	info.context = context;
	info.command_queue = command_queue;
	info.device = devices[device_index];
	return info;
}
