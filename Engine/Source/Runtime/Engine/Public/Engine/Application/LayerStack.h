#pragma once

#include <queue>
#include <Engine/Core/Definitions.h>

#include "Layer.h"

/**
 * @brief A stack of layers
 * The layer stack is used to manage layers at runtime.
 */
class SNOWBITE_API FLayerStack
{
public:
	FLayerStack();
	~FLayerStack();

	void Shutdown();

	/**
	 * Adds the given layer to a queue to be pushed at the end of the frame
	 * @note The lifetime of the layer must be managed by the caller
	 */
	void PushLayer(const std::shared_ptr<ILayer>& Layer);

	/**
	 * Adds the given layer to a queue to be removed at the end of the frame
	 * @note The lifetime of the layer must be managed by the caller
	 */
	void PopLayer(const std::shared_ptr<ILayer>& Layer);

	/**
	 * Pushes and pops the layers that were added and removed during the frame
	 */
	void HandleDeferredLayerChanges();

	// Temporary
	void ShowDebugUI(const char* Title = "LayerStack");

	[[nodiscard]] std::vector<std::shared_ptr<ILayer>> GetLayers() const { return Layers; }

	std::vector<std::shared_ptr<ILayer>>::iterator begin() { return Layers.begin(); }
	std::vector<std::shared_ptr<ILayer>>::iterator end() { return Layers.end(); }
	std::vector<std::shared_ptr<ILayer>>::reverse_iterator rbegin() { return Layers.rbegin(); }
	std::vector<std::shared_ptr<ILayer>>::reverse_iterator rend() { return Layers.rend(); }

	std::vector<std::shared_ptr<ILayer>>::const_iterator begin() const { return Layers.begin(); }
	std::vector<std::shared_ptr<ILayer>>::const_iterator end() const { return Layers.end(); }
	std::vector<std::shared_ptr<ILayer>>::const_reverse_iterator rbegin() const { return Layers.rbegin(); }
	std::vector<std::shared_ptr<ILayer>>::const_reverse_iterator rend() const { return Layers.rend(); }

private:
	std::vector<std::shared_ptr<ILayer>> Layers;
	std::queue<std::shared_ptr<ILayer>> LayersToPush;
	std::queue<std::shared_ptr<ILayer>> LayersToPop;
};
