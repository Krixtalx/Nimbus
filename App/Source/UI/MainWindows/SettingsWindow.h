#pragma once
#include "UI/I_GUIWindow.h"

namespace Nimbus {

	/**
	 * \brief Application settings window: handles application-wide parameters and behaviours
	 */
	class SettingsWindow : public I_GUIWindow {
	protected:
		/**
		 * @brief imgui.ini userdata header read start
		 * @return Pointer to userdata structure
		*/
		static void* readFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);
		/**
		 * @brief imgui.ini userdata value line reader
		 * @param ctx 
		 * @param handler 
		 * @param entry Pointer to userdata structure
		 * @param line read value
		*/
		static void lineFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line);
		/**
		 * @brief imgui.ini userdata writer function
		 * @param ctx 
		 * @param handler 
		 * @param out_buf string buffer with all imgui.ini content where to append new data
		*/
		static void writeFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf);
	public:

		SettingsWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags = 0) :
			I_GUIWindow(id, idx, open, flags) {

			ImGuiSettingsHandler userSettingsHandler;
			userSettingsHandler.TypeName = "UserData";
			userSettingsHandler.TypeHash = ImHashStr("UserData");
			userSettingsHandler.ReadOpenFn = readFn;
			userSettingsHandler.ReadLineFn = lineFn;
			userSettingsHandler.WriteAllFn = writeFn;
			//userSettingsHandler.ApplyAllFn = applySettings;

			ImGui::AddSettingsHandler(&userSettingsHandler);
		}

		/**
		 * \brief Renders the window and contents
		 */
		void renderWindow() override;
	};
}
