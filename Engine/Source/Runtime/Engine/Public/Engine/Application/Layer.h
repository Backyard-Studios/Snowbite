#pragma once

#include <Engine/Core/Definitions.h>

enum class SNOWBITE_API ELayerType
{
	None,
	Overlay,
};

/**
 * @brief Interface for a layer
 * A layer is a part of the engine that can be attached and detached at runtime.
 * It is used to separate different parts of the engine. For example the ui could be a different layer than the game logic.
 */
class SNOWBITE_API ILayer
{
public:
	virtual ~ILayer() = default;

	/**
	 * Called when the layer is attached to the layer stack
	 */
	virtual void OnAttach() = 0;

	/**
	 * Called when the layer is detached from the layer stack
	 */
	virtual void OnDetach() = 0;
	virtual void OnLogicUpdate() = 0;
	virtual void OnRenderUpdate() = 0;
	virtual void OnUIUpdate() = 0;

	virtual ELayerType GetType() const = 0;
	virtual const char* GetName() const = 0;

private:
};
