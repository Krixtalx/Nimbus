#pragma once
#include "UI/I_GUIWindow.h"

namespace Nimbus {
	/**
	 * \brief Application materials list window: allow material selection for details window
	 */
	class MaterialsWindow : public I_GUIWindow {
	protected:

	public:
		MaterialsWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags = 0) :
			I_GUIWindow(id, idx, open, flags) {}
		
		/**
		 * \brief Renders the window and contents
		 */
		void renderWindow() override;
	};
}

