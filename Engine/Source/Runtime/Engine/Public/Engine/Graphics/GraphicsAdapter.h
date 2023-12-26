#pragma once
#include "Engine/Core/Interface.h"

class IGraphicsAdapter : public IInterface<IGraphicsAdapter>
{
public:
	IGraphicsAdapter() : IInterface(this)
	{
	}

	~IGraphicsAdapter() override = default;

	virtual void Destroy() = 0;

	[[nodiscard]] virtual void* GetNativePointer() const = 0;

private:
};
