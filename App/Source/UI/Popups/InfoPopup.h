#pragma once
#include "UI/I_GUIPopup.h"

namespace Nimbus {
	/**
	 * \brief Simple informative popup: shows text and close button
	 */
	class InfoPopup : public I_GUIPopup {
	protected:
		/**
		 * \brief Popup message
		 */
		std::string _infoText;

	public:

		InfoPopup(const std::string& id, const LOCALE_STRINGS idx, const std::string& infoText) :
			I_GUIPopup(id, idx), _infoText(infoText) {}
		
		/**
		 * \brief Sets up ImGUI to open a popup window
		 */
		void openPopup() override;
		
		/**
		 * \brief Renders the popup and its contents
		 * 
		 * \param appState Main application state
		 * \param scene Currently active scene
		 * \return true if popup closes, false otherwise
		 */
		bool renderPopup(ApplicationState* appState, Scene* scene) override;
	};

}


inline void Nimbus::InfoPopup::openPopup() {
	ImGui::OpenPopup(getIdTitle().c_str());
}

inline bool Nimbus::InfoPopup::renderPopup(ApplicationState* appState, Scene* scene) {

	bool closing = false;

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 93), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowSize(ImVec2(300, 93), ImGuiCond_FirstUseEver);
	if (ImGui::BeginPopupModal(getIdTitle().c_str())) {
		ImGui::Text(_infoText.c_str());

		if (GuiUtilities::Button(GENERIC_CONFIRM, _popupID + "_confirm")) {
			closing = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return closing;
};
