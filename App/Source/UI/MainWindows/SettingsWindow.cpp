#include "AppPch.h"
#include "SettingsWindow.h"

#include "Managers/InputManager.h"
#include "UI/GuiUtilities.h"
#include "Utilities/LocaleStrings.h"
#include "UI/Popups/FileDialogPopup.h"

void* Nimbus::SettingsWindow::readFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) {
	return InputManager::getApplicationState();
}

void Nimbus::SettingsWindow::lineFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line) {
	ApplicationState* appState = static_cast<ApplicationState*>(entry);
	ImGuiIO& io = ImGui::GetIO();
	int iVal;
	vec3 vVal;
	float fVal;

	if (sscanf_s(line, "Lang=%i", &iVal) == 1)
	{
		LocaleStrings::getInstance()->setLocale(iVal);
	}
	else if (sscanf_s(line, "FPSmenu=%i", &iVal) == 1)
	{
		appState->_fpsMenuBar = iVal;
	}
	else if (sscanf_s(line, "BackColor=%f,%f,%f", &vVal[0], &vVal[1], &vVal[2]) == 3)
	{
		appState->_backgroundColor = vVal;
	}
	else if (sscanf_s(line, "ShowGrid=%i", &iVal) == 1)
	{
		appState->_renderGrid = iVal;
	}
	else if (sscanf_s(line, "GridColor=%f,%f,%f", &vVal[0], &vVal[1], &vVal[2]) == 3)
	{
		appState->_gridColor = vVal;
	}
	else if (sscanf_s(line, "DoubClickSpeed=%f", &fVal) == 1)
	{
		io.MouseDoubleClickTime = fVal;
	}
	else if (sscanf_s(line, "MouseDragThresh=%f", &fVal) == 1)
	{
		io.MouseDragThreshold = fVal;
	}
}

void Nimbus::SettingsWindow::writeFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf) {
	ApplicationState* appState = InputManager::getApplicationState();
	const ImGuiIO& io = ImGui::GetIO();

	out_buf->reserve(out_buf->size() + (sizeof(ApplicationState) + sizeof(int)) * 6); //sizeof(int) -> idx lang

	out_buf->append("[UserData][Settings]\n");

	out_buf->appendf("Lang=%i\n", LocaleStrings::getInstance()->getCurrentLocaleIdx());
	out_buf->appendf("FPSmenu=%i\n", appState->_fpsMenuBar);
	out_buf->appendf("BackColor=%f,%f,%f\n", appState->_backgroundColor[0], appState->_backgroundColor[1], appState->_backgroundColor[2]);
	out_buf->appendf("ShowGrid=%i\n", appState->_renderGrid);
	out_buf->appendf("GridColor=%f,%f,%f\n", appState->_gridColor[0], appState->_gridColor[1], appState->_gridColor[2]);
	out_buf->appendf("DoubClickSpeed=%f\n", io.MouseDoubleClickTime);
	out_buf->appendf("MouseDragThresh=%f\n", io.MouseDragThreshold);

	out_buf->append("\n");
}

