#pragma once

#include "CommandListType.h"
#include "GraphicsDevice.h"
#include "Engine/Core/Interface.h"

class ICommandQueue : public IInterface<ICommandQueue>
{
public:
	ICommandQueue()
		: IInterface(this)
	{
	}

	~ICommandQueue() override = default;

	[[nodiscard]] virtual HRESULT Initialize(std::shared_ptr<IGraphicsDevice> InDevice, ECommandListType InType) = 0;
	virtual void Destroy() = 0;

	[[nodiscard]] virtual void* GetNativePointer() const = 0;

private:
};
