#pragma once
#include "UI/I_GUIWindow.h"

namespace Nimbus {
	/**
	 * \brief Scene inspector window: lists scene contents
	 */
	class InspectorWindow : public I_GUIWindow {
		friend class DetailsWindow;
		friend class MaterialsWindow;

	protected:
		/**
		 * \brief Available scene content types, used in selection
		 */
		enum class SceneItemType {
			None, Camera, Light, Material, PCloud, Model3D, Image
		};
		/**
		 * \brief Inspector list selected item index;
		 */
		static int _selectedInspectorItem;
		/**
		 * \brief Inspector list selected item name;
		 */
		static std::string _selectedItemName;
		/**
		 * \brief Inspector list selected item type;
		 */
		static SceneItemType _selectedItemType;

		/**
		 * \brief Renders ImGUI context popup with scene content deletion button
		 * 
		 * \param disabled Popup contents disabled?
		 * \return true if delete button pressed, false otherwise
		 */
		static bool showContextPopup(bool disabled);

		/**
		 * \brief Updates inspector selection status
		 * 
		 * \param name New selected item name
		 * \param type New selected item type
		 * \param idx New selected item index
		 */
		static void setInspectorItem(const std::string& name, SceneItemType type, int idx);

		/**
		 * \brief Checks if item parameters equals to currently selected item
		 * 
		 * \param idx Item index to check
		 * \param type Item type to check
		 * \return true if parameters and current selection are the same value, false otherwise
		 */
		static bool isItemSelected(int idx, SceneItemType type);

	public:
		InspectorWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags = 0) :
			I_GUIWindow(id, idx, open, flags) {}

		/**
		 * \brief Resets inspector selection to no item selection
		 */
		static void resetInspectorSelection();

		/**
		 * \brief Renders the window and contents
		 */
		void renderWindow() override;
	};
}
