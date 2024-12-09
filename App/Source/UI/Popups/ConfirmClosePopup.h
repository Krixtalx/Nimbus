#pragma once
#include "UI/I_GUIPopup.h"

namespace Nimbus {
	class ConfirmClosePopup : public I_GUIPopup {

	public:
		bool renderGuiObject(ApplicationState* appState, Scene* scene, void* value) override;
	};
}

inline bool Nimbus::ConfirmExitPopup::renderGuiObject(ApplicationState* appState, Scene* scene, void* value) {
	
	bool closing = false;

	if (ImGui::BeginPopupModal(_popupTitle.c_str())) {
		ImGui::Text("The scene has unsaved changes. Continue?");
		if (GuiUtilities::makeButton("Yes")) {
			
			closing = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (GuiUtilities::makeButton("No")) {
			closing = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	return closing;
};