#pragma once
#include "UI/I_GUIWindow.h"

namespace Nimbus {
	class FBORender;

	class DebugWindow : public I_GUIWindow {
		FBORender* _renderFBO;

	public:
		DebugWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags)
			: I_GUIWindow(id, idx, open, flags), _renderFBO(nullptr) {
		}

		void renderWindow() override;

		void setRenderFBO(FBORender* renderFBO);
	};
}
