#pragma once

namespace Nimbus {
	/**
	 * \brief ImGUI window interface adapter class
	 */
	class I_GUIWindow {
	protected:
		/**
		 * \brief ImGUI window ID
		 */
		std::string _windowID;

		/**
		 * \brief Window title string index in locales collection
		 */
		LOCALE_STRINGS _windowTitleIdx;

		/**
		 * \brief Window open flags
		 */
		bool _windowOpen;

		/**
		 * \brief ImGUI custom window flags
		 */
		ImGuiWindowFlags _windowFlags;


	public:
		I_GUIWindow() = delete;
		I_GUIWindow(const I_GUIWindow& other) = delete;

		I_GUIWindow(const std::string& id, LOCALE_STRINGS idx, bool open, ImGuiWindowFlags flags = 0);
		virtual ~I_GUIWindow() = default;

		/**
		 * \brief Renders the window and contents
		 */
		virtual void renderWindow() = 0;

		std::string getIdTitle() const;

		std::string getTitle() const;

		void toggleWindow() {
			_windowOpen = !_windowOpen;
		}

		bool isOpen() const {
			return _windowOpen;
		}

		void setOpen() {
			_windowOpen = true;
		}
	};

}
