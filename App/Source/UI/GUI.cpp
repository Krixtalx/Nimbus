#include "AppPch.h"
#include "GUI.h"

#include <shellapi.h>

#include "implot.h"
#include "Graphics/TextureList.h"
#include "IO/FileManager.h"
#include "MainWindows/CameraAxisWindow.h"
#include "MainWindows/DetailsWindow.h"
#include "MainWindows/InspectorWindow.h"
#include "MainWindows/MaterialsWindow.h"
#include "MainWindows/SettingsWindow.h"
#include "MainWindows/StatsWindow.h"
#include "Popups/InfoPopup.h"
#include "Popups/PCloudClassifyPopup.h"
#include "UI/InterfaceAdapter.h"
#include "UI/Popups/FileDialogPopup.h"
#include "UI/Popups/PopupSystem.h"
#include "UI/Popups/ConfirmPopup.h"
#include "Utilities/Image.h"


bool Nimbus::GUI::WINDOW_CLOSE_PENDING = false;

#pragma region PROTECTED_METHODS


void Nimbus::GUI::loadImGUIStyle() {
	ImGui::StyleColorsDark();
	// Load custom style
	{
		//InputManager::getApplicationState()->_backgroundColor = glm::vec3(.35f);
		ImGuiStyle& style = ImGui::GetStyle();

		style.AntiAliasedFill = true;
		style.AntiAliasedLines = true;
		style.AntiAliasedLinesUseTex = true;

		style.DisabledAlpha = 0.5f;
		style.WindowPadding = ImVec2(13, 13);
		style.WindowRounding = 8.0f;
		style.FramePadding = ImVec2(5, 5);
		style.FrameRounding = 5.0f;
		style.ItemSpacing = ImVec2(12, 8);
		style.ItemInnerSpacing = ImVec2(8, 6);
		style.IndentSpacing = 25.0f;
		style.ScrollbarSize = 15.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 3.0f;
		style.WindowBorderSize = 0.0f;
		style.ChildBorderSize = 0.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 0.0f;
		style.ChildRounding = 3.0f;
		style.PopupRounding = 2.0f;
		style.LogSliderDeadzone = 4.0f;
		style.TabRounding = 3.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 0.85f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.23f, 0.23f, 0.20f, 0.60f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.90f, 0.70f, 0.00f, 0.10f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.70f, 0.00f, 0.75f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.78f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.70f, 0.00f, 0.75f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.90f, 0.70f, 0.00f, 0.75f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		colors[ImGuiCol_Separator] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.90f, 0.70f, 0.00f, 0.50f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.90f, 0.70f, 0.00f, 0.75f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
		colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.19f, 0.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);

		ImVec4* plotColors = ImPlot::GetStyle().Colors;
		plotColors[ImPlotCol_Line] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	}

	// Load fonts
	{
		ImFontConfig cfg;
		const ImGuiIO& io = ImGui::GetIO();

		std::copy_n("Roboto", 7, cfg.Name);
		io.Fonts->AddFontFromFileTTF("Assets/Fonts/Roboto-Bold.ttf", 13.0f, &cfg);

		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		cfg.MergeMode = true;
		cfg.PixelSnapH = true;
		cfg.GlyphMinAdvanceX = 20.0f;
		cfg.GlyphMaxAdvanceX = 20.0f;
		std::copy_n("FontAwesome", 12, cfg.Name);

		io.Fonts->AddFontFromFileTTF("Assets/Fonts/fa-regular-400.ttf", 12.0f, &cfg, icons_ranges);
		io.Fonts->AddFontFromFileTTF("Assets/Fonts/fa-solid-900.ttf", 12.0f, &cfg, icons_ranges);
	}

	// Set file dialog custom icons
	{
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_FOLDER);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_FILE);

		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_IMAGE);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".tif,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_IMAGE);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".TIF,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_IMAGE);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".jpg", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_IMAGE);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".jpeg", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_IMAGE);

		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".ply", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_CLOUD);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".las,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_CLOUD);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".laz,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_CLOUD);

		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".obj", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_CUBE);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".fbx,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_CUBE);
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gltf,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_CUBE);

		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".txt,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_NOTE_STICKY);

		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".Nimbus,", ImGui::GetStyle().Colors[ImGuiCol_Text], ICON_FA_BRAILLE);
	}
}


