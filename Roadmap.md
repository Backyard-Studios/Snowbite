# Roadmap

The list has no particular order. It is just a list of features that I want to implement.
I will expand the list as time goes on.

- [ ] [Memory Budgets (CPU and GPU)](#memory-budgets)
- [ ] [Memory Allocators](#memory-allocators)
- [ ] [RHI Abstraction](#rhi-abstraction)
- [ ] [Node Editor (Shaders, Materials, Animations)](#node-editor)
- [ ] [Scripting (Angelscript)](#scripting)
- [ ] [Asset System](#asset-system)

Rendering specific features:

- [ ] [Deferred Rendering](#deferred-rendering)
- [ ] [Global Illumination](#global-illumination)
- [ ] [Physically Based Rendering](#physically-based-rendering)

## Platforms

Focus lies on the Windows platform. Linux support is a nice to have, but not a priority.

## Details

### Memory Budgets

A system that keeps track of the memory usage and warns the user when the memory usage is too high.
User should be able to add different categories of memory usage and set a budget for each category.
When custom allocators are used, the user should assign a category to each allocator.

### Memory Allocators

Custom allocators that work with the memory budget system and can be used with the STL containers.

### RHI Abstraction

An abstraction layer for the rendering API. The user should be able to choose the rendering API at runtime.

| API           | Platform       |
| ------------- | -------------- |
| ❎ DirectX 12 | Windows        |
| ✖️ DirectX 11 | Windows        |
| 🌋 Vulkan     | Windows, Linux |

DirectX 11 will be implemented first, after that DirectX 12 and Vulkan.

### Node Editor

A node editor for shaders, materials and animations. The user should be able to create custom nodes through a plugin system (angelscript). Inspired by the node system of [Snowdrop](https://www.massive.se/project/snowdrop-engine/) and [Unreal Engine](https://www.unrealengine.com/).

### Scripting

The user should be able to write scripts in [angelscript](https://www.angelcode.com/angelscript/) or as an native c++ module. On distribution builds the angelscript scripts should be compressed and packed into a file that is readable by the asset system.

### Asset System

An asset system that can load assets from a package file and keep track of the memory usage of each asset. The user should be able to create custom asset types through the scripting system.
The asset system also handles the import and serialization of assets. Custom importers and serializers can be created through the scripting system.

### Deferred Rendering

The engine should only support deferred rendering. Forward rendering will be implemented in the future.

### Global Illumination

The engine should support global illumination. The user should be able to choose between different algorithms, like realtime global illumination or baked global illumination.

### Physically Based Rendering

The engine should support physically based rendering. The user should be able to choose between built-in materials or custom materials through the shader system.
