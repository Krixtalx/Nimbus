//#pragma comment (lib, "Dwmapi")
//#include <dwmapi.h> // This makes some conflicts with other Win32 libs, so no more dark title bar :(

#include "CorePch.h"
#include "InputManager.h"

#include "Camera/Camera.h"
#include "Graphics/Renderer.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <algorithm>

#include "IO/FileManager.h"


//WNDPROC original_proc;
//
//LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//	switch (uMsg) {
//	case WM_NCCALCSIZE:
//	{
//		// Remove the window's standard sizing border
//		if (wParam == TRUE && lParam != NULL) {
//			NCCALCSIZE_PARAMS* pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
//			pParams->rgrc[0].top += 1;
//			pParams->rgrc[0].right -= 1;
//			pParams->rgrc[0].bottom -= 1;
//			pParams->rgrc[0].left += 1;
//		}
//		return 0;
//	}
//	case WM_NCPAINT:
//	{
//		// Prevent the non-client area from being painted
//		return 0;
//	}
//	case WM_NCHITTEST:
//	{
//		// Expand the hit test area for resizing
//		const int borderWidth = 8; // Adjust this value to control the hit test area size
//
//		POINTS mousePos = MAKEPOINTS(lParam);
//		POINT clientMousePos = { mousePos.x, mousePos.y };
//		ScreenToClient(hWnd, &clientMousePos);
//
//		RECT windowRect;
//		GetClientRect(hWnd, &windowRect);
//
//		if (clientMousePos.y >= windowRect.bottom - borderWidth) {
//			if (clientMousePos.x <= borderWidth)
//				return HTBOTTOMLEFT;
//			if (clientMousePos.x >= windowRect.right - borderWidth)
//				return HTBOTTOMRIGHT;
//			return HTBOTTOM;
//		}
//		if (clientMousePos.y <= borderWidth) {
//			if (clientMousePos.x <= borderWidth)
//				return HTTOPLEFT;
//			if (clientMousePos.x >= windowRect.right - borderWidth)
//				return HTTOPRIGHT;
//			return HTTOP;
//		}
//		if (clientMousePos.y <= borderWidth + 20)
//			return HTCAPTION;
//		/*if (Nimbus::InputManager::_dragWindow)
//			return HTCAPTION;*/
//		if (clientMousePos.x <= borderWidth) {
//			return HTLEFT;
//		}
//		if (clientMousePos.x >= windowRect.right - borderWidth) {
//			return HTRIGHT;
//		}
//
//		break;
//	}
//	case WM_NCACTIVATE:
//	{
//		// Prevent non-client area from being redrawn during window activation
//		return TRUE;
//	}
//	}
//
//	return CallWindowProc(original_proc, hWnd, uMsg, wParam, lParam);
//}
//void disableTitlebar(GLFWwindow* window) {
//	HWND hWnd = glfwGetWin32Window(window);
//
//	LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
//	lStyle |= WS_THICKFRAME;
//	lStyle &= ~WS_CAPTION;
//	SetWindowLongPtr(hWnd, GWL_STYLE, lStyle);
//
//	RECT windowRect;
//	GetWindowRect(hWnd, &windowRect);
//	int width = windowRect.right - windowRect.left;
//	int height = windowRect.bottom - windowRect.top;
//
//	original_proc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
//	SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));
//	SetWindowPos(hWnd, nullptr, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOMOVE);
//}

// Static

Nimbus::ApplicationState Nimbus::InputManager::_applicationState;
const vec2 Nimbus::InputManager::_defaultCursorPosition = vec2(-1.0f, -1.0f);

// Public methods

Nimbus::InputManager::InputManager() : _lastCursorPosition(_defaultCursorPosition), _leftClickPressed(false), _rightClickPressed(false), _scrollClickPressed(false) {
	this->buildMovementBuffers();
	this->updateMovementSteps();
}

