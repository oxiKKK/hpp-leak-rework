#include "main.h"

static auto vector_getter = [](void* data, int idx, const char** out_text)
{
	if (out_text == nullptr)
		return false;

	const auto& vector = *static_cast<std::vector<std::string>*>(data);
	*out_text = vector.at(idx).c_str();

	return true;
};

namespace ImGui
{
	bool bKeyAssign = false;

	static void Spacing(size_t count)
	{
		while (--count + 1)
			Spacing();
	}

	static bool ListBoxArray(const char* label, uint* idx, std::vector<std::string>& values, int height_in_items = -1)
	{
		return ListBox(label, reinterpret_cast<int*>(idx), vector_getter, &values, values.size(), height_in_items);
	}

	static bool TabLabels(std::vector<std::string>& labels, int& idx)
	{
		if (!labels.size())
			return false;

		ImGuiStyle& style = GetStyle();

		const auto ItemSpacing = style.ItemSpacing;
		const auto FrameRounding = style.FrameRounding;
		const auto ColorButton = style.Colors[ImGuiCol_Button];
		const auto ColorButtonActive = style.Colors[ImGuiCol_ButtonActive];
		const auto ColorButtonHovered = style.Colors[ImGuiCol_ButtonHovered];
		const auto ColorBorder = style.Colors[ImGuiCol_Border];
		const auto ColorScrollbarBg = style.Colors[ImGuiCol_ScrollbarBg];

		style.ItemSpacing = ImVec2(0, 1);
		style.FrameRounding = 0.f;
		style.Colors[ImGuiCol_ButtonHovered] = ColorButtonHovered;
		style.Colors[ImGuiCol_ButtonActive] = ColorButtonActive;
		style.Colors[ImGuiCol_Border] = ImColor();

		bool bPressed = false;

		for (size_t i = 0; i < labels.size(); ++i)
		{
			style.Colors[ImGuiCol_Button] = i != idx ? ColorScrollbarBg : ColorButton;

			if (i != 0)
				SameLine();

			if ((bPressed = Button(labels.at(i).c_str(), ImVec2(GetWindowSize().x / labels.size(), 18))) == true)
				idx = i;
		}

		Spacing(2);

		style.Colors[ImGuiCol_Button] = ColorButton;
		style.Colors[ImGuiCol_ButtonActive] = ColorButtonActive;
		style.Colors[ImGuiCol_ButtonHovered] = ColorButtonHovered;
		style.Colors[ImGuiCol_Border] = ColorBorder;
		style.ItemSpacing = ItemSpacing;
		style.FrameRounding = FrameRounding;

		return bPressed;
	}

	static void KeyAssignment(const char* label, int* key, float item_offset = 0.f)
	{
		if (key == nullptr)
			return;

		const ImFont* pFont = GetIO().Fonts->Fonts[0];

		if (pFont == nullptr)
			return;

		const std::string c_sKeyString = g_Utils.KeyToString(*key);

		SameLine((item_offset - 12.f) * 2.f - pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, c_sKeyString.c_str()).x - 9.f);
		PushStyleColor(ImGuiCol_Border, ImVec4());

		if (Button(std::string(c_sKeyString + label).c_str()))
		{
			OpenPopup("##KeyAssignment");
			bKeyAssign = true;
		}

		PopStyleColor();
		SetNextWindowSize(ImVec2(125, 40));

		if (BeginPopupModal("##KeyAssignment", static_cast<bool*>(nullptr),
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			Spacing(4);

			const char* pcszMessage = "Press any key";
			SameLine(((GetWindowWidth()) - pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, pcszMessage).x) * 0.5f);
			Text(pcszMessage);

			int nKeyASCII = 0;
			g_Utils.ConvertToASCII(&nKeyASCII);

			if (nKeyASCII || !bKeyAssign)
			{
				*key = nKeyASCII;
				CloseCurrentPopup();
			}

			EndPopup();
		}
	}

	static std::string ControlBase(const char* tab_name, const char* label, float item_offset = 0.f, const char* tooltip_text = nullptr)
	{
		std::string sLabel;

		ImGui::Text(label);

		if (tooltip_text && ImGui::IsItemHovered())
		{
			const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

			if (pFont == nullptr)
				return sLabel;

			const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, tooltip_text);
			const auto offset_padding = 8.f;

			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
			ImGui::SetNextWindowSize(ImVec2(text_size.x + offset_padding, text_size.y + offset_padding));
			ImGui::BeginTooltip();
			ImGui::Spacing(2);
			ImGui::SameLine(offset_padding * 0.5f);
			ImGui::Text(tooltip_text);
			ImGui::EndTooltip();
			ImGui::PopStyleColor();
		}

		ImGui::SameLine(item_offset);

		sLabel.append(tab_name);
		sLabel.append("_");
		sLabel.append(label);

		return sLabel;
	}

	static bool Checkbox(const char* tab_name, const char* label, bool* v, float item_offset = 0.f, const char* tooltip_text = nullptr)
	{
		return Checkbox(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v);
	}

	static bool Combo(const char* tab_name, const char* label, int* v, const char* const items[],
		int count, float item_offset = 0.f, const char* tooltip_text = nullptr)
	{
		return Combo(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v, items, count, -1);
	}

	static bool SliderFloat(const char* tab_name, const char* label, float* v, float v_min, float v_max,
		const char* format = "%.3f", float power = 1.f, float item_offset = 0.f, const char* tooltip_text = nullptr)
	{
		return SliderFloat(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v, v_min, v_max, format, power);
	}

	static bool SliderInt(const char* tab_name, const char* label, int* v, int v_min, int v_max,
		const char* format = "%.3f", float item_offset = 0.f, const char* tooltip_text = nullptr)
	{
		return SliderInt(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v, v_min, v_max, format);
	}

	static bool ColorEdit3(const char* tab_name, const char* label, float* col, ImGuiColorEditFlags flags = 0, float item_offset = 0.f)
	{
		return ColorEdit3(ControlBase(tab_name, label, item_offset, nullptr).c_str(), col, flags);
	}

	static bool ColorEdit4(const char* tab_name, const char* label, float* col, ImGuiColorEditFlags flags = 0, float item_offset = 0.f)
	{
		return ColorEdit4(ControlBase(tab_name, label, item_offset, nullptr).c_str(), col, flags);
	}

	static void ColorButton3(const char* label, float* col, float item_offset = 0.f)
	{
		const ImGuiContext& g = *GImGui;
		SameLine(item_offset);

		if (ColorButton(label, ImColor(col), 0, ImVec2(32, 0)))
		{
			const ImGuiWindow* window = GetCurrentWindow();
			OpenPopup(label);
			SetNextWindowPos(ImVec2(window->DC.LastItemRect.GetBL().x + -1.f,
				window->DC.LastItemRect.GetBL().y + g.Style.ItemSpacing.y));
		}

		if (BeginPopup(label))
		{
			ColorPicker3("##picker", col, ImGuiColorEditFlags_PickerHueBar);
			EndPopup();
		}
	}

	static void ColorButton4(const char* label, float* col, float item_offset = 0.f)
	{
		ImGuiContext& g = *GImGui;
		SameLine(item_offset);

		if (ColorButton(label, ImColor(col), ImGuiColorEditFlags_AlphaPreview, ImVec2(32, 0)))
		{
			const ImGuiWindow* window = GetCurrentWindow();
			g.ColorPickerRef = ImColor(col);
			OpenPopup(label);
			SetNextWindowPos(ImVec2(window->DC.LastItemRect.GetBL().x + -1.f,
				window->DC.LastItemRect.GetBL().y + g.Style.ItemSpacing.y));
		}

		if (BeginPopup(label))
		{
			ColorPicker4("##picker", col, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar |
				ImGuiColorEditFlags_AlphaPreviewHalf, &g.ColorPickerRef.x);
			EndPopup();
		}
	}
}

