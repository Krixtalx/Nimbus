#include "AppPch.h"
#include "Application.h"

#include <future>

#include "glad/glad.h"								
#include <GLFW/glfw3.h>

#include "Managers/MemoryManager.h"
#if _WIN32
#include "UI/Popups/ConfirmPopup.h"
#include "libloaderapi.h"
#else
#include "UI/Popups/InfoPopup.h"
#endif

Nimbus::Application::Application() : _window(nullptr) {
	_renderer = Renderer::getInstance();
	_gui = GUI::getInstance();
}

void Nimbus::Application::releaseResources() {
	MemoryManager::shutdownLoadThreads();
	glfwDestroyWindow(_window); // - Cerramos y destruimos la ventana de la aplicaci�n.
	_window = nullptr;
	glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
}

Nimbus::Application::~Application() {
}

void Nimbus::Application::init(const std::string& title, int width, int height) {
	// - Inicializa GLFW. Es un proceso que s�lo debe realizarse una vez en la aplicaci�n
	if (glfwInit() != GLFW_TRUE) {
		throw std::runtime_error("Failed to initialize GLFW!");
	}

	// - Definimos las caracter�sticas que queremos que tenga el contexto gr�fico
	// OpenGL de la ventana que vamos a crear. Por ejemplo, el n�mero de muestras o el
	// modo Core Profile.
	glfwWindowHint(GLFW_SAMPLES, 4); // - Activa antialiasing con 4 muestras.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // - Esta y las 2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // siguientes activan un contexto
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); // OpenGL Core Profile 4.6.
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	int count = 0;
	const auto monitors = glfwGetMonitors(&count);
	glfwGetMonitorWorkarea(monitors[0], nullptr, nullptr, &width, &height);

	// - Tama�o, t�tulo de la ventana, en ventana y no en pantalla completa, sin compartir recursos con otras ventanas.
	_title = title;
	_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	// - Comprobamos si la creaci�n de la ventana ha tenido �xito.
	if (_window == nullptr) {
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		throw std::runtime_error("Failed to open GLFW window!");
	}

	// - Hace que el contexto OpenGL asociado a la ventana que acabamos de crear pase a
	// ser el contexto actual de OpenGL para las siguientes llamadas a la biblioteca
	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);

	// - Ahora inicializamos GLEW.
	// IMPORTANTE: GLAD debe inicializarse siempre DESPU�S de que se haya
	// inicializado GLFW y creado la ventana
	const int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (version == 0) {
		glfwDestroyWindow(_window);
		glfwTerminate(); // - Liberamos los recursos que ocupaba GLFW.
		throw std::runtime_error("Failed to initialize GLAD!");
	}
	//gladUninstallGLDebug();
	GLFWimage images[1];
	Image img("Assets/Logo.png");
	if (img.getHeight() > 0) {
		img.flipImageVertically();
		images[0].width = img.getWidth();
		images[0].height = img.getHeight();
		images[0].pixels = img.bits();
		if (images[0].width > 0)
			glfwSetWindowIcon(_window, 1, images);
	}

	// - Interrogamos a OpenGL para que nos informe de las propiedades del contexto 3D construido.
	spdlog::info("OpenGL Vendor: {}", (char*)glGetString(GL_VENDOR));
	spdlog::info("OpenGL GPU: {}", (char*)glGetString(GL_RENDERER));
	spdlog::info("OpenGL Version: {}", (char*)glGetString(GL_VERSION));
	spdlog::info("GLSL Version: {}", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	InputManager::getInstance()->init(_window);
	_renderer->prepareOpenGL(width, height, InputManager::getApplicationState());
	_gui->initialize(_window, 6);
}

void Nimbus::Application::loop() {
	// - Ciclo de eventos de la aplicaci�n. La condici�n de parada es que la
	// ventana principal deba cerrarse. Por ejemplo, si el usuario pulsa el
	// bot�n de cerrar la ventana (la X).
	const auto appstate = InputManager::getApplicationState();
	std::future generateLoadTask = std::async(std::launch::async, [&]() {
		while (!appstate->_closeFlag)
			for (const auto& model : _renderer->getActiveScene()->_pointClouds) {
				if (model.second->isEnabled() || !model.second->isControlledByRenderer())
					MemoryManager::generateLoadTask(model.second.get());
			}
	});
	while (!glfwWindowShouldClose(_window)) {
		// - Obtiene y organiza los eventos pendientes, tales como pulsaciones de
		// teclas o de rat�n, etc. Siempre al final de cada iteraci�n del ciclo
		// de eventos y despu�s de glfwSwapBuffers(window);
		glfwPollEvents();
		appstate->_closeFlag = glfwWindowShouldClose(_window);

		_renderer->render();
		_gui->render();
		Nimbus::MemoryManager::updateCurrentRamUsage();

		if (_renderer->sceneDirty()) {
			glfwSetWindowTitle(_window, std::string(_title + " *[" + _renderer->getSceneName() + "]").c_str());
		} else {
			glfwSetWindowTitle(_window, std::string(_title + " [" + _renderer->getSceneName() + "]").c_str());
		}

		// Update application state
		_renderer->updateDeltaTime();
		appstate->_numFps = GUI::getFrameRate();
		appstate->_UIFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
		glfwSetWindowShouldClose(_window, appstate->_closeFlag);

		//// - GLFW usa un doble buffer para que no haya parpadeo. Esta orden
		//// intercambia el buffer back (en el que se ha estado dibujando) por el
		//// que se mostraba hasta ahora (front).
		glfwSwapBuffers(_window);
	}

	generateLoadTask.get();
	this->releaseResources();
}
