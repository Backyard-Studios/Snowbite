#pragma once

#include <Engine/Core/Definitions.h>

#include <memory>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

#include <spdlog/async_logger.h>

class SNOWBITE_API FLogging
{
public:
	[[nodiscard]] static std::shared_ptr<spdlog::async_logger> GetLogger();

private:
	static void Initialize();
	static void Shutdown();

private:
	static std::shared_ptr<spdlog::async_logger> Logger;

	friend SNOWBITE_API uint32_t LaunchSnowbite(int ArgumentCount, char* Arguments[]);
};

#define SB_LOG_TRACE(...) SPDLOG_LOGGER_TRACE(FLogging::GetLogger(), __VA_ARGS__)
#define SB_LOG_INFO(...) SPDLOG_LOGGER_INFO(FLogging::GetLogger(), __VA_ARGS__)
#define SB_LOG_WARN(...) SPDLOG_LOGGER_WARN(FLogging::GetLogger(), __VA_ARGS__)
#define SB_LOG_ERROR(...) SPDLOG_LOGGER_ERROR(FLogging::GetLogger(), __VA_ARGS__)
#define SB_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(FLogging::GetLogger(), __VA_ARGS__)
