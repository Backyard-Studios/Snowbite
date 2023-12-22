#pragma once

#include <Engine/Core/Definitions.h>
#include <Engine/Core/Assert.h>

#include "LayerStack.h"
#include <Windows.h>

class SNOWBITE_API IApplication
{
public:
	virtual ~IApplication() = default;

	virtual HRESULT Initialize()
	{
		LayerStack = std::make_shared<FLayerStack>();
		return S_OK;
	}

	virtual void OnUpdate()
	{
	}

	virtual void Shutdown()
	{
		if (!LayerStack)
			SB_ASSERT_CRITICAL(false, E_FAIL, "Layer stack is not initialized");
		LayerStack->Shutdown();
		SB_SAFE_RESET(LayerStack)
	}

	virtual const char* GetName() const = 0;

	[[nodiscard]] std::shared_ptr<FLayerStack> GetLayerStack() const { return LayerStack; }

protected:
	std::shared_ptr<FLayerStack> LayerStack;
};
