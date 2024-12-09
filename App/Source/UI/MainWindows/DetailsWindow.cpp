#include "AppPch.h"

#include "DetailsWindow.h"

#include "InspectorWindow.h"
#include "DataTypes/Model3D.h"
#include "Graphics/MaterialList.h"
#include "Graphics/Texture.h"
#include "Graphics/TextureList.h"
#include "Graphics/Shaders/RenderingShader.h"
#include "UI/Popups/CloudExportDialogPopup.h"

int Nimbus::DetailsWindow::_shaderInspectorShSel = 0;
int Nimbus::DetailsWindow::_shaderInspectorSubrutSel = 0;
int Nimbus::DetailsWindow::_shaderInspectorUnifSel = 0;

void Nimbus::DetailsWindow::renderWindow() {

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 150), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(getIdTitle().c_str(), &_windowOpen, _windowFlags)) {
		if (InspectorWindow::_selectedInspectorItem < 0) {
			GuiUtilities::Text(DETAILS_NOSELECTION);
		} else {
			static Renderer* _renderer = Renderer::getInstance();


			switch (InspectorWindow::_selectedItemType) {
			case InspectorWindow::SceneItemType::Camera:
				_renderer->getActiveScene()->dirty |= makeCameraItemInspector();
				break;
			case InspectorWindow::SceneItemType::Light:
				_renderer->getActiveScene()->dirty |= makeLightItemInspector();
				break;
			case InspectorWindow::SceneItemType::Material:
				makeMaterialItemInspector();
				break;
			case InspectorWindow::SceneItemType::PCloud:
				_renderer->getActiveScene()->dirty |= makePCloudItemInspector();
				break;
			case InspectorWindow::SceneItemType::Image:
				_renderer->getActiveScene()->dirty |= makeImageItemInspector();
				break;
			default:
				GuiUtilities::Text(DETAILS_NOSELECTION);
				break;
			}
		}
	}
	ImGui::End();
}

void Nimbus::DetailsWindow::resetShaderInspector() {
	_shaderInspectorShSel = 0;
	_shaderInspectorSubrutSel = 0;
	_shaderInspectorUnifSel = 0;
}