Nimbus::GUI::GUI() : _renderer(nullptr), _selectedModule(nullptr), _glfwWindowHandle(nullptr) {
}


void Nimbus::GUI::handleNextPopup() {
	if (!PopupSystem::getInstance()->_pendingPopups.empty() && !_currentPopup) {
		_currentPopup = PopupSystem::getInstance()->_pendingPopups.front();
		PopupSystem::getInstance()->_pendingPopups.pop();

		_currentPopup->openPopup();
	}

	if (_currentPopup) {
		//ImGui::SetNextWindowSizeConstraints(MIN_POPUP_SIZE, MAX_POPUP_SIZE);
		const bool popupClosing = _currentPopup->renderPopup(InputManager::getApplicationState(), _renderer->getActiveScene());
		if (popupClosing or !_currentPopup->isOpen()) {
			delete _currentPopup;
			_currentPopup = nullptr;
		}
	}
}


void Nimbus::GUI::makeMainDockNode() {
	const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

	//https://gist.github.com/PossiblyAShrub/0aea9511b84c34e191eaa90dd7225969
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGui::SetNextWindowPos(imguiViewport->Pos);
	ImGui::SetNextWindowSize(imguiViewport->Size);
	ImGui::SetNextWindowViewport(imguiViewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
		window_flags |= ImGuiWindowFlags_NoBackground;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	const ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		// Check for saved dockspace settings
		static bool iniLoaded = ImGui::DockBuilderGetNode(dockspace_id) != nullptr;

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		// Make initial dockspace if not previous settings available
		if (!iniLoaded) {
			iniLoaded = true;

			ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, imguiViewport->Size);

			// Return value = pointer param set to nullptr
			auto right_dock_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
			auto left_dock_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.25f, nullptr, &dockspace_id);
			
			ImGui::DockBuilderDockWindow("###win_inspector", right_dock_id);
			ImGui::DockBuilderDockWindow("###win_details", left_dock_id);
			ImGui::DockBuilderFinish(right_dock_id);
			ImGui::DockBuilderFinish(left_dock_id);
			ImGui::DockBuilderFinish(dockspace_id);
		} 
	}

	ImGui::End();
}
#pragma endregion


#pragma region PUBLIC_METHODS
Nimbus::GUI::~GUI() {
	_selectedModule = nullptr;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	ImPlot::DestroyContext();
}


void Nimbus::GUI::deleteCurrentScene() const {
	_renderer->deleteScene();

	InspectorWindow::resetInspectorSelection();
}

void Nimbus::GUI::initialize(GLFWwindow* window, const int openGLMinorVersion) {
	_renderer = Renderer::getInstance();
	_glfwWindowHandle = window;

	const std::string openGLVersion = "#version 4" + std::to_string(openGLMinorVersion) + "0 core";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	loadImGUIStyle();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(openGLVersion.c_str());

	_coreWindows.push_back(new InspectorWindow("win_inspector", WINDOW_INSPECTOR, true, ImGuiWindowFlags_HorizontalScrollbar));
	_coreWindows.push_back(new DetailsWindow("win_details", WINDOW_DETAILS, true, ImGuiWindowFlags_HorizontalScrollbar));
	_coreWindows.push_back(new MaterialsWindow("win_mats", WINDOW_MATERIALS, false, 0));
	_coreWindows.push_back(new StatsWindow("win_stats", WINDOW_STATS, false, 0));
	_coreWindows.push_back(new SettingsWindow("win_settings", WINDOW_SETTINGS, false, 0));
	//_coreWindows.push_back(new CameraAxisWindow("win_camgizmo", WINDOW_CAMERAGIZMO, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground));//
	//_coreWindows.push_back(new DebugWindow("win_debug", WINDOW_DETAILS, true, 0));

}


