#pragma once
#include "CommandAllocator.h"
#include "CommandListType.h"
#include "Engine/Core/Interface.h"

class ICommandList : public IInterface<ICommandList>
{
public:
	ICommandList()
		: IInterface(this)
	{
	}

	~ICommandList() override = default;

	[[nodiscard]] virtual HRESULT Initialize(std::shared_ptr<IGraphicsDevice> InDevice, ECommandListType InType) = 0;
	virtual void Destroy() = 0;

	[[nodiscard]] virtual HRESULT Reset(std::shared_ptr<ICommandAllocator> InCommandAllocator) = 0;
	[[nodiscard]] virtual HRESULT Close() = 0;

	[[nodiscard]] virtual void* GetNativePointer() = 0;

private:
};
