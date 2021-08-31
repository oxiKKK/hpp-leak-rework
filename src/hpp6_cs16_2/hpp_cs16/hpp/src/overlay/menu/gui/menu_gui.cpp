#include "framework.h"

CMenuGui::CMenuGui() 
	: m_pPopupModal(std::make_unique<CMenuPopupModal>())
{
}

bool CMenuGui::Begin(std::string name, const ImVec2& size)
{
	ImVec2 size_const = size;

	size_const.x = min(size_const.x, GImGui->IO.DisplaySize.x);
	size_const.y = min(size_const.y, GImGui->IO.DisplaySize.y);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());

	ImGui::SetNextWindowSizeConstraints(size_const, GImGui->IO.DisplaySize);
	ImGui::SetNextWindowSize(size, ImGuiCond_Once);
	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);

	const bool ret = ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar);

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	return ret;
}

void CMenuGui::BeginCanvas(std::string name, const ImVec2& size, bool border)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, GImGui->Style.Colors[ImGuiCol_WindowBg]);
	ImGui::BeginChild(name.c_str(), size, border);
	ImGui::PopStyleColor();
}

void CMenuGui::BeginGroupBox(std::string name, const ImVec2& size, bool border)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
	ImGui::BeginChildCustom(name.c_str(), size, border);
	ImGui::PopStyleVar();
}

void CMenuGui::BeginWidgetsGroup()
{
	ImGui::Spacing(WINDOW_WIDGETS_PADDING_SPACING);
	ImGui::SameLine(WINDOW_WIDGETS_PADDING_SAMELINE);
	ImGui::BeginGroup();
}

void CMenuGui::End()
{
	ImVec2 window_pos = GImGui->CurrentWindow->Pos;
	ImVec2 window_size = GImGui->CurrentWindow->Size;

	GImGui->CurrentWindow->DrawList->PushClipRectFullScreen();

	RectangleShadowSettings shadowSettings;
	shadowSettings.shadowColor = GImGui->Style.Colors[ImGuiCol_BorderShadow];
	ImGui::DrawRectangleShadowVerticesAdaptive(shadowSettings, window_pos, window_size);

	ImGui::End();
}

void CMenuGui::EndCanvas()
{
	ImGui::EndChild();
}

void CMenuGui::EndGroupBox()
{
	ImGui::EndChildCustom();
}

void CMenuGui::EndWidgetsGroup()
{
	ImGui::Spacing(3);
	ImGui::EndGroup();
}

void CMenuGui::SmallGroupBox(std::string name, const ImVec2& size, std::function<void()> widget_first, std::function<void()> widget_second)
{
	BeginGroupBox(name, size);

	std::string widgets_names[2] = { name + "widget_first", name + "widget_second" };

	ImGui::BeginChild(widgets_names[0].c_str(), ImVec2(size.x * 0.5f, size.y));
	ImGui::SetCursorPos(ImVec2(WINDOW_WIDGETS_PADDING_SAMELINE, 9));
	widget_first();
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild(widgets_names[1].c_str(), ImVec2(size.x - ImGui::GetCursorPosX(), size.y));
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + WINDOW_WIDGETS_PADDING_SAMELINE, 9));
	widget_second();
	ImGui::EndChild();

	EndGroupBox();
}

void CMenuGui::UnderBackground()
{
	ImVec2 background_pos = GImGui->CurrentWindow->Pos + ImVec2(0, GImGui->CurrentWindow->Size.y - WINDOW_UNDERGROUND_HEIGHT);
	ImVec2 background_size = ImVec2(GImGui->CurrentWindow->Size.x, WINDOW_UNDERGROUND_HEIGHT);

	g_pRenderer->AddRectFilled(background_pos, background_pos + background_size, GImGui->Style.Colors[ImGuiCol_Tab]);
}

