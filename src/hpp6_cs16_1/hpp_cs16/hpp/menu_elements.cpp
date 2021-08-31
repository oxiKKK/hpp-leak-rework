#include "main.h"

const char* ImGui::Control::m_pcszIdentifyString;
std::deque<bool> ImGui::Control::m_dConditions;
bool ImGui::Control::m_bKeyAssignState;
std::deque<ImVec4> ImGui::Control::m_TextColor;

void ImGui::Spacing(const size_t count)
{
	for (size_t i = 0; i < count; i++)
		Spacing();
}

float ImGui::GetSameLinePadding()
{
	return 12.0F;
}

void ImGui::Control::ShowTooltipMsg(const char* message)
{
	const auto* const pFont = GetIO().Fonts->Fonts[0];
	const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, message);
	const auto SAMELINE_OFFSET = 8.0F;

	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
	ImGui::SetNextWindowSize(ImVec2(text_size.x + SAMELINE_OFFSET, text_size.y + SAMELINE_OFFSET));
	ImGui::BeginTooltip();
	ImGui::Spacing(2);
	ImGui::SameLine(SAMELINE_OFFSET * 0.5F);
	Text(message);
	ImGui::EndTooltip();
	ImGui::PopStyleColor();
}

void ImGui::Control::ClampColor(
	float* color, 
	const size_t size)
{
	for (size_t i = 0; i < size; i++)
		color[i] = ImClamp(color[i], 0.0F, 1.0F);
}

std::string ImGui::Control::Base(
	const char* label,
	const char* message)
{
	Text(label);

	if (message && ImGui::IsItemHovered())
		ShowTooltipMsg(message);

	const auto control_offset = ImGui::GetWindowWidth() * 0.5F;
	ImGui::SameLine(control_offset);

	std::string s_control_base;
	s_control_base.append(m_pcszIdentifyString);
	s_control_base.append("_");
	s_control_base.append(label);
	return s_control_base;
}

void ImGui::Control::SetStringIdentify(const char* str_id)
{
	m_pcszIdentifyString = str_id;
}

void ImGui::Control::PushCondition(const bool condition)
{
	m_dConditions.push_back(condition);
}

void ImGui::Control::PopCondition(size_t count)
{
	while (count > 0)
	{
		m_dConditions.pop_back();
		count--;
	}
}

void ImGui::Control::PushTextColor(const ImVec4& color)
{
	m_TextColor.push_back(color);
}

void ImGui::Control::PopTextColor(size_t count)
{
	while (count > 0)
	{
		m_TextColor.pop_back();
		count--;
	}
}

void ImGui::Control::Text(const char* fmt, ...)
{
	bool text_disabled = true;

	for (auto& condition : m_dConditions)
		if (!(text_disabled = condition))
			break;

	if (m_TextColor.size() > 0)
		ImGui::PushStyleColor(ImGuiCol_Text, m_TextColor.at(m_TextColor.size() - 1));

	if (!text_disabled)
	{
		const auto text_disabled_color = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
		ImGui::PushStyleColor(ImGuiCol_Text, text_disabled_color);
	}

	va_list args;
	va_start(args, fmt);
	ImGui::TextV(fmt, args);
	va_end(args);

	if (!text_disabled)
		ImGui::PopStyleColor();

	if (m_TextColor.size() > 0)
		ImGui::PopStyleColor();

}

bool ImGui::Control::Checkbox(
	bool* v,
	const char* label,
	const char* message)
{
	const auto base = Base(label, message);
	const auto ret = ImGui::Checkbox(base.c_str(), v);
	return ret;
}

bool ImGui::Control::Combo(
	int* current_item,
	const char* label,
	const char* const items[],
	const int size, // items count
	const char* message)
{
	const auto base = Base(label, message);
	const auto ret = ImGui::Combo(base.c_str(), current_item, items, size, -1);
	return ret;
}

