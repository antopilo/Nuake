#include "GPUManaged.h"

using namespace Nuake;

void GPUManaged::SetDebugName(const std::string& inDebugName)
{
	this->debugName = inDebugName;
}

std::string_view GPUManaged::GetDebugName() const
{
	return this->debugName;
}

void GPUManaged::AddGPUCleanUpFunc(CleanUpFunc cleanUpFunc)
{
	this->cleanUpQueue.push(cleanUpFunc);
}

CleanUpStack GPUManaged::GetGPUCleanUpStack()
{
	return this->cleanUpQueue;
}