void Nimbus::InputManager::buildMovementBuffers()
{
	_eventKey = std::vector(NUM_EVENTS, ivec2(0));
	_eventKey[BOOM] = ivec2(GLFW_KEY_UP, GLFW_KEY_DOWN);
	_eventKey[DOLLY] = ivec2(GLFW_KEY_W, GLFW_KEY_S);
	_eventKey[DOLLY_SPEED_UP] = ivec2(GLFW_MOD_SHIFT);
	_eventKey[ORBIT_XZ] = ivec2(GLFW_KEY_Y);
	_eventKey[ORBIT_Y] = ivec2(GLFW_KEY_X);
	_eventKey[PAN] = ivec2(GLFW_KEY_P);
	_eventKey[RESET] = ivec2(GLFW_KEY_R);
	_eventKey[SCREENSHOT] = ivec2(GLFW_KEY_K);
	_eventKey[TILT] = ivec2(GLFW_KEY_T);
	_eventKey[TRUCK] = ivec2(GLFW_KEY_D, GLFW_KEY_A);
	_eventKey[ROTATION] = ivec2(GLFW_KEY_Q, GLFW_KEY_E);
	_eventKey[zoomOutAnim] = ivec2(GLFW_KEY_1);
}

bool Nimbus::InputManager::mouseMovement(const float xPos, const float yPos) {
	Camera* camera = Renderer::getInstance()->getCamera();
	bool retVal = false;

	// if (!_applicationState._isPicking && _lastCursorPosition.x > 0.f) {
	if (!_applicationState._isPicking) {
		// spdlog::info("mouse callback");
		if (_rightClickPressed) {
			if (!epsilonEqual(xPos, _lastCursorPosition.x, glm::epsilon<float>())) {
				camera->pan(-_moveSpeed[PAN] * (xPos - _lastCursorPosition.x) * _applicationState.deltaTime);
			}

			if (!epsilonEqual(yPos, _lastCursorPosition.y, glm::epsilon<float>())) {
				camera->tilt(-_moveSpeed[TILT] * (yPos - _lastCursorPosition.y) * _applicationState.deltaTime);
			}
		} else if (_leftClickPressed) {
			if (!epsilonEqual(yPos, _lastCursorPosition.y, glm::epsilon<float>())) {
				camera->orbitXZ(-_moveSpeed[ORBIT_XZ] * (yPos - _lastCursorPosition.y) * _applicationState.deltaTime);
			}

			if (!epsilonEqual(xPos, _lastCursorPosition.x, glm::epsilon<float>())) {
				camera->orbitY(-_moveSpeed[ORBIT_Y] * (xPos - _lastCursorPosition.x) * _applicationState.deltaTime);
			}
		} else if (_scrollClickPressed) {
			if (!epsilonEqual(xPos, _lastCursorPosition.x, glm::epsilon<float>())) {
				camera->truck(_moveSpeed[TRUCK] * (xPos - _lastCursorPosition.x) * _applicationState.deltaTime * 20);
			}

			if (!epsilonEqual(yPos, _lastCursorPosition.y, glm::epsilon<float>())) {
				camera->boom(-_moveSpeed[BOOM] * (yPos - _lastCursorPosition.y) * _applicationState.deltaTime * 20);
			}
		}
		// _lastCursorPosition = vec2(xPos, yPos);
		retVal = true;
	}
	_lastCursorPosition = vec2(xPos, yPos);

	for (const auto& listener : _mouseDragListeners) {
		listener->mouseMoveEvent(xPos, yPos);
	}
	return retVal;
}

void Nimbus::InputManager::processPressedKeyEvent(const int key, const int mods) {
	Renderer* renderer = Renderer::getInstance();
	Camera* camera = renderer->getCamera();

	if (key == _eventKey[DOLLY][0]) {
		camera->dolly((_moveSpeed[DOLLY] + _moves[DOLLY] * _moveSpeed[DOLLY]) * _applicationState.deltaTime * _movementMultiplier);
		++_moves[DOLLY];
	} else if (key == _eventKey[DOLLY][1]) {
		camera->dolly(-((_moveSpeed[DOLLY] + _moves[DOLLY] * _moveSpeed[DOLLY]) * _applicationState.deltaTime * _movementMultiplier));
		++_moves[DOLLY];
	} else if (key == _eventKey[TRUCK][0]) {
		camera->truck((_moveSpeed[TRUCK] + _moves[TRUCK] * _moveSpeed[TRUCK]) * _applicationState.deltaTime * _movementMultiplier);
		++_moves[TRUCK];
	} else if (key == _eventKey[TRUCK][1]) {
		camera->truck(-((_moveSpeed[TRUCK] + _moves[TRUCK] * _moveSpeed[TRUCK]) * _applicationState.deltaTime * _movementMultiplier));
		++_moves[TRUCK];
	} else if (key == _eventKey[BOOM][0]) {
		camera->boom(_moveSpeed[BOOM] * _applicationState.deltaTime);
	} else if (key == _eventKey[BOOM][1]) {
		camera->crane(_moveSpeed[BOOM] * _applicationState.deltaTime);
	} else if (key == _eventKey[ROTATION][0]) {
		camera->rotate(_moveSpeed[ROTATION] * _applicationState.deltaTime);
	} else if (key == _eventKey[ROTATION][1]) {
		camera->rotate(-_moveSpeed[ROTATION] * _applicationState.deltaTime);
	} else if (key == _eventKey[SCREENSHOT][0]) {
		for (ScreenshotListener* listener : _screenshotListeners) {
			listener->screenshotEvent();
		}
	} else if (key == _eventKey[RESET][0]) {
		camera->reset();
	} else if (key == _eventKey[zoomOutAnim][0]) {
		renderer->zoomOutAnim();
	}

}