bool ImGui::Control::SliderFloat(
	float* v,
	const char* label,
	const float min, // value min
	const float max, // value max
	const char* format,
	const float power,
	const char* message)
{
	const auto base = Base(label, message);
	const auto ret = ImGui::SliderFloat(base.c_str(), v, min, max, format, power);
	*v = ImClamp(*v, min, max);
	return ret;
}

bool ImGui::Control::SliderInt(
	int* v,
	const char* label,
	const int min, // value min
	const int max, // value max
	const char* format,
	const char* message)
{
	const auto base = Base(label, message);
	const auto ret = ImGui::SliderInt(base.c_str(), v, min, max, format);
	*v = ImClamp(*v, min, max);
	return ret;
}

bool ImGui::Control::InputText(
	char* v,
	const char* label,
	const size_t size,
	const ImGuiInputTextFlags flags,
	const char* message)
{
	const auto base = Base(label, message);
	const auto ret = ImGui::InputText(base.c_str(), v, size, flags);
	return ret;
}

bool ImGui::Control::ColorEdit3(
	float* col,
	const char* label)
{
	ClampColor(col, 3);
	const auto base = Base(label);
	const auto ret = ImGui::ColorEdit3(base.c_str(), col);
	return ret;
}

bool ImGui::Control::ColorEdit4(
	float* col,
	const char* label)
{
	ClampColor(col, 4);
	const auto base = Base(label);
	const auto ret = ImGui::ColorEdit4(base.c_str(), col);
	return ret;
}

void ImGui::Control::ColorButton3(
	float* col,
	const char* label,
	const float sameline)
{
	ClampColor(col, 3);
	ImGui::SameLine(sameline != -1.0F ? sameline : ImGui::GetWindowWidth() * 0.5F);

	if (ImGui::ColorButton(label, ImColor(col), 0, ImVec2(32, 0)))
	{
		const auto LastItemRect = ImGui::GetCurrentWindow()->DC.LastItemRect;
		ImGui::OpenPopup(label);
		ImGui::SetNextWindowPos(ImVec2(LastItemRect.GetBL().x - 1.0F,
			LastItemRect.GetBL().y + GImGui->Style.ItemSpacing.y));
	}

	if (ImGui::BeginPopup(label))
	{
		ImGui::ColorPicker3("##picker", col, ImGuiColorEditFlags_PickerHueBar);
		ImGui::EndPopup();
	}
}

void ImGui::Control::ColorButton4(
	float* col,
	const char* label,
	const float sameline)
{
	ClampColor(col, 4);
	ImGui::SameLine(sameline != -1.0F ? sameline : ImGui::GetWindowWidth() * 0.5F);

	if (ImGui::ColorButton(label, ImColor(col), ImGuiColorEditFlags_AlphaPreview, ImVec2(32, 0)))
	{
		const auto LastItemRect = ImGui::GetCurrentWindow()->DC.LastItemRect;
		GImGui->ColorPickerRef = ImColor(col);
		ImGui::OpenPopup(label);
		ImGui::SetNextWindowPos(ImVec2(LastItemRect.GetBL().x - 1.0F,
			LastItemRect.GetBL().y + GImGui->Style.ItemSpacing.y));
	}

	if (ImGui::BeginPopup(label))
	{
		ImGui::ColorPicker4("##picker", col, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar |
			ImGuiColorEditFlags_AlphaPreviewHalf, &GImGui->ColorPickerRef.x);
		ImGui::EndPopup();
	}
}

