#include "AppPch.h"
#include "InspectorWindow.h"

#include "Graphics/Renderer.h"
#include "Graphics/TextureList.h"
#include "UI/GuiUtilities.h"
#include "UI/Popups/FileDialogPopup.h"
#include "UI/Popups/PCloudClassifyPopup.h"

int Nimbus::InspectorWindow::_selectedInspectorItem = -1;
std::string Nimbus::InspectorWindow::_selectedItemName;
Nimbus::InspectorWindow::SceneItemType Nimbus::InspectorWindow::_selectedItemType = SceneItemType::None;

bool Nimbus::InspectorWindow::showContextPopup(const bool disabled) {
	bool deleteClicked = false;
	if (ImGui::BeginPopupContextItem(nullptr, 1 | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
		if (!disabled) {
			deleteClicked = GuiUtilities::Selectable(GENERIC_DELETE, "inspectorwin_context_delete", false);
		} else {
			ImGui::TextDisabled(LocaleStrings::getInstance()->getString(GENERIC_DELETE).c_str());
		}
		ImGui::EndPopup();
	}

	return deleteClicked;
}

void Nimbus::InspectorWindow::setInspectorItem(const std::string& name, const SceneItemType type, const int idx) {
	_selectedItemName = name;
	_selectedItemType = type;
	_selectedInspectorItem = idx;
}

bool Nimbus::InspectorWindow::isItemSelected(const int idx, const SceneItemType type) {
	return idx == _selectedInspectorItem && _selectedItemType == type;
}

void Nimbus::InspectorWindow::resetInspectorSelection() {
	_selectedInspectorItem = -1;
	_selectedItemName = "";
	_selectedItemType = SceneItemType::None;
}

void Nimbus::InspectorWindow::renderWindow() {

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 150), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(getIdTitle().c_str(), &_windowOpen, _windowFlags)) {
		static Renderer* _renderer = Renderer::getInstance();

		if (ImGui::BeginPopupContextWindow(nullptr, 1 | ImGuiPopupFlags_NoOpenOverItems)) {
			if (GuiUtilities::Selectable(MENUBAR_CONTENT_IMPORTCLOUD, ICON_FA_FILE_IMPORT, "inspectorcontext_importcloud", false, 0)) {
				auto fileDialogCallback = [](const std::string& directory, const std::string& filename, const std::string& fileExtension) {
					auto classificationCallback = [fileExtension](const std::string& directory, const std::string& filename, bool useClassification) {
						std::thread loadThread(&Nimbus::FileManager::loadPointCloud, directory + "/" + filename + "." + fileExtension, useClassification);
						loadThread.detach();
						Nimbus::Renderer::getInstance()->getActiveScene()->dirty = true;
						};
					Nimbus::PopupSystem::addPopup(new PCloudClassifyPopup("cloud_classif", POPUP_CLOUDCLASS, directory, filename, classificationCallback));
					};
				Nimbus::PopupSystem::addPopup(new FileDialogPopup("open_cloud", POPUP_OPEN_FILE, FileDialogRegex::FileDialogType::POINT_CLOUDS, ImGuiFileDialogFlags_Modal, fileDialogCallback));
			}

			ImGui::Separator();

			if (GuiUtilities::Selectable(MENUBAR_CONTENT_NEWLIGHT, ICON_FA_LIGHTBULB, "inspectorcontext_newlight", false, 0)) {
				const unsigned lightIdx = _renderer->addLight();
				_renderer->getActiveScene()->_light[lightIdx]->setName("Light #" + std::to_string(lightIdx));
				_renderer->getActiveScene()->dirty = true;
			}

			if (GuiUtilities::Selectable(MENUBAR_CONTENT_NEWCAMERA, ICON_FA_CAMERA, "inspectorcontext_newcamera", false, 0)) {
				const unsigned cameraIdx = _renderer->addCamera();
				_renderer->getActiveScene()->_camera[cameraIdx]->setName("Camera #" + std::to_string(cameraIdx));
				const auto aCam = _renderer->getActiveScene()->getActiveCamera();
				_renderer->getActiveScene()->_camera[cameraIdx]->setRaspect(aCam->getRaspect());
				_renderer->getActiveScene()->dirty = true;
			}

			ImGui::EndPopup();
		}

		ImVec4* styleColors = ImGui::GetStyle().Colors;
		const ImVec4 oldSelColor = styleColors[ImGuiCol_Header], inspSelColor = styleColors[ImGuiCol_Tab];

		if (GuiUtilities::CollapsingHeader(SCENEITEM_CAMERA, ICON_FA_CAMERA, "camera_header")) {
			for (int c = 0; c < _renderer->getActiveScene()->_camera.size(); c++) {
				Camera* cCam = _renderer->getActiveScene()->_camera[c].get();

				styleColors[ImGuiCol_Header] = inspSelColor;

				if (GuiUtilities::Selectable(cCam->getName(), "camera_" + std::to_string(c), isItemSelected(c, SceneItemType::Camera), ImGuiSelectableFlags_AllowDoubleClick)) {
					setInspectorItem(cCam->getName(), SceneItemType::Camera, c);
				}

				if (showContextPopup(_renderer->getActiveScene()->_camera.size() > 1)) {
					_renderer->deleteCamera(c);
					resetInspectorSelection();
					_renderer->getActiveScene()->dirty = true;
				}

				styleColors[ImGuiCol_Header] = oldSelColor;
			}
		}

		if (GuiUtilities::CollapsingHeader(SCENEITEM_LIGHT, ICON_FA_LIGHTBULB, "lights_header")) {
			for (int c = 0; c < _renderer->getActiveScene()->_light.size(); c++) {
				Light* cLight = _renderer->getActiveScene()->_light[c].get();

				styleColors[ImGuiCol_Header] = inspSelColor;

				if (GuiUtilities::Selectable(cLight->getName(), "light_" + std::to_string(c), isItemSelected(c, SceneItemType::Light), ImGuiSelectableFlags_AllowDoubleClick)) {
					setInspectorItem(cLight->getName(), SceneItemType::Light, c);

					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						_renderer->getCamera()->lookPosition(cLight->getProperties()->_position);
						_renderer->getActiveScene()->dirty = true;
					}
				}

				if (showContextPopup(_renderer->getActiveScene()->_light.size() > 1)) {
					_renderer->deleteLight(c);
					resetInspectorSelection();
					_renderer->getActiveScene()->dirty = true;
				}

				styleColors[ImGuiCol_Header] = oldSelColor;
			}
		}

		if (GuiUtilities::CollapsingHeader(SCENEITEM_PCLOUD, ICON_FA_CLOUD, "pcloud_header")) {
			std::string itemToDelete;
			int c = 0;
			for (auto pcIt = _renderer->getActiveScene()->_pointClouds.begin(); pcIt != _renderer->getActiveScene()->_pointClouds.end(); ++pcIt, c++) {
				styleColors[ImGuiCol_Header] = inspSelColor;

				if (GuiUtilities::Selectable(pcIt->second->getName(), "pCloud_" + std::to_string(c), isItemSelected(c, SceneItemType::PCloud), ImGuiSelectableFlags_AllowDoubleClick)) {
					setInspectorItem(pcIt->first, SceneItemType::PCloud, c);

					_renderer->getCamera()->trackModel(pcIt->second.get());
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						_renderer->getCamera()->lookModel(pcIt->second.get());
					}
				}

				if (showContextPopup(false)) {
					itemToDelete = pcIt->first;
				}

				styleColors[ImGuiCol_Header] = oldSelColor;
			}
			if (!itemToDelete.empty()) {
				_renderer->deletePointCloud(itemToDelete);
				_renderer->getCamera()->untrackModel();
				resetInspectorSelection();
				_renderer->getActiveScene()->dirty = true;
			}
		}

		if (GuiUtilities::CollapsingHeader(SCENEITEM_IMAGE, ICON_FA_IMAGES, "images_header")) {
			auto images = TextureList::getInstance()->getAllImageTex();
			int c = 0;
			for (auto texIt = images.begin(); texIt != images.end(); ++texIt, c++) {
				styleColors[ImGuiCol_Header] = inspSelColor;

				if (GuiUtilities::Selectable(*texIt, "image_" + std::to_string(c), isItemSelected(c, SceneItemType::Image), ImGuiSelectableFlags_AllowDoubleClick)) {
					setInspectorItem(*texIt, SceneItemType::Image, c);
				}

				styleColors[ImGuiCol_Header] = oldSelColor;

				// if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
				// 	ImGui::SetDragDropPayload(GuiUtilities::DragDropIDs::INSPECTORLIST_IMAGE.c_str(), texIt->c_str(), itemNameID.size());
				//
				// 	// Drag preview
				// 	ImGui::Text(texIt->c_str());
				// 	auto texDragged = TextureList::getInstance()->isTextureAvailable(*texIt);
				// 	ImGui::Image(reinterpret_cast<ImTextureID>(texDragged->getId()), ImVec2(32, 32));
				//
				// 	ImGui::EndDragDropSource();
				// }
			}
		}
	}
	ImGui::End();
}