void CMenuGui::UnderContents()
{
	ImGui::PushFont(g_pFontList[Verdana_13px]);

	GImGui->CurrentWindow->DC.CursorPos = ImVec2(GImGui->CurrentWindow->Pos.x + 6, 
		GImGui->CurrentWindow->Pos.y + GImGui->CurrentWindow->Size.y - 16);

	if (m_message.empty())
	{
		std::string hackinfo = "V6 for Counter-Strike 1.6 | Online: ";

		if (!g_pGlobals->m_iOnlineNum)
		{
			static int count, i;
			while (i++ < count) { hackinfo.append("."); } i = 0;
			if (++count > 3) count = 0;
		}
		else
			hackinfo.append(std::to_string(g_pGlobals->m_iOnlineNum));

		ImGui::Text(hackinfo.c_str());
	}
	else
	{
		ImGui::Text(m_message.c_str());

		m_message.clear();
	}

	std::string url = "https://hpp.ovh";

	ImVec2 text_size = GImGui->Font->CalcTextSizeA(GImGui->Font->FontSize, FLT_MAX, 0, url.c_str());

	ImGui::SameLine(GImGui->CurrentWindow->Size.x - text_size.x - WINDOW_PADDING_SAMELINE * 0.5f);

	Link(url);

	ImGui::PopFont();
}

void CMenuGui::TabBackground()
{
	ImVec2 background_size = ImVec2(GImGui->CurrentWindow->Size.x, MENU_TAB_HEIGHT);

	g_pRenderer->AddRectFilled(GImGui->CurrentWindow->Pos, GImGui->CurrentWindow->Pos + background_size, GImGui->Style.Colors[ImGuiCol_Tab]);

	ImGui::Image(g_pImageList[MenuTitleBg].data, ImVec2(g_pImageList[MenuTitleBg].size.x - 70.f, MENU_TAB_HEIGHT - 2));
}

bool CMenuGui::TabList(std::vector<TabWidgetsData>& data, int& selected, int& hovered)
{
	ImGui::SameLine(GImGui->CurrentWindow->Size.x / 3.5f);

	ImGuiStyle backup_style = GImGui->Style;

	GImGui->Style.ItemSpacing = ImVec2(0, 1);

	ImVec2 button_size = ImVec2((GImGui->CurrentWindow->Size.x - ImGui::GetCursorPosX()) / data.size(), MENU_TAB_HEIGHT);
	ImVec4 button_color = GImGui->Style.Colors[ImGuiCol_TabHovered]; 

	button_color.w = 0.00f;

	bool changed = false;

	ImGui::PushFont(g_pFontList[Verdana_16px]);

	for (size_t i = 0; i < data.size(); i++)
	{
		GImGui->Style.Colors[ImGuiCol_ButtonActive] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_TabHovered], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_ButtonHovered] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_TabHovered], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_Button] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_TabHovered], ImSaturate(data[i].fading_value));

		bool pressed_button = false;

		if (selected == static_cast<int>(i))
		{
			GImGui->Style.Colors[ImGuiCol_Button] = GImGui->Style.Colors[ImGuiCol_TabActive];
			GImGui->Style.Colors[ImGuiCol_ButtonHovered] = GImGui->Style.Colors[ImGuiCol_TabActive];
			GImGui->Style.Colors[ImGuiCol_ButtonActive] = GImGui->Style.Colors[ImGuiCol_TabActive];

			pressed_button = ImGui::ButtonTabsNormal(data[i].label.c_str(), button_size);
		}
		else if (hovered == static_cast<int>(i))
		{
			pressed_button = ImGui::ButtonTabsNormal(data[i].label.c_str(), button_size);
		}
		else
		{
			pressed_button = ImGui::ImageButtonTab(g_pImageList[i + IconsTabsOffset].data, button_size);
		}

		if (pressed_button)
		{
			changed = selected != static_cast<int>(i);
			selected = static_cast<int>(i);
		}

		if (ImGui::IsItemHovered())
		{
			hovered = static_cast<int>(i);

			data[i].fading_value = min(data[i].fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
		}
		else
		{
			data[i].fading_value = max(data[i].fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);

			if (hovered == static_cast<int>(i))
				hovered = -1;
		}

		if (i < data.size() - 1)
			ImGui::SameLine();
	}

	ImGui::PopFont();

	GImGui->Style = backup_style;

	return changed;
}

