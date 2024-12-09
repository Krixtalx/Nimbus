#include "CorePch.h"
#include "I_GUIWindow.h"

#include "Utilities/LocaleStrings.h"

Nimbus::I_GUIWindow::I_GUIWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags) :
	_windowID("###" + id), _windowTitleIdx(idx), _windowOpen(open), _windowFlags(flags) {

}

std::string Nimbus::I_GUIWindow::getIdTitle() const {
	return LocaleStrings::getInstance()->getString(_windowTitleIdx) + _windowID;
}

std::string Nimbus::I_GUIWindow::getTitle() const {
	return LocaleStrings::getInstance()->getString(_windowTitleIdx);
}
