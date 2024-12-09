#include "AppPch.h"

#include "MaterialsWindow.h"
#include "DetailsWindow.h"
#include "InspectorWindow.h"
#include "Graphics/MaterialList.h"
#include "UI/GuiUtilities.h"

void Nimbus::MaterialsWindow::renderWindow() {
	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 150), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(getIdTitle().c_str(), &_windowOpen, _windowFlags)) {
		ImVec4* styleColors = ImGui::GetStyle().Colors;
		const ImVec4 oldSelColor = styleColors[ImGuiCol_Header], inspSelColor = styleColors[ImGuiCol_Tab];

		int c = 0;
		auto matBegin = Nimbus::MaterialList::getInstance()->getBegin();
		const auto matEnd = Nimbus::MaterialList::getInstance()->getEnd();
		for (; matBegin != matEnd; ++matBegin, c++) {
			styleColors[ImGuiCol_Header] = inspSelColor;

			if (GuiUtilities::Selectable(matBegin->first, "mat_" + std::to_string(c), InspectorWindow::isItemSelected(c, InspectorWindow::SceneItemType::Material))) {

				InspectorWindow::setInspectorItem(matBegin->first, InspectorWindow::SceneItemType::Material, c);
				DetailsWindow::resetShaderInspector();
			}

			styleColors[ImGuiCol_Header] = oldSelColor;
		}
	}
	ImGui::End();
}

