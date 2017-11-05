#pragma once
#include "engine/utils/logger.h"

static void print_error_clBuildProgram(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PROGRAM: LOG_INFO("OpenCL", "CL_INVALID_PROGRAM\nif program is not a valid program object.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "CL_INVALID_VALUE\nif device_list is NULL and num_devices is greater than zero, or if device_list is not NULL and num_devices is zero.\nif pfn_notify is NULL but user_data is not NULL.\n"); break;
		case CL_INVALID_DEVICE: LOG_INFO("OpenCL", "CL_INVALID_DEVICE\nif OpenCL devices listed in device_list are not in the list of devices associated with program.\n"); break;
		case CL_INVALID_BINARY: LOG_INFO("OpenCL", "CL_INVALID_BINARY\nif program is created with clCreateProgramWithBinary and devices listed in device_list do not have a valid program binary loaded.\n"); break;
		case CL_INVALID_BUILD_OPTIONS: LOG_INFO("OpenCL", "CL_INVALID_BUILD_OPTIONS\nif the build options specified by options are invalid.\n"); break;
		case CL_INVALID_OPERATION: LOG_INFO("OpenCL", "CL_INVALID_OPERATION\nif the build of a program executable for any of the devices listed in device_list by a previous call to clBuildProgram for program has not completed.\nif there are kernel objects attached to program.\nif program was not created with clCreateProgramWithSource or clCreateProgramWithBinary.\n"); break;
		case CL_COMPILER_NOT_AVAILABLE: LOG_INFO("OpenCL", "CL_COMPILER_NOT_AVAILABLE\nif program is created with clCreateProgramWithSource and a compiler is not available i.e. CL_DEVICE_COMPILER_AVAILABLE specified in the table of OpenCL Device Queries for clGetDeviceInfo is set to CL_FALSE.\n"); break;
		case CL_BUILD_PROGRAM_FAILURE: LOG_INFO("OpenCL", "CL_BUILD_PROGRAM_FAILURE\nif there is a failure to build the program executable. This error will be returned if clBuildProgram does not return until the build has completed.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetPlatformIDs(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "CL_INVALID_VALUE\nif num_entries is equal to zero and platforms is not NULL, or if both num_platforms and platforms are NULL.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetDeviceIDs(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PLATFORM: LOG_INFO("OpenCL", "CL_INVALID_PLATFORM\nif platform is not a valid platform.\n"); break;
		case CL_INVALID_DEVICE_TYPE: LOG_INFO("OpenCL", "CL_INVALID_DEVICE_TYPE\nif device_type is not a valid value.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "CL_INVALID_VALUE\nif num_entries is equal to zero and devices is not NULL or if both num_devices and devices are NULL.\n"); break;
		case CL_DEVICE_NOT_FOUND: LOG_INFO("OpenCL", "CL_DEVICE_NOT_FOUND\nif no OpenCL devices that matched device_type were found.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetProgramBuildInfo(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_DEVICE: LOG_INFO("OpenCL", "CL_INVALID_DEVICE\nif device is not in the list of devices associated with program.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "CL_INVALID_VALUE\nif param_name is not valid, or if size in bytes specified by param_value_size is < size of return type as described in the table above and param_value is not NULL.\n"); break;
		case CL_INVALID_PROGRAM: LOG_INFO("OpenCL", "CL_INVALID_PROGRAM\nif program is a not a valid program object.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateProgramWithSource(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: LOG_INFO("OpenCL", "CL_INVALID_CONTEXT\nif context is not a valid context.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "CL_INVALID_VALUE\nif count is zero or if strings or any entry in strings is NULL.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateProgramWithBinary(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: LOG_INFO("OpenCL", "if context is not a valid context.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "if device_list is NULL or num_devices is zero; or if lengths or binaries are NULL or if any entry in lengths[i] or binaries[i] is NULL.\n"); break;
		case CL_INVALID_DEVICE: LOG_INFO("OpenCL", "if OpenCL devices listed in device_list are not in the list of devices associated with context.\n"); break;
		case CL_INVALID_BINARY: LOG_INFO("OpenCL", "if an invalid program binary was encountered for any device. binary_status will return specific status for each device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetProgramInfo(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "CL_INVALID_VALUE\nif param_name is not valid, or if size in bytes specified by param_value_size is < size of return type as described in the table above and param_value is not NULL.\n"); break;
		case CL_INVALID_PROGRAM: LOG_INFO("OpenCL", "CL_INVALID_PROGRAM\nif program is not a valid program object.\n"); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: LOG_INFO("OpenCL", "CL_INVALID_PROGRAM_EXECUTABLE\nif param_name is CL_PROGRAM_NUM_KERNELS or CL_PROGRAM_KERNEL_NAMES and a successful program executable has not been built for at least one device in the list of devices associated with program.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clSetKernelArg(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_KERNEL: LOG_INFO("OpenCL", "if kernel is not a valid kernel object.\n"); break;
		case CL_INVALID_ARG_INDEX: LOG_INFO("OpenCL", "if arg_index is not a valid argument index.\n"); break;
		case CL_INVALID_ARG_VALUE:
			LOG_INFO("OpenCL", "if arg_value specified is not a valid value.\n");
			LOG_INFO("OpenCL", "if the argument is an image declared with the read_only qualifier and arg_value refers to an image object created with cl_mem_flags of CL_MEM_WRITE or if the image argument is declared with the write_only qualifier and arg_value refers to an image object created with cl_mem_flags of CL_MEM_READ.\n");
			break;
		case CL_INVALID_MEM_OBJECT: LOG_INFO("OpenCL", "for an argument declared to be a memory object when the specified arg_value is not a valid memory object.\n"); break;
		case CL_INVALID_SAMPLER: LOG_INFO("OpenCL", "for an argument declared to be of type sampler_t when the specified arg_value is not a valid sampler object.\n"); break;
		case CL_INVALID_ARG_SIZE: LOG_INFO("OpenCL", "if arg_size does not match the size of the data type for an argument that is not a memory object or if the argument is a memory object and arg_size != sizeof(cl_mem) or if arg_size is zero and the argument is declared with the __local qualifier or if the argument is a sampler and arg_size != sizeof(cl_sampler).\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clEnqueueNDRangeKernel(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PROGRAM_EXECUTABLE: LOG_INFO("OpenCL", "if there is no successfully built program executable available for device associated with command_queue.\n"); break;
		case CL_INVALID_COMMAND_QUEUE: LOG_INFO("OpenCL", "if command_queue is not a valid command-queue.\n"); break;
		case CL_INVALID_KERNEL: LOG_INFO("OpenCL", "if kernel is not a valid kernel object.\n"); break;
		case CL_INVALID_CONTEXT: LOG_INFO("OpenCL", "if context associated with command_queue and kernel is not the same or if the context associated with command_queue and events in event_wait_list are not the same.\n"); break;
		case CL_INVALID_KERNEL_ARGS: LOG_INFO("OpenCL", "if the kernel argument values have not been specified.\n"); break;
		case CL_INVALID_WORK_DIMENSION: LOG_INFO("OpenCL", "if work_dim is not a valid value (i.e. a value between 1 and 3).\n"); break;
		case CL_INVALID_GLOBAL_WORK_SIZE: LOG_INFO("OpenCL", "if global_work_size is NULL, or if any of the values specified in global_work_size[0], ...global_work_size [work_dim - 1] are 0 or exceed the range given by the sizeof(size_t) for the device on which the kernel execution will be enqueued.\n"); break;
		case CL_INVALID_GLOBAL_OFFSET: LOG_INFO("OpenCL", "if the value specified in global_work_size + the corresponding values in global_work_offset for any dimensions is greater than the sizeof(size_t) for the device on which the kernel execution will be enqueued.\n"); break;
		case CL_INVALID_WORK_GROUP_SIZE:
			LOG_INFO("OpenCL", "if local_work_size is specified and number of work-items specified by global_work_size is not evenly divisable by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier in program source.\n");
			LOG_INFO("OpenCL", "if local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] *... local_work_size[work_dim - 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo.\n");
			LOG_INFO("OpenCL", "if local_work_size is NULL and the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source.\n");
			break;
		case CL_INVALID_WORK_ITEM_SIZE: LOG_INFO("OpenCL", "if the number of work-items specified in any of local_work_size[0], ... local_work_size[work_dim - 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], .... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim - 1].\n"); break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET: LOG_INFO("OpenCL", "if a sub-buffer object is specified as the value for an argument that is a buffer object and the offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.\n"); break;
		case CL_INVALID_IMAGE_SIZE: LOG_INFO("OpenCL", "if an image object is specified as an argument value and the image dimensions (image width, height, specified or compute row and/or slice pitch) are not supported by device associated with queue.\n"); break;
		// case CL_INVALID_IMAGE_FORMAT: LOG_INFO("OpenCL", "if an image object is specified as an argument value and the image format (image channel order and data type) is not supported by device associated with queue.\n"); break;
		case CL_OUT_OF_RESOURCES:
			LOG_INFO("OpenCL", "if there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_DEVICE_MAX_READ_IMAGE_ARGS value for device or the number of write-only image args used in kernel exceed the CL_DEVICE_MAX_WRITE_IMAGE_ARGS value for device or the number of samplers used in kernel exceed CL_DEVICE_MAX_SAMPLERS for device.\n");
			LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n");
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: LOG_INFO("OpenCL", "if there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel.\n"); break;
		case CL_INVALID_EVENT_WAIT_LIST: LOG_INFO("OpenCL", "if event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateKernel(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PROGRAM: LOG_INFO("OpenCL", "if program is not a valid program object.\n"); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: LOG_INFO("OpenCL", "if there is no successfully built executable for program.\n"); break;
		case CL_INVALID_KERNEL_NAME: LOG_INFO("OpenCL", "if kernel_name is not found in program.\n"); break;
		case CL_INVALID_KERNEL_DEFINITION: LOG_INFO("OpenCL", "if the function definition for __kernel function given by kernel_name such as the number of arguments, the argument types are not the same for all devices for which the program executable has been built.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "if kernel_name is NULL.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateContext(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PLATFORM: LOG_INFO("OpenCL", "if properties is NULL and no platform could be selected or if platform value specified in properties is not a valid platform. (If the extension cl_khr_gl_sharing is enabled, then this error is replaced with CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR; see below.)\n"); break;
		case CL_INVALID_PROPERTY:
			LOG_INFO("OpenCL", "if context property name in properties is not a supported property name, if the value specified for a supported property name is not valid, or if the same property name is specified more than once. However if the extension cl_khr_gl_sharing is enabled, then CL_INVALID_PROPERTY is returned if an attribute name other than those listed in the table for properties above or if CL_CONTEXT_INTEROP_USER_SYNC is specified in properties.\n");
			LOG_INFO("OpenCL", "if an attribute name other than those specified in table 4.5 or if CL_CONTEXT_INTEROP_USER_SYNC is specified in properties.\n");
			break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "if devices is NULL; if num_devices is equal to zero; or if pfn_notify is NULL but user_data is not NULL.\n"); break;
		case CL_INVALID_DEVICE: LOG_INFO("OpenCL", "if devices contains an invalid device.\n"); break;
		case CL_DEVICE_NOT_AVAILABLE: LOG_INFO("OpenCL", "if a device in devices is currently not available even though the device was returned by clGetDeviceIDs.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
#if 0
		case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
			LOG_INFO("OpenCL", "when an invalid OpenGL context or share group object handle is specified in properties (only if the cl_khr_gl_sharing extension is enabled).\n");
			LOG_INFO("OpenCL", "if no OpenGL or OpenGL ES context or share group is specified in the attribute list given to clCreateContext and any of the commands in section 9.7 are called. (if the cl_khr_gl_sharing extension is enabled)\n");
			LOG_INFO("OpenCL", "if the cl_khr_gl_sharing extension is enabled and if a context was specified by any of the following means:\n");
			LOG_INFO("OpenCL", "A context specified for an EGL-based OpenGL ES or OpenGL implementation by setting the attributes CL_GL_CONTEXT_KHR and CL_EGL_DISPLAY_KHR.\n");
			LOG_INFO("OpenCL", "A context was specified for a GLX-based OpenGL implementation by setting the attributes CL_GL_CONTEXT_KHR and CL_GLX_DISPLAY_KHR.\n");
			LOG_INFO("OpenCL", "A context was specified for a WGL-based OpenGL implementation by setting the attributes CL_GL_CONTEXT_KHR and CL_WGL_HDC_KHR.\n");
			LOG_INFO("OpenCL", "and any of the following conditions hold:\n");
			LOG_INFO("OpenCL", "The specified display and context attributes do not identify a valid OpenGL or OpenGL ES context.\n");
			LOG_INFO("OpenCL", "The specified context does not support buffer and renderbuffer objects.\n");
			LOG_INFO("OpenCL", "The specified context is not compatible with the OpenCL context being created (for example, it exists in a physically distinct address space, such as another hardware device, or does not support sharing data with OpenCL due to implementation restrictions).\n");
			LOG_INFO("OpenCL", "if a share group was specified for a CGL-based OpenGL implementation by setting the attribute CL_CGL_SHAREGROUP_KHR, and the specified share group does not identify a valid CGL share group object (if the cl_khr_gl_sharing extension is enabled).\n");
			break;
#endif
		case CL_INVALID_OPERATION:
			LOG_INFO("OpenCL", "if Direct3D 10 interoperability is specified by setting CL_INVALID_D3D10_DEVICE_KHR to a non-NULL value, and interoperability with another graphics API is also specified (if the cl_khr_d3d10_sharing extension is enabled).\n");
			LOG_INFO("OpenCL", "if a context was specified as described above and any of the following conditions hold:\n");
			LOG_INFO("OpenCL", "A context or share group object was specified for one of CGL, EGL, GLX, or WGL and the OpenGL implementation does not support that window-system binding API.\n");
			LOG_INFO("OpenCL", "More than one of the attributes CL_CGL_SHAREGROUP_KHR, CL_EGL_DISPLAY_KHR, CL_GLX_DISPLAY_KHR, and CL_WGL_HDC_KHR is set to a non-default value.\n");
			LOG_INFO("OpenCL", "Both of the attributes CL_CGL_SHAREGROUP_KHR and CL_GL_CONTEXT_KHR are set to non-default values.\n");
			LOG_INFO("OpenCL", "Any of the devices specified in the devices argument cannot support OpenCL objects which share the data store of an OpenGL object, as described in section 9.7.\n");
			LOG_INFO("OpenCL", "if interoperability is specified by setting CL_CONTEXT_ADAPTER_D3D9_KHR, CL_CONTEXT_ADAPTER_D3D9EX_KHR or CL_CONTEXT_ADAPTER_DXVA_KHR to a non-NULL value, and interoperability with another graphics API is also specified (only if the cl_khr_dx9_media_sharing extension is supported).\n");
			LOG_INFO("OpenCL", "if Direct3D 11 interoperability is specified by setting CL_INVALID_D3D11_DEVICE_KHR to a non-NULL value, and interoperability with another graphics API is also specified (only if the cl_khr_d3d11_sharing extension is supported).\n");
			break;
#if 0
		case CL_INVALID_ADAPTER_KHR: LOG_INFO("OpenCL", "if any of the values of the properties CL_CONTEXT_ADAPTER_D3D9_KHR, CL_CONTEXT_ADAPTER_D3D9EX_KHR or CL_CONTEXT_ADAPTER_DXVA_KHR is non-NULL and does not specify a valid media adapter with which the cl_device_ids against which this context is to be created may interoperate (only if the cl_khr_dx9_media_sharing extension is supported).\n"); break;
		case CL_INVALID_DX9_MEDIA_ADAPTER_KHR: LOG_INFO("OpenCL", "if the media adapter specified for interoperability is not compatible with the devices against which the context is to be created (only if the cl_khr_dx9_media_sharing extension is supported).\n"); break;
		case CL_INVALID_D3D11_DEVICE_KHR:
			LOG_INFO("OpenCL", "if the value of the property CL_CONTEXT_D3D11_DEVICE_KHR is non-NULL and does not specify a valid Direct3D 11 device with which the cl_device_ids against which this context is to be created may interoperate (only if the cl_khr_d3d11_sharing extension is supported).\n");
			LOG_INFO("OpenCL", "if the Direct3D 11 device specified for interoperability is not compatible with the devices against which the context is to be created (only if the cl_khr_d3d11_sharing extension is supported).\n");
			break;
		case CL_INVALID_D3D10_DEVICE_KHR:
			LOG_INFO("OpenCL", "if the Direct3D 10 device specified for interoperability is not compatible with the devices against which the context is to be created (if the cl_khr_d3d10_sharing extension is enabled).\n");
			LOG_INFO("OpenCL", "if the value of the property CL_CONTEXT_D3D10_DEVICE_KHR is non-NULL and does not specify a valid Direct3D 10 device with which the cl_device_ids against which this context is to be created may interoperate (if the cl_khr_d3d10_sharing extension is enabled).\n");
			break;
#endif
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateCommandQueueWithProperties(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: LOG_INFO("OpenCL", "if context is not a valid context.\n"); break;
		case CL_INVALID_DEVICE: LOG_INFO("OpenCL", "if device is not a valid device or is not associated with context.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "if values specified in properties are not valid.\n"); break;
		case CL_INVALID_QUEUE_PROPERTIES: LOG_INFO("OpenCL", "if values specified in properties are valid but are not supported by the device.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateFromGLBuffer(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: LOG_INFO("OpenCL", "if context is not a valid context or was not created from a GL context.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "if values specified in flags are not valid.\n"); break;
		case CL_INVALID_GL_OBJECT: LOG_INFO("OpenCL", "if bufobj is not a GL buffer object or is a GL buffer object but does not have an existing data store e or the size of the buffer is 0.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateFromGLTexture(cl_int errcode) {
	switch (errcode) {
		// case CL_INVALID_CONTEXT: LOG_INFO("OpenCL", "if context is not a valid context or was not created from a GL context.\n"); break;
		// case CL_INVALID_VALUE: LOG_INFO("OpenCL", "if values specified in flags are not valid.\n"); break;
		// case CL_INVALID_GL_OBJECT: LOG_INFO("OpenCL", "if bufobj is not a GL buffer object or is a GL buffer object but does not have an existing data store e or the size of the buffer is 0.\n"); break;
		// case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		// case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateBuffer(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: LOG_INFO("OpenCL", "if context is not a valid context or was not created from a GL context.\n"); break;
		case CL_INVALID_VALUE: LOG_INFO("OpenCL", "if values specified in flags are not valid.\n"); break;
		case CL_INVALID_BUFFER_SIZE: LOG_INFO("OpenCL", "if size is 0 or is greater than CL_DEVICE_MAX_MEM_ALLOC_SIZE value specified in table of OpenCL Device Queries for clGetDeviceInfo for all devices in context.\n"); break;
		case CL_INVALID_HOST_PTR: LOG_INFO("OpenCL", "if host_ptr is NULL and CL_MEM_USE_HOST_PTR or CL_MEM_COPY_HOST_PTR are set in flags or if host_ptr is not NULL but CL_MEM_COPY_HOST_PTR or CL_MEM_USE_HOST_PTR are not set in flags.\n"); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: LOG_INFO("OpenCL", "if there is a failure to allocate memory for buffer object.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clReleaseMemObject(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_MEM_OBJECT: LOG_INFO("OpenCL", "if memobj is a not a valid memory object.\n"); break;
		case CL_OUT_OF_RESOURCES: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.\n"); break;
		case CL_OUT_OF_HOST_MEMORY: LOG_INFO("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host.\n"); break;
		default: LOG_INFO("OpenCL", "UNKNONW ERROR!"); break;
	}
}

#ifdef CL_ERROR_CHECKING
#define CL_CHECK_ERRORCODE(id, errcode) { if (errcode != CL_SUCCESS) { LOG_INFO("OpenCL", "cl error [%s]: %d (%s:%d)\n", #id, errcode, __FILE__, __LINE__); print_error_##id(errcode); } }
#else
#define CL_CHECK_ERRORCODE(id, errcode)
#endif

