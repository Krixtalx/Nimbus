#pragma once
#include "UI/I_GUIWindow.h"

namespace Nimbus {
	/**
	 * \brief Camera-based axis window: shows world axis relative to camera movements
	 */
	class CameraAxisWindow : public I_GUIWindow {
	protected:

	public:
		CameraAxisWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags = 0) :
			I_GUIWindow(id, idx, open, flags) {}
		
		/**
		 * \brief Renders the window and contents
		 */
		void renderWindow() override;
	};
}
