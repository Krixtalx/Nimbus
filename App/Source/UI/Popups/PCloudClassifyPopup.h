#pragma once
#include "UI/I_GUIPopup.h"
#include "IO/FileManager.h"

namespace Nimbus {

	/**
	 * \brief Custom point cloud classification popup: questions the user how to load the point cloud file
	 */
	class PCloudClassifyPopup : public I_GUIPopup {
		using ClassifyCallback = std::function<void(const std::string& directory, const std::string& filename, bool useClassification)>;
	protected:

		/**
		 * \brief Callback function target directory, usually the point cloud file directory
		 */
		std::string _directory;
		
		/**
		 * \brief Callback function target filename, usually the point cloud filename
		 */
		std::string _filename;
		
		/**
		 * \brief Confirmation callback function
		 */
		ClassifyCallback _selectionCallback;

	public:

		PCloudClassifyPopup(const std::string& id, const LOCALE_STRINGS idx, const std::string& directory, const std::string& filename, const ClassifyCallback& callback) :
			I_GUIPopup(id, idx), _directory(directory), _filename(filename), _selectionCallback(callback) {}
		
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

inline void Nimbus::PCloudClassifyPopup::openPopup() {
	ImGui::OpenPopup(getIdTitle().c_str());
}

inline bool Nimbus::PCloudClassifyPopup::renderPopup(ApplicationState* appState, Scene* scene) {

	bool closing = false;
	static bool useClassification;

	if (ImGui::BeginPopupModal(getIdTitle().c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Checkbox(LocaleStrings::getInstance()->getString(LOCALE_STRINGS::POPUP_CLOUDCLASS_INFO).c_str(), &useClassification);

		if (useClassification) {
			for (int i = 0; i < 13; ++i) {
				ImGui::AlignTextToFramePadding();
				ImGui::Text((LocaleStrings::getInstance()->getString(LOCALE_STRINGS::POPUP_CLOUDCLASS) + " %-3i").c_str(), i);
				ImGui::SameLine();
				ImGui::InputText(("##classifLayer" + std::to_string(i)).c_str(), &Nimbus::FileManager::LASClassificationStrings[i]);
			}
		}

		if (GuiUtilities::makeConfirmButton()) {
			closing = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (closing) {
		_selectionCallback(_directory, _filename, useClassification);
	}

	return closing;
};
