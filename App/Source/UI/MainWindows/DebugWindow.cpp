#include "AppPch.h"
#include "DebugWindow.h"
#include "Graphics/Renderer.h"

void Nimbus::DebugWindow::renderWindow() {
	if (ImGui::Begin("Debug window")) {
		if (_renderFBO)
			ImGui::Image((ImTextureID)(intptr_t)_renderFBO->getColorAttachmentId(), ImVec2(512, 512));
		else {
			_renderFBO = Renderer::getInstance()->getRenderFBO();
		}
		ImGui::End();
	}
}

void Nimbus::DebugWindow::setRenderFBO(FBORender* renderFBO) {
	_renderFBO = renderFBO;
}