bool CMenuGui::TabFeaturesList(std::vector<TabWidgetsData>& data, int& selected, int& hovered)
{
	ImGuiStyle backup_style = GImGui->Style;

	GImGui->Style.ItemSpacing = ImVec2(0, 1);
	GImGui->Style.Colors[ImGuiCol_Border] = ImColor();

	ImVec2 button_size = ImVec2(GImGui->CurrentWindow->Size.x / data.size(), 32);
	ImVec4 button_color = GImGui->Style.Colors[ImGuiCol_ChildBg];
	ImVec4 normal_text_color = GImGui->Style.Colors[ImGuiCol_FrameBgHovered];

	bool changed = false;

	ImGui::PushFont(g_pFontList[Verdana_16px]);

	for (size_t i = 0; i < data.size(); i++)
	{
		GImGui->Style.Colors[ImGuiCol_ButtonActive] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_ButtonHovered] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_Button] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_Text] = ImLerp(normal_text_color, GImGui->Style.Colors[ImGuiCol_FrameBgActive], ImSaturate(data[i].fading_value));

		bool pressed_button = false;

		if (selected == static_cast<int>(i))
		{
			GImGui->Style.Colors[ImGuiCol_Button] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_ButtonHovered] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_ButtonActive] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_Text] = GImGui->Style.Colors[ImGuiCol_FrameBgActive];
			
			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}
		else if (hovered == static_cast<int>(i))
		{
			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}
		else
		{
			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}

		if (pressed_button)
		{
			changed = selected != static_cast<int>(i);
			selected = static_cast<int>(i);
		}

		if (ImGui::IsItemHovered())
		{
			hovered = static_cast<int>(i);

			data[i].fading_value = min(data[i].fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
		}
		else
		{
			data[i].fading_value = max(data[i].fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);

			if (hovered == static_cast<int>(i))
				hovered = -1;
		}

		if (i < data.size() - 1)
			ImGui::SameLine();
	}

	ImGui::PopFont();

	GImGui->Style = backup_style;

	return changed;
}

bool CMenuGui::TabWeaponList(std::vector<TabWidgetsData>& data, int& selected, int& hovered)
{
	ImGuiStyle backup_style = GImGui->Style;

	GImGui->Style.ItemSpacing = ImVec2(0, 1);
	GImGui->Style.Colors[ImGuiCol_Border] = ImColor();

	ImVec2 button_size = ImVec2(GImGui->CurrentWindow->Size.x / data.size(), 32);
	ImVec4 button_color = GImGui->Style.Colors[ImGuiCol_ChildBg];
	ImVec4 normal_text_color = GImGui->Style.Colors[ImGuiCol_FrameBgHovered];

	bool changed = false;

	ImGui::PushFont(g_pFontList[Verdana_16px]);

	for (size_t i = 0; i < data.size(); i++)
	{
		GImGui->Style.Colors[ImGuiCol_ButtonActive] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_ButtonHovered] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_Button] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_Text] = ImLerp(normal_text_color, GImGui->Style.Colors[ImGuiCol_FrameBgActive], ImSaturate(data[i].fading_value));

		bool pressed_button = false;

		if (selected == static_cast<int>(i))
		{
			GImGui->Style.Colors[ImGuiCol_Button] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_ButtonHovered] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_ButtonActive] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_Text] = GImGui->Style.Colors[ImGuiCol_FrameBgActive];

			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}
		else if (hovered == static_cast<int>(i))
		{
			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}
		else
		{
			ImageInfo image = g_pImageList[i + IconsWpnsOffset];
			pressed_button = ImGui::ImageButtonBySize(image.data, button_size, image.size);
		}

		if (pressed_button)
		{
			changed = selected != static_cast<int>(i);
			selected = static_cast<int>(i);
		}

		if (ImGui::IsItemHovered())
		{
			hovered = static_cast<int>(i);

			data[i].fading_value = min(data[i].fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
		}
		else
		{
			data[i].fading_value = max(data[i].fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);

			if (hovered == static_cast<int>(i))
				hovered = -1;
		}

		if (i < data.size() - 1)
			ImGui::SameLine();
	}

	ImGui::PopFont();

	GImGui->Style = backup_style;

	return changed;
}