std::vector<std::string> g_sSettingsList;
unsigned int g_iSettingsSelect = 0;

typedef void(*LPSEARCHFUNC)(LPCTSTR a_lpcszFileName);
static void SearchFiles(LPCTSTR a_lpcszFileName, LPSEARCHFUNC a_lpSearchFunc)
{
	if (a_lpSearchFunc == nullptr)
		return;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFileFind = FindFirstFileA(a_lpcszFileName, &FindFileData);

	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (!strstr(&FindFileData.cFileName[0], "."))
			{
				char szBuffer[MAX_PATH] = { 0 };
				LPSTR lpszPart = nullptr;

				GetFullPathNameA(a_lpcszFileName, sizeof(szBuffer), &szBuffer[0], &lpszPart);
				strcpy(lpszPart, &FindFileData.cFileName[0]);
				a_lpSearchFunc(&FindFileData.cFileName[0]);
			}

		} while (FindNextFileA(hFileFind, &FindFileData));

		FindClose(hFileFind);
	}
}

static void ReadSettings(const char* a_pcszFileName)
{
	if (!strstr(a_pcszFileName, "*.ini"))
		g_sSettingsList.push_back(a_pcszFileName);
}

inline void RefreshSettings()
{
	g_sSettingsList.clear();

	std::string sPath = g_Globals.m_sSettingsPath + "*";

	SearchFiles(sPath.c_str(), ReadSettings);
}

void ReadFromFile(const char* a_pcszFileName, char* a_pszBuffer)
{
	std::ifstream input(a_pcszFileName);
	std::size_t nSize = 0;

	char ch;
	while ((ch = gsl::narrow_cast<char>(input.get())) != -1)
		++nSize;

	input.close();

	FILE* pFile = fopen(a_pcszFileName, "r");

	if (pFile)
	{
		fread(a_pszBuffer, nSize, 1, pFile);
		fclose(pFile);
	}
}

CWindow::CWindow() noexcept :
	_bIsOpened(false),
	_pFunction(nullptr)
{
}

bool CWindow::IsOpened() noexcept
{
	return _bIsOpened;
}

void CWindow::PushWindow(void(*a_pFunction)()) noexcept
{
	_pFunction = a_pFunction;
}

void CWindow::Show() noexcept
{
	_bIsOpened = true;
}

void CWindow::Hide() noexcept
{
	_bIsOpened = false;
}

void CWindow::Toggle() noexcept
{
	_bIsOpened = !_bIsOpened;
}

void CWindow::Draw() noexcept
{
	if (_bIsOpened)
		_pFunction();
}

void CDrawGUI::Frame()
{
	static auto bCursorState = false;
	static auto nFixCenterCursor = 0;

	if ((ImGui::GetIO().MouseDrawCursor = _bIsDrawing) == true)
	{
		if (!bCursorState)
		{
			ShowCursor(FALSE);
			bCursorState = true;
		}

		g_pISurface->SetCursor(dc_arrow);
		g_pISurface->UnlockCursor();

		if (!g_pGameUI->IsGameUIActive() && nFixCenterCursor)
		{
			g_pISurface->SurfaceSetCursorPos(g_Screen.iWidth / 2, g_Screen.iHeight / 2);
			--nFixCenterCursor;
		}
	}
	else
	{
		if (g_pIRunGameEngine->IsInGame() && !g_pGameUI->IsGameUIActive())
			nFixCenterCursor = 2;	// 2 frames min

		if (bCursorState)
		{
			if (!g_pGameUI->IsGameUIActive())
			{
				g_pISurface->SetCursor(dc_none);
				g_pISurface->LockCursor();
			}

			ShowCursor(TRUE);
			bCursorState = false;
		}
	}
}

std::deque<CWindow>& CDrawGUI::GetWindow() noexcept
{
	return _Window;
}

void CDrawGUI::Fadeout()
{
	static auto frames = 0.f;

	const auto width = static_cast<float>(g_Screen.iWidth);
	const auto height = static_cast<float>(g_Screen.iHeight);
	const auto max_alpha = 0.45f;
	const auto fade_duration = 20.f;

	if (_bIsFadeout)
	{
		frames += 1.f * (100.f / ImGui::GetIO().Framerate);

		if (frames <= fade_duration)
		{
			const auto f1 = _bIsDrawing ? 0.f : max_alpha;
			const auto f3 = _bIsDrawing ? max_alpha : 0.f;

			g_DrawList.AddRectFilled(ImVec2(), ImVec2(width, height), ImVec4(0, 0, 0,
				g_Utils.fInterp(1.f, frames, fade_duration, f1, f3)));
		}
		else
		{
			g_DrawList.AddRectFilled(ImVec2(), ImVec2(width, height),
				ImVec4(0, 0, 0, _bIsDrawing ? max_alpha : 0.f));

			_bIsFadeout = false;
			frames = 0.f;
		}
	}
	else
	{
		if (_bIsDrawing)
			g_DrawList.AddRectFilled(ImVec2(), ImVec2(width, height), ImVec4(0, 0, 0, max_alpha));
	}
}

void CDrawGUI::RunFadeout() noexcept
{
	_bIsFadeout = true;
}

void CDrawGUI::Render()
{
	if (!_bIsDrawing)
		return;

	for (auto& window : _Window)
		window.Draw();
}

bool CDrawGUI::IsDrawing() noexcept
{
	return _bIsDrawing;
}

void CDrawGUI::Draw() noexcept
{
	_bIsDrawing = true;
}

void CDrawGUI::Hide() noexcept
{
	_bIsDrawing = false;
}

void CDrawGUI::Toggle() noexcept
{
	_bIsDrawing = !_bIsDrawing;
}

CDrawGUI g_DrawGUI;

CDrawGUI::CDrawGUI() noexcept :
	_bIsFadeout(false),
	_bIsDrawing(false)
{
}

static bool g_bIsUpdateFile = true;

DWORD LoadSettingsThread(LPVOID)
{
	g_Settings.Load(g_sSettingsList.at(g_iSettingsSelect).c_str());
	g_bIsUpdateFile = true;
	return null;
}

DWORD SaveSettingsThread(LPVOID)
{
	g_Settings.Save(g_sSettingsList.at(g_iSettingsSelect).c_str());
	g_bIsUpdateFile = true;
	return null;
}

DWORD RestoreSettingsAsDefaultThread(LPVOID)
{
	g_Settings.Restore(g_sSettingsList.at(g_iSettingsSelect).c_str(), true);
	g_bIsUpdateFile = true;
	return null;
}

DWORD RestoreSettingsWithSavingThread(LPVOID)
{
	g_Settings.Restore(g_sSettingsList.at(g_iSettingsSelect).c_str());
	g_bIsUpdateFile = true;
	return null;
}

enum WindowTabs
{
	WindowTab_RageBot,
	WindowTab_LegitBot,
	WindowTab_Visuals,
	WindowTab_Kreedz,
	WindowTab_Misc,
	WindowTab_GUI,
	WindowTab_Settings
};

