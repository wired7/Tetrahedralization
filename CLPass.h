#pragma once
#include "Pass.h"

class AbstractCLBuffer;
class CLKernel;

class CLPass : public Pass
{
protected:
	std::map<std::string, std::map<std::string, std::pair<AbstractCLBuffer*, int>>> buffers;
	std::map<std::string, std::map<std::string, std::pair<AbstractCLBuffer*, int>>> clGLBuffers;
	std::map<std::string, std::pair<CLKernel*, std::pair<int, int>>> clKernels;
	virtual void executeOwnBehaviour(void);
public:
	CLPass(std::map<std::string, std::pair<CLKernel*, std::pair<int, int>>> kernels) : clKernels(kernels) {};
	~CLPass() {};
	void addBuffer(AbstractCLBuffer* buffer, const std::string& bufferSignature, const std::string& kernelSignature,
		int index, bool isGLBuffer = false);
};
