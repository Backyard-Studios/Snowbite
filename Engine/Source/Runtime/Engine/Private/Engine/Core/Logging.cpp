#include "pch.h"

#include <Engine/Core/Logging.h>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::async_logger> FLogging::Logger = nullptr;

std::shared_ptr<spdlog::async_logger> FLogging::GetLogger()
{
	return Logger;
}

void FLogging::Initialize()
{
	spdlog::init_thread_pool(8192, 1);
	std::vector<spdlog::sink_ptr> Sinks;
	Sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	Logger = std::make_shared<spdlog::async_logger>("Engine", Sinks.begin(), Sinks.end(), spdlog::thread_pool());
	Logger->set_pattern("%^[%T] [%t] [%n] %8l:%$ %v");
	Logger->set_level(spdlog::level::trace);
	register_logger(Logger);
	set_default_logger(Logger);
}

void FLogging::Shutdown()
{
	spdlog::shutdown();
}
