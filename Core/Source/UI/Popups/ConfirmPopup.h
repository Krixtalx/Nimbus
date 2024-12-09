#pragma once
#include "UI/GuiUtilities.h"
#include "UI/I_GUIPopup.h"
namespace Nimbus {

	/**
	 * \brief Simple yes/no popup modal window
	 */
	class ConfirmPopup : public I_GUIPopup {
	protected:
		/**
		 * \brief Main informative text (over yes/no buttons)
		 */
		LOCALE_STRINGS _infoTextIdx;

		/**
		 * \brief Confirmation buttons text:
		 * \brief	true => Yes/No
		 * \brief	false => Confirm/Cancel
		 */
		bool _yesnoButtons;

		/**
		 * \brief "Yes" button press callback action
		 */
		std::function<void(bool)> _selectionCallback;

	public:

		ConfirmPopup(const std::string& id, const LOCALE_STRINGS idx, const LOCALE_STRINGS infoTextIdx, const bool useYesNo, const std::function<void(bool)>& callback) :
			I_GUIPopup(id, idx), _infoTextIdx(infoTextIdx), _yesnoButtons(useYesNo), _selectionCallback(callback) {
		}

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

inline void Nimbus::ConfirmPopup::openPopup() {
	ImGui::OpenPopup(getIdTitle().c_str());
}

inline bool Nimbus::ConfirmPopup::renderPopup(ApplicationState* appState, Scene* scene) {

	bool closing = false;
	bool selection = false;

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 93), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowSize(ImVec2(300, 93), ImGuiCond_FirstUseEver);
	if (ImGui::BeginPopupModal(getIdTitle().c_str())) {
		GuiUtilities::makeLocText(_infoTextIdx, true);
		if (_yesnoButtons ? GuiUtilities::makeYesButton() : GuiUtilities::makeConfirmButton()) {
			selection = true;
			closing = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (_yesnoButtons ? GuiUtilities::makeNoButton() : GuiUtilities::makeCancelButton()) {
			closing = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (closing) {
		_selectionCallback(selection);
	}

	return closing;
};