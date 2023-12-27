#pragma once

#include "Engine/Core/Interface.h"

class FWindow;

class IGraphicsInterface : public IInterface<IGraphicsInterface>

{
public:
	IGraphicsInterface()
		: IInterface(this)
	{
	}

	~IGraphicsInterface() override = default;

	[[nodiscard]] virtual HRESULT BeginFrame() = 0;
	[[nodiscard]] virtual HRESULT EndFrame() = 0;

private:
	[[nodiscard]] virtual HRESULT Initialize(std::shared_ptr<FWindow> InWindow) = 0;
	virtual void Destroy() = 0;

private:
	friend class FRenderer;
};
