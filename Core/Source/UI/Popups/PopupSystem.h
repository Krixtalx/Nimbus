#pragma once
#include "UI/I_GUIPopup.h"
#include <queue>

namespace Nimbus {
	class PopupSystem : public Singleton<PopupSystem> {
		friend class GUI;
		std::queue<I_GUIPopup*> _pendingPopups;
		std::queue<I_GUIPopup*> _waitingPopups;

	public:
		static void addPopup(I_GUIPopup* popup);
	};
}


