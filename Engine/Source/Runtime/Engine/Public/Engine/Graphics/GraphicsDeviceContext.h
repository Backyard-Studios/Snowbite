#pragma once
#include "CommandAllocator.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "Fence.h"

struct FGraphicsDeviceContext
{
	uint32_t BufferCount = 3;
	std::shared_ptr<ICommandQueue> CommandQueue;
	std::vector<std::shared_ptr<ICommandAllocator>> CommandAllocators;
	std::vector<std::shared_ptr<ICommandList>> CommandLists;
	std::vector<std::shared_ptr<IFence>> Fences;
};