void Nimbus::InputManager::processReleasedKeyEvent(const int key, const int mods) {
	if (key == _eventKey[DOLLY][0] || key == _eventKey[DOLLY][1]) {
		_moves[DOLLY] = 0;
	}

	if (key == _eventKey[TRUCK][0] || key == _eventKey[TRUCK][0]) {
		_moves[TRUCK] = 0;
	}
}

Nimbus::InputManager::~InputManager() = default;

void Nimbus::InputManager::init(GLFWwindow* window) {
	_window = window;
	//const auto WINhWnd = glfwGetWin32Window(window);
	//const BOOL USE_DARK_MODE = true;
	//BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
	//	WINhWnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
	//	&USE_DARK_MODE, sizeof(USE_DARK_MODE)));
	//disableTitlebar(_window);


	// - Registramos los callbacks que responder�n a los eventos principales
	glfwSetWindowRefreshCallback(window, windowRefreshCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, mouseCursorCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetDropCallback(window, dropCallback);

	glfwSetWindowSizeLimits(window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void Nimbus::InputManager::updateMovementSteps() {
	_movementMultiplier = 1.0;
	_moveSpeedUp = 1.0f;

	// Calculate the movement speed based on the scene dimensions
	Renderer* renderer = Renderer::getInstance();
	if (renderer->getActiveScene())
	{
		AABB aabb;
		for (auto& pointCloud : renderer->getActiveScene()->_pointClouds | std::views::values)
			aabb.update(pointCloud->getAABB());

		if (aabb != AABB())
		{
			const vec3 aabbDim = aabb.size();
			const float size = std::sqrt(aabbDim.x * aabbDim.x + aabbDim.y * aabbDim.y + aabbDim.z * aabbDim.z);

			_movementMultiplier = std::log(size + 1.0f) / 2.0f;
		}
	}

	_moveSpeed = std::vector<float>(static_cast<size_t>(NUM_EVENTS), .0f);
	_moveSpeed[BOOM] = 0.05f * _movementMultiplier;
	_moveSpeed[DOLLY] = 0.1f * _movementMultiplier;
	_moveSpeed[ORBIT_XZ] = 0.08f * _movementMultiplier;
	_moveSpeed[ORBIT_Y] = 0.07f * _movementMultiplier;
	_moveSpeed[PAN] = 0.03f * _movementMultiplier;
	_moveSpeed[TILT] = 0.03f * _movementMultiplier;
	_moveSpeed[TRUCK] = 0.03f * _movementMultiplier;
	_moveSpeed[ZOOM] = 0.01f * _movementMultiplier;
	_moveSpeed[ROTATION] = 2.0f * _movementMultiplier;

	_moves = std::vector<uint32_t>(static_cast<size_t>(NUM_EVENTS), 0);
}

void Nimbus::InputManager::subscribeResize(ResizeListener* listener) {
	_resizeListeners.push_back(listener);
}

void Nimbus::InputManager::subscribeScreenshot(ScreenshotListener* listener) {
	_screenshotListeners.push_back(listener);
}

void Nimbus::InputManager::subscribeMouseButton(MouseButtonListener* listener) {
	_mouseButtonListeners.push_back(listener);
}

void Nimbus::InputManager::unsubscribeMouseButton(const MouseButtonListener* listener) {
	bool found = false;
	size_t lFound = 0;
	for (lFound = 0; lFound < _mouseButtonListeners.size() && !found; ++lFound) {
		found = _mouseButtonListeners[lFound] == listener;
	}
	if (found) {
		_mouseButtonListeners[lFound] = _mouseButtonListeners[_mouseButtonListeners.size() - 1];
		_mouseButtonListeners.pop_back();
	}
}

void Nimbus::InputManager::subscribeMouseMove(MouseMoveListener* listener) {
	_mouseDragListeners.push_back(listener);
}

void Nimbus::InputManager::unsubscribeMouseMove(const MouseMoveListener* listener) {
	bool found = false;
	size_t lFound = 0;
	for (lFound = 0; lFound < _mouseDragListeners.size() && !found; ++lFound) {
		found = _mouseDragListeners[lFound] == listener;
	}
	if (found) {
		_mouseDragListeners[lFound] = _mouseDragListeners[_mouseDragListeners.size() - 1];
		_mouseDragListeners.pop_back();
	}
}

// - Esta funci�n callback ser� llamada cada vez que se cambie el tama�o del �rea de dibujo OpenGL.
void Nimbus::InputManager::framebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
	const InputManager* inputManager = getInstance();
	for (ResizeListener* listener : inputManager->_resizeListeners) {
		listener->resizeEvent(static_cast<uint16_t>(width), static_cast<uint16_t>(height));
	}
}

// - Esta funci�n callback ser� llamada cada vez que se pulse una tecla dirigida al �rea de dibujo OpenGL.
void Nimbus::InputManager::keyCallback(GLFWwindow* window, const int key, int scancode, const int action, const int mods) {
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}

	InputManager* inputManager = getInstance();

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		inputManager->processPressedKeyEvent(key, mods);
	} else {
		inputManager->processReleasedKeyEvent(key, mods);
	}

	//std::cout << "Key callback called" << std::endl;
}