void CheatMenu()
{
	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	const auto window_size = ImVec2(800, 550);
	const auto flags = ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_ShowBorders;

	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(window_size);

	if (ImGui::Begin("##Menu", static_cast<bool*>(nullptr), flags))
	{
		std::vector<std::string> sTabNames;

		sTabNames.push_back("RageBot");
		sTabNames.push_back("LegitBot");
		sTabNames.push_back("Visuals");
		sTabNames.push_back("Kreedz");
		sTabNames.push_back("Misc");
		sTabNames.push_back("GUI");
		sTabNames.push_back("Settings");

		const auto offset_x = 12.f;
		const auto colums_count = 2;
		const auto child_width = (window_size.x - offset_x * (colums_count + 1)) / colums_count;
		const auto next_child_pos_x = (window_size.x + offset_x) / colums_count;
		const auto item_space = child_width * 0.5f;
		const auto item_width = item_space - offset_x * 2.f;
		const auto title = "Hpp Hack for Counter-Strike 1.6";
		const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, title);

		static int iTabSelected = WindowTab_RageBot;

		ImGui::Spacing(2);
		ImGui::SameLine((window_size.x - text_size.x) * 0.5f);
		ImGui::PushFont(Verdana_16px);
		ImGui::Text(title);
		ImGui::PopFont();
		ImGui::Separator();
		ImGui::TabLabels(sTabNames, iTabSelected);
		ImGui::Separator();
		ImGui::BeginChild("##Controls");
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1));

		switch (iTabSelected)
		{
		case WindowTab_RageBot:
			ImGui::Spacing(3);
			ImGui::SameLine(offset_x);
			ImGui::Checkbox("##RageBot", "Active", &cvar.ragebot_active, 60);
			ImGui::Spacing(4);
			ImGui::SameLine(offset_x);
			ImGui::BeginGroup();

			if (cvar.ragebot_active)
				cvar.legitbot_active = false;

			// block: aimbot
			{
				const char* str_id = "##RageBot(Aimbot)";
				const char* const multipoint_combo[] = { "Off", "Low", "Medium", "High" };
				const char* const hitbox_combo[] = { "Head", "Neck", "Chest", "Stomach", "All hitboxes", "Vital hitboxes" };

				ImGui::Text("Aimbot:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##RageBot_Aimbot", ImVec2(child_width, 260), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Enabled", &cvar.ragebot_aim_enabled, item_space, "Global enable/disable ragebot.");
				ImGui::Checkbox(str_id, "Friendly fire", &cvar.ragebot_friendly_fire, item_space, "Fire in your teammates.");
				ImGui::Combo(str_id, "Hitbox", &cvar.ragebot_hitbox, &hitbox_combo[0], IM_ARRAYSIZE(hitbox_combo), item_space, "Hitbox or group of hitboxes for aiming.");
				ImGui::Combo(str_id, "Multi-point", &cvar.ragebot_multipoint_type, &multipoint_combo[0], IM_ARRAYSIZE(multipoint_combo), item_space);
				ImGui::SliderFloat(str_id, "Multi-point scale", &cvar.ragebot_multipoint_scale, 1, 100, "%.f%%", 1, item_space);
				ImGui::Checkbox(str_id, "Automatic penetration", &cvar.ragebot_automatic_penetration, item_space, "Automatic penetration through walls.");
				ImGui::Checkbox(str_id, "Automatic scope", &cvar.ragebot_automatic_scope, item_space, "Automatic opening scope on snipers weapons.");
				ImGui::Checkbox(str_id, "Silent aim", &cvar.ragebot_silent_aim, item_space, "Silent aim (hidden only for you).");
				ImGui::Checkbox(str_id, "Perfect silent aim", &cvar.ragebot_perfect_silent_aim, item_space, "Perfect silent aim (hidden for all).");
				ImGui::SliderFloat(str_id, "Backtrack (ping spike)", &cvar.ragebot_backtrack, 0, 1500, "%.0f ms", 1, item_space, "Returns the target for several milliseconds.\nIf you can not move, decrease the value.");

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::Spacing();

			// block: fake lag
			{
				const char* str_id = "##RageBot(Fake lag)";
				const char* const triggers_combo[] = { "On land", "In air", "On land + In air" };
				const char* const type_combo[] = { "Dynamic", "Maximum", "Flucture", "Break lag compensation" };

				ImGui::Text("Fake lag:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##RageBot_Fakelag", ImVec2(child_width, 140), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Enabled", &cvar.fakelag_enabled, item_space, "Global enable/disable fake lag.");
				ImGui::Combo(str_id, "Type", &cvar.fakelag_type, &type_combo[0], IM_ARRAYSIZE(type_combo), item_space, "Type of fake lagging.");
				ImGui::Combo(str_id, "Triggers", &cvar.fakelag_triggers, &triggers_combo[0], IM_ARRAYSIZE(triggers_combo), item_space);	// need desk
				ImGui::SliderFloat(str_id, "Variance", &cvar.fakelag_variance, 1, 100, "%.f%%", 1, item_space);			// need desc
				ImGui::SliderFloat(str_id, "Choke limit", &cvar.fakelag_choke_limit, 1, 64, "%.f", 1, item_space);			// need desc
				ImGui::Checkbox(str_id, "Fake lag while shooting", &cvar.fakelag_while_shooting, item_space);

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			ImGui::SameLine(next_child_pos_x);
			ImGui::BeginGroup();

			// block: other
			{
				const char* str_id = "##RageBot(Other)";
				const char* const nospread_combo[] = { "Off", "Default", "Perfect Pitch / Yaw", "Transpose Pitch / Yaw / Roll", "Perfect Pitch / Yaw / Roll" };

				ImGui::Text("Other:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##RageBot_Other", ImVec2(child_width, 140), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Remove recoil", &cvar.ragebot_remove_recoil, item_space, "Disables the recoil of weapons.");
				ImGui::Combo(str_id, "Remove spread", &cvar.ragebot_remove_spread, &nospread_combo[0], IM_ARRAYSIZE(nospread_combo), item_space, "Disables the spread of weapons.");

				ImGui::TextColored(ImVec4(1, 1, 0, 0.8f), "Anti-aim resolver");
				//ImGui::SameLine(child_width / 2);
				//ImGui::Checkbox("##RageBot(Anti-aim resolver)", &cvar.ragebot_anti_aim_resolver);

				ImGui::TextColored(ImVec4(1, 1, 0, 0.8f), "Yaw / Roll aiming");
				//ImGui::SameLine(child_width / 2);
				//ImGui::Checkbox("##RageBot(Yaw / Roll aiming)", &cvar.ragebot_anti_aim_resolver);

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::Spacing();

			// block: anti-aims
			{
				const char* str_id = "##RageBot(Anti-aimbot)";
				const char* const pitch_combo[] = { "Off", "Fakedown", "Down", "Up", "Jitter", "Minimal", "Random" };
				const char* const yaw_combo[] = { "Off", "180", "180 Z", "180 Jitter", "Jitter", "Spin", "Sideways", "Random", "Static" };
				const char* const yaw_while_running_combo[] = { "Off", "180", "180 Jitter", "Jitter", "Spin", "Sideways", "Random", "Static" };
				const char* const fake_yaw_combo[] = { "Off", "180", "Sideways", "Local view", "Random", "Static" };
				const char* const edge_triggers_combo[] = { "Standing", "On land", "Always" };

				ImGui::Text("Anti-aimbot:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##RageBot_Anti-aimbot", ImVec2(child_width, 260), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Enabled", &cvar.antiaim_enabled, item_space, "Global enable/disable anti-aimbot.");
				ImGui::Combo(str_id, "Pitch", &cvar.antiaim_pitch, &pitch_combo[0], IM_ARRAYSIZE(pitch_combo), item_space);	// need desk
				ImGui::Combo(str_id, "Yaw", &cvar.antiaim_yaw, &yaw_combo[0], IM_ARRAYSIZE(yaw_combo), item_space);	// need desk
				ImGui::SliderFloat(str_id, "Yaw static", &cvar.antiaim_yaw_static, -180, 180, u8"%.f°", 1.4f, item_space);			// need desc
				ImGui::SliderInt(str_id, "Spin speed", &cvar.antiaim_yaw_spin, 1, 128, "%.f", item_space);			// need desc
				ImGui::Combo(str_id, "Yaw while running", &cvar.antiaim_yaw_while_running, &yaw_while_running_combo[0], IM_ARRAYSIZE(yaw_while_running_combo), item_space);	// need desk
				ImGui::Combo(str_id, "Fake yaw", &cvar.antiaim_fake_yaw, &fake_yaw_combo[0], IM_ARRAYSIZE(fake_yaw_combo), item_space);	// need desk
				ImGui::SliderFloat(str_id, "Fake yaw static", &cvar.antiaim_fake_yaw_static, -180, 180, u8"%.f°", 1.4f, item_space);			// need desc
				ImGui::Checkbox(str_id, "Edge", &cvar.antiaim_edge, item_space);		// need desc
				ImGui::SliderFloat(str_id, "Edge distance", &cvar.antiaim_edge_distance, 32, 256, "%.f", 1, item_space);			// need desc
				ImGui::SliderFloat(str_id, "Edge yaw offset", &cvar.antiaim_edge_offset, -180, 180, u8"%.f°", 1.4f, item_space);			// need desc
				ImGui::Combo(str_id, "Edge triggers", &cvar.antiaim_edge_triggers, &edge_triggers_combo[0], IM_ARRAYSIZE(edge_triggers_combo), item_space);	// need desk
				ImGui::Checkbox(str_id, "On knife", &cvar.antiaim_on_knife, item_space);		// need desc
				ImGui::SliderFloat(str_id, "Choke limit", &cvar.antiaim_choke_limit, 0, 64, "%.f", 1, item_space, "Choke limit (if zero no faking)");
				ImGui::SliderFloat(str_id, "Roll angle", &cvar.antiaim_roll, -180, 180, u8"%.f°", 1.4f, item_space, "Roll angle (only special servers)");

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			break;

		case WindowTab_LegitBot:
		{
			ImGui::Spacing(3);
			ImGui::SameLine(offset_x);
			ImGui::Checkbox("##LegitBot", "Active", &cvar.legitbot_active, 60);
			ImGui::SameLine(390);

			if (cvar.legitbot_active)
				cvar.ragebot_active = false;

			enum { Pistol, SubMachineGun, Rifle, Shotgun, Sniper };
			const char* const weapon_groups_combo[] = { "Pistol", "SubMachineGun", "Rifle", "Shotgun", "Sniper" };

			static std::array<int, IM_ARRAYSIZE(weapon_groups_combo)> current_weapon;
			static int current_weapon_group = 0;

			ImGui::PushItemWidth(150);
			ImGui::Combo("##LegitBot", "Group", &current_weapon_group, &weapon_groups_combo[0], IM_ARRAYSIZE(weapon_groups_combo));
			ImGui::SameLine();
			ImGui::Text("Weapon");
			ImGui::SameLine();

			if (current_weapon_group == Pistol)
			{
				const char* const weapons_combo[] = { "glock18", "usp", "p228", "deagle", "elite", "fiveseven" };
				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
			}
			else if (current_weapon_group == SubMachineGun)
			{
				const char* const weapons_combo[] = { "m249", "tmp", "p90", "mp5n", "mac10", "ump45" };
				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
			}
			else if (current_weapon_group == Rifle)
			{
				const char* const weapons_combo[] = { "m4a1", "galil", "famas", "aug", "ak47", "sg552" };
				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
			}
			else if (current_weapon_group == Shotgun)
			{
				const char* const weapons_combo[] = { "xm1014", "m3" };
				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
			}
			else if (current_weapon_group == Sniper)
			{
				const char* const weapons_combo[] = { "awp", "scout", "g3sg1", "sg550" };
				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
			}

			ImGui::PopItemWidth();
			ImGui::Spacing(4);
			ImGui::SameLine(offset_x);
			ImGui::BeginGroup();

			int iWeaponID = 0;

			if (current_weapon_group == Pistol)
			{
				switch (current_weapon.at(current_weapon_group))
				{
				case 0: iWeaponID = WEAPON_GLOCK18; break;
				case 1: iWeaponID = WEAPON_USP; break;
				case 2: iWeaponID = WEAPON_P228; break;
				case 3: iWeaponID = WEAPON_DEAGLE; break;
				case 4: iWeaponID = WEAPON_ELITE; break;
				case 5: iWeaponID = WEAPON_FIVESEVEN;
				}
			}
			else if (current_weapon_group == SubMachineGun)
			{
				switch (current_weapon.at(current_weapon_group))
				{
				case 0: iWeaponID = WEAPON_M249; break;
				case 1: iWeaponID = WEAPON_TMP; break;
				case 2: iWeaponID = WEAPON_P90; break;
				case 3: iWeaponID = WEAPON_MP5N; break;
				case 4: iWeaponID = WEAPON_MAC10; break;
				case 5: iWeaponID = WEAPON_UMP45;
				}
			}
			else if (current_weapon_group == Rifle)
			{
				switch (current_weapon.at(current_weapon_group))
				{
				case 0: iWeaponID = WEAPON_M4A1; break;
				case 1: iWeaponID = WEAPON_GALIL; break;
				case 2: iWeaponID = WEAPON_FAMAS; break;
				case 3: iWeaponID = WEAPON_AUG; break;
				case 4: iWeaponID = WEAPON_AK47; break;
				case 5: iWeaponID = WEAPON_SG552;
				}
			}
			else if (current_weapon_group == Shotgun)
			{
				switch (current_weapon.at(current_weapon_group))
				{
				case 0: iWeaponID = WEAPON_XM1014; break;
				case 1: iWeaponID = WEAPON_M3;
				}
			}
			else if (current_weapon_group == Sniper)
			{
				switch (current_weapon.at(current_weapon_group))
				{
				case 0: iWeaponID = WEAPON_AWP; break;
				case 1: iWeaponID = WEAPON_SCOUT; break;
				case 2: iWeaponID = WEAPON_G3SG1; break;
				case 3: iWeaponID = WEAPON_SG550;
				}
			}

			// block: Aimbot
			{
				const char* str_id = "##LegitBot(Aimbot)";
				const char* const aim_accuracy_combo[] = { "None", "Bound box", "Recoil", "Recoil + Spread" };
				const char* const psilent_type_combo[] = { "Manual", "Automatic fire" };
				const char* const psilent_triggers_combo[] = { "Standing", "On land", "In air", "Always" };

				ImGui::Text("Aimbot:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##LegitBot_Aimbot", ImVec2(child_width, 425), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Enabled", &cvar.legitbot.at(iWeaponID).aim, item_space, "Global enable/disable legitbot.");
				ImGui::Checkbox(str_id, "Head", &cvar.legitbot.at(iWeaponID).aim_head, item_space);		// need desk
				ImGui::Checkbox(str_id, "Chest", &cvar.legitbot.at(iWeaponID).aim_chest, item_space);		// need desk
				ImGui::Checkbox(str_id, "Stomach", &cvar.legitbot.at(iWeaponID).aim_stomach, item_space);		// need desk
				ImGui::Checkbox(str_id, "All hitboxes", &cvar.legitbot.at(iWeaponID).aim_all, item_space);		// need desk
				ImGui::SliderInt(str_id, "Reaction time", &cvar.legitbot.at(iWeaponID).aim_reaction_time, 0, 1000, "%.0f ms", item_space, "Delay before aiming to the next player (in milliseconds) after killing the previous one.");
				ImGui::SliderFloat(str_id, "Maximum FOV", &cvar.legitbot.at(iWeaponID).aim_fov, 0, 180, u8"%.1f°", 2, item_space, "Maximal field of view of the aimbot.");
				ImGui::SliderFloat(str_id, "Perfect silent aim max. angle", &cvar.legitbot.at(iWeaponID).aim_psilent_angle, 0, 1, u8"%.1f°", 1, item_space);	// need desc
				ImGui::Combo(str_id, "Perfect silent type", &cvar.legitbot.at(iWeaponID).aim_psilent_type, &psilent_type_combo[0], IM_ARRAYSIZE(psilent_type_combo), item_space);	// need desk
				ImGui::Combo(str_id, "Perfect silent triggers", &cvar.legitbot.at(iWeaponID).aim_psilent_triggers, &psilent_triggers_combo[0], IM_ARRAYSIZE(psilent_triggers_combo), item_space);	// need desk
				ImGui::SliderFloat(str_id, "Smooth (auto aim)", &cvar.legitbot.at(iWeaponID).aim_smooth, 0, 100, "%.2f", 2, item_space);		// need desc
				ImGui::SliderFloat(str_id, "Smooth in attack", &cvar.legitbot.at(iWeaponID).aim_smooth_in_attack, 0, 100, "%.2f", 2, item_space);		// need desc
				ImGui::SliderFloat(str_id, "Smooth scale - FOV", &cvar.legitbot.at(iWeaponID).aim_smooth_scale_fov, 0, 100, "%.0f%%", 1, item_space);		// need desc
				ImGui::SliderInt(str_id, "Recoil compensation pitch", &cvar.legitbot.at(iWeaponID).aim_recoil_compensation_pitch, 0, 100, "%.0f%%", item_space);
				ImGui::SliderInt(str_id, "Recoil compensation yaw", &cvar.legitbot.at(iWeaponID).aim_recoil_compensation_yaw, 0, 100, "%.0f%%", item_space);
				ImGui::SliderFloat(str_id, "Recoil comp. maximum FOV", &cvar.legitbot.at(iWeaponID).aim_recoil_compensation_fov, 0, 180, u8"%.1f°", 2, item_space, "Maximal field of view of the aimbot when working recoil");
				ImGui::Combo(str_id, "Accuracy boost", &cvar.legitbot.at(iWeaponID).aim_accuracy, &aim_accuracy_combo[0], IM_ARRAYSIZE(aim_accuracy_combo), item_space);	// need desk

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			ImGui::SameLine(next_child_pos_x);
			ImGui::BeginGroup();

			// block: triggerbot
			{
				const char* str_id = "##LegitBot(Triggerbot)";
				const char* const trigger_accuracy_combo[] = { "None", "Recoil", "Recoil + Spread" };

				ImGui::Text("Triggerbot:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##LegitBot_Triggerbot", ImVec2(child_width, 240), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Enabled", &cvar.legitbot.at(iWeaponID).trigger, item_space, "Global enable/disable triggerbot.");
				ImGui::KeyAssignment("##TriggerKey", &cvar.legitbot_trigger_key, item_space);
				ImGui::Checkbox(str_id, "Head", &cvar.legitbot.at(iWeaponID).trigger_head, item_space);		// need desk
				ImGui::Checkbox(str_id, "Chest", &cvar.legitbot.at(iWeaponID).trigger_chest, item_space);	// need desk
				ImGui::Checkbox(str_id, "Stomach", &cvar.legitbot.at(iWeaponID).trigger_stomach, item_space);	// need desk
				ImGui::Checkbox(str_id, "All hitboxes", &cvar.legitbot.at(iWeaponID).trigger_all, item_space);	// need desk
				ImGui::Checkbox(str_id, "Automatic penetration", &cvar.legitbot.at(iWeaponID).trigger_penetration, item_space, "Automatic penetration through walls.");
				ImGui::Combo(str_id, "Accuracy boost", &cvar.legitbot.at(iWeaponID).trigger_accuracy, &trigger_accuracy_combo[0], IM_ARRAYSIZE(trigger_accuracy_combo), item_space); // need desk

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}


			ImGui::Spacing();

			// block: Other
			{
				const char* str_id = "##LegitBot(Other)";

				ImGui::Text("Other:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##LegitBot_Other", ImVec2(child_width, 160), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Friendly fire", &cvar.legitbot_friendly_fire, item_space, "Fire in your teammates.");
				ImGui::SliderFloat(str_id, "Triggerbot hitbox scale", &cvar.legitbot_trigger_hitbox_scale, 1, 200, "%.f%%", 1.f, item_space);		// need desc
				ImGui::Checkbox(str_id, "Triggerbot only scoped", &cvar.legitbot_trigger_only_scoped, item_space);  	// need desc

				ImGui::TextColored(ImVec4(1, 1, 0, 0.8f), "Standalone recoil compensation");
				//ImGui::SameLine(child_width / 2);
				//ImGui::Checkbox("##LegitBot(All hitboxes)", &cvar.legitbot_friendly_fire);

				ImGui::SliderFloat(str_id, "Backtrack (ping spike)", &cvar.legitbot_backtrack, 0, 1500, "%.0f ms", 1, item_space, "Returns the target for several milliseconds.\nIf you can not move, decrease the value.");

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			break;
		}

		case WindowTab_Visuals:
			ImGui::Spacing(3);
			ImGui::SameLine(12);
			ImGui::Checkbox("##Visuals", "Active", &cvar.visuals, 60);
			ImGui::Spacing(4);
			ImGui::SameLine(12);
			ImGui::BeginGroup();

			// block: ESP
			{
				const char* str_id = "##Visuals(ESP)";
				const char* const player_box_combo[] = { "Off", "Default box", "Corner box", "Rounded box" };
				const char* const position_combo[] = { "Off", "Left", "Right", "Top", "Bottom" };
				const char* const player_distance_combo[] = { "In units", "In meters" };
				const char* const font_outline_combo[] = { "Off", "Shadow", "Outline" };

				ImGui::Text("ESP:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Visuals_ESP", ImVec2(child_width, 250), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Enabled", &cvar.esp_enabled, item_space, "Global enable/disable esp.");
				ImGui::Checkbox(str_id, "Player", &cvar.esp_player, item_space);		// need desc
				ImGui::Checkbox(str_id, "Teammates", &cvar.esp_player_teammates, item_space);	// need desc
				ImGui::Combo(str_id, "Bounding box", &cvar.esp_player_box, &player_box_combo[0], IM_ARRAYSIZE(player_box_combo), item_space);	// need desk
				ImGui::Checkbox(str_id, "Bounding box outline", &cvar.esp_player_box_outline, item_space);		// need desk
				ImGui::Text("Colors"); ImGui::ColorButton4("##Terrorist color", &cvar.esp_player_box_color_t[0], item_space); ImGui::SameLine(); ImGui::Text("T");
				ImGui::ColorButton4("##Counter-Terrorist color", &cvar.esp_player_box_color_ct[0]); ImGui::SameLine(); ImGui::Text("CT");
				ImGui::Combo(str_id, "Name", &cvar.esp_player_name, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Show the name of the player on the screen.");
				ImGui::Combo(str_id, "Weapon", &cvar.esp_player_weapon, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Show the player's weapon on the screen.");
				ImGui::Combo(str_id, "Distance", &cvar.esp_player_distance, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Show the distance to the player on the screen.");
				ImGui::Combo(str_id, "Distance measuring", &cvar.esp_player_distance_measure, &player_distance_combo[0], IM_ARRAYSIZE(player_distance_combo), item_space);
				ImGui::Text("Background color"); ImGui::ColorButton4("##Background color", &cvar.esp_player_background_color[0], item_space);
				ImGui::Combo(str_id, "Health bar", &cvar.esp_player_health, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Draw the health bar.");
				ImGui::Checkbox(str_id, "Show numeric on health bar", &cvar.esp_player_health_value, item_space, "Show health points on the health bar.");
				ImGui::Combo(str_id, "Armor bar", &cvar.esp_player_armor, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Draw the armor bar.");
				ImGui::Checkbox(str_id, "Skeleton", &cvar.esp_player_skeleton, item_space, "Draw skeleton of the player.");
				ImGui::ColorButton4("##Skeleton color", &cvar.esp_player_skeleton_color[0]);
				ImGui::Checkbox(str_id, "Off-Screen", &cvar.esp_player_off_screen, item_space);	 // need desc
				ImGui::SliderFloat(str_id, "Off-Screen radius", &cvar.esp_player_off_screen_radius, 100, 500, "%.0f", 1, item_space);		// need desc
				ImGui::SliderInt(str_id, "History time", &cvar.esp_player_history_time, 0, 3000, "%.0f ms", item_space);		// need desc
				ImGui::Checkbox(str_id, "Fade out dormant", &cvar.esp_player_fadeout_dormant, item_space, "Enable soft fade out if the player is not valid.");		// need desc
				ImGui::Checkbox(str_id, "Debug info", &cvar.esp_debug_info, item_space);		// need desc
				ImGui::Checkbox(str_id, "Dropped weapons", &cvar.esp_world_dropped_weapons, item_space, "Show weapons on the map.");		// need desc
				ImGui::Checkbox(str_id, "Thrown grenades", &cvar.esp_world_thrown_grenades, item_space);		// need desc
				ImGui::Checkbox(str_id, "Bomb", &cvar.esp_bomb, item_space);		// need desc
				ImGui::ColorButton4("##Bomb color", &cvar.esp_bomb_color[0]);
				ImGui::Checkbox(str_id, "Visualize sounds", &cvar.esp_sound, item_space);		// need desc
				ImGui::SliderInt(str_id, "Sounds fade out time", &cvar.esp_sound_fadeout_time, 0, 1000, "%.0f ms", item_space);	// need desc
				ImGui::SliderFloat(str_id, "Sounds circle size", &cvar.esp_sound_circle_size, 1, 50, "%.1f", 1, item_space);		// need desc
				ImGui::Text("Sound colors"); ImGui::ColorButton4("##Sounds Terrorist color", &cvar.esp_sound_color_t[0], item_space); ImGui::SameLine(); ImGui::Text("T");
				ImGui::ColorButton4("##Sounds Counter-Terrorist color", &cvar.esp_sound_color_ct[0], 0.f); ImGui::SameLine(); ImGui::Text("CT");
				ImGui::SliderFloat(str_id, "Font size", &cvar.esp_font_size, 0.1f, 18, "%.1f px", 1, item_space);		// need desc
				ImGui::Combo(str_id, "Font outline", &cvar.esp_font_outline, &font_outline_combo[0], IM_ARRAYSIZE(font_outline_combo), item_space);	// need desk
				ImGui::ColorEdit4(str_id, "Font color", &cvar.esp_font_color[0], ImGuiColorEditFlags_NoPicker, item_space);

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::Spacing();

			// block: Colored models
			{
				const char* str_id = "##Visuals(Colored models)";
				const char* const colored_models_type[] = { "Off", "Flat", "Wireframe", "Texture", "Material" };

				ImGui::Text("Colored models and glow: (no obs proof)");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Visuals_Colored models", ImVec2(child_width, 150), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Enabled", &cvar.colored_models_enabled, item_space, "Global enable/disable colored models");
				ImGui::Combo(str_id, "Player", &cvar.colored_models_player, &colored_models_type[0], IM_ARRAYSIZE(colored_models_type), item_space);	  // need desk
				ImGui::Checkbox(str_id, "Players hidden", &cvar.colored_models_player_invisible, item_space);	// need desk
				ImGui::Checkbox(str_id, "Teammates", &cvar.colored_models_player_teammates, item_space);	// need desk
				ImGui::Text("Visible colors"); ImGui::ColorButton3("##Terrorist visible color", &cvar.colored_models_color_t[0], item_space); ImGui::SameLine(); ImGui::Text("T");
				ImGui::ColorButton3("##Counter-Terrorist visible color", &cvar.colored_models_color_ct[0]); ImGui::SameLine(); ImGui::Text("CT");
				ImGui::Text("Hidden colors"); ImGui::ColorButton3("##Terrorist hidden color", &cvar.colored_models_color_t_invisible[0], item_space); ImGui::SameLine(); ImGui::Text("T");
				ImGui::ColorButton3("##Counter-Terrorist hidden color", &cvar.colored_models_color_ct_invisible[0]); ImGui::SameLine(); ImGui::Text("CT");
				ImGui::Combo(str_id, "Hands", &cvar.colored_models_hands, &colored_models_type[0], IM_ARRAYSIZE(colored_models_type), item_space);	  // need desk
				ImGui::Text("Hands color"); ImGui::ColorButton3("##Hands color", &cvar.colored_models_hands_color[0], item_space);
				ImGui::Combo(str_id, "Dropped weapons", &cvar.colored_models_dropped_weapons, &colored_models_type[0], IM_ARRAYSIZE(colored_models_type), item_space);	  // need desk
				ImGui::Text("Dropped weapons color"); ImGui::ColorButton3("##Dropped weapons color", &cvar.colored_models_dropped_weapons_color[0], item_space);

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			ImGui::SameLine(next_child_pos_x);
			ImGui::BeginGroup();

			// block: Other
			{
				const char* str_id = "##Visuals(Other)";

				ImGui::Text("Other:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Visuals_Other", ImVec2(child_width, 180), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Snipers crosshair", &cvar.crosshair_snipers, item_space);	// need desc
				ImGui::ColorButton4("##Snipers crosshair color", &cvar.crosshair_snipers_color[0]);
				ImGui::Checkbox(str_id, "Legitbot circle FOV", &cvar.legitbot_fov_circle, item_space);	// need desc
				ImGui::ColorButton4("##Legitbot circle FOV color", &cvar.legitbot_fov_circle_color[0]);
				ImGui::Checkbox(str_id, "Spread circle FOV", &cvar.spread_fov_circle, item_space);	// need desc
				ImGui::ColorButton4("##Spread circle FOV color", &cvar.spread_fov_circle_color[0]);
				ImGui::Checkbox(str_id, "Punch recoil", &cvar.punch_recoil, item_space);	// need desc
				ImGui::ColorButton4("##Punch recoil color", &cvar.punch_recoil_color[0]);
				ImGui::Checkbox(str_id, "Grenade trail", &cvar.grenade_trail, item_space);	// need desc
				ImGui::ColorButton4("##Grenade trail color", &cvar.grenade_trail_color[0]);

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::Spacing();

			// block: Effects
			{
				const char* str_id = "##Visuals(Effects)";

				ImGui::Text("Effects:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Visuals_Effects", ImVec2(child_width, 220), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Remove visual recoil", &cvar.remove_visual_recoil, item_space);	// need desc
				ImGui::SliderInt(str_id, "Force third person", &cvar.thirdperson, 0, 512, "%.0f", item_space);	// need desc
				ImGui::Checkbox(str_id, "Brightness", &cvar.brightness, item_space);	// need desc
				ImGui::ColorButton4("##Brightness color", &cvar.brightness_color[0]);
				ImGui::Checkbox(str_id, "Disable render teammates", &cvar.disable_render_teammates, item_space);	// need desc

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			break;

		case WindowTab_Kreedz:
		{
			const char* const kreedz_type_combo[] = { "Off", "Rage", "Legit" };

			ImGui::Spacing(3);
			ImGui::SameLine(12);
			ImGui::Combo("##Kreedz", "Active", &cvar.kreedz, &kreedz_type_combo[0], IM_ARRAYSIZE(kreedz_type_combo));
			ImGui::Spacing(4);
			ImGui::SameLine(12);
			ImGui::BeginGroup();

			// block: Kreedz
			{
				const char* str_id = "##Kreedz(Rage)";

				ImGui::Text("Rage:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Kreedz_Rage", ImVec2(child_width, 425), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "Bunnyhop", &cvar.ragekreedz_bunnyhop, item_space);	// need desc

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			ImGui::SameLine(next_child_pos_x);
			ImGui::BeginGroup();

			// block: Other
			{
				const char* str_id = "##Kreedz(Legit)";

				ImGui::Text("Legit:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Kreedz_Legit", ImVec2(child_width, 425), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				// here

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			break;
		}
		case WindowTab_Misc:
			ImGui::Spacing(4);
			ImGui::SameLine(12);
			ImGui::BeginGroup();

			// block: Miscellaneous
			{
				const char* str_id = "##Miscellaneous(Miscellaneous)";
				const char* const knifebot_type[] = { "Stab", "Slash" };

				ImGui::Text("Miscellaneous:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Miscellaneous_Miscellaneous", ImVec2(child_width, 452), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				ImGui::Checkbox(str_id, "HUD clear", &cvar.hud_clear, item_space);	// need desc
				ImGui::Checkbox(str_id, "Automatic pistol", &cvar.automatic_pistol, item_space);	// need desc
				ImGui::Checkbox(str_id, "Automatic reload", &cvar.automatic_reload, item_space);	// need desc
				ImGui::Checkbox(str_id, "Knifebot", &cvar.knifebot, item_space);	// need desc
				ImGui::Checkbox(str_id, "Knifebot friendly fire", &cvar.knifebot_friendly_fire, item_space);	// need desc
				ImGui::Combo(str_id, "Knifebot type", &cvar.knifebot_type, &knifebot_type[0], IM_ARRAYSIZE(knifebot_type), item_space);	  // need desk
				ImGui::SliderFloat(str_id, "Knifebot backtrack (ping spike)", &cvar.knifebot_backtrack, 0, 1500, "%.0f ms", 1, item_space, "Returns the target for several milliseconds.\nIf you can not move, decrease the value.");
				ImGui::SliderFloat(str_id, "Knifebot distance stab", &cvar.knifebot_distance_stab, 0, 100, "%.2f", 1, item_space);
				ImGui::SliderFloat(str_id, "Knifebot distance slash", &cvar.knifebot_distance_slash, 0, 100, "%.2f", 1, item_space);
				//	ImGui::SliderFloat(str_id, "Knifebot maximum FOV", &cvar.knifebot_fov, 0, 180, u8"%.1f°", 1.f, flSpaceOffset);

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			ImGui::SameLine(next_child_pos_x);
			ImGui::BeginGroup();

			// block: Other
			{
				const char* str_id = "##Miscellaneous(Other)";

				ImGui::Text("Other:");
				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
				ImGui::BeginChild("##Miscellaneous_Other", ImVec2(child_width, 452), true);
				ImGui::PopStyleColor(2);
				ImGui::PushItemWidth(item_width);
				ImGui::Spacing();
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Spacing(3);

				// here

				ImGui::Spacing(3);
				ImGui::PopItemWidth();
				ImGui::EndGroup();
				ImGui::EndChild();
			}

			ImGui::EndGroup();
			break;

		case WindowTab_GUI:
			break;

		case WindowTab_Settings:
			static char szFind[100] = { 0 };
			static char szReplace[100] = { 0 };
			static char szCreateSettingsBuff[100] = { 0 };
			static char szRenameSettingsBuff[100] = { 0 };
			static char szSettingsBuff[104448] = { 0 };

			const std::array<const char* const, 6> apcszIniFilesList =
			{
				"ragebot.ini",
				"legitbot.ini",
				"visuals.ini",
				"kreedz.ini",
				"misc.ini",
				"gui.ini"
			};

			static auto iIniFileSelected = 0;

			ImGui::Spacing(2);
			ImGui::SameLine(12);
			ImGui::BeginGroup();
			ImGui::Text("Settings:");
			ImGui::PushItemWidth(216);

			RefreshSettings();

			if (ImGui::ListBoxArray("##SettingsList", &g_iSettingsSelect, g_sSettingsList, 26))
				g_bIsUpdateFile = true;

			ImGui::PopItemWidth();
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("Files:");
			ImGui::PushItemWidth(216);

			if (ImGui::ListBox("##IniFiles", &iIniFileSelected, &apcszIniFilesList.at(0), apcszIniFilesList.size()))
				g_bIsUpdateFile = true;

			ImGui::PopItemWidth();
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Spacing(4);
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

			if (ImGui::Button("Load settings##Settings", ImVec2(160, 22)))
				CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)LoadSettingsThread, nullptr, 0, nullptr);

			ImGui::SameLine();

			if (ImGui::Button("Rename settings##Settings", ImVec2(160, 22)))
			{
				if (_stricmp("Default", g_sSettingsList.at(g_iSettingsSelect).c_str()))
				{
					RtlSecureZeroMemory(&szRenameSettingsBuff[0], sizeof(szRenameSettingsBuff));
					strcpy(&szRenameSettingsBuff[0], g_sSettingsList.at(g_iSettingsSelect).c_str());

					ImGui::OpenPopup("Rename settings##Modal");
				}

				g_bIsUpdateFile = true;
			}

			if (ImGui::Button("Save settings##Settings", ImVec2(160, 22)))
				CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)SaveSettingsThread, nullptr, 0, nullptr);

			ImGui::SameLine();

			if (ImGui::Button("Create & Save settings##Settings", ImVec2(160, 22)))
			{
				RtlSecureZeroMemory(&szCreateSettingsBuff[0], sizeof(szCreateSettingsBuff));
				ImGui::OpenPopup("Create & Save##Modal");
			}

			ImGui::Spacing(2);

			if (ImGui::Button("Restore settings##Settings", ImVec2(160, 22)))
			{
				ImGui::OpenPopup("Restore settings##Modal");
				g_bIsUpdateFile = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("Remove settings##Settings", ImVec2(160, 22)))
			{
				if (_stricmp("Default", g_sSettingsList.at(g_iSettingsSelect).c_str()))
					ImGui::OpenPopup("Remove settings##Modal");

				g_bIsUpdateFile = true;
			}

			if (ImGui::Button("Open settings folder##Settings", ImVec2(160, 21)))
			{
				ShellExecuteA(nullptr, "open", g_Globals.m_sSettingsPath.c_str(), nullptr, nullptr, SW_RESTORE);
				g_bIsUpdateFile = true;
			}

			ImGui::SameLine();

			const std::string c_sPath = g_Globals.m_sSettingsPath +
				g_sSettingsList.at(g_iSettingsSelect) + "\\" + apcszIniFilesList.at(iIniFileSelected);

			if (ImGui::Button("Open selected file##Settings", ImVec2(160, 21)))
			{
				ShellExecuteA(nullptr, "open", c_sPath.c_str(), nullptr, nullptr, SW_RESTORE);
				g_bIsUpdateFile = true;
			}

			ImGui::PopStyleColor();
			ImGui::Spacing(2);
			ImGui::SameLine(-224);

			if (g_bIsUpdateFile)
			{
				RtlSecureZeroMemory(&szSettingsBuff[0], sizeof(szSettingsBuff));

				if (g_Utils.FileExists(c_sPath.c_str()))
					ReadFromFile(c_sPath.c_str(), &szSettingsBuff[0]);

				g_bIsUpdateFile = false;
			}

			ImGui::PushFont(ProggyClean_13px);
			ImGui::InputTextMultiline("##src", &szSettingsBuff[0], sizeof(szSettingsBuff), ImVec2(551, 302), ImGuiInputTextFlags_AllowTabInput);
			ImGui::PopFont();
			ImGui::Spacing(2);
			ImGui::SameLine(-224);
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

			if (ImGui::Button("Save##File", ImVec2(80, 21)) && strlen(&szSettingsBuff[0]) > 0)
			{
				FILE* pFile = fopen(c_sPath.data(), "w");

				if (pFile)
				{
					fwrite(&szSettingsBuff[0], sizeof(szSettingsBuff), 1, pFile);
					fclose(pFile);
				}
			}

			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			ImGui::PushItemWidth(184);
			ImGui::InputText("##replace_from", &szFind[0], sizeof(szFind), ImGuiInputTextFlags_AutoSelectAll);
			ImGui::SameLine();
			ImGui::InputText("##replace_to", &szReplace[0], sizeof(szReplace), ImGuiInputTextFlags_AutoSelectAll);
			ImGui::PopItemWidth();
			ImGui::PopStyleVar();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

			if (ImGui::Button("Replace##Settings", ImVec2(80, 21)))
				g_Utils.StringReplace(&szSettingsBuff[0], &szFind[0], &szReplace[0]);

			ImGui::PopStyleColor();
			ImGui::EndGroup();

			ImGui::Spacing(2);
			ImGui::SameLine(offset_x);
			ImGui::PushItemWidth(window_size.x - offset_x * 2.f);
			ImGui::InputText("##AppData", const_cast<char*>(g_Globals.m_sSettingsPath.c_str()),
				g_Globals.m_sSettingsPath.size() + 1, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
			ImGui::PopItemWidth();
			ImGui::PopStyleVar();

			const auto modal_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
			const auto modal_size = ImVec2(250, 80);
			const auto button_size = ImVec2(109, 20);

			// Popup Modal: "Create & Save" button
			ImGui::SetNextWindowSize(modal_size);

			if (ImGui::BeginPopupModal("Create & Save##Modal", static_cast<bool*>(nullptr), modal_flags))
			{
				ImGui::Spacing(3);
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Text("Settings name:");
				ImGui::PushItemWidth(226);
				ImGui::InputText("##CreateSettings", &szCreateSettingsBuff[0], sizeof(szCreateSettingsBuff));
				ImGui::PopItemWidth();
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

				if (ImGui::Button("Create", button_size) && strlen(&szCreateSettingsBuff[0]) > 0)
				{
					if (_stricmp(&szCreateSettingsBuff[0], "Default"))
					{
						CreateDirectoryA(std::string(g_Globals.m_sSettingsPath + &szCreateSettingsBuff[0]).c_str(), nullptr);

						RefreshSettings();

						for (size_t i = 0; i < g_sSettingsList.size(); ++i)
						{
							if (!strcmp(g_sSettingsList.at(i).c_str(), &szCreateSettingsBuff[0]))
							{
								g_iSettingsSelect = i;
								break;
							}
						}

						CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)SaveSettingsThread, nullptr, 0, nullptr);
					}

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel", button_size))
					ImGui::CloseCurrentPopup();

				ImGui::PopStyleColor();
				ImGui::EndGroup();
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowSize(modal_size);

			// Popup Modal: "Rename settings" button
			if (ImGui::BeginPopupModal("Rename settings##Modal", static_cast<bool*>(nullptr), modal_flags))
			{
				ImGui::Spacing(3);
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::Text("Settings name:");
				ImGui::PushItemWidth(226);
				ImGui::InputText("##RenameSettings", &szRenameSettingsBuff[0], sizeof(szRenameSettingsBuff));
				ImGui::PopItemWidth();
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

				if (ImGui::Button("Rename", button_size) && strlen(&szRenameSettingsBuff[0]) > 0)
				{
					if (_stricmp(g_sSettingsList.at(g_iSettingsSelect).c_str(), "Default"))
					{
						const auto src = g_Globals.m_sSettingsPath + g_sSettingsList.at(g_iSettingsSelect);
						const auto dest = g_Globals.m_sSettingsPath + &szRenameSettingsBuff[0];

						MoveFileA(src.c_str(), dest.c_str());

						g_bIsUpdateFile = true;
					}

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel", button_size))
					ImGui::CloseCurrentPopup();

				ImGui::PopStyleColor();
				ImGui::EndGroup();
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowSize(modal_size);

			// Popup Modal: "Remove settings" button
			if (ImGui::BeginPopupModal("Remove settings##Modal", static_cast<bool*>(nullptr), modal_flags))
			{
				const auto msg = "Are you sure you want to delete?";
				const auto msg_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, msg);

				ImGui::Spacing(6);
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::SameLine(((ImGui::GetWindowWidth() - offset_x * 2.f) - msg_size.x) * 0.5f);
				ImGui::Text(msg);
				ImGui::Spacing(2);
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

				if (ImGui::Button("Delete", button_size))
				{
					g_Settings.Delete(std::string(g_Globals.m_sSettingsPath + g_sSettingsList.at(g_iSettingsSelect)).c_str());

					g_bIsUpdateFile = true;

					if (g_iSettingsSelect >= g_sSettingsList.size())
						g_iSettingsSelect = g_sSettingsList.size() - 1;

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel", button_size))
					ImGui::CloseCurrentPopup();

				ImGui::PopStyleColor();
				ImGui::EndGroup();
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowSize(modal_size);

			// Popup Modal: "Restore settings" button
			if (ImGui::BeginPopupModal("Restore settings##Modal", static_cast<bool*>(nullptr), modal_flags))
			{
				const auto msg = "Choose the type of restore";
				const auto msg_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, msg);

				ImGui::Spacing(6);
				ImGui::SameLine(offset_x);
				ImGui::BeginGroup();
				ImGui::SameLine(((ImGui::GetWindowWidth() - offset_x * 2.f) - msg_size.x) * 0.5f);
				ImGui::Text(msg);
				ImGui::Spacing(2);
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

				if (ImGui::Button("Set as default", button_size))
				{
					CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)RestoreSettingsAsDefaultThread, nullptr, 0, nullptr);
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Save settings", button_size))
				{
					CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)RestoreSettingsWithSavingThread, nullptr, 0, nullptr);
					ImGui::CloseCurrentPopup();
				}

				ImGui::PopStyleColor();
				ImGui::EndGroup();
				ImGui::EndPopup();
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}
}