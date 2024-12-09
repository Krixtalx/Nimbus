#pragma once
#include "UI/I_GUIPopup.h"

namespace Nimbus {
	class PointCloud;

	/**
	 * \brief ImGUIFileDialog popup wrapper for point cloud file selection
	 */
	class CloudExportDialogPopup : public I_GUIPopup {
		using FileDialogCallback = std::function<void(const std::string& directory, const std::string& filename, Nimbus::PointCloud& targetCloud) > ;

	protected:
		/**
		 * \brief Scene target cloud to export
		 */
		Nimbus::PointCloud* _sceneTarget;

		/**
		 * \brief ImGUIFileDialog file formats for searching
		 */
		std::string _fileFormat;

		/**
		 * \brief IMGUIFIleDialog popup flags
		 */
		ImGuiFileDialogFlags _dialogFlags;

		/**
		 * \brief Popup result callback function
		 */
		FileDialogCallback _selectionCallback;

	public:

		CloudExportDialogPopup(const std::string& id, const LOCALE_STRINGS idx, Nimbus::PointCloud* sceneTarget,
		                       std::string format, const ImGuiFileDialogFlags flags, const FileDialogCallback& callback) :
			I_GUIPopup(id, idx), _sceneTarget(sceneTarget), _fileFormat(std::move(format)), _dialogFlags(flags), _selectionCallback(callback) {}
		
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

inline void Nimbus::CloudExportDialogPopup::openPopup() {
	ImGuiFileDialog::Instance()->OpenDialog(_popupID, getTitle(), _fileFormat.c_str(), "", 1, nullptr, _dialogFlags);
}

inline bool Nimbus::CloudExportDialogPopup::renderPopup(ApplicationState* appState, Scene* scene) {

	if (ImGuiFileDialog::Instance()->Display(_popupID, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse, ImVec2(500, 250), ImVec2(FLT_MAX, FLT_MAX))) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string fileDialogSelection = ImGuiFileDialog::Instance()->GetCurrentFileName();
			fileDialogSelection = fileDialogSelection.substr(0, fileDialogSelection.find_last_of('.'));
			const std::string fileDialogDirectory = ImGuiFileDialog::Instance()->GetCurrentPath();

			_selectionCallback(fileDialogDirectory, fileDialogSelection, *_sceneTarget);

		}

		ImGuiFileDialog::Instance()->Close();
	}

	return !ImGuiFileDialog::Instance()->IsOpened();
};



