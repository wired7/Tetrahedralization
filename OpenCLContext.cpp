#pragma once
#include "OpenCLContext.h"
#include "GLFWWindowContext.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "glfw3native.h"

#define MAX_SOURCE_SIZE (0x100000)

OpenCLContext::OpenCLContext()
{
	// Get platform and device information
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;

	ret = clGetPlatformIDs(NULL, NULL, &ret_num_platforms);
	std::cout << ret_num_platforms << " platforms" << std::endl;
	platformIDs.resize(ret_num_platforms);
	ret = clGetPlatformIDs(ret_num_platforms, &(platformIDs[0]), NULL);

	if (ret != CL_SUCCESS)
	{
		std::cout << "Couldn't find an OpenCL platform!" << std::endl;
	}

	std::vector<cl_device_id> gpuDeviceIDs;
	std::vector<cl_device_id> cpuDeviceIDs;

	for (int i = 0; i < ret_num_platforms; i++)
	{
		std::cout << "PLATFORM " << i << ":" << std::endl;
		std::cout << "\tPROFILE: ";
		platformInfo<char>(platformIDs[i], CL_PLATFORM_PROFILE);
		std::cout << "\tVERSION: ";
		platformInfo<char>(platformIDs[i], CL_PLATFORM_VERSION);
		std::cout << "\tNAME: ";
		platformInfo<char>(platformIDs[i], CL_PLATFORM_NAME);
		std::cout << "\tVENDOR: ";
		platformInfo<char>(platformIDs[i], CL_PLATFORM_VENDOR);
		std::cout << "\tEXTENSIONS SUPPORTED: ";
		platformInfo<char>(platformIDs[i], CL_PLATFORM_EXTENSIONS);

		ret = clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_ALL, NULL, NULL, &ret_num_devices);
		std::cout << "\t" << ret_num_devices << " devices" << std::endl;
		int currentDeviceCount = deviceIDs.size();
		deviceIDs.resize(currentDeviceCount + ret_num_devices);
		ret = clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_ALL, ret_num_devices, &(deviceIDs[currentDeviceCount]), NULL);

		if (ret != CL_SUCCESS)
		{
			std::cout << "Couldn't find an OpenCL device!" << std::endl;
		}

		for (int j = currentDeviceCount; j < currentDeviceCount + ret_num_devices; j++)
		{
			std::cout << "\tDEVICE " << j << ":" << std::endl;
			std::cout << "\t\tNAME: ";
			deviceInfo<char>(deviceIDs[j], CL_DEVICE_NAME);

			cl_uint output;
			std::cout << "\t\tMEM SIZE: ";
			clGetDeviceInfo(deviceIDs[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_uint), &output, NULL);
			std::cout << output << std::endl;
			std::cout << "\t\tCOMPUTE UNITS: ";
			clGetDeviceInfo(deviceIDs[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &output, NULL);
			std::cout << output << std::endl;
			std::cout << "\t\tMAX FREQUENCY: ";
			clGetDeviceInfo(deviceIDs[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &output, NULL);
			std::cout << output << std::endl;

			cl_device_type deviceType;
			clGetDeviceInfo(deviceIDs[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL);
			deviceTypes.push_back(deviceType);

			if (deviceType == CL_DEVICE_TYPE_GPU)
			{
				gpuDeviceIDs.push_back(deviceIDs[j]);
			}
			else if (deviceType == CL_DEVICE_TYPE_CPU)
			{
				cpuDeviceIDs.push_back(deviceIDs[j]);
			}
		}

		std::cout << std::endl;
	}

	if (WindowContext::context != nullptr)
	{	
		cl_context_properties props[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetWGLContext(((GLFWWindowContext*)WindowContext::context)->window),
			CL_WGL_HDC_KHR, (cl_context_properties)GetDC(glfwGetWin32Window(((GLFWWindowContext*)WindowContext::context)->window)),
			CL_CONTEXT_PLATFORM, (cl_context_properties)platformIDs[1],
			0
		};

		// Create an OpenCL context.
		context = clCreateContext(props, gpuDeviceIDs.size(), &(gpuDeviceIDs[0]), NULL, NULL, &ret);

		if (!context || ret != CL_SUCCESS)
		{
			std::cout << "Couldn't create an OpenCL context." << std::endl;
		}

		// Create command queues
		for (int i = 0; i < gpuDeviceIDs.size(); i++)
		{
			commandQueues.push_back(clCreateCommandQueue(context, gpuDeviceIDs[i], 0, &ret));
		}

		for (int i = 0; i < cpuDeviceIDs.size(); i++)
		{
			commandQueues.push_back(clCreateCommandQueue(context, cpuDeviceIDs[i], 0, &ret));
		}
		
	}
	else
	{
		// Create an OpenCL context
		context = clCreateContext(NULL, deviceIDs.size(), &(deviceIDs[0]), NULL, NULL, &ret);

		// Create command queues
		commandQueues.push_back(clCreateCommandQueue(context, deviceIDs[0], 0, &ret));
	}
}


OpenCLContext::~OpenCLContext()
{
}


CLKernel::CLKernel(std::string filePath, std::string kernelName, OpenCLContext* context) : context(context)
{
	readFile(filePath);
	const char* source_str = kernelString.c_str();
	const size_t length = kernelString.size();
	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context->context, 1, (const char **)&source_str, &length, &(context->ret));

	// Build the program
	context->ret = clBuildProgram(program, 1, &(context->deviceIDs[1]), NULL, NULL, NULL);

	// Check for proper compilation
	size_t log_size;
	char* build_log;

	context->ret = clGetProgramBuildInfo(program, context->deviceIDs[1], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	build_log = new char[log_size + 1];

	context->ret = clGetProgramBuildInfo(program, context->deviceIDs[1], CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
	build_log[log_size] = '\0';
	printf("--- Build log ---\n ");
	fprintf(stderr, "%s\n", build_log);
	delete[] build_log;

	// Create the OpenCL kernel
	kernel = clCreateKernel(program, kernelName.c_str(), &context->ret);
}

CLKernel::~CLKernel()
{

}

void CLKernel::readFile(std::string filePath)
{
	std::ifstream in;
	in.open(filePath);

	if (!in.is_open()) {
		std::cerr << "Unable to open file " << filePath;
	}
	else
	{
		std::stringstream sstr;
		sstr << in.rdbuf();
		kernelString = sstr.str();
	}

	in.close();
}

void CLKernel::execute(size_t global_item_size, size_t local_item_size)
{
	// Execute the OpenCL kernel on the list
	clEnqueueNDRangeKernel(context->commandQueues[0], kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
}