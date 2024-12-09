#pragma once

#include "Utilities/LocaleStrings.h"

namespace Nimbus {
	class Scene;
	struct ApplicationState;

	/**
	 * \brief Adapter pattern for GUI windows and functionality.
	 */
	class InterfaceAdapter {
	protected:
		/** \brief Method localized name index */
		LOCALE_STRINGS _locNameIdx;

		/** \brief Method icon */
		std::string _icon;

		/** \brief Method internal ID */
		std::string _methodId;

		bool _windowOpen;

		vec2 _sizeConstraintsMin;
		vec2 _sizeConstraintsMax;

	public:
		virtual ~InterfaceAdapter() = default;
		InterfaceAdapter() = delete;
		InterfaceAdapter(const std::string& id, const std::string& icon, LOCALE_STRINGS idx, const vec2& sizeConstaintsMin = vec2(0, 0), const vec2& sizeConstaintsMax = vec2(FLT_MAX, FLT_MAX));
		InterfaceAdapter(const InterfaceAdapter& o) = default;

		/**
		 * \brief Custom module GUI window contents
		 *
		 * \param appState Pointer to current application state
		 * \param sceneContent Pointer to currently active scene
		 */
		virtual void renderGuiObject(ApplicationState* appState, Scene* sceneContent) = 0;

		std::string getLocalizedName() const {
			return LocaleStrings::getInstance()->getString(_locNameIdx);
		};
		std::string getIconName() const {
			return _icon + getLocalizedName();
		};
		std::string getWindowName() const {
			return getLocalizedName() + _methodId;
		}

		bool* getWindowOpenPtr() {
			return &_windowOpen;
		};

		void enableWindow() {
			_windowOpen = true;
		}

		LOCALE_STRINGS getLocaleNameIdx() const {
			return _locNameIdx;
		}

		vec2 getConstraintsMin() const
		{
			return _sizeConstraintsMin;
		}
		vec2 getConstraintsMax() const
		{
			return _sizeConstraintsMax;
		}
	};
}
