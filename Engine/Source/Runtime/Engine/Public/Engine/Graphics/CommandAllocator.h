#pragma once

#include "Engine/Graphics/CommandListType.h"

class IGraphicsDevice;

class ICommandAllocator : public IInterface<ICommandAllocator>
{
public:
	ICommandAllocator() : IInterface(this)
	{
	}

	~ICommandAllocator() override = default;

	[[nodiscard]] virtual HRESULT Initialize(std::shared_ptr<IGraphicsDevice> Device, ECommandListType InType) = 0;
	virtual void Destroy() = 0;

	[[nodiscard]] virtual HRESULT Reset() = 0;

	[[nodiscard]] virtual void* GetNativePointer() = 0;

private:
};
