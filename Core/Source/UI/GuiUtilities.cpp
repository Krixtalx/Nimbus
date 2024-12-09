#include "CorePch.h"
#include "GuiUtilities.h"

#include "DataTypes/PointCloud.h"
#include "Fonts/IconsFontAwesome6.h"
#include "Graphics/Scene.h"
#include "Graphics/Texture.h"

#define ItemSeparation if (in_table) { ImGui::TableNextColumn(); NextItemFillColumn(); } else ImGui::SameLine()

#define LocString LocaleStrings::getInstance()->getString
#define LocLabelId (LocString(label_idx) + "###" + id).c_str()

#define TableSingleHeight ((ImGui::CalcTextSize("|").y + ImGui::GetStyle().FramePadding.y * 2.0f) * 2.0f)

#pragma region MENUS

ImVec2 Nimbus::GuiUtilities::ImageTooltipDrawArea::getAreaSize() const {
	return _areaEnd - _areaBegin;
}

ImVec2 Nimbus::GuiUtilities::ImageTooltipDrawArea::getUVSize() const {
	return _uvEnd - _uvBegin;
}

bool Nimbus::GuiUtilities::BeginMenu(const LOCALE_STRINGS label_idx, const std::string& id, const bool& disabled) {
	return ImGui::BeginMenu(LocLabelId, !disabled);
}

bool Nimbus::GuiUtilities::MenuItem(const LOCALE_STRINGS label_idx, const std::string& id, bool* selected, const bool& disabled) {
	return ImGui::MenuItem(LocLabelId, nullptr, selected, !disabled);
}

bool Nimbus::GuiUtilities::MenuItem(const LOCALE_STRINGS label_idx, const std::string& icon, const std::string& id, bool* selected,
								 const bool& disabled) {
	return ImGui::MenuItem((icon + LocLabelId).c_str(), nullptr, selected, !disabled);
}

#pragma endregion

#pragma region TEXT_VIEWS


void Nimbus::GuiUtilities::Text(const char* str, const ImVec4& color, const bool& alignToPaddingV, const bool& wrap) {
	if (std::string(str).empty()) {
		return;
	}

	if (alignToPaddingV) {
		ImGui::AlignTextToFramePadding();
	}

	ImGui::PushStyleColor(ImGuiCol_Text, color);

	if (wrap) {
		ImGui::TextWrapped(str);
	} else {
		ImGui::Text(str);
	}

		ImGui::PopStyleColor();
}

void Nimbus::GuiUtilities::Text(const LOCALE_STRINGS label_idx, const ImVec4& color, const bool& alignToPaddingV, const bool& wrap) {
	if (label_idx == EMPTY) {
		return;
	}

	Text(LocString(label_idx).c_str(), color, alignToPaddingV, wrap);
}

void Nimbus::GuiUtilities::Text(const std::list<LOCALE_STRINGS>& label_idx, const ImVec4& color, const bool& alignToPaddingV,
							 const bool& wrap) {
	std::string combinedText;
	for (const auto& label : label_idx) {
		combinedText += LocString(label);
	}
	Text(combinedText.c_str(), color, alignToPaddingV, wrap);
}

void Nimbus::GuiUtilities::Text(const std::string& str, const ImVec4& color, const bool& alignToPaddingV,
							 const bool& wrap) {
	if (str.empty()) {
		return;
	}

	Text(str.c_str(), color, alignToPaddingV, wrap);
}

void Nimbus::GuiUtilities::SeparatorText(const LOCALE_STRINGS label_idx) {
	ImGui::SeparatorText(LocString(label_idx).c_str());
}

void Nimbus::GuiUtilities::ProgressBar(LOCALE_STRINGS label_idx, float progress, float width, int textAlign, float height) {
	ProgressBar(LocString(label_idx), progress, width, textAlign, height);
}

void Nimbus::GuiUtilities::ProgressBar(const std::string& str, float progress, float width, int textAlign,
									float height) {
	auto textSize = ImGui::CalcTextSize(str.c_str());

	float rectHeight = (height > 0 ? height : ImGui::GetFrameHeight());
	auto rectBegin = ImGui::GetCursorPos() + ImGui::GetWindowPos();
	auto rectEnd = ImVec2(rectBegin.x + width, rectBegin.y + rectHeight);
	auto progRectBegin = ImVec2(rectBegin.x + width * progress, rectBegin.y);

	auto textBegin = ImVec2(rectBegin.x, rectBegin.y + rectHeight * 0.5f - textSize.y * 0.5f);
	if (textAlign < 0) {
		textBegin.x += ImGui::GetStyle().FramePadding.x;
	} else if (textAlign > 0) {
		textBegin.x += width - ImGui::GetStyle().FramePadding.x - textSize.x;
	} else {
		textBegin.x += width * 0.5f - textSize.x * 0.5f;
	}

	auto bgColor = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
	auto fgColor = ImGui::ColorConvertFloat4ToU32(Colors::Nimbus_ORANGE);
	auto bgTextColor = ImGui::ColorConvertFloat4ToU32(Colors::MAIN_TEXT_COLOR);
	auto fgTextColor = ImGui::ColorConvertFloat4ToU32(Colors::INVERSE_TEXT_COLOR);

	auto drawList = ImGui::GetWindowDrawList();

	drawList->AddRectFilled(rectBegin, rectEnd, fgColor, ImGui::GetStyle().FrameRounding);
	drawList->AddText(textBegin, fgTextColor, str.c_str());
	drawList->PushClipRect(progRectBegin, rectEnd + ImGui::GetStyle().FramePadding);
	drawList->AddRectFilled(rectBegin, rectEnd, bgColor, ImGui::GetStyle().FrameRounding - 1.5f);
	drawList->AddText(textBegin, bgTextColor, str.c_str());
	drawList->PopClipRect();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + rectHeight + ImGui::GetStyle().FramePadding.y * 2);
}