void CMenuGui::TabSeparator()
{
	ImGuiStyle backup_style = GImGui->Style;

	GImGui->Style.ItemSpacing = ImVec2(0, 1);
	GImGui->Style.Colors[ImGuiCol_Separator] = backup_style.Colors[ImGuiCol_TabActive];

	ImGui::Separator(4);

	GImGui->Style = backup_style;
}

void CMenuGui::TabShadow(const ImVec2& pos, const ImVec2& size, const ImVec4& color)
{
	ImColor border_col_low_alpha = color;
	ImColor border_col_no_alpha = color;

	border_col_low_alpha.value.w = 0.45f;
	border_col_no_alpha.value.w = 0.00f;

	GImGui->CurrentWindow->DrawList->AddRectFilledMultiColor(pos, size, border_col_low_alpha, border_col_low_alpha, border_col_no_alpha, border_col_no_alpha);
}

void CMenuGui::SetID(const char* str_id)
{
	m_id = str_id;
}

void CMenuGui::SetChildSize(ImVec2& size)
{
	size.x = (GImGui->CurrentWindow->Size.x - WINDOW_PADDING_SAMELINE * 4.f) / 3.f;
	size.y = GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_SAMELINE;
}

void CMenuGui::Text(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextV(fmt, args);
	va_end(args);
}

bool CMenuGui::Checkbox(bool* v, std::string label, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::Checkbox(hashed_string.c_str(), v);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::Combo(int* v, std::string label, const char* items[], int size, std::string message, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::Combo(hashed_string.c_str(), v, items, size, -1, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::MultiCombo(bool* v, std::string label, const char* items[], int size, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::MultiCombo(hashed_string.c_str(), items, v, size);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::Slider(float* v, std::string label, float min, float max, const char* format, float power, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);
	
	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::SliderFloat(hashed_string.c_str(), v, min, max, format, power);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::Slider(int* v, std::string label, int min, int max, const char* format, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::SliderInt(hashed_string.c_str(), v, min, max, format);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

static bool vector_getter(void* data, int id, const char** out_text)
{
	const auto& vector = *static_cast<std::vector<std::string>*>(data);
	*out_text = vector.at(id).c_str();
	return true;
};

bool CMenuGui::ListBox(const char* label, int* current_item, std::vector<std::string>& values, int height_in_items)
{
	ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_FrameBg));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_FrameBg));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetColorU32(ImGuiCol_FrameBg));

	const bool ret = ImGui::ListBox(label, current_item, vector_getter, &values, values.size(), height_in_items);

	ImGui::PopStyleColor(3);

	return ret;
}

void CMenuGui::Separator()
{
	ImGui::Spacing();
	ImGui::Separator(GImGui->CurrentWindow->Size.x - WINDOW_PADDING_SAMELINE);
	ImGui::Spacing();
}

