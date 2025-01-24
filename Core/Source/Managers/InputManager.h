#pragma once
#include "ApplicationState.h"
#include "Utilities/Singleton.h"

struct GLFWwindow;

namespace Nimbus {
	class ResizeListener {
	protected:
		~ResizeListener() = default;
	public:
		virtual void resizeEvent(uint16_t width, uint16_t height) = 0;
	};

	class ScreenshotListener {
	public:
		virtual void screenshotEvent() = 0;
	};

	class MouseButtonListener
	{
	public:
		virtual ~MouseButtonListener() = default;
		virtual void mouseButtonEvent(int button, int action, int mods) = 0;
	};

	class MouseMoveListener
	{
	public:
		virtual ~MouseMoveListener() = default;
		virtual void mouseMoveEvent(float x, float y) = 0;
	};

	class InputManager : public Singleton<InputManager> {
		friend Singleton;

	private:
		enum Events {
			SCREENSHOT,
			BOOM, DOLLY, DOLLY_SPEED_UP, ORBIT_XZ, ORBIT_Y, PAN, RESET, TILT, TRUCK, ZOOM, ROTATION,
			zoomOutAnim,
			NUM_EVENTS
		};

	private:
		static ApplicationState			_applicationState;
		static const vec2				_defaultCursorPosition;

	public:
		inline static bool				_dragWindow = false;

	private:
		std::vector<ivec2>				_eventKey;
		vec2							_lastCursorPosition;
		bool							_leftClickPressed, _rightClickPressed, _scrollClickPressed;
		std::vector<uint32_t>			_moves;
		float							_movementMultiplier;
		std::vector<float>				_moveSpeed;
		float							_moveSpeedUp;
		GLFWwindow* _window;

	private:
		// Observer pattern
		std::vector<ResizeListener*>		_resizeListeners;
		std::vector<ScreenshotListener*>	_screenshotListeners;
		std::vector<MouseButtonListener*>	_mouseButtonListeners;
		std::vector<MouseMoveListener*>		_mouseDragListeners;

	private:
		InputManager();
		void buildMovementBuffers();
		bool mouseMovement(float xPos, float yPos);
		void processPressedKeyEvent(int key, int mods);
		void processReleasedKeyEvent(int key, int mods);

		static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void windowRefreshCallback(GLFWwindow* window);
		static void dropCallback(GLFWwindow* window, int count, const char** paths);

	public:
		virtual ~InputManager();
		static ApplicationState* getApplicationState() { return &_applicationState; }
		void init(GLFWwindow* window);
		void updateMovementSteps();

	public:
		void subscribeResize(ResizeListener* listener);
		void subscribeScreenshot(ScreenshotListener* listener);
		void subscribeMouseButton(MouseButtonListener* listener);
		void unsubscribeMouseButton(const MouseButtonListener* listener);
		void subscribeMouseMove(MouseMoveListener* listener);
		void unsubscribeMouseMove(const MouseMoveListener* listener);
	};
}