void Nimbus::GUI::render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	MIN_POPUP_SIZE = ImVec2(16, 9) * 2.0;
	MAX_POPUP_SIZE = ImGui::GetIO().DisplaySize * 0.5;
	MIN_WINDOW_SIZE = ImVec2(16, 9) * 4.0;
	MAX_WINDOW_SIZE = ImGui::GetIO().DisplaySize;

	ApplicationState* appState = InputManager::getApplicationState();

	//ImGui::Begin("ImGui style editor");
	//ImGui::ShowStyleEditor();
	//ImGui::End();

	makeMainDockNode();
	try {
		for (const auto& window : _coreWindows) {
			if (window->isOpen()) {
				try {
					window->renderWindow();
				} catch (std::exception& e) {
					window->toggleWindow();
					throw std::exception{ (std::string("[MW] ") + e.what()).c_str() };
				}
			}
		}

		for (const auto& otherWindowPair : _otherWindows) {
			if (otherWindowPair.second->isOpen()) {
				try {
					otherWindowPair.second->renderWindow();
				} catch (std::exception& e) {
					otherWindowPair.second->toggleWindow();
					throw std::exception{ (std::string("[SW] ") + e.what()).c_str() };
				}
			}
		}
	} catch (std::exception& e) {
		spdlog::error(std::string("[GUI]") + e.what());
		Nimbus::PopupSystem::addPopup(new InfoPopup("window_error", GENERIC_ERROR_INFO, e.what()));
	}

	if (_selectedModule) {
		try {
			auto constraintsMin = _selectedModule->getConstraintsMin();
			auto constraintsMax = _selectedModule->getConstraintsMax();
			ImGui::SetNextWindowSizeConstraints(ImVec2(constraintsMin.x, constraintsMin.y), ImVec2(constraintsMax.x, constraintsMax.y));
			ImGui::SetNextWindowSize(ImVec2(constraintsMin.x, constraintsMin.y), ImGuiCond_FirstUseEver);
			if (ImGui::Begin(_selectedModule->getWindowName().c_str(), _selectedModule->getWindowOpenPtr())) {
				_selectedModule->renderGuiObject(appState, _renderer->getActiveScene());

				if (!*_selectedModule->getWindowOpenPtr()) {
					_selectedModule = nullptr;
				}
			}

		} catch (std::exception& e) {
			Nimbus::PopupSystem::addPopup(new InfoPopup("module_error", GENERIC_ERROR_INFO, e.what()));
		}
		ImGui::End();

	}

	if (ImGui::BeginMainMenuBar()) {
		// File menu
		if (ImGui::BeginMenu((ICON_FA_FILE + LocaleStrings::getInstance()->getString(MENUBAR_FILE)).c_str())) {
			if (ImGui::MenuItem((ICON_FA_FILE_MEDICAL + LocaleStrings::getInstance()->getString(MENUBAR_FILE_NEWSCENE)).c_str())) {
				_renderer->newScene();

				InspectorWindow::resetInspectorSelection();
			}
			if (ImGui::MenuItem((ICON_FA_FOLDER_OPEN + LocaleStrings::getInstance()->getString(MENUBAR_FILE_OPENSCENE)).c_str())) {
				auto fileDialogCallback = [](const std::string& directory, const std::string& filename, const std::string& fileExtension) {
					Renderer::getInstance()->loadScene(directory, filename);
				};
				Nimbus::PopupSystem::addPopup(new FileDialogPopup("##open_scene_dialog", POPUP_OPEN_SCENE, FileDialogRegex::FileDialogType::SCENE, ImGuiFileDialogFlags_Modal, fileDialogCallback));
			}
			if (ImGui::BeginMenu((ICON_FA_CAMERA_RETRO + LocaleStrings::getInstance()->getString(MENUBAR_FILE_ACTIVESCENES)).c_str())) {
				const auto& loadedScenes = _renderer->_scenes;

				for (int s = 0; s < loadedScenes.size(); s++) {
					const auto scene = loadedScenes[s].get();
					if (ImGui::MenuItem(std::string(scene->_sceneName + (scene->dirty ? ICON_FA_ASTERISK : "")).c_str(), nullptr, _renderer->_activeScene == s)) {
						_renderer->_activeScene = s;

						InspectorWindow::resetInspectorSelection();
					}
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem((ICON_FA_ERASER + LocaleStrings::getInstance()->getString(MENUBAR_FILE_CLOSESCENE)).c_str())) {
				if (_renderer->getActiveScene()->dirty) {
					auto confirmedCallback = [](const bool confirmed) {
						if (confirmed) {
							Nimbus::GUI::getInstance()->deleteCurrentScene();
						}
					};
					Nimbus::PopupSystem::addPopup(new ConfirmPopup("confirm_delete", POPUP_CONFIRM_SAVE, POPUP_UNSAVED_WARN, true, confirmedCallback));
				} else {
					deleteCurrentScene();
				}
			}
			if (ImGui::MenuItem((ICON_FA_BIOHAZARD + LocaleStrings::getInstance()->getString(MENUBAR_FILE_CLOSEALL)).c_str())) {
				bool confirm = false;
				for (auto itB = _renderer->_scenes.begin(); itB != _renderer->_scenes.end() && !confirm; ++itB) {
					if (itB->get()->dirty) {
						confirm = true;
					}
				}
				if (confirm) {
					auto confirmedCallback = [](const bool confirmed) {
						if (confirmed) {
							while (!Renderer::getInstance()->deleteScene());
						}
					};
					Nimbus::PopupSystem::addPopup(new ConfirmPopup("confirm_continue", POPUP_CONFIRM_SAVE, POPUP_UNSAVED_WARN, true, confirmedCallback));
				} else {
					while (!_renderer->deleteScene());
				}
			}

			ImGui::Separator();

			bool exportScene = false;
			if (ImGui::MenuItem((ICON_FA_FLOPPY_DISK + LocaleStrings::getInstance()->getString(MENUBAR_FILE_SAVESCENE)).c_str())) {
				const std::string sceneParentDir = _renderer->getActiveScene()->_scenePath.substr(0, _renderer->getActiveScene()->_scenePath.find_last_of(std::filesystem::path::preferred_separator));
				const std::string sceneDir = sceneParentDir + "/" + _renderer->getActiveScene()->_sceneName;
				const std::string sceneFile = sceneDir + "/" + _renderer->getActiveScene()->_sceneName + ".Nimbus";
				if (std::filesystem::exists(sceneFile) && !std::filesystem::is_directory(sceneFile)) {
					_renderer->saveScene(sceneParentDir, _renderer->getActiveScene()->_sceneName);
					_renderer->getActiveScene()->dirty = false;
				} else {
					exportScene = true;
				}
			}
			if (ImGui::MenuItem((ICON_FA_FILE_EXPORT + LocaleStrings::getInstance()->getString(MENUBAR_FILE_SAVEAS)).c_str())) {
				exportScene = true;
			}

			if (exportScene) {
				auto fileDialogCallback = [](const std::string& directory, const std::string& filename, const std::string& fileExtension) {
					Renderer::getInstance()->saveScene(directory, filename);
				};
				Nimbus::PopupSystem::addPopup(new FileDialogPopup("##dialog_save_scene", POPUP_SAVE_SCENE, FileDialogRegex::FileDialogType::SCENE, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite, fileDialogCallback));
			}

			ImGui::Separator();

			if (ImGui::MenuItem((ICON_FA_DOOR_OPEN + LocaleStrings::getInstance()->getString(MENUBAR_FILE_QUIT)).c_str())) {
				appState->_closeFlag = true;
			}

			ImGui::EndMenu();
		}

		// Content menu
		if (ImGui::BeginMenu((ICON_FA_UPLOAD + LocaleStrings::getInstance()->getString(MENUBAR_CONTENT)).c_str())) {

			// Import point cloud
			if (ImGui::MenuItem((ICON_FA_FILE_IMPORT + LocaleStrings::getInstance()->getString(MENUBAR_CONTENT_IMPORTCLOUD)).c_str())) {
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

			// New light
			if (ImGui::MenuItem((ICON_FA_LIGHTBULB + LocaleStrings::getInstance()->getString(MENUBAR_CONTENT_NEWLIGHT)).c_str())) {
				const unsigned lightIdx = _renderer->addLight();
				_renderer->getActiveScene()->_light[lightIdx]->setName("Light #" + std::to_string(lightIdx));
				_renderer->getActiveScene()->dirty = true;
			}

			// New camera
			if (ImGui::MenuItem((ICON_FA_CAMERA + LocaleStrings::getInstance()->getString(MENUBAR_CONTENT_NEWCAMERA)).c_str())) {
				const unsigned cameraIdx = _renderer->addCamera();
				_renderer->getActiveScene()->_camera[cameraIdx]->setName("Camera #" + std::to_string(cameraIdx));
				const auto aCam = _renderer->getActiveScene()->getActiveCamera();
				_renderer->getActiveScene()->_camera[cameraIdx]->setRaspect(aCam->getRaspect());
				_renderer->getActiveScene()->dirty = true;
			}

			// Import image
			if (ImGui::MenuItem((ICON_FA_IMAGE + LocaleStrings::getInstance()->getString(MENUBAR_CONTENT_IMPORTIMAGE)).c_str())) {
				auto fileDialogCallback = [](const std::string& directory, const std::string& filename, const std::string& fileExtension) {
					const auto imageFile = new Nimbus::Image(directory + "/" + filename + ".png");

					const auto newTexture = new Nimbus::Texture(imageFile);
					TextureList::getInstance()->saveTexture(directory + "/" + filename + ".png", newTexture);

					delete imageFile;
				};
				Nimbus::PopupSystem::addPopup(new FileDialogPopup("open_image", POPUP_OPEN_IMAGE, FileDialogRegex::FileDialogType::IMAGES, ImGuiFileDialogFlags_Modal, fileDialogCallback));
			}

			ImGui::Separator();

			//Reload shaders
			if (ImGui::MenuItem((ICON_FA_REPEAT + LocaleStrings::getInstance()->getString(RECOMPILE_SHADERS)).c_str())) {
				Nimbus::Renderer::getInstance()->reloadShaders();
			}

			ImGui::EndMenu();
		}

		// Windows menu
		if (ImGui::BeginMenu((ICON_FA_DISPLAY + LocaleStrings::getInstance()->getString(MENUBAR_WINDOWS)).c_str())) {
			for (const auto& window : _coreWindows) {
				if (ImGui::MenuItem(window->getTitle().c_str(), nullptr, window->isOpen())) {
					window->toggleWindow();
					if (window->isOpen())
						ImGui::SetWindowFocus(window->getIdTitle().c_str());
				}
			}

			ImGui::EndMenu();
		}

		if (appState->_fpsMenuBar) {
			const std::string fpsText = "FPS: xx.xxxxx";
			ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(fpsText.c_str()).x - 5);
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		}
		ImGui::EndMainMenuBar();
	}
	//ImGui::PopStyleVar();

	if (WINDOW_CLOSE_PENDING)
		appState->_closeFlag = false;

	if (appState->_closeFlag) {
		bool confirm = false;
		for (auto itB = _renderer->_scenes.begin(); itB != _renderer->_scenes.end() && !confirm; ++itB) {
			if (itB->get()->dirty) {
				confirm = true;
			}
		}
		if (confirm) {
			auto confirmedCallback = [](const bool confirmed) {
				InputManager::getApplicationState()->_closeFlag = confirmed;
				GUI::WINDOW_CLOSE_PENDING = false;
			};
			Nimbus::PopupSystem::addPopup(new ConfirmPopup("confirm_quit", POPUP_CONFIRM_SAVE, POPUP_UNSAVED_QUIT, true, confirmedCallback));
			WINDOW_CLOSE_PENDING = true;
			appState->_closeFlag = false;
		}

	}
	handleNextPopup();

	ImGui::GetStyle().WindowBorderSize = 1.0f;

	ImGui::RenderNotifications();
	ImGui::GetStyle().WindowBorderSize = 0.0f;

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


#pragma endregion
