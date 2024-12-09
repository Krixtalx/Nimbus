#include "CorePch.h"
#include "PopupSystem.h"

void Nimbus::PopupSystem::addPopup(I_GUIPopup* popup) {
	getInstance()->_pendingPopups.push(popup);
}