void CMenuGui::Link(std::string url)
{
	static std::vector<LinkData> url_list;

	int id = -1;

	if (!url_list.empty())
	{
		for (size_t i = 0; i < url_list.size(); i++)
		{
			if (url_list[i].url.compare(url))
				continue;

			id = static_cast<int>(i);
			break;
		}
	}

	if (id == -1)
	{
		url_list.push_back(LinkData{ url, false, false });
		id = static_cast<int>(url_list.size()) - 1;
	}

	bool ret = false;

	for (size_t i = 0; i < url_list.size(); i++)
	{
		if (id == static_cast<int>(i))
			continue;

		if (url_list[i].hovered || url_list[i].clicked)
			ret = true;
	}

	ImGui::Text(url.c_str());

	if (ret)
		return;

	url_list[id].hovered = ImGui::IsItemHovered();

	if (url_list[id].hovered)
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

		GImGui->CurrentWindow->Flags |= ImGuiWindowFlags_NoMove;

		if (ImGui::IsItemClicked(0))
			url_list[id].clicked = true;

		if (url_list[id].clicked && ImGui::IsMouseReleased(0))
		{
			ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOW);
			url_list[id].clicked = false;
		}
	}
	else
	{
		GImGui->CurrentWindow->Flags &= ~ImGuiWindowFlags_NoMove;

		url_list[id].clicked = false;
	}
}

#define WAIT_FOR_KEY(wait, assign)\
if (GetAsyncKeyState(wait) & 0x8000) *keynum = assign;

