#include "AppPch.h"
#include "StatsWindow.h"

#include "implot.h"
#include "Graphics/GPUResources.h"
#include "Graphics/Renderer.h"
#include "Managers/MemoryManager.h"
#include "UI/GuiUtilities.h"

void Nimbus::StatsWindow::updateStats() {

	static double now = 0;
	now += ImGui::GetIO().DeltaTime; // DeltaTime comes in seconds; "now" is then also seconds

	if (now > _maxTime) {
		now -= _halfMaxTime;

		// Move vector to half
		const size_t halfSize = _timePlot.size() / 2;

		for (int i = halfSize - 1; i >= 0; i--) {
			const size_t iHalf = i + halfSize;
			_timePlot[i] = _timePlot[iHalf] - _halfMaxTime;
			_fpsPlot[i] = _fpsPlot[iHalf];
			_frametimePlot[i] = _frametimePlot[iHalf];


			_timePlot.pop_back();
			_fpsPlot.pop_back();
			_frametimePlot.pop_back();
		}
	}

	_timePlot.push_back(now);

	const double nowFPS = ImGui::GetIO().Framerate;
	if (nowFPS > _maxFps) _maxFps = nowFPS;
	_fpsPlot.push_back(nowFPS);

	const double nowFrametime = ImGui::GetIO().DeltaTime;
	if (nowFrametime > _maxFramet) _maxFramet = nowFrametime;
	_frametimePlot.push_back(ImGui::GetIO().DeltaTime);
}

void Nimbus::StatsWindow::renderWindow() {
	updateStats();

	//ImGui::ShowMetricsWindow();

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 420), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowSize(ImVec2(600, 420), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(getIdTitle().c_str(), &_windowOpen, _windowFlags)) {
		ImGui::Text(std::format("FPS: {:.0f}", ImGui::GetIO().Framerate).c_str());
		if (GuiUtilities::CollapsingHeader(STATS_PLOT_FPS, "fps_collheader")) {
			if (ImPlot::BeginPlot("##FPS_plot")) {
				ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, _maxTime);
				ImPlot::SetupAxisLimits(ImAxis_Y1, _minFps, _maxFps);

				ImPlot::PlotLine("", _timePlot.data(), _fpsPlot.data(), _timePlot.size());

				ImPlot::EndPlot();
			}
		}

		ImGui::Text(std::format("Frametime: {:.2f}", ImGui::GetIO().DeltaTime * 1000).c_str());
		if (GuiUtilities::CollapsingHeader(STATS_PLOT_FRAMETIME, "frametime_collheader")) {
			if (ImPlot::BeginPlot("##Frametime_plot")) {
				ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, _maxTime);
				ImPlot::SetupAxisLimits(ImAxis_Y1, _minFramet, _maxFramet);

				ImPlot::PlotLine("", _timePlot.data(), _frametimePlot.data(), _timePlot.size());

				ImPlot::EndPlot();
			}
		}
		ImGui::Text(std::format("RAM: {:.1f}MB", Nimbus::MemoryManager::currentRamUsage()).c_str());
		ImGui::Text(std::format("Vram: {:.1f}MB", Nimbus::MemoryManager::currentVramUsage()).c_str());
		ImGui::Text(std::format("SSBOs memory usage: {:.1f}MB", Nimbus::GPUResources::totalUsedMemory / 1024.f / 1024.f).c_str());
		ImGui::Text(std::format("Rendered Points: {} points", Renderer::getInstance()->getAppState()->_renderedPoints[0]).c_str());
	}
	ImGui::End();
}