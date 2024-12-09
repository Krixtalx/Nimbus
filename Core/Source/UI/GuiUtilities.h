#pragma once
#include <implot.h>

#include "UI/Addons/ImGuiNotify.hpp"

namespace Nimbus {
	class Texture;
	class PointCloud;
	class Scene;
}

namespace Nimbus {
	namespace GuiUtilities {
		struct Parameters {
			static constexpr float FLOAT_INPUT_SPEED = 0.01f;
			static constexpr size_t TEXT_INPUT_LIMIT = 256;
			static constexpr float IMAGE_PREVIEW_PADDING_PERC = 0.05f;
		};

		struct Colors {
			static constexpr ImVec4 Nimbus_ORANGE = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);;
			static constexpr ImVec4 ERROR_TEXT_COLOR = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
			static constexpr ImVec4 SUCCESS_TEXT_COLOR = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
			static constexpr ImVec4 WARN_TEXT_COLOR = ImVec4(1.f, 0.78f, 0.05f, 1.0f);
			static constexpr ImVec4 MAIN_SELECTION_COLOR = ImVec4(0.90f, 0.70f, 0.0f, 1.0f);
			static constexpr ImVec4 ALT_SELECTION_COLOR = ImVec4(1.00f, 0.75f, 0.15f, 1.0f);
			static constexpr ImVec4 MAIN_TEXT_COLOR = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			static constexpr ImVec4 INVERSE_TEXT_COLOR = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
			static constexpr ImVec4 SELECTION_RECTBG_COLOR = ImVec4(0.25f, 0.25f, 0.60f, 0.30f);
			static constexpr ImVec4 SELECTION_RECTBORDER_COLOR = ImVec4(0.25f, 0.25f, 0.85f, 0.50f);
		};

		struct DragDropIDs {
			inline static const std::string INSPECTORLIST_IMAGE = "INSPECTOR_IMAGE";
			inline static const std::string INSPECTORDETAILS_HEADER = "INSPECTOR_HEADER";
		};

		struct ImageTooltipDrawArea {
			ImVec2 _areaBegin;
			ImVec2 _areaEnd;
			ImDrawList* _areaDrawList;

			ImVec2 _uvBegin;
			ImVec2 _uvEnd;

			ImVec2 getAreaSize() const;

			ImVec2 getUVSize() const;
		};

	#pragma region MENUS

		bool BeginMenu(LOCALE_STRINGS label_idx, const std::string& id, const bool& disabled = false);
		bool MenuItem(LOCALE_STRINGS label_idx, const std::string& id, bool* selected, const bool& disabled = false);
		bool MenuItem(LOCALE_STRINGS label_idx, const std::string& icon, const std::string& id, bool* selected, const bool& disabled = false);

	#pragma endregion

	#pragma region TEXT_VIEWS

		void Text(const char* str, const ImVec4& color = Colors::MAIN_TEXT_COLOR, const bool& alignToPaddingV = true, const bool& wrap = false); // Main Text function, handles char string without formatting
		void Text(LOCALE_STRINGS label_idx, const ImVec4& color = Colors::MAIN_TEXT_COLOR, const bool& alignToPaddingV = true, const bool& wrap = false);
		void Text(const std::list<LOCALE_STRINGS>& label_idx, const ImVec4& color = Colors::MAIN_TEXT_COLOR, const bool& alignToPaddingV = true, const bool& wrap = false);
		void Text(const std::string& str, const ImVec4& color = Colors::MAIN_TEXT_COLOR, const bool& alignToPaddingV = true, const bool& wrap = false);
		void SeparatorText(LOCALE_STRINGS label_idx);
		void ProgressBar(LOCALE_STRINGS label_idx, float progress, float width, int textAlign = 0, float height = 0); // Use textAlign < 0 for left, textAlign == 0 for center, and textAlign > 0 for right
		void ProgressBar(const std::string& str, float progress, float width, int textAlign = 0, float height = 0); // Use textAlign < 0 for left, textAlign == 0 for center, and textAlign > 0 for right
		void BulletText(const char* str, const ImVec4& color = Colors::MAIN_TEXT_COLOR, const bool& alignToPaddingV = true);
		void BulletText(LOCALE_STRINGS label_idx, const ImVec4& color = Colors::MAIN_TEXT_COLOR, const bool& alignToPaddingV = true);
		void BulletText(const std::string& str, const ImVec4& color = Colors::MAIN_TEXT_COLOR, const bool& alignToPaddingV = true);
		void DatabaseQueryText();

	#pragma endregion

	#pragma region GENERAL_CONTROLS

		bool Button(LOCALE_STRINGS label_idx, const std::string& id, const bool& disabled = false);
		bool Button(const std::string& icon, const std::string& id, const bool& disabled = false);

		bool Checkbox(LOCALE_STRINGS label_idx, const std::string& id, bool* value, const bool& in_table = false, const bool& disabled = false);

