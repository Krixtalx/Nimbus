#pragma once
#include "Utilities/Singleton.h"

#include "UI/I_GUIPopup.h"
#include "UI/I_GUIWindow.h"

namespace Nimbus {
	class Texture;
	class PointCloud;
	class InterfaceAdapter;
	class Renderer;
	class Application;
	/**
	 * \brief ImGUI wrapper and interface manager
	 */
	class GUI final : public Singleton<GUI> {
		friend Singleton;
	public:
		ImVec2 MIN_POPUP_SIZE;
		ImVec2 MAX_POPUP_SIZE;
		ImVec2 MIN_WINDOW_SIZE;
		ImVec2 MAX_WINDOW_SIZE;

		static bool WINDOW_CLOSE_PENDING;

		float testProg = 0.0f;
	protected:
#pragma region PROTECTED_MEMBERS

		/**
		 * \brief Pointer to main Renderer instance
		 */
		Renderer* _renderer;

		/**
		 * \brief Pointer to currently active popup window
		 */
		I_GUIPopup* _currentPopup = nullptr;

		/**
		 * \brief List of main application windows
		 */
		std::list<I_GUIWindow*> _coreWindows;

		enum class OTHER_WINDOWS {
			DATABASE_NEWRAW,
			DATABASE_DOWNLOADED,
			POINT_PICKING,
			ERROR_REPORT,
			count
		};
		std::unordered_map<OTHER_WINDOWS, I_GUIWindow*> _otherWindows;

		/**
		 * \brief Pointer to currently active functionality module
		 */
		InterfaceAdapter* _selectedModule;

		GLFWwindow* _glfwWindowHandle;


#pragma endregion

#pragma region PROTECTED_METHODS

		static void loadImGUIStyle();

		/**
		 * \brief Renders and handles next popup window in queue
		 */
		void handleNextPopup();

		/**
		 * \brief Makes docking node over main OpenGL rendering viewport
		 */
		static void makeMainDockNode();

		GUI();

#pragma endregion

	public:

#pragma region PUBLIC_METHODS

		virtual ~GUI();

		/**
		 * \brief Deletes currently active scene and resets inspector status
		 */
		void deleteCurrentScene() const;

		/**
		 * \brief Starts ImGUI context
		 *
		 * \param window Main view window
		 * \param openGLMinorVersion
		 */
		void initialize(GLFWwindow* window, int openGLMinorVersion);

		/**
		 * \brief Draws ImGUI elements
		 */
		void render();

		static uint16_t getFrameRate() {
			return static_cast<uint16_t>(ImGui::GetIO().Framerate);
		}

		static bool isMouseActive() {
			return ImGui::GetIO().WantCaptureMouse;
		}

		static bool isKeyboardActive() {
			return ImGui::GetIO().WantCaptureKeyboard;
		}

#pragma endregion
	};
}

