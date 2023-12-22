#pragma once

#include <Engine/Core/Definitions.h>

#include "RHIType.h"

/**
 * @brief Static RHI interface.
 * This interface is used to abstract the underlying graphics API from the engine.
 */
class SNOWBITE_API IStaticRHI
{
public:
	virtual ~IStaticRHI() = default;

	virtual HRESULT Initialize() = 0;
	virtual void Shutdown() = 0;

	/**
	 * @brief Begins a frame.
	 * @return S_OK if successful, otherwise an error code.
	 */
	virtual HRESULT BeginFrame() = 0;
	/**
	 * @brief Ends a frame.
	 * @param bShouldVSync Whether or not to wait for the next VSync.
	 * @return S_OK if successful, otherwise an error code.
	 */
	virtual HRESULT EndFrame(bool bShouldVSync) = 0;
	/**
	 * @brief Waits for a frame to finish.
	 * @param Index The index of the frame to wait for.
	 * @return S_OK if successful, otherwise an error code.
	 */
	virtual HRESULT WaitForFrame(uint32_t Index) = 0;
	/**
	 * @brief Waits for a number of frames to finish.
	 * @param Count The number of frames to wait for.
	 * @return S_OK if successful, otherwise an error code.
	 */
	virtual HRESULT FlushFrames(uint32_t Count) = 0;

	virtual void BeginUIFrame() = 0;
	virtual void EndUIFrame() = 0;

	virtual HRESULT Resize(uint32_t InWidth, uint32_t InHeight) = 0;

	virtual void SetBackBufferClearColor(const FClearColor& ClearColor) = 0;

	/**
	 * @brief Gets the number of buffers in the swap chain.
	 * @return The number of buffers in the swap chain.
	 */
	[[nodiscard]] virtual uint32_t GetBufferCount() const = 0;
	/**
	 * @brief Gets the index of the current buffer in the swap chain.
	 * @return The index of the current buffer in the swap chain.
	 */
	[[nodiscard]] virtual uint32_t GetBufferIndex() const = 0;

	[[nodiscard]] virtual const char* GetName() const = 0;
	[[nodiscard]] virtual ERHIType GetType() const = 0;

private:
};
