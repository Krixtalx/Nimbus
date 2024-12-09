#include "AppPch.h"

#include <fmt/color.h>
#include <locale>

#include "Application.h"
#include "Graphics/Renderer.h"
#include "Managers/MemoryManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h" // support for rotating file logging

const std::string version = "1.0.0";


int main(int argc, char** argv) {
	Nimbus::Renderer::getInstance();
	try {
		setlocale(LC_ALL, ".UTF8");
		//Setup logger
		const auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console->set_level(spdlog::level::info);

		const auto fileLog = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/log.txt", 1024 * 1024 * 5, 5);
		fileLog->set_level(spdlog::level::trace);

		spdlog::sinks_init_list sinkList = { fileLog, console };

		spdlog::logger logger("Nimbus", sinkList.begin(), sinkList.end());
		logger.set_level(spdlog::level::debug);

		spdlog::set_default_logger(std::make_shared<spdlog::logger>("Nimbus", sinkList));

		try {
			Nimbus::Application app;
			spdlog::info("Starting Nimbus renderer v{}", version);
			Nimbus::MemoryManager::spawnLoadThreads();
			app.init("Nimbus");
			app.loop();
		} catch (const std::exception& exception) {
			spdlog::critical(exception.what());
		}
		spdlog::info("Finishing Nimbus renderer");
		spdlog::shutdown();

	} catch (const spdlog::spdlog_ex& ex) {
		fmt::print(stderr, fg(fmt::color::red), "Logger initialization failed: {}\n", ex.what());
	}

	// - Una vez terminado el ciclo de eventos, liberar recursos, etc.
	fmt::print("Finishing application...\n");
}