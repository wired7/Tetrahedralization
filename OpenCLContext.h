#pragma once
#include <CL/cl.h>
#include <CL/opencl.h>
#include "CL/cl.hpp"
#include "glew.h"

class CLKernel;
class OpenCLContext
{
public:
	cl_context context;
	std::vector<cl_platform_id> platformIDs;
	std::vector<cl_device_id> gpuDeviceIDs;
	std::vector<cl_device_id> deviceIDs;
	std::vector<cl_device_type> deviceTypes;
	cl_int ret;
	std::vector<cl_command_queue> commandQueues;
	OpenCLContext();
	~OpenCLContext();
	template<typename T> void platformInfo(cl_platform_id platform_id, cl_platform_info info);
	template<typename T> void deviceInfo(cl_device_id device_id, cl_device_info info);
};

template<typename T> void OpenCLContext::platformInfo(cl_platform_id platform_id, cl_platform_info info)
{
	size_t stringSize;
	clGetPlatformInfo(platform_id, info, NULL, NULL, &stringSize);
	T* output = new T[stringSize];
	clGetPlatformInfo(platform_id, info, stringSize * sizeof(T), output, NULL);
	std::cout << output << std::endl;
	delete[] output;
}

template<typename T> void OpenCLContext::deviceInfo(cl_device_id device_id, cl_device_info info)
{
	size_t stringSize;
	clGetDeviceInfo(device_id, info, NULL, NULL, &stringSize);
	T* output = new T[stringSize];
	clGetDeviceInfo(device_id, info, stringSize * sizeof(T), output, NULL);
	std::cout << output << std::endl;
	delete[] output;
}

class CLKernel
{
public:
	std::string kernelString;
	cl_kernel kernel;
	OpenCLContext* context;
	CLKernel(std::string filePath, std::string kernelName, OpenCLContext* context);
	~CLKernel();
	void readFile(std::string filePath);
	void execute(size_t global_item_size, size_t local_item_size);
};

class AbstractCLBuffer
{
public:
	cl_mem bufferPointer;
	OpenCLContext* context;
	cl_int ret;
	AbstractCLBuffer() {};
	AbstractCLBuffer(OpenCLContext* context) : context(context) {};
	virtual void bindBuffer() = 0;
	virtual void updateBuffer() = 0;
	virtual void enableBuffer(CLKernel* kernel, int argumentIndex) = 0;
	virtual void readBuffer() = 0;
};

template<class T> class CLBuffer : public AbstractCLBuffer
{
public:
	std::vector<T> bufferData;
	int readWrite;
	CLBuffer() {};
	CLBuffer(OpenCLContext* context, std::vector<T>& data, int readOnly = CL_MEM_READ_ONLY);
	~CLBuffer();
	virtual void bindBuffer();
	virtual void updateBuffer();
	virtual void enableBuffer(CLKernel* kernel, int argumentIndex);
	virtual void readBuffer();
};

template<class T> CLBuffer<T>::CLBuffer(OpenCLContext* context, std::vector<T>& data, int readWrite) :
	AbstractCLBuffer(context), bufferData(data), readWrite(readWrite)
{
}

template<class T> CLBuffer<T>::~CLBuffer()
{
}

template<class T> void CLBuffer<T>::bindBuffer()
{
	bufferPointer = clCreateBuffer(context->context, readWrite, bufferData.size() * sizeof(T), NULL, &ret);
	updateBuffer();
}

template<class T> void CLBuffer<T>::updateBuffer()
{
	std::vector<T> v(bufferData);
	ret = clEnqueueWriteBuffer(context->commandQueues[0], bufferPointer, CL_TRUE, 0, v.size() * sizeof(T), &(v[0]), 0, NULL, NULL);
}

template<class T> void CLBuffer<T>::enableBuffer(CLKernel* kernel, int argumentIndex)
{
	ret = clSetKernelArg(kernel->kernel, argumentIndex, sizeof(cl_mem), (void *)&bufferPointer);
}

template<class T> void CLBuffer<T>::readBuffer()
{
	std::vector<T> v;
	v.resize(bufferData.size());

	ret = clEnqueueReadBuffer(context->commandQueues[0], bufferPointer, CL_FALSE, 0, v.size() * sizeof(T), &(v[0]), 0, NULL, NULL);
	bufferData = v;
}

template<class T> class CLGLBuffer : public CLBuffer<T>
{
public:
	CLGLBuffer(OpenCLContext* context, GLuint VBO, int readOnly = CL_MEM_READ_ONLY);
	~CLGLBuffer();
};

template<class T> CLGLBuffer<T>::CLGLBuffer(OpenCLContext* context, GLuint VBO, int readWrite)
{
	this->context = context;
	this->readWrite = readWrite;
	bufferPointer = clCreateFromGLBuffer(context->context, readWrite, VBO, &ret);

}

template<class T> CLGLBuffer<T>::~CLGLBuffer()
{
}