void ImGui::Control::KeyAssignment(
	int* key,
	const char* label,
	const bool center,
	const bool panic)
{
	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];
	const std::string s_key = g_Utils.KeyToString(*key);
	const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, s_key.c_str());
	const auto s_button_label = s_key + label;

	std::string s_popup_label;
	s_popup_label.append("##KeyAssignment");
	s_popup_label.append(label);

	const auto window_width = ImGui::GetWindowWidth();
	const auto button_offset = center
		? window_width * 0.5F 
		: (window_width * 0.5F - GetSameLinePadding()) * 2.0F - text_size.x - 9.0F;

	ImGui::SameLine(panic ? 0.0F : button_offset);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

	if (ImGui::Button(s_button_label.c_str()))
	{
		ImGui::OpenPopup(s_popup_label.c_str());
		KeyAssignBegin();
	}

	ImGui::PopStyleColor();

	if (ModalPopup::Begin(s_popup_label.c_str(), ImVec2(125, 38)))
	{
		const char* message = "Press any key";
		const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, message);
		const auto center_offset = ((ImGui::GetWindowWidth()) - text_size.x) * 0.5F;

		ImGui::Spacing(4);
		ImGui::SameLine(center_offset);
		ImGui::Text(message);

		int keynum;
		g_Utils.ConvertToASCII(&keynum);

		if (keynum != 0 || !m_bKeyAssignState)
		{
			*key = keynum;
			ImGui::CloseCurrentPopup();
		}

		ModalPopup::End();
	}
}

static auto vector_getter = [](
	void* data, 
	const int id, 
	const char** out_text)
{
	const auto& vector = *static_cast<std::vector<std::string>*>(data);
	*out_text = vector.at(id).c_str();
	return true;
};

bool ImGui::Control::ListBoxArray(
	const char* label, 
	int* current_item, 
	std::vector<std::string>& values, 
	const int height_in_items)
{
	return ImGui::ListBox(label, current_item, vector_getter, &values, values.size(), height_in_items);
}

bool ImGui::Control::ComboArray(
	const char* label,
	int* current_item,
	std::vector<std::string>& values, 
	const int height_in_items)
{
	return ImGui::Combo(label, current_item, vector_getter, &values, values.size(), height_in_items);
}

void ImGui::Control::KeyAssignBegin()
{
	if (!m_bKeyAssignState)
		m_bKeyAssignState = true;
}

void ImGui::Control::KeyAssingEnd()
{
	if (m_bKeyAssignState)
		m_bKeyAssignState = false;
}

bool ImGui::Control::IsKeyAssingActive()
{
	return m_bKeyAssignState;
}

bool ImGui::Child::Begin(
	const char* label, 
	const ImVec2 size)
{
	const auto border_color = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
	ImGui::PushStyleColor(ImGuiCol_Border, border_color);
	const auto ret = ImGui::BeginChild(label, size, true);
	ImGui::PopStyleColor();
	ImGui::Spacing();
	ImGui::SameLine(GetSameLinePadding());
	ImGui::BeginGroup();
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5F - GetSameLinePadding() * 2.0F);
	ImGui::Spacing(3);
	return ret;
}

void ImGui::Child::End()
{
	ImGui::Spacing(3);
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	ImGui::EndChild();
}

void ImGui::Tab::Begin(
	bool* v, 
	const char* label, 
	int* key, 
	const int state)
{
	if (v)
	{
		ImGui::Spacing(3);
		ImGui::SameLine(GetSameLinePadding());
		ImGui::Text("Active");
		ImGui::SameLine(60);
		ImGui::Checkbox(label, v);
		
		if (key)
		{
			Control::KeyAssignment(key, label, false, true);
			
			if (state)
			{
				ImGui::SameLine();
				ImGui::Text("panic is active");
			}
		}
	}

	ImGui::Spacing(4);
	ImGui::SameLine(GetSameLinePadding());
	ImGui::BeginGroup();
}

void ImGui::Tab::End()
{
	ImGui::EndGroup();
}

bool ImGui::ModalPopup::Begin(
	const char* label, 
	const ImVec2 size)
{
	ImGui::SetNextWindowSize(size);
	const auto ret = ImGui::BeginPopupModal(label, static_cast<bool*>(0),
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	return ret;
}

void ImGui::ModalPopup::End()
{
	ImGui::EndPopup();
}