bool Nimbus::DetailsWindow::makeCameraItemInspector() {
	static Renderer* _renderer = Renderer::getInstance();


	const auto selectedCamera = _renderer->getActiveScene()->_camera[InspectorWindow::_selectedInspectorItem].get();

	bool updateMatrices = false;

	if (GuiUtilities::Radio(DETAILS_CAM_ACTIVE, "details_camera_active", InspectorWindow::_selectedInspectorItem == _renderer->getActiveScene()->_activeCamera)) {
		_renderer->getActiveScene()->_activeCamera = InspectorWindow::_selectedInspectorItem;
	}

	const std::vector<std::string> projectionTitles = {
		LocaleStrings::getInstance()->getString(CAMERA_PROJ_PERSP),
		LocaleStrings::getInstance()->getString(CAMERA_PROJ_ORTHO)
	};
	if (GuiUtilities::BeginCombo(DETAILS_CAM_PROJECTION, "details_camera_projection", projectionTitles[selectedCamera->_properties._cameraType])) {
		for (int i = 0; i < projectionTitles.size(); i++) {
			const bool isSelected = i == selectedCamera->_properties._cameraType;

			if (GuiUtilities::Selectable(projectionTitles[i], "details_camera_projection_" + projectionTitles[i], isSelected)) {
				selectedCamera->_properties._cameraType = i;
				updateMatrices |= true;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginTable("###details_camera_table", 2, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		updateMatrices |= GuiUtilities::DragScalarN(DETAILS_POSITION, "details_camera_position", ImGuiDataType_Double, &selectedCamera->_properties._eye[0], 3, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, nullptr, nullptr, nullptr, 0, true);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		updateMatrices |= GuiUtilities::DragScalarN(DETAILS_CAM_LOOKAT, "details_camera_lookat", ImGuiDataType_Double, &selectedCamera->_properties._lookAt[0], 3, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, nullptr, nullptr, nullptr, 0, true);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		updateMatrices |= GuiUtilities::DragFloat(DETAILS_CAM_ZNEAR, "details_camera_znear", &selectedCamera->_properties._zNear, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX, "%.3f", 0, true);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		updateMatrices |= GuiUtilities::DragFloat(DETAILS_CAM_ZFAR, "details_camera_zfar", &selectedCamera->_properties._zFar, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX, "%.3f", 0, true);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		updateMatrices |= GuiUtilities::DragFloat(DETAILS_CAM_FOVY, "details_camera_fovy", &selectedCamera->_properties._fovY, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX, "%.3f", 0, true);

		ImGui::EndTable();
	}

	if (selectedCamera->_properties._cameraType == CameraProjection::ORTHOGRAPHIC) {
		updateMatrices |= GuiUtilities::DragFloatV(DETAILS_CAM_ORTHOCOORDS, "details_camera_orthocoords", selectedCamera->_properties._bottomLeftCorner, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX);
	}

	if (updateMatrices) {
		selectedCamera->updateMatrices();
	}

	return updateMatrices;
}

bool Nimbus::DetailsWindow::makeLightItemInspector() {
	static Renderer* _renderer = Renderer::getInstance();


	const auto selectedLight = _renderer->getActiveScene()->_light[InspectorWindow::_selectedInspectorItem].get();

	const char* lightTitle[] = { "Ambient", "Point", "Directional", "Spot", "Rim" };
	const char* attenuationTitle[] = { "Default", "Boundary Distance", "Pixar" };

	bool valuesChanged = false;

	if (ImGui::BeginTable("##light_props", 2, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableNextColumn();
		GuiUtilities::Combo(DETAILS_LIGHT_TYPE, "details_light_type", &selectedLight->_lightType, lightTitle, IM_ARRAYSIZE((lightTitle)), true);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		GuiUtilities::Combo(DETAILS_LIGHT_ATTN, "details_light_attn", &selectedLight->_attenuationType, attenuationTitle, IM_ARRAYSIZE((attenuationTitle)), true);

		ImGui::EndTable();
	}

	if (GuiUtilities::CollapsingHeader(DETAILS_3DPARAMS, "details_lighttype")) {
		if (ImGui::BeginTable("###details_lighttype_table", 2, ImGuiTableFlags_SizingStretchProp)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if (selectedLight->_lightType == LightModel::POINT_LIGHT or selectedLight->_lightType == LightModel::SPOT_LIGHT) {
				valuesChanged |= GuiUtilities::DragFloatV(DETAILS_POSITION, "details_light_position", selectedLight->_properties._position, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX, "%.3f", 0, true);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
			}
			if (selectedLight->_lightType == LightModel::DIRECTIONAL_LIGHT or selectedLight->_lightType == LightModel::SPOT_LIGHT) {
				valuesChanged |= GuiUtilities::DragFloatV(DETAILS_LIGHT_DIRECTION, "details_light_direction", selectedLight->_properties._direction, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX, "%.3f", 0, true);
			}

			ImGui::EndTable();
		}
	}

	if (GuiUtilities::CollapsingHeader(DETAILS_COLORS, "details_light_typeparams")) {
		if (ImGui::BeginTable("###details_typeparams_table", 2, ImGuiTableFlags_SizingStretchProp)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			valuesChanged |= GuiUtilities::ColorPicker(DETAILS_LIGHT_COLOR_IA, "details_light_color_ia", selectedLight->_properties._Ia, 0, true);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (selectedLight->_lightType != LightModel::AMBIENT_LIGHT) {
				valuesChanged |= GuiUtilities::ColorPicker(DETAILS_LIGHT_COLOR_ID, "details_light_color_id", selectedLight->_properties._Id, 0, true);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::ColorPicker(DETAILS_LIGHT_COLOR_IS, "details_light_color_is", selectedLight->_properties._Is, 0, true);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
			}
			if (selectedLight->_lightType == LightModel::SPOT_LIGHT) {
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_COLOR_SMOOTHING, "details_light_color_spotsmoothing", &selectedLight->_properties._exponentS, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 10.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_COLOR_PENUMBRACOS, "details_light_color_penumbracos", &selectedLight->_properties._penumbraAngle, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 90.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_COLOR_UMBRACOS, "details_light_color_umbracos", &selectedLight->_properties._umbraAngle, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, selectedLight->_properties._penumbraAngle, 90.0f, "%.3f", 0, true);
			}
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader((LocaleStrings::getInstance()->getString(LOCALE_STRINGS::DETAILS_LIGHT_ATTN) + "###details_lightparams").c_str())) {
		if (ImGui::BeginTable("###details_lightparams_table", 2, ImGuiTableFlags_SizingStretchProp)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if (selectedLight->_attenuationType == LightAttenuation::DEFAULT_DISTANCE) {
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_ATTN_C1, "details_light_attn_c1", &selectedLight->_properties._c1, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_ATTN_C2, "details_light_attn_c2", &selectedLight->_properties._c2, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_ATTN_C3, "details_light_attn_c3", &selectedLight->_properties._c3, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);
			}

			if (selectedLight->_attenuationType == LightAttenuation::CONTROLLED_DISTANCE) {
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_MINDISTANCE, "details_light_mindist", &selectedLight->_properties._minDistance, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_MAXDISTANCE, "details_light_maxdist", &selectedLight->_properties._maxDistance, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);
			}

			if (selectedLight->_attenuationType == LightAttenuation::PIXAR) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_PIXAR_C, "details_light_pixar_c", &selectedLight->_properties._factorC, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_PIXAR_CDIST, "details_light_pixar_cdist", &selectedLight->_properties._distC, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_PIXAR_MAXATTNFACT, "details_light_pixar_maxattn", &selectedLight->_properties._maxAttFactor, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_PIXAR_SEEXP, "details_light_pixar_seexp", &selectedLight->_properties._exponentSE, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_PIXAR_K0, "details_light_pixar_k0", &selectedLight->_properties._k0, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				valuesChanged |= GuiUtilities::DragFloat(DETAILS_LIGHT_PIXAR_K1, "details_light_pixar_k1", &selectedLight->_properties._k1, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, .0f, 1.0f, "%.3f", 0, true);
			}

			ImGui::EndTable();
		}
	}

	return valuesChanged;
}

bool Nimbus::DetailsWindow::makePCloudItemInspector() const {
	static Renderer* _renderer = Renderer::getInstance();

	bool valuesChanged = false;

	const auto selectedCloud = _renderer->getActiveScene()->_pointClouds[InspectorWindow::_selectedItemName].get();

	if (ImGui::BeginTable("##pcloud_btnsTable", 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextColumn();
		// ImGui::TableNextRow();

		if (GuiUtilities::Button(POPUP_EXPORT_CLOUD, "details_cloud_exportbtn")) {
			auto fileDialogCallback = [](const std::string& directory, const std::string& filename, Nimbus::PointCloud& sceneTarget) {
				Nimbus::FileManager::savePointCloud(directory + "/" + filename + ".ply", sceneTarget);
			};
			Nimbus::PopupSystem::addPopup(new CloudExportDialogPopup("export_cloud", LOCALE_STRINGS::POPUP_EXPORT_CLOUD, _renderer->getActiveScene()->_pointClouds[InspectorWindow::_selectedItemName].get(), ".ply", ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite, fileDialogCallback));
		}

		ImGui::EndTable();
	}

	if (ImGui::BeginTable("##pcloud_propsTable", 2, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		GuiUtilities::InputText(SCENEITEM_MATERIAL, "details_cloud_matname", &selectedCloud->_material->_name, "", ImGuiInputTextFlags_ReadOnly, true);
		// ImGui::TableNextColumn();
		// GuiUtilities::setNextTableItemAlign(WINDOW_DETAILS, false);
		// if (GuiUtilities::Button(WINDOW_DETAILS, "details_cloud_showmatbutton")) {
		// 	int c = 0;
		// 	auto matBegin = Nimbus::MaterialList::getInstance()->getBegin();
		// 	const auto matEnd = Nimbus::MaterialList::getInstance()->getEnd();
		// 	for (; matBegin != matEnd; ++matBegin, c++) {
		// 		if (selectedCloud->_material->_name == matBegin->first) {
		// 			InspectorWindow::setInspectorItem(matBegin->first, InspectorWindow::SceneItemType::Material, c);
		//
		// 			_shaderInspectorShSel = _shaderInspectorSubrutSel = _shaderInspectorUnifSel = -1;
		// 		}
		// 	}
		// }

		// Point count
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		int pCount = selectedCloud->getNumberOfPoints();
		GuiUtilities::InputInt(DETAILS_PCLOUD_POINTCOUNT, "details_cloud_pcount", &pCount, ImGuiInputTextFlags_ReadOnly, true);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		int meshletCount = selectedCloud->getMeshletNumber();
		GuiUtilities::InputInt(GENERIC_MESHLET, "details_cloud_meshlets", &meshletCount, ImGuiInputTextFlags_ReadOnly, true);

		ImGui::EndTable();
	}

	//GuiUtilities::Checkbox(DETAILS_PCLOUD_DEBUGVIEW, "details_cloud_debugview", &selectedCloud->_enableDebugView);

	int selectedValue = 0;
	if (ImGui::BeginTable("details_cloud_viewmodetable", 2, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		std::array<const char*, PointCloud::_renderAttributes.size()> availableRenderModes;
		std::array<int, PointCloud::_renderAttributes.size()> availableRenderModesIdx;
		availableRenderModes[0] = PointCloud::_renderAttributes[0];
		availableRenderModesIdx[0] = 0;
		int numValues = 1;
		for (int i = 1; i < PointCloud::_renderAttributes.size(); ++i) {
			if (selectedCloud->_availableDataBands[i] > 0) {
				availableRenderModes[numValues] = PointCloud::_renderAttributes[i];
				availableRenderModesIdx[numValues] = i;
				if ((u8)selectedCloud->_attToUse == i)
					selectedValue = numValues;
				numValues++;
			}
		}
		if (GuiUtilities::Combo(DETAILS_VIEW_MODE, "details_cloud_viewmodecombo", &selectedValue, availableRenderModes.data(), numValues, true)) {
			selectedCloud->_attToUse = (Attribute)availableRenderModesIdx[selectedValue];
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (selectedValue > 0) {
			const int bandLimit = selectedCloud->_availableDataBands[availableRenderModesIdx[selectedValue]] - 1;
			if (bandLimit > 0) {
				GuiUtilities::SliderInt(DETAILS_BAND, "###slider_band", &selectedCloud->_attributeBand, 0, bandLimit, "%.d", 0, true);
			}
		}
		ImGui::EndTable();
	}

	/*if (selectedCloud->haveNormals())
		GuiUtilities::Checkbox(DETAILS_NORMALS_VIEW, "details_cloud_viewnormals", &selectedCloud->_viewNormals);*/


	if (GuiUtilities::CollapsingHeader(DETAILS_3DPARAMS, "details_pcloud_3dparams")) {
		valuesChanged |= makeBasicTransformControls(*selectedCloud, "pCloud" + std::to_string(InspectorWindow::_selectedInspectorItem));
	}

	if (GuiUtilities::CollapsingHeader(DETAILS_PCLOUD_SUBCLOUDS, "details_pcloud_subclouds")) {
		auto& subClouds = selectedCloud->getSubclouds();

		if (!subClouds.empty()) {
			static int subCloudIdx = 0;
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (GuiUtilities::BeginCombo(EMPTY, std::string("details_subcloudcombo" + selectedCloud->getName()), subClouds[subCloudIdx]->getName())) {
				for (size_t sc = 0; sc < subClouds.size(); sc++) {
					const bool selected = sc == subCloudIdx;

					if (GuiUtilities::Selectable(subClouds[sc]->getName(), "details_subcloud_" + subClouds[sc]->getName(), selected)) {
						subCloudIdx = sc;
					}

					if (selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Indent(16.0f);

			int subcPCount = subClouds[subCloudIdx]->getNumberOfPoints();
			GuiUtilities::InputInt(DETAILS_PCLOUD_POINTCOUNT, "details_subcloud_pcount", &subcPCount, ImGuiInputTextFlags_ReadOnly, true);

			if (GuiUtilities::CollapsingHeader(DETAILS_3DPARAMS, "subcloud3d_details")) {
				valuesChanged |= makeBasicTransformControls(*subClouds[subCloudIdx], "subCloud" + std::to_string(InspectorWindow::_selectedInspectorItem) + "-" + std::to_string(subCloudIdx));
			}

			ImGui::Unindent(16.0f);
		} else {
			GuiUtilities::Text(DETAILS_PCLOUD_NOSUBCLOUD);
		}
	}

	return valuesChanged;
}

bool Nimbus::DetailsWindow::makeImageItemInspector() {
	const bool valuesChanged = false;
	Texture* selectedTexture = TextureList::getInstance()->isTextureAvailable(InspectorWindow::_selectedItemName);

	if (selectedTexture != nullptr) {
		makeBasicTextureViewer(*selectedTexture);
	}

	return valuesChanged;
}

void Nimbus::DetailsWindow::makeMaterialItemInspector() {
	Material* selectedMaterial = Nimbus::MaterialList::getInstance()->isMaterialAvailable(InspectorWindow::_selectedItemName);
	if (!selectedMaterial) {
		GuiUtilities::Text(DETAILS_MATS_UNAVAILABLE);
		return;
	}

	if (GuiUtilities::CollapsingHeader(DETAILS_MATS_PROPERTIES, "details_material_props")) {
		if (ImGui::BeginTable(std::string("##" + InspectorWindow::_selectedItemName + "_propsList").c_str(), 2, ImGuiTableFlags_SizingStretchProp)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			GuiUtilities::InputText(GENERIC_NAME, InspectorWindow::_selectedItemName + "_matName", &selectedMaterial->_name, "", ImGuiInputTextFlags_ReadOnly, true);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			GuiUtilities::DragFloat(DETAILS_MATS_PROPSHINE, InspectorWindow::_selectedItemName + "_matShin", &selectedMaterial->_shininess, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX, "%.3f", 0, true);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			GuiUtilities::DragFloat(DETAILS_MATS_PROPDISP, InspectorWindow::_selectedItemName + "_matDispFact", &selectedMaterial->_displacementFactor, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, FLT_MIN, FLT_MAX, "%.3f", 0, true);

			ImGui::EndTable();
		}
	}
	if (GuiUtilities::CollapsingHeader(DETAILS_MATS_TEXTURES, "details_material_textures")) {
		auto matTexture = selectedMaterial->_texture;
		static int selectedTexLayer = 0;
		if (GuiUtilities::BeginCombo(DETAILS_MATS_LAYER, "details_material_texturescombo", Texture::TEXTURE_LAYER_IDS[selectedTexLayer])) {
			for (int texLayer = 0; texLayer < Texture::NUM_TEXTURE_TYPES; texLayer++) {
				const bool is_selected = texLayer == selectedTexLayer;
				if (GuiUtilities::Selectable(Texture::TEXTURE_LAYER_IDS[texLayer], InspectorWindow::_selectedItemName + "_layer" + std::to_string(texLayer), is_selected)) {
					selectedTexLayer = texLayer;
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		Texture* selectedTexture = selectedMaterial->_texture[selectedTexLayer];
		if (!selectedTexture) {
			GuiUtilities::Text(DETAILS_MATS_NOTEXTURE);
		} else {
			makeBasicTextureViewer(*selectedTexture);
		}
	}
	if (GuiUtilities::CollapsingHeader(DETAILS_MATS_SHADSUBROUT, "details_material_shader")) {
		const RenderingShader* materialShader = selectedMaterial->_associatedShader;
		if (ImGui::BeginTable("##matShader_propTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableNextRow();

			// Shader
			ImGui::TableNextColumn();

			if (ImGui::BeginListBox(std::string("##" + InspectorWindow::_selectedItemName + "_shaderList").c_str(), ImVec2(-FLT_MIN, 0))) {
				for (int s = 0; s < ShaderEnum::ShaderName.size(); s++) {
					const bool is_selected = s == _shaderInspectorShSel;
					if (GuiUtilities::Selectable(ShaderEnum::ShaderName[s], InspectorWindow::_selectedItemName + "_shader" + std::to_string(s), is_selected)) {
						_shaderInspectorShSel = s;
						_shaderInspectorSubrutSel = -1;
						_shaderInspectorUnifSel = -1;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndListBox();
			}

			// Subroutine
			ImGui::TableNextColumn();

			if (ImGui::BeginListBox(std::string("##" + InspectorWindow::_selectedItemName + "_subroutineList").c_str(), ImVec2(-FLT_MIN, 0))) {
				if (_shaderInspectorShSel != -1) {
					for (int s = 0; s < ShaderEnum::SubroutineName[_shaderInspectorShSel].size(); s++) {
						const bool is_selected = s == _shaderInspectorSubrutSel;
						const bool is_enabled = materialShader->support(_shaderInspectorShSel, s);

						ImGui::BeginDisabled(!is_enabled);
						if (GuiUtilities::Selectable(ShaderEnum::SubroutineName[_shaderInspectorShSel][s], InspectorWindow::_selectedItemName + "_subroutine" + std::to_string(s), is_selected)) {
							_shaderInspectorSubrutSel = s;
							_shaderInspectorUnifSel = -1;
						}
						ImGui::EndDisabled();

						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
				}
				ImGui::EndListBox();
			}

			// Uniform
			ImGui::TableNextColumn();

			if (ImGui::BeginListBox(std::string("##" + InspectorWindow::_selectedItemName + "_uniformList").c_str(), ImVec2(-FLT_MIN, 0))) {
				if (_shaderInspectorShSel != -1 && _shaderInspectorSubrutSel != -1) {
					for (int u = 0; u < ShaderEnum::SubroutineUniformNames[_shaderInspectorShSel][_shaderInspectorSubrutSel].size(); u++) {
						// const bool is_selected = u == _shaderInspectorUnifSel;
						const bool is_selected =
							ShaderEnum::SubroutineUniformNames[_shaderInspectorShSel][_shaderInspectorSubrutSel][u] ==
							selectedMaterial->_enabledSubroutines[_shaderInspectorShSel][_shaderInspectorSubrutSel];

						std::string uniformName = ShaderEnum::SubroutineUniformNames[_shaderInspectorShSel][_shaderInspectorSubrutSel][u];
						if (GuiUtilities::Selectable(uniformName, InspectorWindow::_selectedItemName + "_uniform" + std::to_string(u), is_selected)) {
							_shaderInspectorUnifSel = u;
							selectedMaterial->_enabledSubroutines[_shaderInspectorShSel][_shaderInspectorSubrutSel] = uniformName;
							selectedMaterial->_activeSubroutine[_shaderInspectorShSel] = u;
							//selectedMaterial->_band = 0;
						}

						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::EndTable();
						
		}
	}
}

bool Nimbus::DetailsWindow::makeBasicTransformControls(Nimbus::Model3D& model, const std::string& trfIndex) const {
	bool valuesChanged = false;

	valuesChanged |= GuiUtilities::Checkbox(DETAILS_ENABLED, "modelEnabled_" + trfIndex, &model._enabled);

	ImGui::NewLine();

	if (ImGui::BeginTable("posrotscl_table", 3, ImGuiTableFlags_SizingStretchProp)) {
		// Position controls
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn();
			valuesChanged |= GuiUtilities::DragScalarN(DETAILS_POSITION, "infloat3Pos_" + trfIndex, ImGuiDataType_Double, &model._pos[0], 3, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, nullptr, nullptr, nullptr, 0, true);
			ImGui::TableNextColumn();
			if (GuiUtilities::Button(ICON_FA_ROTATE_LEFT, "undoPos_" + trfIndex)) {
				model._pos[0] = 0.0f;
				model._pos[1] = 0.0f;
				model._pos[2] = 0.0f;
				valuesChanged = true;
			}
		}
		// Rotation controls
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn();
			valuesChanged |= GuiUtilities::DragScalarN(DETAILS_ROTATION, "infloat3Rot_" + trfIndex, ImGuiDataType_Double, &model._rot[0], 3, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, nullptr, nullptr, nullptr, 0, true);
			ImGui::TableNextColumn();
			if (GuiUtilities::Button(ICON_FA_ROTATE_LEFT, "undoRot_" + trfIndex)) {
				model._rot[0] = 0.0f;
				model._rot[1] = 0.0f;
				model._rot[2] = 0.0f;
				valuesChanged = true;
			}
		}
		// Scale controls
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn();
			valuesChanged |= GuiUtilities::DragScalarN(DETAILS_SCALE, "infloat3Scl_" + trfIndex, ImGuiDataType_Double, &model._scale[0], 3, GuiUtilities::Parameters::FLOAT_INPUT_SPEED, nullptr, nullptr, nullptr, 0, true);
			ImGui::TableNextColumn();
			if (GuiUtilities::Button(ICON_FA_ROTATE_LEFT, "undoScl_" + trfIndex)) {
				model._scale[0] = 1.0f;
				model._scale[1] = 1.0f;
				model._scale[2] = 1.0f;
				valuesChanged = true;
			}
		}

		ImGui::EndTable();
	}

	if(ImGui::BeginTable("offset_table", 2, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		GuiUtilities::InputScalarN(GENERIC_OFFSET, "model_offset", ImGuiDataType_Double, &model._offset, 3, nullptr, nullptr, "%.3f", ImGuiInputTextFlags_ReadOnly);

		ImGui::EndTable();
	}

	if (trfIndex.empty()) {
		return valuesChanged;
	}

	if (GuiUtilities::CollapsingHeader(GENERIC_AABB, "model_aabb_header")) {
		const AABB& modelAABB = model.getAABB();

		if (ImGui::BeginTable("aabbDataTable", 2, ImGuiTableFlags_SizingStretchProp)) {
			// AABB center
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			auto aabbCenter = modelAABB.center();
			auto aabbExtent = modelAABB.extent();
			auto aabbMin = modelAABB.min();
			auto aabbMax = modelAABB.max();

			GuiUtilities::InputScalarN(DETAILS_AABB_CENTER, "model_aabb_center", ImGuiDataType_Double, &aabbCenter, 3, nullptr, nullptr, "%.3f", ImGuiInputTextFlags_ReadOnly, true);

			ImGui::TableNextRow();
			// AABB extent
			ImGui::TableNextColumn();
			GuiUtilities::InputScalarN(DETAILS_AABB_EXTENT, "model_aabb_extent", ImGuiDataType_Float, &aabbExtent, 3, nullptr, nullptr, "%.3f", ImGuiInputTextFlags_ReadOnly, true);

			ImGui::TableNextRow();
			// AABB min
			ImGui::TableNextColumn();
			GuiUtilities::InputScalarN(DETAILS_AABB_MIN, "model_aabb_min", ImGuiDataType_Float, &aabbMin, 3, nullptr, nullptr, "%.3f", ImGuiInputTextFlags_ReadOnly, true);

			ImGui::TableNextRow();
			// AABB max
			ImGui::TableNextColumn();
			GuiUtilities::InputScalarN(DETAILS_AABB_MAX, "model_aabb_max", ImGuiDataType_Float, &aabbMax, 3, nullptr, nullptr, "%.3f", ImGuiInputTextFlags_ReadOnly, true);

			ImGui::EndTable();
		}
	}

	return valuesChanged;
}

void Nimbus::DetailsWindow::makeBasicTextureViewer(Texture& selectedTexture) {
	const ImVec2 texSize(static_cast<float>(selectedTexture.getWidth()), static_cast<float>(selectedTexture.getHeight()));
	const float rAspInv = texSize.y / texSize.x;
	ImVec2 availSize = ImGui::GetWindowSize();
	const float xPadding = ImGui::GetCursorPosX() - ImGui::GetWindowPos().x;
	availSize.x -= 2.0f * xPadding;

	ImGui::Text(selectedTexture.getFilename().c_str());

	ImGui::NewLine();

	ImGui::Image((ImTextureID)(intptr_t)(selectedTexture.getId()), ImVec2(availSize.x, availSize.x * rAspInv));
}

