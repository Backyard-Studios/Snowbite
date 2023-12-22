#pragma once

#include <Engine/Core/Definitions.h>

#include "RHIType.h"

class SNOWBITE_API IStaticRHI
{
public:
	virtual ~IStaticRHI() = default;

	virtual HRESULT Initialize() = 0;
	virtual void Shutdown() = 0;

	virtual const char* GetName() const = 0;
	virtual ERHIType GetType() const = 0;

	virtual HRESULT BeginFrame() = 0;
	virtual HRESULT EndFrame(bool bShouldVSync) = 0;
	virtual HRESULT WaitForFrame(uint32_t Index) = 0;
	virtual HRESULT FlushFrames(uint32_t Count) = 0;

	virtual void BeginUIFrame() = 0;
	virtual void EndUIFrame() = 0;

	virtual HRESULT Resize(uint32_t InWidth, uint32_t InHeight) = 0;

	virtual void SetBackBufferClearColor(const FClearColor& ClearColor) = 0;

	virtual uint32_t GetBufferCount() const = 0;
	virtual uint32_t GetBufferIndex() const = 0;

private:
};