void Nimbus::GuiUtilities::BulletText(const char* str, const ImVec4& color, const bool& alignToPaddingV) {
	if (std::string(str).empty()) {
		return;
	}

	if (alignToPaddingV) {
		ImGui::AlignTextToFramePadding();
	}

	ImGui::PushStyleColor(ImGuiCol_Text, color);

	ImGui::BulletText(str);

		ImGui::PopStyleColor();
	
}

void Nimbus::GuiUtilities::BulletText(LOCALE_STRINGS label_idx, const ImVec4& color, const bool& alignToPaddingV) {
	BulletText(LocString(label_idx), color, alignToPaddingV);
}

void Nimbus::GuiUtilities::BulletText(const std::string& str, const ImVec4& color, const bool& alignToPaddingV) {
	BulletText(str.c_str(), color, alignToPaddingV);
}

void Nimbus::GuiUtilities::DatabaseQueryText() {
	ImGui::SameLine();
	ImGui::Text(loadingIcon().c_str());
	ImGui::SameLine(0.0f, 0.0f);
	Text(DATABASE_QUERY_QUERYING);
}

#pragma endregion

#pragma region GENERAL_CONTROLS

bool Nimbus::GuiUtilities::Button(const LOCALE_STRINGS label_idx, const std::string& id, const bool& disabled) {
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::INVERSE_TEXT_COLOR);

	ImGui::BeginDisabled(disabled);
	const bool pressed = ImGui::Button(LocLabelId);
	ImGui::EndDisabled();

	ImGui::PopStyleColor();
	return pressed;
}

bool Nimbus::GuiUtilities::Button(const std::string& icon, const std::string& id, const bool& disabled) {
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::INVERSE_TEXT_COLOR);

	ImGui::BeginDisabled(disabled);
	const bool pressed = ImGui::Button((icon + "###" + id).c_str());
	ImGui::EndDisabled();

	ImGui::PopStyleColor();
	return pressed;
}

bool Nimbus::GuiUtilities::Checkbox(const LOCALE_STRINGS label_idx, const std::string& id, bool* value, const bool& in_table, const bool& disabled) {

	ImGui::BeginDisabled(disabled);
	const bool pressed = ImGui::Checkbox(LocLabelId, value);
	ImGui::EndDisabled();

	return pressed;
}

bool Nimbus::GuiUtilities::Radio(const LOCALE_STRINGS label_idx, const std::string& id, int* value, const int set_value, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	const bool pressed = ImGui::RadioButton(LocLabelId, value, set_value);
	ImGui::EndDisabled();

	return pressed;
}

bool Nimbus::GuiUtilities::Radio(const LOCALE_STRINGS label_idx, const std::string& id, const int value, const bool& in_table,
							  const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	const bool pressed = ImGui::RadioButton(LocLabelId, value);
	ImGui::EndDisabled();

	return pressed;
}


bool Nimbus::GuiUtilities::Selectable(const char* label, const std::string& id, const bool& selected,
								   const ImGuiSelectableFlags flags, const ImVec2& item_size) {
	ImGui::PushStyleColor(ImGuiCol_Header, Colors::MAIN_SELECTION_COLOR);
	if (selected)
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::INVERSE_TEXT_COLOR);
	const bool pressed = ImGui::Selectable((std::string(label) + "###" + id).c_str(), selected, flags, item_size);
	ImGui::PopStyleColor(selected ? 2 : 1);
	return pressed;
}

bool Nimbus::GuiUtilities::Selectable(const LOCALE_STRINGS label_idx, const std::string& id, const bool& selected,
								   const ImGuiSelectableFlags flags) {
	return Selectable(LocString(label_idx).c_str(), id, selected, flags);
}

bool Nimbus::GuiUtilities::Selectable(const std::string& label, const std::string& id, const bool& selected,
								   const ImGuiSelectableFlags flags) {
	return Selectable(label.c_str(), id, selected, flags);
}

bool Nimbus::GuiUtilities::Selectable(const LOCALE_STRINGS label_idx, const std::string& icon, const std::string& id,
								   const bool& selected, const ImGuiSelectableFlags flags) {
	return Selectable((icon + LocString(label_idx)).c_str(), id, selected, flags);
}

