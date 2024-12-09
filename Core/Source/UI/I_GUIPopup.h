#pragma once

#include "Utilities/LocaleStrings.h"

namespace Nimbus {
	class Scene;
	struct ApplicationState;

	/**
	 * \brief ImGUI popups interface adapter class
	 */
	class I_GUIPopup {
	protected:
		/**
		 * \brief ImGUI popup window ID
		 */
		std::string _popupID;

		/**
		 * \brief Popup title string index in locales collection
		 */
		LOCALE_STRINGS _popupTitleIdx;
		
		I_GUIPopup(const std::string& id, const LOCALE_STRINGS idx) : _popupID("###" + id), _popupTitleIdx(idx) {}

	public:
		I_GUIPopup() = delete;
		I_GUIPopup(const I_GUIPopup& other) = delete;

		virtual ~I_GUIPopup() = default;

		/**
		 * \brief Sets up ImGUI to open a popup window
		 */
		virtual void openPopup() = 0;

		/**
		 * \brief Renders the popup and its contents
		 *
		 * \param appState Main application state
		 * \param scene Currently active scene
		 * \return true if popup closes, false otherwise
		 */
		virtual bool renderPopup(ApplicationState* appState, Scene* scene) = 0;

		std::string getTitle() const {
			return LocaleStrings::getInstance()->getString(_popupTitleIdx);
		}

		std::string getIdTitle() const {
			return LocaleStrings::getInstance()->getString(_popupTitleIdx) + _popupID;
		}

		bool isOpen() const {
			return ImGui::IsPopupOpen(getIdTitle().c_str());
		}

	};

}

