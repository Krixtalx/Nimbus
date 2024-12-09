#pragma once

namespace Nimbus {
	class Renderer;
	class GUI;

	class Application{
		Renderer* _renderer;
		GUI* _gui;
		GLFWwindow* _window;
		std::string _title;
		bool initialized = false;

	private:
		void releaseResources();

	public:
		Application();
		virtual ~Application();
		void init(const std::string& title, int width = 1920, int height = 1080);
		void loop();
		bool checkUpdate(const std::string& currentVersion, const std::string& remoteVersionFile = "") const;
	};
}

