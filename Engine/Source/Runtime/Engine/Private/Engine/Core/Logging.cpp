#include "pch.h"

#include <Engine/Core/Logging.h>

#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::async_logger> FLogging::EngineLogger;
std::shared_ptr<spdlog::async_logger> FLogging::GameLogger;

std::shared_ptr<spdlog::async_logger> FLogging::GetEngineLogger()
{
	return EngineLogger;
}

std::shared_ptr<spdlog::async_logger> FLogging::GetGameLogger()
{
	return GameLogger;
}

void FLogging::Initialize()
{
	spdlog::init_thread_pool(8192, 1);
#ifdef SB_DEBUG
	spdlog::level::level_enum Level = spdlog::level::trace;
	const char* Pattern = "%^[%Y-%m-%d %T] [%=6n] %-8l%$ %v";
#else
	spdlog::level::level_enum Level = spdlog::level::warn;
	const char* Pattern = "%^[%Y-%m-%d %T] [%=6n] %-8l%$ %v";
#endif

	const std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> ErrorFileSink = std::make_shared<
		spdlog::sinks::rotating_file_sink_mt>("Logs/Error.log", 1048576 * 5, 3);
	ErrorFileSink->set_level(spdlog::level::err);

	std::vector<spdlog::sink_ptr> Sinks;
	Sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	Sinks.push_back(ErrorFileSink);

	EngineLogger = std::make_shared<spdlog::async_logger>("Engine", Sinks.begin(), Sinks.end(),
	                                                      spdlog::thread_pool());
	EngineLogger->set_level(Level);
	EngineLogger->set_pattern(Pattern);

	GameLogger = std::make_shared<spdlog::async_logger>("Game", Sinks.begin(), Sinks.end(),
	                                                    spdlog::thread_pool());
	GameLogger->set_level(Level);
	GameLogger->set_pattern(Pattern);

	register_logger(EngineLogger);
	register_logger(GameLogger);
	set_default_logger(GetEngineLogger());
	spdlog::set_level(Level);
	spdlog::set_pattern(Pattern);
}

void FLogging::Shutdown()
{
	spdlog::shutdown();
}
