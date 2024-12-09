#include "AppPch.h"
#include "CameraAxisWindow.h"


void Nimbus::CameraAxisWindow::renderWindow() {
	struct WindowResizeConstrains {
		static void SquareWindowConstrain(ImGuiSizeCallbackData* data) {
			data->DesiredSize.x = data->DesiredSize.y = glm::min(data->DesiredSize.x, data->DesiredSize.y);
		}
	};

	ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX), WindowResizeConstrains::SquareWindowConstrain);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Size.x * 0.8f - 100, ImGui::CalcTextSize("|").y + ImGui::GetStyle().FramePadding.y * 2.0f), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	const bool windowOpen = ImGui::Begin(getIdTitle().c_str(), &_windowOpen, _windowFlags);
	ImGui::PopStyleVar();

	if (windowOpen) {
		const auto drawList = ImGui::GetWindowDrawList();
		const ImVec2 winAreaMax = ImGui::GetWindowContentRegionMax() + ImGui::GetWindowPos();
		const ImVec2 winAreaMin = ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos();
		const ImVec2 winAreaSize = winAreaMax - winAreaMin;
		const ImVec2 winAreaHalfSize = winAreaSize * 0.5f;
		const ImVec2 winAreaCenter = winAreaMin + winAreaHalfSize;

		drawList->AddRectFilled(winAreaMin, winAreaMax, IM_COL32(64, 64, 64, 64), 4, ImDrawFlags_RoundCornersAll);

		auto const activeCamera = Renderer::getInstance()->getCamera();
		auto const cameraParams = activeCamera->getProperties();

		auto gizmoX = vec3({ 1.0f, 0.0f, 0.0f });
		auto gizmoY = vec3({ 0.0f, 1.0f, 0.0f });
		auto gizmoZ = vec3({ 0.0f, 0.0f, 1.0f });

		mat4 orthoProj = glm::ortho(-1, 1, -1, 1);

		mat4 cameraView = glm::lookAt(cameraParams._eye - cameraParams._lookAt, dvec3(0, 0, 0), cameraParams._up);
		mat4 cameraProj = orthoProj * cameraView;

		vec4 viewX = cameraView * vec4(gizmoX, 1.0f);
		vec4 viewY = cameraView * vec4(gizmoY, 1.0f);
		vec4 viewZ = cameraView * vec4(gizmoZ, 1.0f);
		vec4 projX = cameraProj * vec4(gizmoX, 1.0f);
		vec4 projY = cameraProj * vec4(gizmoY, 1.0f);
		vec4 projZ = cameraProj * vec4(gizmoZ, 1.0f);

		projX /= projX.w;
		projY /= projY.w;
		projZ /= projZ.w;

		// X at front
		if (viewX.z > viewY.z && viewX.z > viewZ.z)
		{
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projY.x, -projY.y) * (winAreaHalfSize), IM_COL32(0, 255, 0, 255), 5);
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projZ.x, -projZ.y) * (winAreaHalfSize), IM_COL32(0, 0, 255, 255), 5);
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projX.x, -projX.y) * (winAreaHalfSize), IM_COL32(255, 0, 0, 255), 5);
		} // Y at front
		else if (viewY.z > viewX.z && viewY.z > viewZ.z)
		{
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projX.x, -projX.y) * (winAreaHalfSize), IM_COL32(255, 0, 0, 255), 5);
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projZ.x, -projZ.y) * (winAreaHalfSize), IM_COL32(0, 0, 255, 255), 5);
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projY.x, -projY.y) * (winAreaHalfSize), IM_COL32(0, 255, 0, 255), 5);
		} // Z at front
		else
		{
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projX.x, -projX.y) * (winAreaHalfSize), IM_COL32(255, 0, 0, 255), 5);
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projY.x, -projY.y) * (winAreaHalfSize), IM_COL32(0, 255, 0, 255), 5);
			drawList->AddLine(winAreaCenter, winAreaCenter + ImVec2(projZ.x, -projZ.y) * (winAreaHalfSize), IM_COL32(0, 0, 255, 255), 5);
		}
	}
	ImGui::End();
}
