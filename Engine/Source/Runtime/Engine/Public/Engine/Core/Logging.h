#pragma once

#include <Engine/Core/Definitions.h>

#ifdef SB_DEBUG
#	define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
#	define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN
#endif
#include <spdlog/spdlog.h>

#include "spdlog/async_logger.h"

class SNOWBITE_API FLogging
{
public:
	[[nodiscard]] static std::shared_ptr<spdlog::async_logger> GetEngineLogger();
	[[nodiscard]] static std::shared_ptr<spdlog::async_logger> GetGameLogger();

private:
	static void Initialize();
	static void Shutdown();

private:
	static std::shared_ptr<spdlog::async_logger> EngineLogger;
	static std::shared_ptr<spdlog::async_logger> GameLogger;

	friend SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char** Arguments);
	friend int main(int ArgumentCount, char** ArgumentArray);
	friend int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow);
};

#ifdef SB_LIBRARY_EXPORT
#	define SB_LOG_TRACE(...) SPDLOG_LOGGER_TRACE(FLogging::GetEngineLogger(), __VA_ARGS__)
#	define SB_LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(FLogging::GetEngineLogger(), __VA_ARGS__)
#	define SB_LOG_INFO(...) SPDLOG_LOGGER_INFO(FLogging::GetEngineLogger(), __VA_ARGS__)
#	define SB_LOG_WARN(...) SPDLOG_LOGGER_WARN(FLogging::GetEngineLogger(), __VA_ARGS__)
#	define SB_LOG_ERROR(...) SPDLOG_LOGGER_ERROR(FLogging::GetEngineLogger(), __VA_ARGS__)
#	define SB_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(FLogging::GetEngineLogger(), __VA_ARGS__)
#else
#	define SB_LOG_TRACE(...) SPDLOG_LOGGER_TRACE(FLogging::GetGameLogger(), __VA_ARGS__)
#	define SB_LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(FLogging::GetGameLogger(), __VA_ARGS__)
#	define SB_LOG_INFO(...) SPDLOG_LOGGER_INFO(FLogging::GetGameLogger(), __VA_ARGS__)
#	define SB_LOG_WARN(...) SPDLOG_LOGGER_WARN(FLogging::GetGameLogger(), __VA_ARGS__)
#	define SB_LOG_ERROR(...) SPDLOG_LOGGER_ERROR(FLogging::GetGameLogger(), __VA_ARGS__)
#	define SB_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(FLogging::GetGameLogger(), __VA_ARGS__)
#endif