// - Esta funci�n callback ser� llamada cada vez que se pulse alg�n bot�n del rat�n sobre el �rea de dibujo OpenGL.
void Nimbus::InputManager::mouseButtonCallback(GLFWwindow* window, const int button, const int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}

	InputManager* inputManager = getInstance();

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		//GUI::getInstance()->pointerPicking(mods & GLFW_MOD_CONTROL, action == GLFW_PRESS, action == GLFW_RELEASE, action == GLFW_REPEAT);
		inputManager->_leftClickPressed = action == GLFW_PRESS;
		// inputManager->_lastCursorPosition = _defaultCursorPosition;
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		inputManager->_rightClickPressed = action == GLFW_PRESS;
		// inputManager->_lastCursorPosition = _defaultCursorPosition;
	} else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		inputManager->_scrollClickPressed = action == GLFW_PRESS;
		// inputManager->_lastCursorPosition = _defaultCursorPosition;
	}
	if ((action == GLFW_PRESS || action == GLFW_REPEAT))
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	for (const auto& btnListerner : inputManager->_mouseButtonListeners) {
		btnListerner->mouseButtonEvent(button, action, mods);
	}
}

void Nimbus::InputManager::mouseCursorCallback(GLFWwindow* window, const double xpos, const double ypos) {
	if (!_applicationState._UIFocus) {
		InputManager* inputManager = getInstance();
		inputManager->mouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
	}
}

// - Esta funci�n callback ser� llamada cada vez que se mueva la rueda del rat�n sobre el �rea de dibujo OpenGL.
void Nimbus::InputManager::scrollCallback(GLFWwindow* window, double xoffset, const double yoffset) {
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}

	const InputManager* inputManager = getInstance();
	Camera* camera = Renderer::getInstance()->getCamera();

	camera->zoom(static_cast<float>(yoffset) * inputManager->_moveSpeed[ZOOM]);

	camera->changed = true;
}

// - Esta funci�n callback ser� llamada cada vez que el �rea de dibujo OpenGL deba ser redibujada.
void Nimbus::InputManager::windowRefreshCallback(GLFWwindow* window) {
	Renderer::getInstance()->render();
	glfwSwapBuffers(window);
}

void Nimbus::InputManager::dropCallback(GLFWwindow* window, const int count, const char** paths) {
	std::vector<std::string> stringPaths;
	stringPaths.reserve(count);
	for (int i = 0; i < count; ++i) {
		stringPaths.emplace_back(paths[i]);
	}
	if (count == 1) {
		std::thread t([=]() {
			FileManager::loadPointCloud(stringPaths[0]);
		});
		t.detach();
	} else {
		std::thread t([=]() {
			FileManager::loadMergePointClouds(stringPaths);
		});
		t.detach();
	}
}
