#pragma once

#include "GraphicsInterface.h"
#include "Engine/Core/Interface.h"

class ISwapChain : public IInterface<ISwapChain>
{
public:
	ISwapChain()
		: IInterface(this)
	{
	}

	~ISwapChain() override = default;

	[[nodiscard]] virtual HRESULT Initialize(std::shared_ptr<IGraphicsInterface> InGraphicsInterface, HWND WindowHandle)
	= 0;
	virtual void Destroy() = 0;

	[[nodiscard]] virtual HRESULT Present(bool bShouldVSync) = 0;
	virtual void Resize(const FUInt2& InSize) = 0;
	[[nodiscard]] virtual uint32_t GetBackBufferIndex() = 0;
	[[nodiscard]] virtual FUInt2 GetBackBufferSize() = 0;

	[[nodiscard]] virtual void* GetNativePointer() = 0;

private:
};