bool Nimbus::GuiUtilities::Selectable(const std::string& label, const std::string& icon, const std::string& id,
								   const bool& selected, const ImGuiSelectableFlags flags) {
	return Selectable((icon + label).c_str(), id, selected, flags);
}

bool Nimbus::GuiUtilities::Selectable(const char* label, const std::string& icon, const std::string& id,
								   const bool& selected, const ImGuiSelectableFlags flags) {
	return Selectable((icon + label).c_str(), id, selected, flags);
}

#pragma endregion

#pragma region INPUT_CONTROLS

bool Nimbus::GuiUtilities::InputText(const LOCALE_STRINGS label_idx, const std::string& id, std::string* str, const std::string& hint,
								  const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputText(("###" + id).c_str(), str, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputTextMultiline(const LOCALE_STRINGS label_idx, const std::string& id, std::string* str,
										   const float height, const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputTextMultiline(("###" + id).c_str(), str, ImVec2(0, height), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputInt(const LOCALE_STRINGS label_idx, const std::string& id, int* val, const ImGuiInputTextFlags flags, const bool& in_table,
								 const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputInt(("###" + id).c_str(), val, 0, 0, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputIntV(const LOCALE_STRINGS label_idx, const std::string& id, ivec2& val,
								  const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputInt2(("###" + id).c_str(), val.data.data, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputIntV(const LOCALE_STRINGS label_idx, const std::string& id, ivec3& val,
								  const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputInt3(("###" + id).c_str(), val.data.data, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputIntV(const LOCALE_STRINGS label_idx, const std::string& id, ivec4& val,
								  const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputInt4(("###" + id).c_str(), val.data.data, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderInt(const LOCALE_STRINGS label_idx, const std::string& id, int* val, const int v_min, const int v_max,
								  const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderInt(("###" + id).c_str(), val, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderIntV(const LOCALE_STRINGS label_idx, const std::string& id, ivec2& val, const int v_min, const int v_max,
								   const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderInt2(("###" + id).c_str(), val.data.data, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderIntV(const LOCALE_STRINGS label_idx, const std::string& id, ivec3& val, const int v_min, const int v_max,
								   const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderInt3(("###" + id).c_str(), val.data.data, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderIntV(const LOCALE_STRINGS label_idx, const std::string& id, ivec4& val, const int v_min, const int v_max,
								   const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderInt4(("###" + id).c_str(), val.data.data, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputFloat(const LOCALE_STRINGS label_idx, const std::string& id, float* val, const std::string& format,
								   const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputFloat(("###" + id).c_str(), val, 0, 0, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec2& val, const std::string& format,
									const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputFloat2(("###" + id).c_str(), val.data.data, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec3& val,
									const std::string& format, const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputFloat3(("###" + id).c_str(), val.data.data, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec4& val,
									const std::string& format, const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputFloat4(("###" + id).c_str(), val.data.data, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderFloat(const LOCALE_STRINGS label_idx, const std::string& id, float* val, const float v_min,
									const float v_max, const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderFloat(("###" + id).c_str(), val, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec2& val, const float v_min,
									 const float v_max, const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderFloat2(("###" + id).c_str(), val.data.data, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec3& val, const float v_min,
									 const float v_max, const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderFloat3(("###" + id).c_str(), val.data.data, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::SliderFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec4& val, const float v_min,
									 const float v_max, const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::SliderFloat4(("###" + id).c_str(), val.data.data, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::DragFloat(const LOCALE_STRINGS label_idx, const std::string& id, float* val, const float v_speed, const float v_min, const float v_max,
								  const std::string& format, const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::DragFloat(("###" + id).c_str(), val, v_speed, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::DragFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec2& val, const float v_speed,
								   const float v_min, const float v_max, const std::string& format, const ImGuiSliderFlags flags, const bool& in_table,
								   const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::DragFloat2(("###" + id).c_str(), val.data.data, v_speed, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::DragFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec3& val, const float v_speed,
								   const float v_min, const float v_max, const std::string& format, const ImGuiSliderFlags flags, const bool& in_table,
								   const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::DragFloat3(("###" + id).c_str(), val.data.data, v_speed, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::DragFloatV(const LOCALE_STRINGS label_idx, const std::string& id, vec4& val, const float v_speed,
								   const float v_min, const float v_max, const std::string& format, const ImGuiSliderFlags flags, const bool& in_table,
								   const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::DragFloat4(("###" + id).c_str(), val.data.data, v_speed, v_min, v_max, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputScalar(const LOCALE_STRINGS label_idx, const std::string& id, const ImGuiDataType data_type,
									void* p_data, const void* p_step, const void* p_step_fast, const std::string& format, const ImGuiInputTextFlags flags,
									const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputScalar(("###" + id).c_str(), data_type, p_data, p_step, p_step_fast, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::InputScalarN(const LOCALE_STRINGS label_idx, const std::string& id, const ImGuiDataType data_type,
									 void* p_data, const int components, const void* p_step, const void* p_step_fast, const std::string& format,
									 const ImGuiInputTextFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::InputScalarN(("###" + id).c_str(), data_type, p_data, components, p_step, p_step_fast, format.c_str(), flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::DragScalar(const LOCALE_STRINGS label_idx, const std::string& id, const ImGuiDataType data_type,
								   void* p_data, const float v_speed, const void* p_min, const void* p_max, const char* format, const ImGuiSliderFlags flags,
								   const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::DragScalar(("###" + id).c_str(), data_type, p_data, v_speed, p_min, p_max, format, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::DragScalarN(const LOCALE_STRINGS label_idx, const std::string& id, const ImGuiDataType data_type,
									void* p_data, const int components, const float v_speed, const void* p_min, const void* p_max, const char* format,
									const ImGuiSliderFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::DragScalarN(("###" + id).c_str(), data_type, p_data, components, v_speed, p_min, p_max, format, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::ColorPicker(const LOCALE_STRINGS label_idx, const std::string& id, vec3& color,
									const ImGuiColorEditFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::ColorEdit3(("###" + id).c_str(), color.data.data, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::ColorPicker(const LOCALE_STRINGS label_idx, const std::string& id, vec4& color,
									const ImGuiColorEditFlags flags, const bool& in_table, const bool& disabled) {
	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	ItemSeparation;
	const bool modified = ImGui::ColorEdit4(("###" + id).c_str(), color.data.data, flags);
	ImGui::EndDisabled();

	return modified;
}

bool Nimbus::GuiUtilities::TimePicker(const LOCALE_STRINGS label_idx, const std::string& id, std::string& time,
								   const bool& in_table, const bool& disabled) {
	int h = std::stoi(time.substr(0, 2)), m = std::stoi(time.substr(3, 2));

	ImGui::BeginDisabled(disabled);
	Text(label_idx);
	if (in_table) { ImGui::TableNextColumn(); } else ImGui::SameLine();

	const float width = ImGui::GetColumnWidth() * 0.5f - ImGui::CalcTextSize(":").x - ImGui::GetStyle().FramePadding.x * 2.0f;
	ImGui::SetNextItemWidth(width);
	bool modified = ImGui::InputInt(("###" + id + "_h").c_str(), &h, 0, 0);
	ImGui::SameLine();
	Text(":");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(width);
	modified |= ImGui::InputInt(("###" + id + "_m").c_str(), &m, 0, 0);

	ImGui::EndDisabled();

	h = std::clamp(h, 0, 24);
	m = std::clamp(m, 0, 59);

	if (modified) {
		char hora_fmt[6] = "00:00";
		snprintf(hora_fmt, 6, "%02d:%02d", h, m);
		time = std::string(hora_fmt);
	}

	return modified;
}

#pragma endregion

#pragma region CONTAINERS

bool Nimbus::GuiUtilities::BeginCombo(const LOCALE_STRINGS label_idx, const std::string& id, const std::string& preview_value,
								   const ImGuiComboFlags flags, const bool& in_table) {
	Text(label_idx);
	ItemSeparation;
	const bool open = ImGui::BeginCombo(("###" + id).c_str(), preview_value.c_str(), flags);

	return open;
}

bool Nimbus::GuiUtilities::ComboItem(const LOCALE_STRINGS label_idx, const std::string& id, const bool& selected,
								  const ImGuiSelectableFlags flags) {
	return Selectable(LocString(label_idx), id, selected, flags);
}

bool Nimbus::GuiUtilities::ComboItem(const std::string& label, const std::string& id, const bool& selected,
								  const ImGuiSelectableFlags flags) {
	return Selectable(label.c_str(), id, selected, flags);
}

bool Nimbus::GuiUtilities::ComboItem(const char* label, const std::string& id, const bool& selected,
								  const ImGuiSelectableFlags flags) {
	return Selectable(label, id, selected, flags);
}

bool Nimbus::GuiUtilities::Combo(const LOCALE_STRINGS label_idx, const std::string& id, int* current_item,
							  const char* const items[], const int items_count, const bool& in_table) {
	Text(label_idx);
	if (in_table) { ImGui::TableNextColumn(); NextItemFillColumn(); } else ImGui::SameLine();
	const bool currentItemValid = *current_item >= 0 && *current_item < items_count;
	const bool open = ImGui::BeginCombo(("###" + id).c_str(), currentItemValid ? items[*current_item] : "");

	if (open) {
		for (int i = 0; i < items_count; ++i) {
			const bool is_selected = i == *current_item;
			if (Selectable(items[i], id + "_comboi_" + std::to_string(i), is_selected)) {
				*current_item = i;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	return open;
}

bool Nimbus::GuiUtilities::CollapsingHeader(const LOCALE_STRINGS label_idx, const std::string& id, const ImGuiTreeNodeFlags flags) {
	return ImGui::CollapsingHeader(LocLabelId, flags);
}

bool Nimbus::GuiUtilities::CollapsingHeader(const LOCALE_STRINGS label_idx, const std::string& icon, const std::string& id,
										 const ImGuiTreeNodeFlags flags) {
	return ImGui::CollapsingHeader((icon + std::string(LocLabelId)).c_str(), flags);
}

bool Nimbus::GuiUtilities::BeginTabBar(const std::string& id, ImGuiTabBarFlags flags)
{
	return ImGui::BeginTabBar(id.c_str(), flags);
}

bool Nimbus::GuiUtilities::BeginTabItem(LOCALE_STRINGS label_idx, const std::string& id, bool* p_open,
	ImGuiTabItemFlags flags)
{
	return ImGui::BeginTabItem(LocLabelId, p_open, flags);
}

#pragma endregion

#pragma region ITEM_SIZING

void Nimbus::GuiUtilities::NextItemFillRegion() {
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
}

void Nimbus::GuiUtilities::NextItemFillColumn() {
	ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
}

float Nimbus::GuiUtilities::GetTableRowHeight() {
	return TableSingleHeight;
}

float Nimbus::GuiUtilities::CalcWidgetWidth(const LOCALE_STRINGS label_idx) {
	return ImGui::CalcTextSize(LocString(label_idx).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f;
}

#pragma endregion

#pragma region TABLE_ITEM_ALIGN

void Nimbus::GuiUtilities::NextTableItemCenter(const LOCALE_STRINGS label_idx) {
	const float buttonWidth = ImGui::CalcTextSize(LocString(label_idx).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - buttonWidth) * 0.5f);
}

void Nimbus::GuiUtilities::NextTableItemRight(const LOCALE_STRINGS label_idx) {
	const float buttonWidth = ImGui::CalcTextSize(LocString(label_idx).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - buttonWidth));
}

void Nimbus::GuiUtilities::NextTableItemCenter(const std::string& label) {
	const float buttonWidth = ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - buttonWidth) * 0.5f);
}

void Nimbus::GuiUtilities::NextTableItemRight(const std::string& label) {
	const float buttonWidth = ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - buttonWidth));
}

#pragma endregion

#pragma region TOAST_NOTIFICATIONS

void Nimbus::GuiUtilities::InsertToast(const LOCALE_STRINGS label_idx, const ImGuiToastType& type, const int& duration) {
	ImGui::InsertNotification(ImGuiToast{ type, duration, LocString(label_idx).c_str() });
}

void Nimbus::GuiUtilities::InsertToast(const LOCALE_STRINGS label_idx, const ImGuiToastType& type, float& progress) {
	ImGui::InsertNotification(ImGuiToast{ type, progress, LocString(label_idx).c_str() });
}

#pragma endregion

#pragma region Nimbus_UTILITIES

bool Nimbus::GuiUtilities::ComboPointCloud(PointCloud*& ptr, Scene* activeScene, const bool& in_table, const bool& disabled) {
	PointCloud* origPtr = ptr;

	ImGui::AlignTextToFramePadding();
	Text(SCENEITEM_PCLOUD);

	if (!ptr && !activeScene->_pointClouds.empty()) {
		ptr = activeScene->_pointClouds.begin()->second.get();
	} else if (ptr && activeScene->_pointClouds.empty()) {
		ptr = nullptr;
	}

	ItemSeparation;

	ImGui::BeginDisabled(disabled);
	if (ImGui::BeginCombo("##pointCloudCombo",
						  !ptr ? LocString(SELECTION_NOCLOUD).c_str() : ptr->getName().c_str())) {
		for (auto& val : activeScene->_pointClouds | std::views::values) {
			const bool is_selected = ptr == val.get();

			if (GuiUtilities::Selectable(val->getName(), fmt::format("pointCloudCombo_{}", val->getName()), is_selected)) {
				ptr = val.get();
			}

			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	return ptr != origPtr;
}

#pragma endregion

#pragma region PLOTS

void Nimbus::GuiUtilities::Histogram(const LOCALE_STRINGS& label_idx, const std::string& id, const std::vector<float>& values, ImPlotRange range, ImPlotHistogramFlags flags,
								  bool in_table) {
	Text(label_idx);
	ItemSeparation;
	if (ImPlot::BeginPlot(("###plotcontext_" + id).c_str())) {

		ImPlot::PlotHistogram(("###" + id).c_str(), values.data(), values.size(), values.size(), 1, range, flags);
		ImPlot::EndPlot();
	}

}

void Nimbus::GuiUtilities::BarPlot(const LOCALE_STRINGS& label_idx, const std::string& id,
								const std::vector<float>& values, ImPlotBarsFlags flags, bool in_table) {
	Text(label_idx);
	ItemSeparation;
	if (ImPlot::BeginPlot(("###plotcontext_" + id).c_str())) {
		ImPlot::PlotBars(("###" + id).c_str(), values.data(), values.size(), 0.67, 0, flags);
		ImPlot::EndPlot();
	}
}

void Nimbus::GuiUtilities::BarPlot(const LOCALE_STRINGS& label_idx, const std::string& id,
								const std::vector<float>& values, const LOCALE_STRINGS& axis_X_label, const LOCALE_STRINGS& axis_Y_label,
								ImPlotBarsFlags plot_flags, ImPlotAxisFlags axis_X_flags, ImPlotAxisFlags axis_Y_flags, bool in_table) {
	Text(label_idx);
	ItemSeparation;
	if (ImPlot::BeginPlot(("###plotcontext_" + id).c_str())) {
		ImPlot::SetupAxes(LocString(axis_X_label).c_str(), LocString(axis_Y_label).c_str(), axis_X_flags, axis_Y_flags);
		ImPlot::PlotBars(("###" + id).c_str(), values.data(), values.size(), 0.67, 0, plot_flags);
		ImPlot::EndPlot();
	}
}

void Nimbus::GuiUtilities::BarPlot(const LOCALE_STRINGS& label_idx, const std::string& id,
								const std::vector<u32>& values, const LOCALE_STRINGS& axis_X_label, const LOCALE_STRINGS& axis_Y_label,
								ImPlotBarsFlags plot_flags, ImPlotAxisFlags axis_X_flags, ImPlotAxisFlags axis_Y_flags, bool in_table) {
	Text(label_idx);
	ItemSeparation;
	if (ImPlot::BeginPlot(("###plotcontext_" + id).c_str())) {
		ImPlot::SetupAxes(LocString(axis_X_label).c_str(), LocString(axis_Y_label).c_str(), axis_X_flags, axis_Y_flags);
		ImPlot::PlotBars(("###" + id).c_str(), values.data(), values.size(), 0.67, 0, plot_flags);
		ImPlot::EndPlot();
	}
}

bool Nimbus::GuiUtilities::BeginPlot(const std::string& id, const LOCALE_STRINGS& axis_X_label,
								  const LOCALE_STRINGS& axis_Y_label, ImPlotAxisFlags axis_X_flags, ImPlotAxisFlags axis_Y_flags) {
	if (ImPlot::BeginPlot(("###plotcontext_" + id).c_str())) {
		ImPlot::SetupAxes(LocString(axis_X_label).c_str(), LocString(axis_Y_label).c_str(), axis_X_flags, axis_Y_flags);
		return true;
	}
	return false;
}

#pragma endregion

void Nimbus::GuiUtilities::leaveSpace(const unsigned numSlots) {
	for (unsigned i = 0; i < numSlots; ++i) ImGui::Spacing();
}

void Nimbus::GuiUtilities::renderText(const vec3& xyz, const std::string& title, const char delimiter) {
	const std::string txt = title + (title.empty() ? "" : ": ") + std::to_string(xyz.x) + delimiter + ' ' + std::to_string(xyz.y) + delimiter + ' ' + std::to_string(xyz.z);
	ImGui::Text(txt.c_str());
}

void Nimbus::GuiUtilities::renderText(const mat4& mat, const std::string& title, const char delimiter) {
	std::string txt = title + (title.empty() ? "" : ": ");
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			txt += std::to_string(mat[r][c]) + delimiter + "	";
		}
		txt += '\n';
	}
	ImGui::Text(txt.c_str());
}

void Nimbus::GuiUtilities::renderText(const float* mat, const std::string& title, const char delimiter) {
	std::string txt = title + (title.empty() ? "" : ": ");
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			txt += std::to_string(mat[c + (r * 4)]) + delimiter + "	";
		}
		txt += '\n';
	}
	ImGui::Text(txt.c_str());
}

void Nimbus::GuiUtilities::renderTexture(const Texture* texture) {
	ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture->getId())), ImVec2(texture->getWidth(), texture->getHeight()));
}

bool Nimbus::GuiUtilities::makeButton(const std::string& title, const bool& disabled) {
	if (disabled)
		ImGui::BeginDisabled(disabled);

	const bool press = makeButton(title.c_str());
	if (disabled)
		ImGui::EndDisabled();

	return press;
}

bool Nimbus::GuiUtilities::makeButton(const char* title, const ImVec2& size, const bool& centered) {
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::INVERSE_TEXT_COLOR);
	if (centered) {
		const auto windowWidth = ImGui::GetWindowSize().x;
		const auto textWidth = ImGui::CalcTextSize(title).x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	}
	const bool buttonPress = ImGui::Button(title, size);


	ImGui::PopStyleColor();
	return buttonPress;
}

bool Nimbus::GuiUtilities::makeYesButton(const bool& disabled) {
	if (disabled)
		ImGui::BeginDisabled();

	const bool press = makeButton(LocaleStrings::getInstance()->getString(LOCALE_STRINGS::GENERIC_YES));
	if (disabled)
		ImGui::EndDisabled();

	return press;
}

bool Nimbus::GuiUtilities::makeNoButton(const bool& disabled) {
	if (disabled)
		ImGui::BeginDisabled();

	const bool press = makeButton(LocaleStrings::getInstance()->getString(LOCALE_STRINGS::GENERIC_NO));
	if (disabled)
		ImGui::EndDisabled();

	return press;
}

bool Nimbus::GuiUtilities::makeConfirmButton(const bool& disabled) {
	if (disabled)
		ImGui::BeginDisabled();

	const bool press = makeButton(LocaleStrings::getInstance()->getString(LOCALE_STRINGS::GENERIC_CONFIRM));
	if (disabled)
		ImGui::EndDisabled();

	return press;
}

bool Nimbus::GuiUtilities::makeCancelButton(const bool& disabled) {
	if (disabled)
		ImGui::BeginDisabled();

	const bool press = makeButton(LocaleStrings::getInstance()->getString(LOCALE_STRINGS::GENERIC_CANCEL));
	if (disabled)
		ImGui::EndDisabled();

	return press;
}

void Nimbus::GuiUtilities::makeLocText(const LOCALE_STRINGS idx, const bool& wrap) {
	if (wrap)
		ImGui::TextWrapped(LocaleStrings::getInstance()->getString(idx).c_str());
	else
		ImGui::Text(LocaleStrings::getInstance()->getString(idx).c_str());
}

bool Nimbus::GuiUtilities::makeLocButton(const LOCALE_STRINGS idx, const bool& disabled) {
	return makeButton(LocaleStrings::getInstance()->getString(idx), disabled);
}

bool Nimbus::GuiUtilities::makeLocInputText(const LOCALE_STRINGS idx, std::string* textString, const ImGuiInputTextFlags flags) {
	return ImGui::InputText(LocaleStrings::getInstance()->getString(idx).c_str(), textString, flags);
}

void Nimbus::GuiUtilities::makeLocSepText(const LOCALE_STRINGS idx) {
	ImGui::SeparatorText(LocaleStrings::getInstance()->getString(idx).c_str());
}

std::string Nimbus::GuiUtilities::loadingIcon() {
	const int fr = static_cast<int>(ImGui::GetTime() / 0.1f) & 4;
	return fr < 2 ? ICON_FA_CIRCLE : ICON_FA_CIRCLE_DOT;
}

void Nimbus::GuiUtilities::makeSelectableTextBox(const LOCALE_STRINGS idx, const std::string& str, const std::string& id, const bool& in_table, const bool& fillWidth) {
	static float textBoxOffset = 10.0f;
	std::string	ncStr = str;

	ImGui::AlignTextToFramePadding();
	makeLocText(idx);
	ImGui::SameLine();
	if (in_table)
		ImGui::TableNextColumn();
	if (fillWidth)
		ImGui::SetNextItemWidth(-FLT_MIN);
	else
		ImGui::SetNextItemWidth(ImGui::CalcTextSize(ncStr.c_str()).x + textBoxOffset);
	ImGui::InputText(("##" + id).c_str(), &ncStr, ImGuiInputTextFlags_ReadOnly);
}

void Nimbus::GuiUtilities::makeHelpMarker(const LOCALE_STRINGS idx, const bool useIcon) {
	if (useIcon) {
		ImGui::TextDisabled(ICON_FA_INFO);
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
		ImGui::SetTooltip(LocaleStrings::getInstance()->getString(idx).c_str());
	}
}

float Nimbus::GuiUtilities::calcSingleRowTableHeight() {
	return (ImGui::CalcTextSize("|").y + ImGui::GetStyle().FramePadding.y * 2.0f) * 2.0f + ImGui::GetStyle().ScrollbarSize;
}

void Nimbus::GuiUtilities::makeColorGradient(float colorA[3], float colorB[3], const ImVec2 gradient_size) {
	const auto drawList = ImGui::GetWindowDrawList();
	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	const ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
	const ImU32 col_a = ImGui::GetColorU32(IM_COL32(colorA[0] * 255, colorA[1] * 255, colorA[2] * 255, 255));
	const ImU32 col_b = ImGui::GetColorU32(IM_COL32(colorB[0] * 255, colorB[1] * 255, colorB[2] * 255, 255));
	drawList->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
	ImGui::InvisibleButton("##gradient1", gradient_size);
}

void Nimbus::GuiUtilities::setNextTableItemAlign(const LOCALE_STRINGS idx, const bool center) {
	const float buttonWidth = ImGui::CalcTextSize(LocaleStrings::getInstance()->getString(idx).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - buttonWidth) * (center ? 0.5f : 1.0f));
}

void Nimbus::GuiUtilities::makeImageViewFill(const Texture* _image, const ImVec2 padding, const bool useTableSize) {
	const ImVec2 texSize(_image->getWidth(), _image->getHeight());
	const float rAspInv = texSize.y / texSize.x;
	const float availableWidth = useTableSize ? ImGui::GetColumnWidth() : ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - padding.x;
	const float availableHeight = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - padding.y - (useTableSize ? ImGui::GetStyle().CellPadding.y : 0.0f);
	const float imageHeight = availableWidth * rAspInv;
	ImVec2 imageSize;


	if (imageHeight > availableHeight) {
		imageSize = ImVec2(availableHeight / rAspInv, availableHeight);
	} else {
		imageSize = ImVec2(availableWidth, availableWidth * rAspInv);
	}


	ImGui::Image((ImTextureID)(intptr_t)(_image->getId()), imageSize);
}

Nimbus::GuiUtilities::ImageTooltipDrawArea Nimbus::GuiUtilities::makeImageZoomable(const Texture* image, const ImVec2 padding, const bool sizeFromTable, const float zoomRegionSize, const float zoomFactor, const bool showCrosshair) {
	ImageTooltipDrawArea dArea{
		._areaBegin = ImVec2(0, 0),
		._areaEnd = ImVec2(0, 0),
		._areaDrawList = nullptr,
		._uvBegin = ImVec2(0, 0),
		._uvEnd = ImVec2(0, 0)
	};

	const ImVec2 texSize(image->getWidth(), image->getHeight());
	const float rAspInv = texSize.y / texSize.x;
	const float availableWidth = sizeFromTable ? ImGui::GetColumnWidth() : ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - padding.x;
	const float availableHeight = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - padding.y - (sizeFromTable ? ImGui::GetStyle().CellPadding.y : 0.0f);
	const float imageHeight = availableWidth * rAspInv;
	ImVec2 imageSize;


	if (imageHeight > availableHeight) {
		imageSize = ImVec2(availableHeight / rAspInv, availableHeight);
	} else {
		imageSize = ImVec2(availableWidth, availableWidth * rAspInv);
	}


	const ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::Image((ImTextureID)(intptr_t)(image->getId()), imageSize);
	//
	// if (!showZoomTooltip)
	// 	return dArea;

	if (ImGui::BeginItemTooltip()) {
		const auto io = ImGui::GetIO();
		const float region_sz = zoomRegionSize;
		const float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
		const float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
		const float zoom = zoomFactor;
		// if (region_x < 0.0f) { region_x = 0.0f; }
		// else if (region_x > imageSize.x - region_sz) { region_x = imageSize.x - region_sz; }
		// if (region_y < 0.0f) { region_y = 0.0f; }
		// else if (region_y > imageSize.y - region_sz) { region_y = imageSize.y - region_sz; }
		dArea._uvBegin = ImVec2((region_x) / imageSize.x, (region_y) / imageSize.y);
		dArea._uvEnd = ImVec2((region_x + region_sz) / imageSize.x, (region_y + region_sz) / imageSize.y);

		const ImVec2 drawAreaInit = ImGui::GetCursorScreenPos();
		ImVec2 drawAreaEnd;
		if (showCrosshair) {
			ImGui::Image((ImTextureID)(intptr_t)(image->getId()), ImVec2(region_sz * zoom, region_sz * zoom), dArea._uvBegin, dArea._uvEnd);
			drawAreaEnd = ImVec2(drawAreaInit.x + region_sz * zoom, drawAreaInit.y + region_sz * zoom);

			ImDrawList* guiDrawList = ImGui::GetWindowDrawList();
			const ImVec2 halfDrawArea((drawAreaEnd.x - drawAreaInit.x) * 0.5f + drawAreaInit.x,
									  (drawAreaEnd.y - drawAreaInit.y) * 0.5f + drawAreaInit.y);

			guiDrawList->AddLine(ImVec2(halfDrawArea.x, halfDrawArea.y - 2),
								 ImVec2(halfDrawArea.x, halfDrawArea.y - 18), ImColor(1.0f, 1.0f, 1.0f));
			guiDrawList->AddLine(ImVec2(halfDrawArea.x, halfDrawArea.y + 2),
								 ImVec2(halfDrawArea.x, halfDrawArea.y + 18), ImColor(1.0f, 1.0f, 1.0f));
			guiDrawList->AddLine(ImVec2(halfDrawArea.x - 2, halfDrawArea.y),
								 ImVec2(halfDrawArea.x - 18, halfDrawArea.y), ImColor(1.0f, 1.0f, 1.0f));
			guiDrawList->AddLine(ImVec2(halfDrawArea.x + 2, halfDrawArea.y),
								 ImVec2(halfDrawArea.x + 18, halfDrawArea.y), ImColor(1.0f, 1.0f, 1.0f));
		} else {
			ImGui::Image((ImTextureID)(intptr_t)(image->getId()), ImVec2(region_sz * zoom, region_sz * zoom), dArea._uvBegin, dArea._uvEnd);
			drawAreaEnd = ImVec2(drawAreaInit.x + region_sz * zoom, drawAreaInit.y + region_sz * zoom);
		}
		dArea._areaBegin = drawAreaInit;
		dArea._areaEnd = drawAreaEnd;
		dArea._areaDrawList = ImGui::GetWindowDrawList();

		ImGui::EndTooltip();
	}

	return dArea;
}

void Nimbus::GuiUtilities::makeCloudSelector(PointCloud*& ptr, Scene* activeScene) {
	ImGui::AlignTextToFramePadding();
	GuiUtilities::makeLocText(LOCALE_STRINGS::SCENEITEM_PCLOUD);
	ImGui::SameLine();
	if (!ptr && !activeScene->_pointClouds.empty()) {
		ptr = activeScene->_pointClouds.begin()->second.get();
	} else if (ptr && activeScene->_pointClouds.empty()) {
		ptr = nullptr;
	}

	if (ImGui::BeginCombo("##pointCloudCombo",
						  !ptr
						  ? LocaleStrings::getInstance()->getString(SELECTION_NOCLOUD).c_str()
						  : ptr->getName().c_str())) {
		for (auto& val : activeScene->_pointClouds | std::views::values) {
			const bool is_selected = ptr == val.get();

			if (GuiUtilities::Selectable(val->getName(), fmt::format("pointCloudCombo_{}", val->getName()), is_selected)) {
				ptr = val.get();
			}

			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}