void Nimbus::SettingsWindow::renderWindow() {

	ImGui::SetNextWindowSizeConstraints(ImVec2(400, 339), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowSize(ImVec2(400, 339), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(getIdTitle().c_str(), &_windowOpen, _windowFlags)) {
		ApplicationState* appState = InputManager::getApplicationState();
		// Language
		{
			if (ImGui::BeginTable("SettingsWin_lang_table", 2, ImGuiTableFlags_SizingStretchProp)) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				if (GuiUtilities::BeginCombo(SETTINGS_LANGUAGE, "lang_combo", LocaleStrings::getInstance()->getCurrentLocaleStr(), 0, true)) {

					auto& locales = LocaleStrings::getInstance()->getLocales();
					for (int i = 0; i < locales.size(); i++) {
						const bool is_selected = i == LocaleStrings::getInstance()->getCurrentLocaleIdx();
						if (GuiUtilities::ComboItem(locales[i], "lang_combo_" + std::to_string(i), is_selected)) {
							LocaleStrings::getInstance()->setLocale(i);
						}

						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}
				ImGui::EndTable();
			}
		}

		if (GuiUtilities::CollapsingHeader(SETTINGS_GENERAL, "generalsettings_header")) {
			if (ImGui::BeginTable("##generalTable", 2, ImGuiTableFlags_SizingStretchProp)) {
				// FPS on menubar
				ImGui::TableNextColumn();
				GuiUtilities::Checkbox(SETTINGS_GENERAL_SHOWFPS, "showFPS", &appState->_fpsMenuBar, true);

				// Background color
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				GuiUtilities::ColorPicker(SETTINGS_GENERAL_BGCOLOR, "bgroundColor", appState->_backgroundColor, 0, true);

				// Draw grid
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				GuiUtilities::Checkbox(SETTINGS_GENERAL_GRIDSHOW, "renderGrid", &appState->_renderGrid, true);

				//Meshlets aabbs
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				GuiUtilities::Checkbox(SETTINGS_GENERAL_SHOWMESHLETAABB, "meshletsAABBs", &appState->_renderMeshletsAABBs, true);

				// Render with EDL
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				GuiUtilities::Checkbox(SETTINGS_GENERAL_USEEDL, "renderWithEDL", &appState->_renderWithEDL, true);
				if (appState->_renderWithEDL) {
					ImGui::DragFloat("EDL Strength", &appState->_edlStrength, 1, 1, 1200);
				}

				// Grid color
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				GuiUtilities::ColorPicker(SETTINGS_GENERAL_GRIDCOLOR, "gridColor", appState->_gridColor, 0, true);


				ImGui::EndTable();
			}
		}

		if (GuiUtilities::CollapsingHeader(GENERIC_COLOR, "settingscolor_header")) {
			/*static ImGradientMark* draggingMark = nullptr;
			static ImGradientMark* selectedMark = nullptr;

			static auto selColorGradient = appState->_fusionGradient.begin()->first;

			if (ImGui::BeginTable("SettingsWin_fusioncolor_table", 2, ImGuiTableFlags_SizingStretchProp)) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				if (GuiUtilities::BeginCombo(SETTINGS_COLOR_GRADIENT, "settingscolor_combo", selColorGradient, 0, true)) {
					for (auto it = appState->_fusionGradient.begin(); it != appState->_fusionGradient.end(); ++it) {
						const bool is_selected = selColorGradient == it->first;
						if (GuiUtilities::ComboItem(it->first, "settingscolor_combo_" + it->first, is_selected)) {
							selColorGradient = it->first;
							draggingMark = nullptr;
							selectedMark = nullptr;
						}
						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}
				ImGui::EndTable();
			}

			ImGui::GradientEditor(&appState->_fusionGradient[selColorGradient], draggingMark, selectedMark, 4);*/
		}

		if (GuiUtilities::CollapsingHeader(SETTINGS_INPUT, "settingsinput_header")) {
			if (ImGui::BeginTable("##inputTable", 2, ImGuiTableFlags_SizingStretchProp)) {
				ImGuiIO& io = ImGui::GetIO();

				// // Double click speed
				// ImGui::TableNextColumn();
				// GuiUtilities::DragFloat(SETTINGS_INPUT_DCLICKSPEED, "dClickSpeed", &io.MouseDoubleClickTime, 0.01f, 0.01f, 1.0f, ".2f", 0, true);

				ImGui::TableNextRow();
				// Drag threshold
				ImGui::TableNextColumn();
				GuiUtilities::DragFloat(SETTINGS_INPUT_DRAGTHRESH, "dragThreshold", &io.MouseDragThreshold, 1.0f, 1.0f, 255.0f, "%.0f", 0, true);


				ImGui::EndTable();
			}
		}

	}
	ImGui::End();

	// ImGui::ShowMetricsWindow();
}
