#pragma once

#include "GraphicsAdapter.h"
#include "GraphicsDeviceContext.h"
#include "../Core/Interface.h"

class IGraphicsDevice : public IInterface<IGraphicsDevice>
{
public:
	IGraphicsDevice()
		: IInterface(this)
	{
	}

	~IGraphicsDevice() override = default;

	[[nodiscard]] virtual HRESULT Initialize() = 0;
	virtual void Destroy() = 0;

	[[nodiscard]] virtual void* GetNativePointer() const = 0;
	[[nodiscard]] virtual std::shared_ptr<IGraphicsAdapter> GetAdapter() const = 0;

	[[nodiscard]] FGraphicsDeviceContext GetContext() const { return Context; }

protected:
	FGraphicsDeviceContext Context;
};
