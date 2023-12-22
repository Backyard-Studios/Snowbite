#pragma once

#include <Engine/Core/Definitions.h>

/**
 * @brief Buffering mode enumeration.
 * Buffering mode is used to determine how many back buffers are used for rendering.
 */
enum class SNOWBITE_API EBufferingMode : uint32_t
{
	DoubleBuffering = 2,
	TripleBuffering = 3
};

inline uint32_t GetBufferingModeCount(const EBufferingMode BufferingMode)
{
	return static_cast<uint32_t>(BufferingMode);
}

inline const char* GetBufferingModeName(const EBufferingMode BufferingMode)
{
	switch (BufferingMode)
	{
	case EBufferingMode::DoubleBuffering:
		return "DoubleBuffering";
	case EBufferingMode::TripleBuffering:
		return "TripleBuffering";
	default:
		return "Unknown";
	}
}