static void WaitForPressKey(int* keynum)
{
	WAIT_FOR_KEY(VK_F1, K_F1);
	WAIT_FOR_KEY(VK_F2, K_F2);
	WAIT_FOR_KEY(VK_F3, K_F3);
	WAIT_FOR_KEY(VK_F4, K_F4);
	WAIT_FOR_KEY(VK_F5, K_F5);
	WAIT_FOR_KEY(VK_F6, K_F6);
	WAIT_FOR_KEY(VK_F7, K_F7);
	WAIT_FOR_KEY(VK_F8, K_F8);
	WAIT_FOR_KEY(VK_F9, K_F9);
	WAIT_FOR_KEY(VK_F10, K_F10);
	WAIT_FOR_KEY(VK_F11, K_F11);
	WAIT_FOR_KEY(VK_F12, K_F12);
	WAIT_FOR_KEY(0x30, K_0);
	WAIT_FOR_KEY(0x31, K_1);
	WAIT_FOR_KEY(0x32, K_2);
	WAIT_FOR_KEY(0x33, K_3);
	WAIT_FOR_KEY(0x34, K_4);
	WAIT_FOR_KEY(0x35, K_5);
	WAIT_FOR_KEY(0x36, K_6);
	WAIT_FOR_KEY(0x37, K_7);
	WAIT_FOR_KEY(0x38, K_8);
	WAIT_FOR_KEY(0x39, K_9);
	WAIT_FOR_KEY(VK_NUMPAD0, K_KP_INS);
	WAIT_FOR_KEY(VK_NUMPAD1, K_KP_END);
	WAIT_FOR_KEY(VK_NUMPAD2, K_KP_DOWNARROW);
	WAIT_FOR_KEY(VK_NUMPAD3, K_KP_PGDN);
	WAIT_FOR_KEY(VK_NUMPAD4, K_KP_LEFTARROW);
	WAIT_FOR_KEY(VK_NUMPAD5, K_KP_5);
	WAIT_FOR_KEY(VK_NUMPAD6, K_KP_RIGHTARROW);
	WAIT_FOR_KEY(VK_NUMPAD7, K_KP_HOME);
	WAIT_FOR_KEY(VK_NUMPAD8, K_KP_UPARROW);
	WAIT_FOR_KEY(VK_NUMPAD9, K_KP_PGUP);
	WAIT_FOR_KEY(VK_SPACE, K_SPACE);
	WAIT_FOR_KEY(VK_RETURN, K_ENTER);
	WAIT_FOR_KEY(VK_TAB, K_TAB);
	WAIT_FOR_KEY(VK_UP, K_UPARROW);
	WAIT_FOR_KEY(VK_DOWN, K_DOWNARROW);
	WAIT_FOR_KEY(VK_LEFT, K_LEFTARROW);
	WAIT_FOR_KEY(VK_RIGHT, K_RIGHTARROW);
	WAIT_FOR_KEY(18, K_ALT);
	WAIT_FOR_KEY(VK_CONTROL, K_CTRL);
	WAIT_FOR_KEY(VK_SHIFT, K_SHIFT);
	WAIT_FOR_KEY(34, K_PGDN);
	WAIT_FOR_KEY(33, K_PGUP);
	WAIT_FOR_KEY(VK_HOME, K_HOME);
	WAIT_FOR_KEY(VK_END, K_END);
	WAIT_FOR_KEY(0x41, K_A);
	WAIT_FOR_KEY(0x42, K_B);
	WAIT_FOR_KEY(0x43, K_C); 
	WAIT_FOR_KEY(0x44, K_D);
	WAIT_FOR_KEY(0x45, K_E);
	WAIT_FOR_KEY(0x46, K_F);
	WAIT_FOR_KEY(0x47, K_G);
	WAIT_FOR_KEY(0x48, K_H);
	WAIT_FOR_KEY(0x49, K_I);
	WAIT_FOR_KEY(0x4A, K_J);
	WAIT_FOR_KEY(0x4B, K_K);
	WAIT_FOR_KEY(0x4C, K_L);
	WAIT_FOR_KEY(0x4D, K_M);
	WAIT_FOR_KEY(0x4E, K_N);
	WAIT_FOR_KEY(0x4F, K_O);
	WAIT_FOR_KEY(0x50, K_P);
	WAIT_FOR_KEY(0x51, K_Q);
	WAIT_FOR_KEY(0x52, K_R);
	WAIT_FOR_KEY(0x53, K_S);
	WAIT_FOR_KEY(0x54, K_T);
	WAIT_FOR_KEY(0x55, K_U);
	WAIT_FOR_KEY(0x56, K_V);
	WAIT_FOR_KEY(0x57, K_W);
	WAIT_FOR_KEY(0x58, K_X);
	WAIT_FOR_KEY(0x59, K_Y);
	WAIT_FOR_KEY(0x5A, K_Z);
	WAIT_FOR_KEY(0xDB, K_BL);
	WAIT_FOR_KEY(0xDD, K_BR);
	WAIT_FOR_KEY(0xBC, K_COMMA);
	WAIT_FOR_KEY(0xBE, K_POINT);
	WAIT_FOR_KEY(VK_LBUTTON, K_MOUSE1);
	WAIT_FOR_KEY(VK_RBUTTON, K_MOUSE2);
	WAIT_FOR_KEY(VK_MBUTTON, K_MOUSE3);
	WAIT_FOR_KEY(VK_XBUTTON1, K_MOUSE4);
	WAIT_FOR_KEY(VK_XBUTTON2, K_MOUSE5);
	//WAIT_FOR_KEY(VK_ESCAPE, -1);
	//WAIT_FOR_KEY(VK_BACK, -1);
}

bool CMenuGui::Key(void* v, std::string label, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::KeyButton(hashed_string.c_str(), v, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::KeyToggle(void* v, std::string label, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::KeyButtonToggle(hashed_string.c_str(), v, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::KeyPress(void* v, std::string label, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::KeyButtonPress(hashed_string.c_str(), v, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::ColorEdit(float* v, std::string label)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::ColorEdit(hashed_string.c_str(), v);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

void CMenuGui::BeginCheckChangeValue()
{
	if (!m_check_change_value)
		m_check_change_value = true;
}

void CMenuGui::EndCheckChangeValue()
{
	if (m_check_change_value)
	{
		m_check_change_value = false;
		m_changed_value = false;
	}
}

bool CMenuGui::IsChangedValue()
{
	return m_changed_value;
}

float& CMenuGui::FindWidgetsFadingValue(std::string label)
{
	auto it = m_widgets_list.find(label);

	if (it == m_widgets_list.end())
	{
		m_widgets_list.insert({ label, 0.00f });
		it = m_widgets_list.find(label);
	}

	return it->second;
}

void CMenuGui::UpdateWidgetsFadingValue(float& fading_value)
{
	if (ImGui::IsItemHovered())
		fading_value = min(fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
	else
		fading_value = max(fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);
}