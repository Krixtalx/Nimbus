#pragma once
#include "UI/I_GUIPopup.h"
#include "UI/Addons/FileDialog/ImGuiFileDialog.h"
#include <Utilities/FileDialogRegex.hpp>

namespace Nimbus {

	/**
	 * \brief ImGUIFileDialog popup wrapper for system file selection
	 */
	class FileDialogPopup : public I_GUIPopup {
		using FileDialogCallback = std::function<void(const std::string& directory, const std::string& filename, const std::string& fileExtension)>;
		using FileDialogCallbackMulti = std::function<void(std::map<std::string, std::string> files)>;

	protected:
		/**
		 * \brief ImGUIFileDialog file formats for searching
		 */
		std::string _fileFormat;

		/**
		 * \brief IMGUIFIleDialog popup flags
		 */
		ImGuiFileDialogFlags _dialogFlags;

		/**
		 * \brief Number of max selected files
		*/
		int _selectionCount;

		/**
		 * \brief Popup result callback function
		 */
		FileDialogCallback _selectionCallback;

		/**
		 * \brief Multiple selection result callback function
		 */
		FileDialogCallbackMulti _multiSelectionCallback;

	public:

		/**
		 * \brief Opens file selection dialog for a single item
		 *
		 * \param id Popup id for internal use
		 * \param idx Locale string index for title string
		 * \param type File type with assigned format REGEX
		 * \param flags Dialog flags
		 * \param callback Selection callback function
		 */
		FileDialogPopup(const std::string& id, LOCALE_STRINGS idx, FileDialogRegex::FileDialogType type, ImGuiFileDialogFlags flags, FileDialogCallback callback);

		/**
		 * \brief Opens file selection dialog for multiple items
		 *
		 * \param id Popup id for internal use
		 * \param idx Locale string index for title string
		 * \param type File type with assigned format REGEX
		 * \param maxSelItems Item selection limit (0 for no limit)
		 * \param flags Dialog flags
		 * \param callback Selection callback function
		 */
		FileDialogPopup(const std::string& id, LOCALE_STRINGS idx, FileDialogRegex::FileDialogType type, int maxSelItems, ImGuiFileDialogFlags flags, FileDialogCallbackMulti callback);

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

inline Nimbus::FileDialogPopup::FileDialogPopup(
	const std::string& id,
	const LOCALE_STRINGS idx,
	const FileDialogRegex::FileDialogType type,
	const ImGuiFileDialogFlags flags,
	FileDialogCallback callback)
	:
	I_GUIPopup(id, idx),
	_dialogFlags(flags),
	_selectionCount(-1),
	_selectionCallback(std::move(callback)) {
	_fileFormat = getRegex(type);
}

inline Nimbus::FileDialogPopup::FileDialogPopup(
	const std::string& id,
	const LOCALE_STRINGS idx,
	const FileDialogRegex::FileDialogType type,
	const int maxSelItems,
	const ImGuiFileDialogFlags flags,
	FileDialogCallbackMulti callback)
	:
	I_GUIPopup(id, idx),
	_dialogFlags(flags),
	_selectionCount(maxSelItems),
	_multiSelectionCallback(std::move(callback)) {
	_fileFormat = getRegex(type);
}

inline void Nimbus::FileDialogPopup::openPopup() {
	ImGuiFileDialog::Instance()->OpenDialog(_popupID, getTitle(), _fileFormat.empty() ? nullptr : _fileFormat.c_str(), "", _selectionCount, nullptr, _dialogFlags);
}

inline bool Nimbus::FileDialogPopup::renderPopup(ApplicationState* appState, Scene* scene) {

	ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_FirstUseEver);
	if (ImGuiFileDialog::Instance()->Display(_popupID, 32 | ImGuiWindowFlags_NoDocking, ImVec2(500, 250), ImVec2(FLT_MAX, FLT_MAX))) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			if (_selectionCount == -1) {
				const std::string fileDialogDirectory = ImGuiFileDialog::Instance()->GetCurrentPath();
				std::string fileDialogSelection = ImGuiFileDialog::Instance()->GetCurrentFileName();
				const std::string fileDialogExtension = fileDialogSelection.substr(fileDialogSelection.find_last_of('.') + 1);
				fileDialogSelection = fileDialogSelection.substr(0, fileDialogSelection.find_last_of('.'));

				_selectionCallback(fileDialogDirectory, fileDialogSelection, fileDialogExtension);
			}
			else {
				const auto multiselection = ImGuiFileDialog::Instance()->GetSelection();

				_multiSelectionCallback(multiselection);
			}

		}

		ImGuiFileDialog::Instance()->Close();
	}

	return !ImGuiFileDialog::Instance()->IsOpened();
};


