#pragma once

#include <Engine/Core/Definitions.h>

#include <memory>

// ReSharper disable once CppClangTidyCertDcl58Cpp
SB_EXPORT_STL_CONTAINER(std::shared_ptr, FEngine)

class SNOWBITE_API FEngine
{
public:
	FEngine() = default;
	~FEngine() = default;
	SB_DISABLE_COPY_AND_MOVE(FEngine)

private:
	inline static std::shared_ptr<FEngine> EngineInstance = nullptr;

private:
	friend SNOWBITE_API uint32_t LaunchSnowbite(int argc, char* argv[]);
	friend SNOWBITE_API std::shared_ptr<FEngine> GetEngine();
};

/**
 * Returns the engine instance
 */
SNOWBITE_API std::shared_ptr<FEngine> GetEngine();
