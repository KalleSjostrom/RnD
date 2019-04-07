#pragma once

static void print_error_clBuildProgram(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PROGRAM: log_info("OpenCL", "CL_INVALID_PROGRAM\nif program is not a valid program object."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "CL_INVALID_VALUE\nif device_list is NULL and num_devices is greater than zero, or if device_list is not NULL and num_devices is zero.\nif pfn_notify is NULL but user_data is not NULL."); break;
		case CL_INVALID_DEVICE: log_info("OpenCL", "CL_INVALID_DEVICE\nif OpenCL devices listed in device_list are not in the list of devices associated with program."); break;
		case CL_INVALID_BINARY: log_info("OpenCL", "CL_INVALID_BINARY\nif program is created with clCreateProgramWithBinary and devices listed in device_list do not have a valid program binary loaded."); break;
		case CL_INVALID_BUILD_OPTIONS: log_info("OpenCL", "CL_INVALID_BUILD_OPTIONS\nif the build options specified by options are invalid."); break;
		case CL_INVALID_OPERATION: log_info("OpenCL", "CL_INVALID_OPERATION\nif the build of a program executable for any of the devices listed in device_list by a previous call to clBuildProgram for program has not completed.\nif there are kernel objects attached to program.\nif program was not created with clCreateProgramWithSource or clCreateProgramWithBinary."); break;
		case CL_COMPILER_NOT_AVAILABLE: log_info("OpenCL", "CL_COMPILER_NOT_AVAILABLE\nif program is created with clCreateProgramWithSource and a compiler is not available i.e. CL_DEVICE_COMPILER_AVAILABLE specified in the table of OpenCL Device Queries for clGetDeviceInfo is set to CL_FALSE."); break;
		case CL_BUILD_PROGRAM_FAILURE: log_info("OpenCL", "CL_BUILD_PROGRAM_FAILURE\nif there is a failure to build the program executable. This error will be returned if clBuildProgram does not return until the build has completed."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetPlatformIDs(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_VALUE: log_info("OpenCL", "CL_INVALID_VALUE\nif num_entries is equal to zero and platforms is not NULL, or if both num_platforms and platforms are NULL."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetDeviceIDs(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PLATFORM: log_info("OpenCL", "CL_INVALID_PLATFORM\nif platform is not a valid platform."); break;
		case CL_INVALID_DEVICE_TYPE: log_info("OpenCL", "CL_INVALID_DEVICE_TYPE\nif device_type is not a valid value."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "CL_INVALID_VALUE\nif num_entries is equal to zero and devices is not NULL or if both num_devices and devices are NULL."); break;
		case CL_DEVICE_NOT_FOUND: log_info("OpenCL", "CL_DEVICE_NOT_FOUND\nif no OpenCL devices that matched device_type were found."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetProgramBuildInfo(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_DEVICE: log_info("OpenCL", "CL_INVALID_DEVICE\nif device is not in the list of devices associated with program."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "CL_INVALID_VALUE\nif param_name is not valid, or if size in bytes specified by param_value_size is < size of return type as described in the table above and param_value is not NULL."); break;
		case CL_INVALID_PROGRAM: log_info("OpenCL", "CL_INVALID_PROGRAM\nif program is a not a valid program object."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateProgramWithSource(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: log_info("OpenCL", "CL_INVALID_CONTEXT\nif context is not a valid context."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "CL_INVALID_VALUE\nif count is zero or if strings or any entry in strings is NULL."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateProgramWithBinary(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: log_info("OpenCL", "if context is not a valid context."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "if device_list is NULL or num_devices is zero; or if lengths or binaries are NULL or if any entry in lengths[i] or binaries[i] is NULL."); break;
		case CL_INVALID_DEVICE: log_info("OpenCL", "if OpenCL devices listed in device_list are not in the list of devices associated with context."); break;
		case CL_INVALID_BINARY: log_info("OpenCL", "if an invalid program binary was encountered for any device. binary_status will return specific status for each device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clGetProgramInfo(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_VALUE: log_info("OpenCL", "CL_INVALID_VALUE\nif param_name is not valid, or if size in bytes specified by param_value_size is < size of return type as described in the table above and param_value is not NULL."); break;
		case CL_INVALID_PROGRAM: log_info("OpenCL", "CL_INVALID_PROGRAM\nif program is not a valid program object."); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: log_info("OpenCL", "CL_INVALID_PROGRAM_EXECUTABLE\nif param_name is CL_PROGRAM_NUM_KERNELS or CL_PROGRAM_KERNEL_NAMES and a successful program executable has not been built for at least one device in the list of devices associated with program."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "CL_OUT_OF_RESOURCES\nif there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "CL_OUT_OF_HOST_MEMORY\nif there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clSetKernelArg(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_KERNEL: log_info("OpenCL", "if kernel is not a valid kernel object."); break;
		case CL_INVALID_ARG_INDEX: log_info("OpenCL", "if arg_index is not a valid argument index."); break;
		case CL_INVALID_ARG_VALUE:
			log_info("OpenCL", "if arg_value specified is not a valid value.");
			log_info("OpenCL", "if the argument is an image declared with the read_only qualifier and arg_value refers to an image object created with cl_mem_flags of CL_MEM_WRITE or if the image argument is declared with the write_only qualifier and arg_value refers to an image object created with cl_mem_flags of CL_MEM_READ.");
			break;
		case CL_INVALID_MEM_OBJECT: log_info("OpenCL", "for an argument declared to be a memory object when the specified arg_value is not a valid memory object."); break;
		case CL_INVALID_SAMPLER: log_info("OpenCL", "for an argument declared to be of type sampler_t when the specified arg_value is not a valid sampler object."); break;
		case CL_INVALID_ARG_SIZE: log_info("OpenCL", "if arg_size does not match the size of the data type for an argument that is not a memory object or if the argument is a memory object and arg_size != sizeof(cl_mem) or if arg_size is zero and the argument is declared with the __local qualifier or if the argument is a sampler and arg_size != sizeof(cl_sampler)."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clEnqueueNDRangeKernel(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PROGRAM_EXECUTABLE: log_info("OpenCL", "if there is no successfully built program executable available for device associated with command_queue."); break;
		case CL_INVALID_COMMAND_QUEUE: log_info("OpenCL", "if command_queue is not a valid command-queue."); break;
		case CL_INVALID_KERNEL: log_info("OpenCL", "if kernel is not a valid kernel object."); break;
		case CL_INVALID_CONTEXT: log_info("OpenCL", "if context associated with command_queue and kernel is not the same or if the context associated with command_queue and events in event_wait_list are not the same."); break;
		case CL_INVALID_KERNEL_ARGS: log_info("OpenCL", "if the kernel argument values have not been specified."); break;
		case CL_INVALID_WORK_DIMENSION: log_info("OpenCL", "if work_dim is not a valid value (i.e. a value between 1 and 3)."); break;
		case CL_INVALID_GLOBAL_WORK_SIZE: log_info("OpenCL", "if global_work_size is NULL, or if any of the values specified in global_work_size[0], ...global_work_size [work_dim - 1] are 0 or exceed the range given by the sizeof(size_t) for the device on which the kernel execution will be enqueued."); break;
		case CL_INVALID_GLOBAL_OFFSET: log_info("OpenCL", "if the value specified in global_work_size + the corresponding values in global_work_offset for any dimensions is greater than the sizeof(size_t) for the device on which the kernel execution will be enqueued."); break;
		case CL_INVALID_WORK_GROUP_SIZE:
			log_info("OpenCL", "if local_work_size is specified and number of work-items specified by global_work_size is not evenly divisable by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier in program source.");
			log_info("OpenCL", "if local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] *... local_work_size[work_dim - 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo.");
			log_info("OpenCL", "if local_work_size is NULL and the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source.");
			break;
		case CL_INVALID_WORK_ITEM_SIZE: log_info("OpenCL", "if the number of work-items specified in any of local_work_size[0], ... local_work_size[work_dim - 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], .... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim - 1]."); break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET: log_info("OpenCL", "if a sub-buffer object is specified as the value for an argument that is a buffer object and the offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue."); break;
		case CL_INVALID_IMAGE_SIZE: log_info("OpenCL", "if an image object is specified as an argument value and the image dimensions (image width, height, specified or compute row and/or slice pitch) are not supported by device associated with queue."); break;
		// case CL_INVALID_IMAGE_FORMAT: log_info("OpenCL", "if an image object is specified as an argument value and the image format (image channel order and data type) is not supported by device associated with queue."); break;
		case CL_OUT_OF_RESOURCES:
			log_info("OpenCL", "if there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_DEVICE_MAX_READ_IMAGE_ARGS value for device or the number of write-only image args used in kernel exceed the CL_DEVICE_MAX_WRITE_IMAGE_ARGS value for device or the number of samplers used in kernel exceed CL_DEVICE_MAX_SAMPLERS for device.");
			log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device.");
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: log_info("OpenCL", "if there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel."); break;
		case CL_INVALID_EVENT_WAIT_LIST: log_info("OpenCL", "if event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateKernel(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PROGRAM: log_info("OpenCL", "if program is not a valid program object."); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: log_info("OpenCL", "if there is no successfully built executable for program."); break;
		case CL_INVALID_KERNEL_NAME: log_info("OpenCL", "if kernel_name is not found in program."); break;
		case CL_INVALID_KERNEL_DEFINITION: log_info("OpenCL", "if the function definition for __kernel function given by kernel_name such as the number of arguments, the argument types are not the same for all devices for which the program executable has been built."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "if kernel_name is NULL."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateContext(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_PLATFORM: log_info("OpenCL", "if properties is NULL and no platform could be selected or if platform value specified in properties is not a valid platform. (If the extension cl_khr_gl_sharing is enabled, then this error is replaced with CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR; see below.."); break;
		case CL_INVALID_PROPERTY:
			log_info("OpenCL", "if context property name in properties is not a supported property name, if the value specified for a supported property name is not valid, or if the same property name is specified more than once. However if the extension cl_khr_gl_sharing is enabled, then CL_INVALID_PROPERTY is returned if an attribute name other than those listed in the table for properties above or if CL_CONTEXT_INTEROP_USER_SYNC is specified in properties.");
			log_info("OpenCL", "if an attribute name other than those specified in table 4.5 or if CL_CONTEXT_INTEROP_USER_SYNC is specified in properties.");
			break;
		case CL_INVALID_VALUE: log_info("OpenCL", "if devices is NULL; if num_devices is equal to zero; or if pfn_notify is NULL but user_data is not NULL."); break;
		case CL_INVALID_DEVICE: log_info("OpenCL", "if devices contains an invalid device."); break;
		case CL_DEVICE_NOT_AVAILABLE: log_info("OpenCL", "if a device in devices is currently not available even though the device was returned by clGetDeviceIDs."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
#if 0
		case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
			log_info("OpenCL", "when an invalid OpenGL context or share group object handle is specified in properties (only if the cl_khr_gl_sharing extension is enabled).");
			log_info("OpenCL", "if no OpenGL or OpenGL ES context or share group is specified in the attribute list given to clCreateContext and any of the commands in section 9.7 are called. (if the cl_khr_gl_sharing extension is enabled.");
			log_info("OpenCL", "if the cl_khr_gl_sharing extension is enabled and if a context was specified by any of the following means.");
			log_info("OpenCL", "A context specified for an EGL-based OpenGL ES or OpenGL implementation by setting the attributes CL_GL_CONTEXT_KHR and CL_EGL_DISPLAY_KHR.");
			log_info("OpenCL", "A context was specified for a GLX-based OpenGL implementation by setting the attributes CL_GL_CONTEXT_KHR and CL_GLX_DISPLAY_KHR.");
			log_info("OpenCL", "A context was specified for a WGL-based OpenGL implementation by setting the attributes CL_GL_CONTEXT_KHR and CL_WGL_HDC_KHR.");
			log_info("OpenCL", "and any of the following conditions hold.");
			log_info("OpenCL", "The specified display and context attributes do not identify a valid OpenGL or OpenGL ES context.");
			log_info("OpenCL", "The specified context does not support buffer and renderbuffer objects.");
			log_info("OpenCL", "The specified context is not compatible with the OpenCL context being created (for example, it exists in a physically distinct address space, such as another hardware device, or does not support sharing data with OpenCL due to implementation restrictions).");
			log_info("OpenCL", "if a share group was specified for a CGL-based OpenGL implementation by setting the attribute CL_CGL_SHAREGROUP_KHR, and the specified share group does not identify a valid CGL share group object (if the cl_khr_gl_sharing extension is enabled).");
			break;
#endif
		case CL_INVALID_OPERATION:
			log_info("OpenCL", "if Direct3D 10 interoperability is specified by setting CL_INVALID_D3D10_DEVICE_KHR to a non-NULL value, and interoperability with another graphics API is also specified (if the cl_khr_d3d10_sharing extension is enabled).");
			log_info("OpenCL", "if a context was specified as described above and any of the following conditions hold.");
			log_info("OpenCL", "A context or share group object was specified for one of CGL, EGL, GLX, or WGL and the OpenGL implementation does not support that window-system binding API.");
			log_info("OpenCL", "More than one of the attributes CL_CGL_SHAREGROUP_KHR, CL_EGL_DISPLAY_KHR, CL_GLX_DISPLAY_KHR, and CL_WGL_HDC_KHR is set to a non-default value.");
			log_info("OpenCL", "Both of the attributes CL_CGL_SHAREGROUP_KHR and CL_GL_CONTEXT_KHR are set to non-default values.");
			log_info("OpenCL", "Any of the devices specified in the devices argument cannot support OpenCL objects which share the data store of an OpenGL object, as described in section 9.7.");
			log_info("OpenCL", "if interoperability is specified by setting CL_CONTEXT_ADAPTER_D3D9_KHR, CL_CONTEXT_ADAPTER_D3D9EX_KHR or CL_CONTEXT_ADAPTER_DXVA_KHR to a non-NULL value, and interoperability with another graphics API is also specified (only if the cl_khr_dx9_media_sharing extension is supported).");
			log_info("OpenCL", "if Direct3D 11 interoperability is specified by setting CL_INVALID_D3D11_DEVICE_KHR to a non-NULL value, and interoperability with another graphics API is also specified (only if the cl_khr_d3d11_sharing extension is supported).");
			break;
#if 0
		case CL_INVALID_ADAPTER_KHR: log_info("OpenCL", "if any of the values of the properties CL_CONTEXT_ADAPTER_D3D9_KHR, CL_CONTEXT_ADAPTER_D3D9EX_KHR or CL_CONTEXT_ADAPTER_DXVA_KHR is non-NULL and does not specify a valid media adapter with which the cl_device_ids against which this context is to be created may interoperate (only if the cl_khr_dx9_media_sharing extension is supported)."); break;
		case CL_INVALID_DX9_MEDIA_ADAPTER_KHR: log_info("OpenCL", "if the media adapter specified for interoperability is not compatible with the devices against which the context is to be created (only if the cl_khr_dx9_media_sharing extension is supported)."); break;
		case CL_INVALID_D3D11_DEVICE_KHR:
			log_info("OpenCL", "if the value of the property CL_CONTEXT_D3D11_DEVICE_KHR is non-NULL and does not specify a valid Direct3D 11 device with which the cl_device_ids against which this context is to be created may interoperate (only if the cl_khr_d3d11_sharing extension is supported).");
			log_info("OpenCL", "if the Direct3D 11 device specified for interoperability is not compatible with the devices against which the context is to be created (only if the cl_khr_d3d11_sharing extension is supported).");
			break;
		case CL_INVALID_D3D10_DEVICE_KHR:
			log_info("OpenCL", "if the Direct3D 10 device specified for interoperability is not compatible with the devices against which the context is to be created (if the cl_khr_d3d10_sharing extension is enabled).");
			log_info("OpenCL", "if the value of the property CL_CONTEXT_D3D10_DEVICE_KHR is non-NULL and does not specify a valid Direct3D 10 device with which the cl_device_ids against which this context is to be created may interoperate (if the cl_khr_d3d10_sharing extension is enabled).");
			break;
#endif
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateCommandQueueWithProperties(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: log_info("OpenCL", "if context is not a valid context."); break;
		case CL_INVALID_DEVICE: log_info("OpenCL", "if device is not a valid device or is not associated with context."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "if values specified in properties are not valid."); break;
		case CL_INVALID_QUEUE_PROPERTIES: log_info("OpenCL", "if values specified in properties are valid but are not supported by the device."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateFromGLBuffer(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: log_info("OpenCL", "if context is not a valid context or was not created from a GL context."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "if values specified in flags are not valid."); break;
		case CL_INVALID_GL_OBJECT: log_info("OpenCL", "if bufobj is not a GL buffer object or is a GL buffer object but does not have an existing data store e or the size of the buffer is 0."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateFromGLTexture(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: log_info("OpenCL", "if context is not a valid context or was not created from a GL context."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "if values specified in flags are not valid or if value specified in texture_target is not one of the values specified in the description of texture_target."); break;
		case CL_INVALID_MIP_LEVEL: log_info("OpenCL", "if miplevel is greater than zero and the OpenGL implementation does not support creating from non-zero mipmap levels. OR if miplevel is less than the value of levelbase (for OpenGL implementations) or zero (for OpenGL ES implementations); or greater than the value of q (for both OpenGL and OpenGL ES). levelbase and q are defined for the texture in section 3.8.10 (Texture Completeness) of the OpenGL 2.1 specification and section 3.7.10 of the OpenGL ES 2.0."); break;
		case CL_INVALID_GL_OBJECT: log_info("OpenCL", "if texture is not a GL texture object whose type matches texture_target, if the specified miplevel of texture is not defined, or if the width or height of the specified miplevel is zero or if the GL texture object is incomplete.."); break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: log_info("OpenCL", "if the OpenGL texture internal format does not map to a supported OpenCL image format."); break;
		case CL_INVALID_OPERATION: log_info("OpenCL", "if texture is a GL texture object created with a border width value greater than zero."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clCreateBuffer(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_CONTEXT: log_info("OpenCL", "if context is not a valid context or was not created from a GL context."); break;
		case CL_INVALID_VALUE: log_info("OpenCL", "if values specified in flags are not valid."); break;
		case CL_INVALID_BUFFER_SIZE: log_info("OpenCL", "if size is 0 or is greater than CL_DEVICE_MAX_MEM_ALLOC_SIZE value specified in table of OpenCL Device Queries for clGetDeviceInfo for all devices in context."); break;
		case CL_INVALID_HOST_PTR: log_info("OpenCL", "if host_ptr is NULL and CL_MEM_USE_HOST_PTR or CL_MEM_COPY_HOST_PTR are set in flags or if host_ptr is not NULL but CL_MEM_COPY_HOST_PTR or CL_MEM_USE_HOST_PTR are not set in flags."); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: log_info("OpenCL", "if there is a failure to allocate memory for buffer object."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

static void print_error_clReleaseMemObject(cl_int errcode) {
	switch (errcode) {
		case CL_INVALID_MEM_OBJECT: log_info("OpenCL", "if memobj is a not a valid memory object."); break;
		case CL_OUT_OF_RESOURCES: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the device."); break;
		case CL_OUT_OF_HOST_MEMORY: log_info("OpenCL", "if there is a failure to allocate resources required by the OpenCL implementation on the host."); break;
		default: log_info("OpenCL", "UNKNONW ERROR!"); break;
	}
}

#ifdef CL_ERROR_CHECKING
#define CL_CHECK_ERRORCODE(id, errcode) { if (errcode != CL_SUCCESS) { log_info("OpenCL", "cl error [%s]: %d (%s:%d)\n", #id, errcode, __FILE__, __LINE__); print_error_##id(errcode); } }
#else
#define CL_CHECK_ERRORCODE(id, errcode)
#endif