		bool Radio(LOCALE_STRINGS label_idx, const std::string& id, int* value, int set_value, const bool& in_table = false, const bool& disabled = false);
		bool Radio(LOCALE_STRINGS label_idx, const std::string& id, int value, const bool& in_table = false, const bool& disabled = false);

		bool Selectable(const char* label, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0, const ImVec2& item_size = ImVec2(0, 0)); // Main Selectable function; handles styling and sizing
		bool Selectable(LOCALE_STRINGS label_idx, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);
		bool Selectable(const std::string& label, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);
		bool Selectable(LOCALE_STRINGS label_idx, const std::string& icon, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);
		bool Selectable(const std::string& label, const std::string& icon, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);
		bool Selectable(const char* label, const std::string& icon, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);

	#pragma endregion

	#pragma region INPUT_CONTROLS

		bool InputText(LOCALE_STRINGS label_idx, const std::string& id, std::string* str, const std::string& hint, ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputTextMultiline(LOCALE_STRINGS label_idx, const std::string& id, std::string* str, float height = 0, ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool InputInt(LOCALE_STRINGS label_idx, const std::string& id, int* val, ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputIntV(LOCALE_STRINGS label_idx, const std::string& id, ivec2& val, ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputIntV(LOCALE_STRINGS label_idx, const std::string& id, ivec3& val, ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputIntV(LOCALE_STRINGS label_idx, const std::string& id, ivec4& val, ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool SliderInt(LOCALE_STRINGS label_idx, const std::string& id, int* val, int v_min, int v_max, const std::string& format = "%.d", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool SliderIntV(LOCALE_STRINGS label_idx, const std::string& id, ivec2& val, int v_min, int v_max, const std::string& format = "%.d", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool SliderIntV(LOCALE_STRINGS label_idx, const std::string& id, ivec3& val, int v_min, int v_max, const std::string& format = "%.d", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool SliderIntV(LOCALE_STRINGS label_idx, const std::string& id, ivec4& val, int v_min, int v_max, const std::string& format = "%.d", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool InputFloat(LOCALE_STRINGS label_idx, const std::string& id, float* val, const std::string& format = "%.3f", ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec2& val, const std::string& format = "%.3f", ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec3& val, const std::string& format = "%.3f", ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec4& val, const std::string& format = "%.3f", ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool SliderFloat(LOCALE_STRINGS label_idx, const std::string& id, float* val, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool SliderFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec2& val, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool SliderFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec3& val, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool SliderFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec4& val, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool DragFloat(LOCALE_STRINGS label_idx, const std::string& id, float* val, float v_speed, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool DragFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec2& val, float v_speed, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool DragFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec3& val, float v_speed, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool DragFloatV(LOCALE_STRINGS label_idx, const std::string& id, vec4& val, float v_speed, float v_min, float v_max, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool InputScalar(LOCALE_STRINGS label_idx, const std::string& id, ImGuiDataType data_type, void* p_data, const void* p_step = nullptr, const void* p_step_fast = nullptr, const std::string& format = "%.3f", ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool InputScalarN(LOCALE_STRINGS label_idx, const std::string& id, ImGuiDataType data_type, void* p_data, int components, const void* p_step = nullptr, const void* p_step_fast = nullptr, const std::string& format = "%.3f", ImGuiInputTextFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool DragScalar(LOCALE_STRINGS label_idx, const std::string& id, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min = nullptr, const void* p_max = nullptr, const char* format = nullptr, ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool DragScalarN(LOCALE_STRINGS label_idx, const std::string& id, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min = nullptr, const void* p_max = nullptr, const char* format = nullptr, ImGuiSliderFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool ColorPicker(LOCALE_STRINGS label_idx, const std::string& id, vec3& color, ImGuiColorEditFlags flags = 0, const bool& in_table = false, const bool& disabled = false);
		bool ColorPicker(LOCALE_STRINGS label_idx, const std::string& id, vec4& color, ImGuiColorEditFlags flags = 0, const bool& in_table = false, const bool& disabled = false);

		bool TimePicker(LOCALE_STRINGS label_idx, const std::string& id, std::string& time, const bool& in_table = false, const bool& disabled = false);

	#pragma endregion

	#pragma region CONTAINERS

		bool BeginCombo(LOCALE_STRINGS label_idx, const std::string& id, const std::string& preview_value, ImGuiComboFlags flags = 0, const bool& in_table = false);
		bool ComboItem(LOCALE_STRINGS label_idx, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);
		bool ComboItem(const std::string& label, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);
		bool ComboItem(const char* label, const std::string& id, const bool& selected, ImGuiSelectableFlags flags = 0);
		bool Combo(LOCALE_STRINGS label_idx, const std::string& id, int* current_item, const char* const items[], int items_count, const bool& in_table = false);
		bool CollapsingHeader(LOCALE_STRINGS label_idx, const std::string& id, ImGuiTreeNodeFlags flags = 0);
		bool CollapsingHeader(LOCALE_STRINGS label_idx, const std::string& icon, const std::string& id, ImGuiTreeNodeFlags flags = 0);
		bool BeginTabBar(const std::string& id, ImGuiTabBarFlags flags = 0);
		bool BeginTabItem(LOCALE_STRINGS label_idx, const std::string& id, bool* p_open = 0, ImGuiTabItemFlags flags = 0);

	#pragma endregion

	#pragma region ITEM_SIZING

		void NextItemFillRegion();
		void NextItemFillColumn();

		float GetTableRowHeight();
		float CalcWidgetWidth(LOCALE_STRINGS label_idx);

	#pragma endregion

		// #pragma region WINDOW_SIZING
		//
		// 		void SetNextWindowMinSize(const ImVec2& size);
		// 		void SetNextMinSize(const ImVec2& size);
		//
		// #pragma endregion

	#pragma region TABLE_ITEM_ALIGN

		void NextTableItemCenter(LOCALE_STRINGS label_idx);
		void NextTableItemRight(LOCALE_STRINGS label_idx);
		void NextTableItemCenter(const std::string& label);
		void NextTableItemRight(const std::string& label);

	#pragma endregion

	#pragma region TOAST_NOTIFICATIONS

		void InsertToast(LOCALE_STRINGS label_idx, const ImGuiToastType& type, const int& duration = 5000);
		void InsertToast(LOCALE_STRINGS label_idx, const ImGuiToastType& type, float& progress);

	#pragma endregion

	#pragma region Nimbus_UTILITIES

		bool ComboPointCloud(PointCloud*& ptr, Scene* activeScene, const bool& in_table = false, const bool& disabled = false);

	#pragma endregion


	#pragma region PLOTS

		void Histogram(const LOCALE_STRINGS& label_idx, const std::string& id, const std::vector<float>& values, ImPlotRange range = ImPlotRange(), ImPlotHistogramFlags flags = 0, bool in_table = false);
		void BarPlot(const LOCALE_STRINGS& label_idx, const std::string& id, const std::vector<float>& values, ImPlotBarsFlags flags = 0, bool in_table = false);
		void BarPlot(const LOCALE_STRINGS& label_idx, const std::string& id, const std::vector<float>& values, const LOCALE_STRINGS& axis_X_label, const LOCALE_STRINGS& axis_Y_label, ImPlotBarsFlags plot_flags = 0, ImPlotAxisFlags axis_X_flags = 0, ImPlotAxisFlags axis_Y_flags = 0, bool in_table = false);
		void BarPlot(const LOCALE_STRINGS& label_idx, const std::string& id, const std::vector<u32>& values, const LOCALE_STRINGS& axis_X_label, const LOCALE_STRINGS& axis_Y_label, ImPlotBarsFlags plot_flags = 0, ImPlotAxisFlags axis_X_flags = 0, ImPlotAxisFlags axis_Y_flags = 0, bool in_table = false);
		bool BeginPlot(const std::string& id, const LOCALE_STRINGS& axis_X_label, const LOCALE_STRINGS& axis_Y_label, ImPlotAxisFlags axis_X_flags = 0, ImPlotAxisFlags axis_Y_flags = 0);

	#pragma endregion


#pragma region PENDING_REMOVAL
		void leaveSpace(unsigned numSlots);
		void renderText(const vec3& xyz, const std::string& title = "", char delimiter = ',');
		void renderText(const mat4& mat, const std::string& title = "", char delimiter = ',');
		void renderText(const float* mat, const std::string& title = "", char delimiter = ',');

		void renderTexture(const Texture* texture);

		bool makeButton(const std::string& title, const bool& disabled = false);
		bool makeButton(const char* title, const ImVec2& size = ImVec2(0, 0), const bool& centered = false);
		bool makeYesButton(const bool& disabled = false);
		bool makeNoButton(const bool& disabled = false);
		bool makeConfirmButton(const bool& disabled = false);
		bool makeCancelButton(const bool& disabled = false);

		void makeLocText(LOCALE_STRINGS idx, const bool& wrap = false);
		bool makeLocButton(LOCALE_STRINGS idx, const bool& disabled = false);
		bool makeLocInputText(LOCALE_STRINGS idx, std::string* textString, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
		void makeLocSepText(LOCALE_STRINGS idx);

		std::string loadingIcon();

		void makeSelectableTextBox(LOCALE_STRINGS idx, const std::string& str, const std::string& id, const bool& in_table = false, const bool& fillWidth = false);
		void makeHelpMarker(LOCALE_STRINGS idx, bool useIcon = true);

		float calcSingleRowTableHeight();

		void makeColorGradient(float colorA[3], float colorB[3], ImVec2 gradient_size);

		void setNextTableItemAlign(LOCALE_STRINGS idx, bool center);

		void makeImageViewFill(const Texture* _image, ImVec2 verticalPadding, bool useTableSize = false);
		ImageTooltipDrawArea makeImageZoomable(const Texture* image, ImVec2 padding, bool sizeFromTable, float zoomRegionSize, float zoomFactor, bool showCrosshair);

		void makeCloudSelector(PointCloud*& ptr, Scene* activeScene);
#pragma endregion
	}
}
