#pragma once

#include "Engine/Core/Interface.h"

class IGraphicsDevice;

class IFence : public IInterface<IFence>
{
public:
	IFence()
		: IInterface(this)
	{
	}

	~IFence() override = default;

	[[nodiscard]] virtual HRESULT Initialize(std::shared_ptr<IGraphicsDevice> InDevice) = 0;
	virtual void Destroy() = 0;

	[[nodiscard]] virtual HRESULT Signal(std::shared_ptr<ICommandQueue> InQueue) = 0;
	[[nodiscard]] virtual HRESULT Wait() = 0;

	[[nodiscard]] virtual void* GetNativePointer() const = 0;

	[[nodiscard]] uint32_t GetValue() const { return Value; }

protected:
	uint32_t Value = 0;
};
