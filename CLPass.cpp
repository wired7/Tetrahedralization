#include "CLPass.h"
#include "OpenCLContext.h"

void CLPass::addBuffer(AbstractCLBuffer* buffer, const std::string& bufferSignature, const std::string& kernelSignature, int index, bool isGLBuffer)
{
	std::map<std::string, std::map<std::string, std::pair<AbstractCLBuffer*, int>>>* targetMap = &buffers;

	if (isGLBuffer)
	{
		targetMap = &clGLBuffers;
	}

	(*targetMap)[kernelSignature][bufferSignature] = std::make_pair(buffer, index);
}

void CLPass::executeOwnBehaviour(void)
{
	glFinish();

	for (const auto& clKernel : clKernels)
	{
		for (const auto& buffer : buffers[clKernel.first])
		{
			buffer.second.first->enableBuffer(clKernel.second.first, buffer.second.second);
		}

		for (const auto& clGLBuffer : clGLBuffers[clKernel.first])
		{
			clGLBuffer.second.first->enableBuffer(clKernel.second.first, clGLBuffer.second.second);
			clEnqueueAcquireGLObjects(clKernel.second.first->context->commandQueues[0], 1, &clGLBuffer.second.first->bufferPointer, 0, 0, 0);
		}

		clKernel.second.first->execute(clKernel.second.second.first, clKernel.second.second.second);

		clFinish(clKernel.second.first->context->commandQueues[0]);

		for (const auto& clGLBuffer : clGLBuffers[clKernel.first])
		{
			clEnqueueReleaseGLObjects(clKernel.second.first->context->commandQueues[0], 1, &clGLBuffer.second.first->bufferPointer, 0, 0, 0);
		}

		clFinish(clKernel.second.first->context->commandQueues[0]);
	}
}