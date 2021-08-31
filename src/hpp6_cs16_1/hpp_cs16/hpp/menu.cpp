#include "main.h"

static Menu::window_parameters_s w;
Menu::TabsList Menu::Tab::m_CurrentTabId;
WeaponIdType Menu::Tab::m_CurrentWeaponId;
char Menu::szSettingsBuffer[40];
unsigned int Menu::iSelectedSettings;
unsigned int Menu::iSelectedIniFile;
std::vector<std::string> Menu::sSettingsList;
bool Menu::bUpdateFileContent;

typedef void(*LPSEARCHFUNC)(LPCTSTR lpcszFileName);
static void SearchFiles(LPCTSTR lpcszFileName, LPSEARCHFUNC lpSearchFunc)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFileFind = FindFirstFileA(lpcszFileName, &FindFileData);

	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!strstr(FindFileData.cFileName, "."))
			{
				char szBuffer[MAX_PATH];
				LPSTR lpszPart = nullptr;

				GetFullPathNameA(lpcszFileName, sizeof(szBuffer), szBuffer, &lpszPart);
				strcpy(lpszPart, FindFileData.cFileName);
				lpSearchFunc(FindFileData.cFileName);
			}

		} while (FindNextFileA(hFileFind, &FindFileData));

		FindClose(hFileFind);
	}
}

static void ReadSettings(const char* pcszFileName)
{
	if (!strstr(pcszFileName, "*.ini"))
		Menu::sSettingsList.push_back(pcszFileName);
}

void Menu::RefreshSettings()
{
	sSettingsList.clear();

	std::string settings_path_str;
	settings_path_str.append(g_Globals.m_sSettingsPath);
	settings_path_str.append("*");
	SearchFiles(settings_path_str.c_str(), ReadSettings);
}

static void ReadFromFile(const char* pcszFileName, char* pszBuffer)
{
	std::ifstream input(pcszFileName);
	std::size_t size = 0;

	char ch;
	while ((ch = (char)input.get()) != -1)
		size++;

	input.close();

	auto* const pFile = fopen(pcszFileName, "r");

	if (pFile)
	{
		fread(pszBuffer, size, 1, pFile);
		fclose(pFile);
	}
}

static void ApplyWindowParameters()
{
	const int COLUMS_COUNT = 2;

	w.WindowSize = ImVec2(850, 550);
	w.ModalSize = ImVec2(250, 80);
	w.ModalButtonSize = ImVec2(109, 20);
	w.Title = const_cast<char*>("Hpp Hack for Counter-Strike 1.6");
	w.ChildWidth = (w.WindowSize.x - ImGui::GetSameLinePadding() * (COLUMS_COUNT + 1)) / COLUMS_COUNT;
	w.NextChildPosX = (w.WindowSize.x + ImGui::GetSameLinePadding()) / COLUMS_COUNT;
	w.ChildCenterPos = w.ChildWidth * 0.5F;
	w.ItemWidth = w.ChildCenterPos - ImGui::GetSameLinePadding() * 2.0F;
}

static void ApplyColorsChanges()
{
	auto& style = ImGui::GetStyle();

	for (size_t i = 0; i < ImGuiCol_COUNT; i++)
		style.Colors[i] = ImColor(cvar.gui[i]);
}

void Menu::ModalPopup::SettingsSaveAs()
{
	ImGui::Spacing(3);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::BeginGroup();
	ImGui::Text("Settings name:");
	ImGui::PushItemWidth(226);
	ImGui::InputText("##SettingsSaveAs", Menu::szSettingsBuffer, sizeof(Menu::szSettingsBuffer));
	ImGui::PopItemWidth();
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
	ImGui::Spacing();

	if (ImGui::Button("Save", w.ModalButtonSize) && strlen(Menu::szSettingsBuffer) > 0)
	{
		if (_stricmp(Menu::szSettingsBuffer, "Default"))
		{
			std::string settings_path_str;
			settings_path_str.append(g_Globals.m_sSettingsPath);
			settings_path_str.append(Menu::szSettingsBuffer);
			CreateDirectoryA(settings_path_str.c_str(), 0);
			Menu::RefreshSettings();

			for (size_t i = 0; i < Menu::sSettingsList.size(); i++)
				if (!strcmp(Menu::sSettingsList.at(i).c_str(), Menu::szSettingsBuffer))
					Menu::iSelectedSettings = i;

			Settings::Save();
		}

		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel", w.ModalButtonSize))
		ImGui::CloseCurrentPopup();

	ImGui::PopStyleColor();
	ImGui::EndGroup();
	ImGui::ModalPopup::End();
}

void Menu::ModalPopup::SettingsRename()
{
	ImGui::Spacing(3);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::BeginGroup();
	ImGui::Text("Settings name:");
	ImGui::PushItemWidth(226);
	ImGui::InputText("##SettingsRename", Menu::szSettingsBuffer, sizeof(Menu::szSettingsBuffer));
	ImGui::PopItemWidth();
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
	ImGui::Spacing();

	if (ImGui::Button("Rename", w.ModalButtonSize) && strlen(Menu::szSettingsBuffer) > 0)
	{
		if (strcmp(Menu::sSettingsList.at(Menu::iSelectedSettings).c_str(), Menu::szSettingsBuffer))
		{
			std::string source, destination;
			source.append(g_Globals.m_sSettingsPath);
			source.append(Menu::sSettingsList.at(Menu::iSelectedSettings));
			destination.append(g_Globals.m_sSettingsPath);
			destination.append(Menu::szSettingsBuffer);
			MoveFileA(source.c_str(), destination.c_str());
		}

		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel", w.ModalButtonSize))
		ImGui::CloseCurrentPopup();

	ImGui::PopStyleColor();
	ImGui::EndGroup();
	ImGui::ModalPopup::End();
}

void Menu::ModalPopup::SettingsRemove()
{
	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];
	const char* pcszMessage = "Are you sure you want to delete?";
	const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, pcszMessage);

	ImGui::Spacing(6);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::BeginGroup();
	ImGui::SameLine(((ImGui::GetWindowWidth() - ImGui::GetSameLinePadding() * 2.0F) - text_size.x) * 0.5F);
	ImGui::Text(pcszMessage);
	ImGui::Spacing(2);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
	ImGui::Spacing();

	if (ImGui::Button("Delete", w.ModalButtonSize))
	{
		std::string settings_path_str;
		settings_path_str.append(g_Globals.m_sSettingsPath);
		settings_path_str.append(Menu::sSettingsList.at(Menu::iSelectedSettings));
		g_Settings.Remove(settings_path_str.c_str());
		Menu::RefreshSettings();

		if (Menu::iSelectedSettings >= Menu::sSettingsList.size())
			Menu::iSelectedSettings = Menu::sSettingsList.size() - 1;

		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel", w.ModalButtonSize))
		ImGui::CloseCurrentPopup();

	ImGui::PopStyleColor();
	ImGui::EndGroup();
	ImGui::ModalPopup::End();
}

void Menu::ModalPopup::SettingsRestore()
{
	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];
	const char* pcszMessage = "Choose the type of restore";
	const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, pcszMessage);

	ImGui::Spacing(6);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::BeginGroup();
	ImGui::SameLine(((ImGui::GetWindowWidth() - ImGui::GetSameLinePadding() * 2.0F) - text_size.x) * 0.5F);
	ImGui::Text(pcszMessage);
	ImGui::Spacing(2);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
	ImGui::Spacing();

	if (ImGui::Button("Set as default", w.ModalButtonSize))
	{
		g_Settings.Restore(Menu::sSettingsList.at(Menu::iSelectedSettings).c_str(), true);
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Save settings", w.ModalButtonSize))
	{
		g_Settings.Restore(Menu::sSettingsList.at(Menu::iSelectedSettings).c_str());
		ImGui::CloseCurrentPopup();
	}

	ImGui::PopStyleColor();
	ImGui::EndGroup();
	ImGui::ModalPopup::End();
}

void Menu::Settings::Load()
{
	g_Settings.Load(Menu::sSettingsList.at(Menu::iSelectedSettings).c_str());
}

void Menu::Settings::Save()
{
	g_Settings.Save(Menu::sSettingsList.at(Menu::iSelectedSettings).c_str());
}

void Menu::Settings::RestorePopupOpen()
{
	ImGui::OpenPopup("Restore##Modal");
}

void Menu::Settings::SaveAsPopupOpen()
{
	RtlSecureZeroMemory(szSettingsBuffer, sizeof(szSettingsBuffer));
	ImGui::OpenPopup("Save as##Modal");
}

void Menu::Settings::RemovePopupOpen()
{
	if (_stricmp("Default", sSettingsList.at(iSelectedSettings).c_str()))
		ImGui::OpenPopup("Remove##Modal");
}

void Menu::Settings::RenamePopupOpen()
{
	if (_stricmp("Default", sSettingsList.at(iSelectedSettings).c_str()))
	{
		RtlSecureZeroMemory(szSettingsBuffer, sizeof(szSettingsBuffer));
		strcpy(szSettingsBuffer, sSettingsList.at(iSelectedSettings).c_str());
		ImGui::OpenPopup("Rename##Modal");
	}
}

void Menu::Settings::OpenSettingsFolder()
{
	std::string folder_path_str;
	folder_path_str.append(g_Globals.m_sSettingsPath);
	folder_path_str.append(sSettingsList.at(iSelectedSettings));
	ShellExecuteA(0, "open", folder_path_str.c_str(), 0, 0, SW_RESTORE);
}

void Menu::Settings::OpenSettingsFile()
{
	std::string file_path_str;
	file_path_str.append(g_Globals.m_sSettingsPath);
	file_path_str.append(sSettingsList.at(iSelectedSettings));
	file_path_str.append("\\");
	file_path_str.append(pcszFilesList[iSelectedIniFile]);
	ShellExecuteA(0, "open", file_path_str.c_str(), 0, 0, SW_RESTORE);
}

void Menu::Settings::OpenSettingsPath()
{
	ShellExecuteA(0, "open", g_Globals.m_sSettingsPath.c_str(), 0, 0, SW_RESTORE);
}

Menu::TabsList Menu::Tab::GetCurrentTab()
{
	return m_CurrentTabId;
}

void Menu::Tab::ClearCurrentWeaponId()
{
	m_CurrentWeaponId = WEAPON_NONE;
}

bool Menu::Tab::Labels()
{
	static std::vector<std::string> tab_names_str;
	auto& style = ImGui::GetStyle();

	const auto ItemSpacing = style.ItemSpacing;
	const auto FrameRounding = style.FrameRounding;
	const auto ColorButton = style.Colors[ImGuiCol_Button];
	const auto ColorButtonActive = style.Colors[ImGuiCol_ButtonActive];
	const auto ColorButtonHovered = style.Colors[ImGuiCol_ButtonHovered];
	const auto ColorBorder = style.Colors[ImGuiCol_Border];
	const auto ColorScrollbarBg = style.Colors[ImGuiCol_ScrollbarBg];

	style.ItemSpacing = ImVec2(0, 1);
	style.FrameRounding = 0.0F;
	style.Colors[ImGuiCol_ButtonHovered] = ColorButtonHovered;
	style.Colors[ImGuiCol_ButtonActive] = ColorButtonActive;
	style.Colors[ImGuiCol_Border] = ImColor();

	bool bIsPressed = false;

	if (!tab_names_str.size())
	{
		tab_names_str.push_back("RageBot");
		tab_names_str.push_back("LegitBot");
		tab_names_str.push_back("Visuals");
		tab_names_str.push_back("Kreedz");
		tab_names_str.push_back("Misc");
		tab_names_str.push_back("GUI");
		tab_names_str.push_back("Settings");
	}

	for (size_t i = 0; i < tab_names_str.size(); i++)
	{
		style.Colors[ImGuiCol_Button] = i != m_CurrentTabId ? ColorScrollbarBg : ColorButton;

		if (i > 0) ImGui::SameLine();

		if (bIsPressed = ImGui::Button(tab_names_str.at(i).c_str(),
			ImVec2(ImGui::GetWindowSize().x / tab_names_str.size(), 18)))
			m_CurrentTabId = static_cast<Menu::TabsList>(i);
	}

	ImGui::Spacing(2);
	ImGui::Separator();

	style.Colors[ImGuiCol_Button] = ColorButton;
	style.Colors[ImGuiCol_ButtonActive] = ColorButtonActive;
	style.Colors[ImGuiCol_ButtonHovered] = ColorButtonHovered;
	style.Colors[ImGuiCol_Border] = ColorBorder;
	style.ItemSpacing = ItemSpacing;
	style.FrameRounding = FrameRounding;

	return bIsPressed;
}

void Menu::Tab::RageBot()
{
	ImGui::Tab::Begin(&cvar.ragebot_active, "##RageBot");

	if (cvar.ragebot_active)
		cvar.legitbot_active = false;

	// ==================================================================================================================
	// Aimbot
	// ==================================================================================================================

	static const char* const multipoint[] = { "Off", "Low", "Medium", "High" };
	static const char* const hitbox[] = { "Head", "Chest", "Stomach", "All hitboxes", "Vital hitboxes" };

	ImGui::Control::SetStringIdentify("##RageBot (Aimbot)");
	ImGui::Control::Text("Aimbot:");
	ImGui::Child::Begin("##RageBot(Aimbot)", ImVec2(w.ChildWidth, 270));
	ImGui::Control::Checkbox(&cvar.ragebot_aim_enabled, "Enabled", "Global ragebot on/off.");
	ImGui::Control::PushCondition(cvar.ragebot_aim_enabled);
	ImGui::Control::Checkbox(&cvar.ragebot_aim_friendly_fire, "Friendly fire", "Fire in your teammates.");
	ImGui::Control::Checkbox(&cvar.ragebot_aim_auto_fire, "Automatic fire", "Automatic fire whenever possible.");
	ImGui::Control::SliderFloat(&cvar.ragebot_aim_fov, "Maximum FOV", 0.0F, 180.0F, u8"%.1f°", 2.0F, "Maximal field of view of the aimbot.");
	ImGui::Control::Combo(&cvar.ragebot_aim_hitbox, "Hitbox", COMBO_ARRAY(hitbox), "Hitbox or group of hitboxes for aiming.\n - Head;\n - Chest;\n - Stomach;\n - All hitboxes;\n - Vital hitboxes.");
	ImGui::Control::Combo(&cvar.ragebot_aim_multipoint, "Multi-point", COMBO_ARRAY(multipoint), "Additional points for hitboxes.\n - Off;\n - Low;\n - Medium;\n - High.");
	ImGui::Control::PushCondition(cvar.ragebot_aim_multipoint);
	ImGui::Control::SliderFloat(&cvar.ragebot_aim_multipoint_scale, "Multi-point scale", 1.0F, 100.0F, "%.f%%"); // что ето такое
	ImGui::Control::PopCondition();
	ImGui::Control::Checkbox(&cvar.ragebot_aim_auto_penetration, "Automatic penetration", "Automatic penetration through walls.");
	ImGui::Control::Checkbox(&cvar.ragebot_aim_auto_scope, "Automatic scope", "Automatic opening scope on snipers weapons.");
	ImGui::Control::Checkbox(&cvar.ragebot_aim_silent, "Silent aim", "Invisible aiming for you.");
	ImGui::Control::Checkbox(&cvar.ragebot_aim_perfect_silent, "Perfect silent aim", "Invisible aiming for all.");
	ImGui::Control::Checkbox(&cvar.ragebot_fake_walk_enabled, "Fake walk enabled"); // что ето такое
	ImGui::Control::KeyAssignment(&cvar.ragebot_fake_walk_key_press, "##FakeWalkKey");
	ImGui::Control::PushCondition(cvar.ragebot_fake_walk_enabled);
	ImGui::Control::SliderInt(&cvar.ragebot_fake_walk_choke_limit, "Fake walk choke limit", 0, 62, "%.f", "Old build servers and clients have 16 cmds limit.");
	ImGui::Control::Checkbox(&cvar.ragebot_fake_walk_decrease_fps, "Fake walk decrease fps"); // что ето такое
	ImGui::Control::PopCondition(2);
	ImGui::Child::End();

	ImGui::Spacing();

	// ==================================================================================================================
	// Fake lag
	// ==================================================================================================================

	static const char* const triggers[] = { "On land", "In air", "On land + In air" };
	static const char* const type[] = { "Dynamic", "Maximum", "Break lag compensation" };

	ImGui::Control::SetStringIdentify("##RageBot (Fakelag)");
	ImGui::Control::Text("Fake lag:");
	ImGui::Child::Begin("##RageBot(Fakelag)", ImVec2(w.ChildWidth, 138));
	ImGui::Control::Checkbox(&cvar.fakelag_enabled, "Enabled", "Global fake lag on/off.");
	ImGui::Control::PushCondition(cvar.fakelag_enabled);
	ImGui::Control::Combo(&cvar.fakelag_type, "Type", COMBO_ARRAY(type), "Type of fake lagging.");
	ImGui::Control::Combo(&cvar.fakelag_triggers, "Triggers", COMBO_ARRAY(triggers)); // что ето такое
	ImGui::Control::SliderFloat(&cvar.fakelag_variance, "Variance", 1.0F, 100.0F, "%.f%%"); // что ето такое
	ImGui::Control::SliderInt(&cvar.fakelag_choke_limit, "Choke limit", 1, 62, "%.f"); // что ето такое
	ImGui::Control::Checkbox(&cvar.fakelag_while_shooting, "Fake lag while shooting");
	ImGui::Control::Checkbox(&cvar.fakelag_on_enemy_visible, "Fake lag on enemy visible");
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::EndGroup();
	ImGui::SameLine(w.NextChildPosX);
	ImGui::BeginGroup();

	// ==================================================================================================================
	// Other
	// ==================================================================================================================

	static const char* const nospread[] = { "Off", "Default", "Perfect Pitch / Yaw", "Transpose Pitch / Yaw / Roll", "Perfect Pitch / Yaw / Roll" };
	static const char* const resolver[] = { "Off", "Gait yaw, fakedown, fakeup" };

	ImGui::Control::SetStringIdentify("##RageBot (Other)");
	ImGui::Control::Text("Other:");
	ImGui::Child::Begin("##RageBot(Other)", ImVec2(w.ChildWidth, 138));
	ImGui::Control::Checkbox(&cvar.ragebot_remove_recoil, "Remove recoil", "Disables the recoil of weapons.");
	ImGui::Control::Combo(&cvar.ragebot_remove_spread, "Remove spread", COMBO_ARRAY(nospread), "Disables the spread of weapons.\n - Default;\n - Perfect Pitch / Yaw;\n - Transpose Pitch / Yaw / Roll;\n - Perfect Pitch / Yaw / Roll.");
	ImGui::Control::Checkbox(&cvar.ragebot_tapping_mode, "Tapping mode", "Accuracy boost. Not shooting if punchangle > 0.");
	ImGui::Control::Checkbox(&cvar.ragebot_delay_shot, "Delay shot", "Skip enemy, if enemy breaking lag compensation.");
	ImGui::Control::Combo(&cvar.ragebot_resolver, "Anti-aim resolver", COMBO_ARRAY(resolver));
	ImGui::TextColored(ImVec4(1.00F, 1.00F, 0.00F, 0.80F), "Yaw / Roll aiming");
	ImGui::Child::End();

	ImGui::Spacing();

	// ==================================================================================================================
	// Anti-aimbot
	// ==================================================================================================================

	static const char* const pitch[] = { "Off", "Fakedown", "Fakeup", "Down", "Up", "Jitter", "Random" };
	static const char* const yaw[] = { "Off", "Backwards", "Gait breaker", "Static" };
	static const char* const yaw_while_running[] = { "Off", "Backwards", "Static" };
	static const char* const fake_yaw[] = { "Off", "Backwards", "Sideways", "Local view", "Random", "Spin", "Static" };
	static const char* const edge_triggers[] = { "Standing", "On land", "Always" };

	ImGui::Control::SetStringIdentify("##RageBot (Anti-aimbot)");
	ImGui::Control::Text("Anti-aimbot:");
	ImGui::Child::Begin("##RageBot(Anti-aimbot)", ImVec2(w.ChildWidth, 270));
	ImGui::Control::Checkbox(&cvar.antiaim_enabled, "Enabled", "Global anti-aimbot on/off.");
	ImGui::Control::PushCondition(cvar.antiaim_enabled);
	ImGui::Control::Checkbox(&cvar.antiaim_teammates, "Teammates"); // что ето такое
	ImGui::Control::Combo(&cvar.antiaim_pitch, "Pitch angle", COMBO_ARRAY(pitch)); // что ето такое
	ImGui::Control::Combo(&cvar.antiaim_yaw, "Yaw angle", COMBO_ARRAY(yaw)); // что ето такое
	ImGui::Control::SliderFloat(&cvar.antiaim_yaw_static, "Yaw static angle", -180.0F, 180.0F, u8"%.f°", 1.4F); // что ето такое
	ImGui::Control::Combo(&cvar.antiaim_yaw_while_running, "Yaw angle while running", COMBO_ARRAY(yaw_while_running)); // что ето такое
	ImGui::Control::Combo(&cvar.antiaim_fake_yaw, "Fake yaw angle", COMBO_ARRAY(fake_yaw)); // что ето такое
	ImGui::Control::SliderFloat(&cvar.antiaim_fake_yaw_static, "Fake yaw static angle", -180.0F, 180.0F, u8"%.f°", 1.4F); // что ето такое
	ImGui::Control::SliderInt(&cvar.antiaim_yaw_spin, "Spin speed", 1, 128); // что ето такое
	ImGui::Control::Checkbox(&cvar.antiaim_on_knife, "On knife"); // что ето такое
	ImGui::Control::SliderFloat(&cvar.antiaim_choke_limit, "Choke limit", 0.0F, 62.0F, "%.f", 1.0F, "Choke limit (if zero fake = real)");
	ImGui::Control::SliderFloat(&cvar.antiaim_roll, "Roll angle", -180.0F, 180.0F, u8"%.f°", 1.4F, "Roll angle (only special servers)");
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::Tab::End();
}

void Menu::Tab::LegitBot()
{
	static const char* const weapon_groups[] = { "Pistol", "SubMachineGun", "Rifle", "Shotgun", "Sniper" };
	static const char* const weapons_pistol[] = { "glock18", "usp", "p228", "deagle", "elite", "fiveseven" };
	static const char* const weapons_submachinegun[] = { "m249", "tmp", "p90", "mp5n", "mac10", "ump45" };
	static const char* const weapons_rifle[] = { "m4a1", "galil", "famas", "aug", "ak47", "sg552" };
	static const char* const weapons_shotgun[] = { "xm1014", "m3" };
	static const char* const weapons_sniper[] = { "awp", "scout", "g3sg1", "sg550" };

	static int current_weapon[IM_ARRAYSIZE(weapon_groups)];
	static int current_weapon_group = 0;

	ImGui::Spacing(3);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::Text("Active");
	ImGui::SameLine(60);
	ImGui::Checkbox("##LegitBot", &cvar.legitbot_active);
	ImGui::SameLine(424);
	ImGui::PushItemWidth(150);
	ImGui::Text("Group");
	ImGui::SameLine();
	ImGui::Combo("##LegitBot (Group)", &current_weapon_group, COMBO_ARRAY(weapon_groups));
	ImGui::SameLine();
	ImGui::Text("\tWeapon");
	ImGui::SameLine();

	if (cvar.legitbot_active)
		cvar.ragebot_active = false;

	if (m_CurrentWeaponId == WEAPON_NONE)
	{
		const char* pszWeaponName = g_Utils.GetWeaponNameByIndex(g_Weapon.data.m_iWeaponID);

		for (size_t i = 0; i < IM_ARRAYSIZE(weapons_pistol); i++)
		{
			if (!strcmp(pszWeaponName, weapons_pistol[i]))
			{
				current_weapon_group = WeaponGroup_Pistol;
				current_weapon[current_weapon_group] = i;
				break;
			}
		}

		for (size_t i = 0; i < IM_ARRAYSIZE(weapons_submachinegun); i++)
		{
			if (!strcmp(pszWeaponName, weapons_submachinegun[i]))
			{
				current_weapon_group = WeaponGroup_SubMachineGun;
				current_weapon[current_weapon_group] = i;
				break;
			}
		}

		for (size_t i = 0; i < IM_ARRAYSIZE(weapons_rifle); i++)
		{
			if (!strcmp(pszWeaponName, weapons_rifle[i]))
			{
				current_weapon_group = WeaponGroup_Rifle;
				current_weapon[current_weapon_group] = i;
				break;
			}
		}

		for (size_t i = 0; i < IM_ARRAYSIZE(weapons_shotgun); i++)
		{
			if (!strcmp(pszWeaponName, weapons_shotgun[i]))
			{
				current_weapon_group = WeaponGroup_Shotgun;
				current_weapon[current_weapon_group] = i;
				break;
			}
		}

		for (size_t i = 0; i < IM_ARRAYSIZE(weapons_sniper); i++)
		{
			if (!strcmp(pszWeaponName, weapons_sniper[i]))
			{
				current_weapon_group = WeaponGroup_Sniper;
				current_weapon[current_weapon_group] = i;
				break;
			}
		}
	}

	switch (current_weapon_group)
	{
	case WeaponGroup_Pistol:
		ImGui::Combo("##LegitBot(Weapon)", &current_weapon[current_weapon_group], COMBO_ARRAY(weapons_pistol));
		break;
	case WeaponGroup_SubMachineGun:
		ImGui::Combo("##LegitBot(Weapon)", &current_weapon[current_weapon_group], COMBO_ARRAY(weapons_submachinegun));
		break;
	case WeaponGroup_Rifle:
		ImGui::Combo("##LegitBot(Weapon)", &current_weapon[current_weapon_group], COMBO_ARRAY(weapons_rifle));
		break;
	case WeaponGroup_Shotgun:
		ImGui::Combo("##LegitBot(Weapon)", &current_weapon[current_weapon_group], COMBO_ARRAY(weapons_shotgun));
		break;
	case WeaponGroup_Sniper:
		ImGui::Combo("##LegitBot(Weapon)", &current_weapon[current_weapon_group], COMBO_ARRAY(weapons_sniper));
	}

	ImGui::PopItemWidth();
	ImGui::Spacing(4);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::BeginGroup();

	if (current_weapon_group == WeaponGroup_Pistol)
	{
		switch (current_weapon[current_weapon_group])
		{
		case 0: m_CurrentWeaponId = WEAPON_GLOCK18; break;
		case 1: m_CurrentWeaponId = WEAPON_USP; break;
		case 2: m_CurrentWeaponId = WEAPON_P228; break;
		case 3: m_CurrentWeaponId = WEAPON_DEAGLE; break;
		case 4: m_CurrentWeaponId = WEAPON_ELITE; break;
		case 5: m_CurrentWeaponId = WEAPON_FIVESEVEN;
		}
	}
	else if (current_weapon_group == WeaponGroup_SubMachineGun)
	{
		switch (current_weapon[current_weapon_group])
		{
		case 0: m_CurrentWeaponId = WEAPON_M249; break;
		case 1: m_CurrentWeaponId = WEAPON_TMP; break;
		case 2: m_CurrentWeaponId = WEAPON_P90; break;
		case 3: m_CurrentWeaponId = WEAPON_MP5N; break;
		case 4: m_CurrentWeaponId = WEAPON_MAC10; break;
		case 5: m_CurrentWeaponId = WEAPON_UMP45;
		}
	}
	else if (current_weapon_group == WeaponGroup_Rifle)
	{
		switch (current_weapon[current_weapon_group])
		{
		case 0: m_CurrentWeaponId = WEAPON_M4A1; break;
		case 1: m_CurrentWeaponId = WEAPON_GALIL; break;
		case 2: m_CurrentWeaponId = WEAPON_FAMAS; break;
		case 3: m_CurrentWeaponId = WEAPON_AUG; break;
		case 4: m_CurrentWeaponId = WEAPON_AK47; break;
		case 5: m_CurrentWeaponId = WEAPON_SG552;
		}
	}
	else if (current_weapon_group == WeaponGroup_Shotgun)
	{
		switch (current_weapon[current_weapon_group])
		{
		case 0: m_CurrentWeaponId = WEAPON_XM1014; break;
		case 1: m_CurrentWeaponId = WEAPON_M3;
		}
	}
	else if (current_weapon_group == WeaponGroup_Sniper)
	{
		switch (current_weapon[current_weapon_group])
		{
		case 0: m_CurrentWeaponId = WEAPON_AWP; break;
		case 1: m_CurrentWeaponId = WEAPON_SCOUT; break;
		case 2: m_CurrentWeaponId = WEAPON_G3SG1; break;
		case 3: m_CurrentWeaponId = WEAPON_SG550;
		}
	}

	// ==================================================================================================================
	// Aimbot
	// ==================================================================================================================

	static const char* const aim_accuracy[] = { "None", "Bound box", "Recoil", "Recoil + Spread" };
	static const char* const psilent_type[] = { "Manual", "Automatic fire" };
	static const char* const psilent_triggers[] = { "Standing", "On land", "In air", "Always" };

	ImGui::Control::SetStringIdentify("##LegitBot (Aimbot)");
	ImGui::Control::Text("Aimbot:");
	ImGui::Child::Begin("##LegitBot(Aimbot)", ImVec2(w.ChildWidth, 433));
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim, "Enabled", "On/off legitbot for weapon.");
	ImGui::Control::PushCondition(cvar.legitbot[m_CurrentWeaponId].aim);
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim_head, "Head", "Aiming at the hitbox head.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim_chest, "Chest", "Aiming at the hitbox chest.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim_stomach, "Stomach", "Aiming at the hitbox stomatch.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim_arms, "Arms", "Aiming at the hitbox arms.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim_legs, "Legs", "Aiming at the hitbox legs.");
	ImGui::Control::SliderInt(&cvar.legitbot[m_CurrentWeaponId].aim_reaction_time, "Reaction time", 0, 1000, "%.0f ms", "Delay before aiming to the next player after killing the previous one.");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_fov, "Maximum FOV", 0.0F, 180.0F, u8"%.1f°", 2.0F, "Maximal field of view of the aimbot.");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_smooth_auto, "Smooth (auto aim)", 0.0F, 300.0F, "%.2f", 2.0F, "Smoothing auto aiming aimbot.");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_smooth_in_attack, "Smooth in attack", 0.0F, 300.0F, "%.2f", 2.0F, "Smoothing aiming aimbot (+attack).");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_smooth_scale_fov, "Smooth scale - FOV", 0.0F, 100.0F, "%.0f%%"); // что ето такое
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_recoil_fov, "Recoil compensation max. FOV", 0.0F, 180.0F, u8"%.1f°", 2.0F, "Maximal field of view of the aimbot when working recoil compensation.");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_recoil_smooth, "Recoil compensation smooth", 0.0F, 300.0F, "%.2f", 2.0F, "Smoothing recoil compensation.");
	ImGui::Control::SliderInt(&cvar.legitbot[m_CurrentWeaponId].aim_recoil_pitch, "Recoil compensation pitch", 0, 100, "%.0f%%", "The percentage of efficiency of recoil compensation (pitch angle).");
	ImGui::Control::SliderInt(&cvar.legitbot[m_CurrentWeaponId].aim_recoil_yaw, "Recoil compensation yaw", 0, 100, "%.0f%%", "The percentage of efficiency of recoil compensation (yaw angle).");
	ImGui::Control::SliderInt(&cvar.legitbot[m_CurrentWeaponId].aim_recoil_start, "Recoil compensation start", 0, 15, cvar.legitbot[m_CurrentWeaponId].aim_recoil_start ? "%.0f bullet" : "if punchangle2D > 0");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_maximum_lock_on_time, "Maximum lock-on time", 0.0F, 10000.0F,
		cvar.legitbot[m_CurrentWeaponId].aim_maximum_lock_on_time ? "%.0f ms" : "infinity", 1.0F, "The aimbot time when the target is in the field of view of aimbot.");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_delay_before_firing, "Delay before firing", 0.0F, 1000.0F, "%.0f ms", 2.0F, "Delay before firing.");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_delay_before_aiming, "Delay before aiming", 0.0F, 1000.0F, "%.0f ms", 2.0F, "Delay before aiming.");
	ImGui::Control::SliderFloat(&cvar.legitbot[m_CurrentWeaponId].aim_psilent_angle, "Perfect silent aim max. angle", 0.0F, 1.0F, u8"%.1f°"); // что ето такое
	ImGui::Control::PushCondition(cvar.legitbot[m_CurrentWeaponId].aim_psilent_angle);
	ImGui::Control::Combo(&cvar.legitbot[m_CurrentWeaponId].aim_psilent_type, "Perfect silent type", COMBO_ARRAY(psilent_type), "Type of work perfect silent aimbot.\n - Manual;\n - Automatic fire.");
	ImGui::Control::Combo(&cvar.legitbot[m_CurrentWeaponId].aim_psilent_triggers, "Perfect silent triggers", COMBO_ARRAY(psilent_triggers)); // что ето такое
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim_psilent_tapping_mode, "Perfect silent tapping mode", "Not shooting if punchangle > 0.");// что ето такое
	ImGui::Control::PopCondition();
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].aim_penetration, "Automatic penetration", "Automatic penetration through walls.");
	ImGui::Control::Combo(&cvar.legitbot[m_CurrentWeaponId].aim_accuracy, "Accuracy boost", COMBO_ARRAY(aim_accuracy)); // что ето такое
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::EndGroup();
	ImGui::SameLine(w.NextChildPosX);
	ImGui::BeginGroup();

	// ==================================================================================================================
	// Triggerbot
	// ==================================================================================================================

	static const char* const trigger_accuracy_combo[] = { "None", "Recoil", "Recoil + Spread" };

	ImGui::Control::SetStringIdentify("##LegitBot (Triggerbot)");
	ImGui::Control::Text("Triggerbot:");
	ImGui::Child::Begin("##LegitBot(Triggerbot)", ImVec2(w.ChildWidth, 216));
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].trigger, "Enabled", "Enable/disable triggerbot for weapon.");
	ImGui::Control::PushCondition(cvar.legitbot[m_CurrentWeaponId].trigger);
	ImGui::Control::KeyAssignment(&cvar.legitbot_trigger_key, "##TriggerKey");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].trigger_head, "Head", "Scan the hitbox head.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].trigger_chest, "Chest", "Scan the hitbox chest.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].trigger_stomach, "Stomach", "Scan the hitbox stomach.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].trigger_arms, "Arms", "Scan the hitbox arms.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].trigger_legs, "Legs", "Scan the hitbox legs.");
	ImGui::Control::Checkbox(&cvar.legitbot[m_CurrentWeaponId].trigger_penetration, "Automatic penetration", "Automatic penetration through walls.");
	ImGui::Control::Combo(&cvar.legitbot[m_CurrentWeaponId].trigger_accuracy, "Accuracy boost", COMBO_ARRAY(trigger_accuracy_combo)); // что ето такое
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::Spacing();

	// ==================================================================================================================
	// Other
	// ==================================================================================================================

	static const char* const trigger_key_type[] = { "Press", "Toggle" };

	ImGui::Control::SetStringIdentify("##LegitBot (Other)");
	ImGui::Control::Text("Other:");
	ImGui::Child::Begin("##LegitBot(Other)", ImVec2(w.ChildWidth, 192));
	ImGui::Control::Checkbox(&cvar.legitbot_friendlyfire, "Friendly fire", "Fire in your teammates.");
	ImGui::Control::SliderFloat(&cvar.legitbot_flashed_limit, "Flashed limit", 0.0F, 100.0F, "%.0f%%", 1.0F, "If sf.fadealpha(%%) > limit, legitbot and trigger not working.");
	ImGui::Control::Checkbox(&cvar.legitbot_smoke_check, "Smoke check", "Checking smoke at target.");
	ImGui::Control::PushCondition(cvar.legitbot_trigger_key);
	ImGui::Control::Combo(&cvar.legitbot_trigger_key_type, "Triggerbot key type", COMBO_ARRAY(trigger_key_type), "Types of key.\n - Press;\n - Toggle.");
	ImGui::Control::PopCondition();
	ImGui::Control::SliderFloat(&cvar.legitbot_trigger_hitbox_scale, "Triggerbot hitbox scale", 1.0F, 200.0F, "%.f%%"); // что ето такое
	ImGui::Control::Checkbox(&cvar.legitbot_trigger_only_scoped, "Triggerbot only scoped", "Shoot only when opened sniper scope.");
	ImGui::Control::Checkbox(&cvar.legitbot_automatic_scope, "Legitbot automatic scope", "Automatic opening sniper scope.");
	ImGui::Control::Checkbox(&cvar.legitbot_dependence_fps, "Legitbot dependence fps", "100 fps = smooth value");
	ImGui::Control::Checkbox(&cvar.legitbot_position_adjustment, "Position adjustment", "Exploit. Not work on old build servers. Recommend set fps_max 99.5(less misses).");
	ImGui::Child::End();

	ImGui::Tab::End();
}

void Menu::Tab::Visuals()
{
	ImGui::Tab::Begin(&cvar.visuals, "##Visuals", &cvar.visuals_panic_key, state.visuals_panic);

	// ==================================================================================================================
	// ESP
	// ==================================================================================================================

	static const char* const player_box[] = { "Off", "Default box", "Corner box", "Rounded box" };
	static const char* const position[] = { "Off", "Left", "Right", "Top", "Bottom" };
	static const char* const player_distance[] = { "In units", "In meters" };
	static const char* const font_outline[] = { "Off", "Shadow", "Outline" };

	ImGui::Control::SetStringIdentify("##Visuals (ESP)");
	ImGui::Control::Text("ESP:");
	ImGui::Child::Begin("##Visuals(ESP)", ImVec2(w.ChildWidth, 258));
	ImGui::Control::Checkbox(&cvar.esp_player_enabled, "Enabled", "Global ESP on/off.");
	ImGui::Control::PushCondition(cvar.esp_player_enabled);
	ImGui::Control::Checkbox(&cvar.esp_player_teammates, "Teammates", "Display esp on teammates.");
	ImGui::Control::Combo(&cvar.esp_player_box, "Bounding box", COMBO_ARRAY(player_box), "Draw boxes on players.\n - Default box;\n - Corner box;\n - Rounded box.");
	ImGui::Control::PushCondition(cvar.esp_player_box);
	ImGui::Control::Text("Bounding box colors");
	ImGui::Control::ColorButton4(cvar.esp_player_box_color_t, "##T color"); ImGui::SameLine();
	ImGui::Control::Text("T");
	ImGui::Control::ColorButton4(cvar.esp_player_box_color_ct, "##CT color", 0.0F); ImGui::SameLine();
	ImGui::Control::Text("CT");
	ImGui::Control::Checkbox(&cvar.esp_player_box_outline, "Bounding box outline", "Draw the outline on the boxes.");
	ImGui::Control::PopCondition();
	ImGui::Control::Combo(&cvar.esp_player_name, "Name", COMBO_ARRAY(position), "Display player nicknames.\n - Left;\n - Right;\n - Top;\n - Bottom.");
	ImGui::Control::Combo(&cvar.esp_player_weapon, "Weapon", COMBO_ARRAY(position), "Display player weapons.\n - Left;\n - Right;\n - Top;\n - Bottom.");
	ImGui::Control::Combo(&cvar.esp_player_distance, "Distance", COMBO_ARRAY(position), "Show distance to players.\n - Left;\n - Right;\n - Top;\n - Bottom.");
	ImGui::Control::PushCondition(cvar.esp_player_distance);
	ImGui::Control::Combo(&cvar.esp_player_distance_measure, "Distance measurement", COMBO_ARRAY(player_distance), "The measurement of the distance to the players.\n - In units;\n - In meters.");
	ImGui::Control::PopCondition();
	ImGui::Control::Combo(&cvar.esp_player_health, "Health bar", COMBO_ARRAY(position), "Draw a player health bar.\n - Left;\n - Right;\n - Top;\n - Bottom.");
	ImGui::Control::PushCondition(cvar.esp_player_health);
	ImGui::Control::Checkbox(&cvar.esp_player_health_value, "Show numeric on health bar", "Display the number of health points on the health bar.");
	ImGui::Control::PopCondition();
	ImGui::Control::Combo(&cvar.esp_player_armor, "Armor bar", COMBO_ARRAY(position), "Draw a player armor bar.\n - Left;\n - Right;\n - Top;\n - Bottom.");
	ImGui::Control::Checkbox(&cvar.esp_player_skeleton, "Skeleton", "Draw the skeleton on the players.");
	ImGui::Control::PushCondition(cvar.esp_player_skeleton);
	ImGui::Control::ColorButton4(cvar.esp_player_skeleton_color, "##Skeleton color", 0.0F);
	ImGui::Control::PopCondition();
	ImGui::Control::Checkbox(&cvar.esp_player_out_of_pov_arrow, "Out of POV arrow", "Draw location arrows for players out of sight.");
	ImGui::Control::PushCondition(cvar.esp_player_out_of_pov_arrow);
	ImGui::Control::SliderFloat(&cvar.esp_player_out_of_pov_arrow_radius, "Out of POV arrow radius", 100.0F, 500.0F, "%.0f");
	ImGui::Control::PopCondition();
	ImGui::Control::SliderFloat(&cvar.esp_player_history_time, "History time", 0.0F, 2.5F, "%.1f sec", 1.0F, "Time to display visuals after a player's state is false.");
	ImGui::Control::Checkbox(&cvar.esp_player_fadeout_dormant, "Fade out dormant", "Smooth fading of visuals after a player’s state is false.");
	ImGui::Control::Checkbox(&cvar.esp_sound, "Visualize sounds", "Draw sound esp");
	ImGui::Control::PushCondition(cvar.esp_sound);
	ImGui::Control::SliderFloat(&cvar.esp_sound_fadeout_time, "Sound fade out time", 0.0F, 4.0F, "%.1f sec", 1.0F, "Fade out time of sound esp.");
	ImGui::Control::SliderFloat(&cvar.esp_sound_circle_size, "Sound circle size", 1.0F, 50.0F, "%.1f", 1.0F, "The size of the circles in sound esp.");
	ImGui::Control::Text("Sound colors");
	ImGui::Control::ColorButton4(cvar.esp_sound_color_t, "##Sound Terrorist color"); ImGui::SameLine();
	ImGui::Control::Text("T");
	ImGui::Control::ColorButton4(cvar.esp_sound_color_ct, "##Sound Counter-Terrorist color", 0.0F); ImGui::SameLine();
	ImGui::Control::Text("CT");
	ImGui::Control::PopCondition();
	ImGui::Control::SliderFloat(&cvar.esp_font_size, "Font size", 0.1F, 18.0F, "%.1f px");
	ImGui::Control::Combo(&cvar.esp_font_outline, "Font outline", COMBO_ARRAY(font_outline), "Draw the outline on the font.\n - Shadow;\n - Outline.");
	ImGui::Control::Text("Font color");
	ImGui::Control::ColorButton4(cvar.esp_font_color, "##Font color");
	ImGui::Control::Text("Background color");
	ImGui::Control::ColorButton4(cvar.esp_player_background_color, "##Background color");
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::Spacing();

	// ==================================================================================================================
	// Colored models
	// ==================================================================================================================

	static const char* const colored_models_type[] = { "Off", "Flat", "Wireframe", "Texture", "Material" };

	ImGui::Control::SetStringIdentify("##Visuals (Colored models)");
	ImGui::Control::Text("Colored models & glow:");
	ImGui::Child::Begin("##Visuals(Colored models)", ImVec2(w.ChildWidth, 150));
	ImGui::Control::Checkbox(&cvar.colored_models_enabled, "Enabled", "Global colored models on/off.");
	ImGui::Control::PushCondition(cvar.colored_models_enabled);
	ImGui::Control::Combo(&cvar.colored_models_player, "Player", COMBO_ARRAY(colored_models_type), "Paint players with color.\n - Flat;\n - Wireframe;\n - Texture;\n - Material.");
	ImGui::Control::PushCondition(cvar.colored_models_player);
	ImGui::Control::Checkbox(&cvar.colored_models_player_behind_wall, "Player (behind wall)", "Paint the players behind the wall with color.");
	ImGui::Control::Checkbox(&cvar.colored_models_player_teammates, "Teammates", "Paint players teammates with color.");
	ImGui::Control::Text("Visible colors");
	ImGui::Control::ColorButton4(cvar.colored_models_color_t, "##Terrorist visible color"); ImGui::SameLine();
	ImGui::Control::Text("T");
	ImGui::Control::ColorButton4(cvar.colored_models_color_ct, "##Counter-Terrorist visible color", 0.0F); ImGui::SameLine();
	ImGui::Control::Text("CT");
	ImGui::Control::PushCondition(cvar.colored_models_player_behind_wall);
	ImGui::Control::Text("Hidden colors");
	ImGui::Control::ColorButton4(cvar.colored_models_color_t_behind_wall, "##Terrorist hidden color"); ImGui::SameLine();
	ImGui::Control::Text("T");
	ImGui::Control::ColorButton4(cvar.colored_models_color_ct_behind_wall, "##Counter-Terrorist hidden color", 0.0F); ImGui::SameLine();
	ImGui::Control::Text("CT");
	ImGui::Control::PopCondition(2);
	ImGui::Control::Combo(&cvar.colored_models_hands, "Hands", COMBO_ARRAY(colored_models_type), "Paint hands with color.\n - Flat;\n - Wireframe;\n - Texture;\n - Material.");
	ImGui::Control::PushCondition(cvar.colored_models_hands);
	ImGui::Control::Text("Hands color");
	ImGui::Control::ColorButton4(cvar.colored_models_hands_color, "##Hands color");
	ImGui::Control::PopCondition();
	ImGui::Control::Combo(&cvar.colored_models_dropped_weapons, "Dropped weapons", COMBO_ARRAY(colored_models_type), "Paint weapons with color.\n - Flat;\n - Wireframe;\n - Texture;\n - Material.");
	ImGui::Control::PushCondition(cvar.colored_models_dropped_weapons);
	ImGui::Control::Text("Dropped weapons color");
	ImGui::Control::ColorButton4(cvar.colored_models_dropped_weapons_color, "##Dropped weapons color.");
	ImGui::Control::PopCondition();
	ImGui::Control::Checkbox(&cvar.colored_models_backtrack, "Backtrack position", "Paint backtrack position with color.");
	ImGui::Control::PushCondition(cvar.colored_models_backtrack);
	ImGui::Control::Text("Backtrack position color");
	ImGui::Control::ColorButton4(cvar.colored_models_backtrack_color, "##Backtrack position color");
	ImGui::Control::PopCondition(2);

	ImGui::Spacing();
	ImGui::Separator(w.ChildWidth - ImGui::GetSameLinePadding());
	ImGui::Spacing();

	ImGui::Control::Checkbox(&cvar.glow_enabled, "Glow enabled", "Global glow on/off.");
	ImGui::Control::PushCondition(cvar.glow_enabled);
	ImGui::Control::Checkbox(&cvar.glow_behind_wall, "Glow behind wall", "Paint the glow on the players behind the wall.");
	ImGui::Control::Checkbox(&cvar.glow_teammates, "Glow teammates", "Paint the glow on the players teammates.");
	ImGui::Control::Text("Glow colors");
	ImGui::Control::ColorButton3(cvar.glow_color_t, "##Terrorist glow visible color"); ImGui::SameLine();
	ImGui::Control::Text("T");
	ImGui::Control::ColorButton3(cvar.glow_color_ct, "##Counter-Terrorist glow visible color", 0.0F); ImGui::SameLine();
	ImGui::Control::Text("CT");
	ImGui::Control::SliderInt(&cvar.glow_amout, "Glow amout", 1, 128, "%.0f", "The amount of thickness of the glow.");
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::EndGroup();
	ImGui::SameLine(w.NextChildPosX);
	ImGui::BeginGroup();

	// ==================================================================================================================
	// Other
	// ==================================================================================================================

	ImGui::Control::SetStringIdentify("##Visuals (Other)");
	ImGui::Control::Text("Other:");
	ImGui::Child::Begin("##Visuals(Other)", ImVec2(w.ChildWidth, 200));
	ImGui::Control::Checkbox(&cvar.shared_esp, "Shared ESP", "Data exchange between users cheat.");
	ImGui::Control::Checkbox(&cvar.screen_log, "Screen log", "Display logs on the screen.");
	ImGui::Control::Checkbox(&cvar.world_dropped_weapons, "Dropped weapons", "Display weapons on the map.");
	ImGui::Control::Checkbox(&cvar.world_thrown_grenades, "Thrown grenades", "Display throwing grenades.");
	ImGui::Control::Checkbox(&cvar.bomb, "Bomb", "Show the bomb on the map.");
	ImGui::Control::ColorButton4(cvar.bomb_color, "##Bomb color", 0.0F);
	/*ImGui::Checkbox("Snipers crosshair", &cvar.crosshair_snipers);	// need desc
	ImGui::ColorButton4("##Snipers crosshair color", &cvar.crosshair_snipers_color[0]);*/
	ImGui::Control::Checkbox(&cvar.aimbot_fov_circle, "Aimbot FOV circle", "Display aimbot field of view.");
	ImGui::Control::ColorButton4(cvar.aimbot_fov_circle_color, "##Aimbot FOV circle color", 0.0F);
	ImGui::Control::Checkbox(&cvar.spread_circle, "Spread circle", "Display spread range.");
	ImGui::Control::ColorButton4(cvar.spread_circle_color, "##Spread circle color", 0.0F);
	ImGui::Control::Checkbox(&cvar.punch_recoil, "Punch recoil", "Display recoil point.");
	ImGui::Control::ColorButton4(cvar.punch_recoil_color, "##Punch recoil color", 0.0F);
	/*ImGui::Checkbox("Grenade trail", &cvar.grenade_trail);	// need desc
	ImGui::ColorButton4("##Grenade trail color", &cvar.grenade_trail_color[0]);
	ImGui::Checkbox("Bullet impacts", &cvar.bullet_impacts);	// need desc
	ImGui::ColorButton4("##Bullet impacts color", &cvar.bullet_impacts_color[0]);*/
	ImGui::Child::End();

	ImGui::Spacing();

	// ==================================================================================================================
	// Effects
	// ==================================================================================================================

	ImGui::Control::SetStringIdentify("##Visuals (Effects)");
	ImGui::Control::Text("Effects:");
	ImGui::Child::Begin("##Visuals(Effects)", ImVec2(w.ChildWidth, 208));
	ImGui::Control::SliderFloat(&cvar.screenfade_limit, "Screen fade limit", 0.0F, 100.0F, "%.0f%%", 1.0F, "If sf.fadealpha(%%) > limit, set sf.fadealpha = limit.");
	ImGui::Control::Checkbox(&cvar.remove_visual_recoil, "Remove visual recoil");
	ImGui::Control::SliderInt(&cvar.thirdperson, "Force third person", 0, 512, "%.0f", "Distance of the camera from the player.");
	ImGui::Control::PushCondition(cvar.thirdperson);
	ImGui::Control::Text("Force third person key");
	ImGui::Control::KeyAssignment(&cvar.thirdperson_key_toggle, "##ThirdPersonKey", true);
	ImGui::Control::PopCondition();
	ImGui::Control::Checkbox(&cvar.lightmap, "Lightmap");
	ImGui::Control::ColorButton4(cvar.lightmap_color, "##Lightmap color", 0.0F);
	//ImGui::ColorButton4("##Brightness color", &cvar.brightness_color[0]);
	ImGui::Control::Checkbox(&cvar.disable_render_teammates, "Disable render teammates", "Turning off the rendering of teammate players.");
	ImGui::Control::SliderInt(&cvar.custom_render_fov, "Custom render fov", 45, 150, "%.0f", "Changing the game field of view.");
	ImGui::Child::End();

	ImGui::Tab::End();
}

void Menu::Tab::Misc()
{
	ImGui::Tab::Begin();

	// ==================================================================================================================
	// Miscellaneous
	// ==================================================================================================================

	ImGui::Control::SetStringIdentify("##Miscellaneous (Miscellaneous)");
	ImGui::Control::Text("Miscellaneous:");
	ImGui::Child::Begin("##Miscellaneous(Miscellaneous)", ImVec2(w.ChildWidth, 168));
	ImGui::Control::Checkbox(&cvar.fakelatency_enabled, "Fake latency enabled", "Returns the position of the player in his past position.");
	ImGui::Control::PushCondition(cvar.fakelatency_enabled);
	ImGui::Control::KeyAssignment(&cvar.fakelatency_key_press, "##FakeLatencyKey");
	ImGui::Control::SliderFloat(&cvar.fakelatency_value, "Fake latency", 0.0F, 500.0F, "%.0f ms", 1.0F, "Returns a target a few milliseconds ago.\nIf your ping is very different from this value, reduce it.");
	ImGui::Control::PopCondition();
	//	ImGui::Checkbox("Disable screenshake", &cvar.disable_screenshake);	// need desc
	ImGui::Control::Checkbox(&cvar.automatic_pistol, "Automatic pistol");
	ImGui::Control::Checkbox(&cvar.automatic_reload, "Automatic reload");
	ImGui::Control::Checkbox(&cvar.name_stealer_enabled, "Name stealer enabled", "Stealing player nicknames.");
	ImGui::Control::PushCondition(cvar.name_stealer_enabled);
	ImGui::Control::SliderFloat(&cvar.name_stealer_interval, "Name stealer interval", 1.0F, 10.0F, "%.1f sec", 1.0F, "Nickname change interval.");
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::Spacing();

	// ==================================================================================================================
	// Knifebot
	// ==================================================================================================================

	static const char* const knifebot_type[] = { "Stab", "Slash" };
	static const char* const knifebot_hitbox_combo[] = { "Head", "Auto" };

	ImGui::Control::SetStringIdentify("##Miscellaneous (Knifebot)");
	ImGui::Control::Text("Knifebot:");
	ImGui::Child::Begin("##Miscellaneous(Knifebot)", ImVec2(w.ChildWidth, 267));
	ImGui::Control::Checkbox(&cvar.knifebot_enabled, "Enabled", "Automatic knife attack.");
	ImGui::Control::PushCondition(cvar.knifebot_enabled);
	ImGui::Control::KeyAssignment(&cvar.knifebot_key_toggle, "##KnifebotKey");
	ImGui::Control::Checkbox(&cvar.knifebot_friendly_fire, "Friendly fire", "Attack teammates.");
	ImGui::Control::Combo(&cvar.knifebot_type, "Type", COMBO_ARRAY(knifebot_type), "Type of attack knifebot\n - Stab;\n - Slash.");
	ImGui::Control::SliderFloat(&cvar.knifebot_distance_stab, "Distance stab", 16.0F, 100.0F, "%.2f", 1.0F, "\"+attack2\" distance.");
	ImGui::Control::SliderFloat(&cvar.knifebot_distance_slash, "Distance slash", 16.0F, 100.0F, "%.2f", 1.0F, "\"+attack\" distance.");
	ImGui::Control::SliderFloat(&cvar.knifebot_fov, "Maximum FOV", 0.0F, 180.0F, u8"%.1f°", 2.0F, "Maximal field of view of the knifebot.");
	ImGui::Control::Checkbox(&cvar.knifebot_aim_silent, "Silent aimbot", "Invisible aiming for you.");
	ImGui::Control::Checkbox(&cvar.knifebot_aim_perfect_silent, "Perfect silent aimbot", "Invisible aiming for all.");
	ImGui::Control::Combo(&cvar.knifebot_aim_hitbox, "Aimbot hitbox", COMBO_ARRAY(knifebot_hitbox_combo), "Hitbox or group of hitboxes for aiming.\n - Head;\n - Auto.");
	ImGui::Control::Checkbox(&cvar.knifebot_dont_shoot_spectators, "Don't shoot spectators");
	ImGui::Control::Checkbox(&cvar.knifebot_dont_shoot_in_back, "Don't shoot in back");
	ImGui::Control::Checkbox(&cvar.knifebot_position_adjustment, "Position adjustment");
	ImGui::Control::SliderFloat(&cvar.knifebot_hitbox_scale, "Hitbox scale", 1.0F, 100.0F, "%.f%%"); // что ето такое
	ImGui::Control::PopCondition();
	ImGui::Child::End();

	ImGui::EndGroup();
	ImGui::SameLine(w.NextChildPosX);
	ImGui::BeginGroup();

	// ==================================================================================================================
	// Other
	// ==================================================================================================================

	ImGui::Control::SetStringIdentify("##Miscellaneous (Other)");
	ImGui::Control::Text("Other:");
	ImGui::Child::Begin("##Miscellaneous(Other)", ImVec2(w.ChildWidth, 460));
	ImGui::Control::Checkbox(&cvar.hide_from_obs, "Hide from OBS", "Hiding visuals from Open Broadcaster Software.");
	ImGui::Control::Checkbox(&cvar.hud_clear, "HUD clear", "Clears server hud.");
	ImGui::Control::Checkbox(&cvar.motd_block, "MOTD block", "Block server motd.");
	ImGui::Control::Checkbox(&cvar.demochecker_bypass, "Demochecker bypass", "Bypassing bad angles detected by demochecker. (Accuracy drops)");
	ImGui::Control::Checkbox(&cvar.replace_zombie_models, "Replace zombie models");
	ImGui::Control::Checkbox(&cvar.maximize_window_after_respawn, "Maximize window after respawn");
	ImGui::Control::Checkbox(&cvar.maximize_window_in_new_round, "Maximize window in new round");	
	ImGui::Control::Checkbox(&cvar.misc.steamspoofid_enabled, "SteamID spoof enabled"); // что ето
	ImGui::Control::PushCondition(cvar.misc.steamspoofid_enabled);
	ImGui::Control::InputText(cvar.misc.steamspoofid_custom_id, "SteamID spoof custom", sizeof(cvar.misc.steamspoofid_custom_id), ImGuiInputTextFlags_CharsDecimal); // что ето
	ImGui::Control::Checkbox(&cvar.misc.steamspoofid_random, "SteamID spoof random"); // что ето
	ImGui::Control::PopCondition();
	ImGui::Control::Checkbox(&cvar.debug_console, "Debug console", "y danila 7.5 sm");
	ImGui::Control::Checkbox(&cvar.debug_visuals, "Debug visuals", "y romi 9999 sm");

	if (atoi(cvar.misc.steamspoofid_custom_id) <= 0)
		strcpy_s(cvar.misc.steamspoofid_custom_id, "1");

	ImGui::Spacing();
	ImGui::Separator(w.ChildWidth - ImGui::GetSameLinePadding());
	ImGui::Spacing();

	if (ImGui::Button("Player List", ImVec2(w.ChildWidth - ImGui::GetSameLinePadding() * 2, 22)))
		g_DrawGUI.GetWindowByName("player list").Toggle();

	//if (ImGui::Button("Enable sv_cheats (Allow cheats)", ImVec2(w.ChildWidth - ImGui::GetSameLinePadding() * 2, 22)))
	//	*g_Globals.m_pbAllowCheats = true;

	if (ImGui::Button("Unload", ImVec2(w.ChildWidth - ImGui::GetSameLinePadding() * 2, 22)))
		g_Globals.m_bUnloadLibrary = true;

	ImGui::Child::End();

	ImGui::Tab::End();
}

void Menu::Tab::Kreedz()
{
	ImGui::Tab::Begin(&cvar.kreedz.active, "##Kreedz");

	// ==================================================================================================================
	// Basics techniques
	// ==================================================================================================================

	static const char* const bunnyhop_correct_type[] = { "Efficiency adjustment", "Percent of perfect jumps", "Percent of FOG (advanced)" };
	static const char* const groundstrafe_correct_type[] = { "Efficiency adjustment", "Percent of FOG (advanced)" };

	ImGui::Control::SetStringIdentify("##Kreedz (Basics techniques)");
	ImGui::Control::Text("Basics techniques:");
	ImGui::Child::Begin("##Kreedz(Basics techniques)", ImVec2(w.ChildWidth, 433));
	ImGui::Control::Text("Bunnyhop key");
	ImGui::Control::KeyAssignment(&cvar.kreedz.bunnyhop_key, "##BhopKey", true);
	ImGui::Control::Checkbox(&cvar.kreedz.bunnyhop_scroll_emulation, "Bunnyhop scroll emulation", "Use mouse wheel scroll emulation (recommended use 100 fps).");
	ImGui::Control::PushCondition(cvar.kreedz.bunnyhop_scroll_emulation);
	ImGui::Control::Checkbox(&cvar.kreedz.bunnyhop_scroll_helper, "Bunnyhop scroll helper"); // что ето такое
	ImGui::Control::PopCondition();
	ImGui::Control::Combo(&cvar.kreedz.bunnyhop_correct_type, "Bunnyhop correct type", COMBO_ARRAY(bunnyhop_correct_type)); // что ето
	ImGui::Control::PushCondition(cvar.kreedz.bunnyhop_correct_type != 2);
	ImGui::Control::SliderInt(&cvar.kreedz.bunnyhop_correct_value, "Bunnyhop correct value", 0, 100, "%.0f", cvar.kreedz.bunnyhop_correct_type == 0 ? "The higher the efficiency, the faster and better bunnyhop." : "Percent of perfect jumps.");
	ImGui::Control::PopCondition();
	ImGui::Control::PushCondition(cvar.kreedz.bunnyhop_correct_type == 2);
	ImGui::Control::SliderInt(&cvar.kreedz.bunnyhop_frames_on_ground[0], "Bunnyhop frames on ground [1]", 0, 100 - cvar.kreedz.bunnyhop_frames_on_ground[1] - cvar.kreedz.bunnyhop_frames_on_ground[2], "%.0f"); // что
	ImGui::Control::SliderInt(&cvar.kreedz.bunnyhop_frames_on_ground[1], "Bunnyhop frames on ground [2]", 0, 100 - cvar.kreedz.bunnyhop_frames_on_ground[0] - cvar.kreedz.bunnyhop_frames_on_ground[2], "%.0f"); // что
	ImGui::Control::SliderInt(&cvar.kreedz.bunnyhop_frames_on_ground[2], "Bunnyhop frames on ground [3]", 0, 100 - cvar.kreedz.bunnyhop_frames_on_ground[0] - cvar.kreedz.bunnyhop_frames_on_ground[1], "%.0f"); // что
	ImGui::Control::PopCondition();
	ImGui::Control::SliderInt(&cvar.kreedz.bunnyhop_ground_equal, "Bunnyhop ground equal", 0, 20, cvar.kreedz.bunnyhop_ground_equal == 0 ? "Off" : "%.0f", "Check for identical jumps.");
	ImGui::Control::SliderFloat(&cvar.kreedz.bunnyhop_standup_distance, "Bunnyhop standup distance", 0.f, 50.f, cvar.kreedz.bunnyhop_standup_distance == 0 ? "Off" : "%.0f", 1.f); // что ето
	ImGui::Control::Checkbox(&cvar.kreedz.bunnyhop_break_jump_animation, "Bunnyhop break jump animation"); // что ето такое
	ImGui::Control::PushCondition(cvar.kreedz.bunnyhop_break_jump_animation);
	ImGui::Control::Checkbox(&cvar.kreedz.bunnyhop_notouch_ground_illusion, "Bunnyhop no touch ground illusion"); // что ето
	ImGui::Control::PopCondition();

	ImGui::Spacing();
	ImGui::Separator(w.ChildWidth - ImGui::GetSameLinePadding());
	ImGui::Spacing();

	ImGui::Control::Text("Groundstrafe key");
	ImGui::Control::KeyAssignment(&cvar.kreedz.gstrafe_key, "##GstrafeKey", true);
	ImGui::Control::Text("Groundstrafe standup key");
	ImGui::Control::KeyAssignment(&cvar.kreedz.gstrafe_standup_key, "##StandupGstrafeKey", true);
	ImGui::Control::Checkbox(&cvar.kreedz.gstrafe_scroll_emulation, "Groundstrafe scroll emulation", "Use mouse wheel scroll emulation (recommended use 100 fps).");
	ImGui::Control::SliderFloat(&cvar.kreedz.gstrafe_slowdown_scale, "Groundstrafe slowdown scale", 0.0F, 2.0F, "%.1f", 1.0F, "Ground strafing slowdown scale.");
	ImGui::Control::SliderFloat(&cvar.kreedz.gstrafe_slowdown_angle, "Groundstrafe slowdown angle", 0.f, 45.f, "%.1f", 1.f); // что ето
	ImGui::Control::Combo(&cvar.kreedz.gstrafe_correct_type, "Groundstrafe correct type", COMBO_ARRAY(groundstrafe_correct_type)); // что ето
	ImGui::Control::PushCondition(cvar.kreedz.gstrafe_correct_type != 1);
	ImGui::Control::SliderInt(&cvar.kreedz.gstrafe_correct_value, "Groundstrafe correct value", 0, 100, "%.0f", "The higher the efficiency, the faster and better groundstrafe.");
	ImGui::Control::PopCondition();
	ImGui::Control::PushCondition(cvar.kreedz.gstrafe_correct_type == 1);
	ImGui::Control::SliderInt(&cvar.kreedz.gstrafe_frames_on_ground[0], "Groundstrafe frames on ground [1]", 0, 100 - cvar.kreedz.gstrafe_frames_on_ground[1] - cvar.kreedz.gstrafe_frames_on_ground[2], "%.0f"); // что
	ImGui::Control::SliderInt(&cvar.kreedz.gstrafe_frames_on_ground[1], "Groundstrafe frames on ground [2]", 0, 100 - cvar.kreedz.gstrafe_frames_on_ground[0] - cvar.kreedz.gstrafe_frames_on_ground[2], "%.0f"); // что
	ImGui::Control::SliderInt(&cvar.kreedz.gstrafe_frames_on_ground[2], "Groundstrafe frames on ground [3]", 0, 100 - cvar.kreedz.gstrafe_frames_on_ground[0] - cvar.kreedz.gstrafe_frames_on_ground[1], "%.0f"); // что
	ImGui::Control::PopCondition();
	ImGui::Control::SliderInt(&cvar.kreedz.gstrafe_ground_equal, "Groundstrafe ground equal", 0, 20, cvar.kreedz.gstrafe_ground_equal == 0 ? "Off" : "%.0f", "Check for identical ducks.");
	ImGui::Control::Checkbox(&cvar.kreedz.gstrafe_jump_animation, "Groundstrafe jump animation"); // что ето такое

	ImGui::Spacing();
	ImGui::Separator(w.ChildWidth - ImGui::GetSameLinePadding());
	ImGui::Spacing();

	ImGui::Control::Text("Jumpbug key");
	ImGui::Control::KeyAssignment(&cvar.kreedz.jumpbug_key, "##JumpbugKey", true);
	ImGui::Control::SliderInt(&cvar.kreedz.jumpbug_auto_damage_min, "Jumpbug auto damage min", 0, 100, cvar.kreedz.jumpbug_auto_damage_min == 0 ? "Off" : "%.0f HP"); // что ето такое брат
	ImGui::Control::Checkbox(&cvar.kreedz.jumpbug_auto_damage_fatal, "Jumpbug auto damage fatal");
	ImGui::Control::Checkbox(&cvar.kreedz.jumpbug_slowdown_velocity, "Jumpbug slowdown velocity"); // что ето такое
	
	ImGui::Spacing();
	ImGui::Separator(w.ChildWidth - ImGui::GetSameLinePadding());
	ImGui::Spacing();

	ImGui::Control::Text("Edgebug key");
	ImGui::Control::KeyAssignment(&cvar.kreedz.edgebug_key, "##EdgebugKey", true);
	ImGui::Control::SliderFloat(&cvar.kreedz.edgebug_efficiency, "Edgebug efficiency", 0.f, 100.f, "%.0f", 1.f); // что ето
	ImGui::Control::SliderFloat(&cvar.kreedz.edgebug_auto_efficiency, "Edgebug auto efficiency", 0.f, 100.f, cvar.kreedz.edgebug_auto_efficiency == 0.f ? "Off" : "%.0f", 1.f); // что ето
	ImGui::Child::End();

	ImGui::EndGroup();
	ImGui::SameLine(w.NextChildPosX);
	ImGui::BeginGroup();

	// ==================================================================================================================
	// Strafe
	// ==================================================================================================================

	static const char* const strafe_direction[] = { "Forward", "Left side", "Right side", "Backward", "Automatic" };

	ImGui::Control::SetStringIdentify("##Kreedz (Strafe)");
	ImGui::Control::Text("Strafe:");
	ImGui::Child::Begin("##Kreedz(Strafe)", ImVec2(w.ChildWidth, 208));
	ImGui::Control::Text("Strafe invisible key");
	ImGui::Control::KeyAssignment(&cvar.kreedz.strafe_invisible_key, "##StrafeInvisibleKey", true);
	ImGui::Control::Combo(&cvar.kreedz.strafe_invisible_direction, "Strafe invisible direction", COMBO_ARRAY(strafe_direction)); // что ето
	ImGui::Control::SliderInt(&cvar.kreedz.strafe_invisible_skipframes, "Strafe invisible skip frames", 0, 100, "%.0f"); // что ето
	ImGui::Control::SliderFloat(&cvar.kreedz.strafe_invisible_fps_helper, "Strafe invisible fps helper", 0.f, 2000.f, cvar.kreedz.strafe_invisible_fps_helper == 0.f ? "Off" : "%.0f", 1.f); // что ето
	ImGui::Child::End();

	ImGui::Spacing();

	// ==================================================================================================================
	// Other
	// ==================================================================================================================

	ImGui::Control::SetStringIdentify("##Kreedz (Other)");
	ImGui::Control::Text("Other:");
	ImGui::Child::Begin("##Kreedz(Other)", ImVec2(w.ChildWidth, 200));
	// here
	ImGui::Child::End();

	ImGui::Tab::End();
}

void Menu::Tab::GUI()
{
	static int iCurrentSelectedColor = 0;
	static int iCurrentSelectedTheme = 0;
	static bool bIsCopiedState = false;
	static bool bIsFirstColorPickerRef = false;
	static float flColorBuffer[4] = { 0.0F };

	static const char* const gui_color_list[] =
	{
		"Text",
		"TextDisabled",
		"WindowBg",
		"ChildBg",
		"PopupBg",
		"Border",
		"BorderShadow",
		"FrameBg",
		"FrameBgHovered",
		"FrameBgActive",
		"TitleBg",
		"TitleBgActive",
		"TitleBgCollapsed",
		"MenuBarBg",
		"ScrollbarBg",
		"ScrollbarGrab",
		"ScrollbarGrabHovered",
		"ScrollbarGrabActive",
		"CheckMark",
		"SliderGrab",
		"SliderGrabActive",
		"Button",
		"ButtonHovered",
		"ButtonActive",
		"Header",
		"HeaderHovered",
		"HeaderActive",
		"Separator",
		"SeparatorHovered",
		"SeparatorActive",
		"ResizeGrip",
		"ResizeGripHovered",
		"ResizeGripActive",
		"CloseButton",
		"CloseButtonHovered",
		"CloseButtonActive",
		"PlotLines",
		"PlotLinesHovered",
		"PlotHistogram",
		"PlotHistogramHovered",
		"TextSelectedBg",
		"ModalWindowDarkening",
		"DragDropTarget"
	};

	const char* const color_themes[] =
	{
		"Default",
		"Simplified White Red",
		"Simplified Night Red",
		"Simplified White Orange",
		"Simplified Night Orange",
		"Simplified White Cyan",
		"Simplified Night Cyan"
	};

	const float flWidthSize = w.WindowSize.x * 0.5F - ImGui::GetSameLinePadding() * 1.5F;
	const float flButtonWidthSize = (w.ChildWidth - 7.0F - ImGui::GetSameLinePadding() * 2.0F) * 0.5F;
	const float flButtonHeightSize = 22.0F;
	const float flPickerSize = flWidthSize + 80.0F;

	ImGui::Tab::Begin();
	ImGui::BeginGroup();
	ImGui::PushItemWidth(w.ChildWidth);

	if (ImGui::ListBox("##GuiColorList", &iCurrentSelectedColor, COMBO_ARRAY(gui_color_list), 28) || !bIsFirstColorPickerRef)
	{
		const auto LastItemRect = ImGui::GetCurrentWindow()->DC.LastItemRect;
		GImGui->ColorPickerRef = ImColor(cvar.gui[iCurrentSelectedColor]);
		bIsFirstColorPickerRef = true;
	}

	ImGui::PopItemWidth();
	ImGui::EndGroup();
	ImGui::SameLine(w.ChildWidth + ImGui::GetSameLinePadding() * 0.5F);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
	ImGui::BeginChild("##PickerChild", ImVec2(flPickerSize, flPickerSize));
	ImGui::PopStyleColor();
	ImGui::ColorPicker4("##GuiColorPicker", cvar.gui[iCurrentSelectedColor], ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar |
		ImGuiColorEditFlags_AlphaPreviewHalf, &GImGui->ColorPickerRef.x);
	ImGui::Spacing(2);
	w.ChildWidth += ImGui::GetSameLinePadding()* 0.5F;
	ImGui::Child::Begin("##SettingsAddons", ImVec2(w.ChildWidth, 133));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

	if (ImGui::Button("Copy current color##GuiColor", ImVec2(flButtonWidthSize, flButtonHeightSize)))
	{
		for (size_t i = 0; i < IM_ARRAYSIZE(flColorBuffer); i++)
			flColorBuffer[i] = cvar.gui[iCurrentSelectedColor][i];
		bIsCopiedState = true;
	}

	ImGui::SameLine();

	if (!bIsCopiedState)
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

	if (ImGui::Button("Paste saved color##GuiColor", ImVec2(flButtonWidthSize, flButtonHeightSize)) && bIsCopiedState)
		for (size_t i = 0; i < IM_ARRAYSIZE(cvar.gui[iCurrentSelectedColor]); i++)
			cvar.gui[iCurrentSelectedColor][i] = flColorBuffer[i];

	if (!bIsCopiedState)
		ImGui::PopStyleColor();

	if (ImGui::Button("Set to default theme##GuiColor", ImVec2(flButtonWidthSize, flButtonHeightSize)))
		Themes::DefaultColorTheme();

	ImGui::SameLine();

	if (ImGui::Button("Restore previous color##GuiColor", ImVec2(flButtonWidthSize, flButtonHeightSize)))
		for (size_t i = 0; i < IM_ARRAYSIZE(cvar.gui[iCurrentSelectedColor]); i++)
			cvar.gui[iCurrentSelectedColor][i] = GImGui->ColorPickerRef[i];

	ImGui::Spacing(4);
	ImGui::Text("Themes:");
	ImGui::PushItemWidth(w.ChildWidth - 63.0F - ImGui::GetSameLinePadding() * 2.0F);
	ImGui::Combo("##ColorThemes", &iCurrentSelectedTheme, COMBO_ARRAY(color_themes));
	ImGui::PopItemWidth();
	ImGui::SameLine();

	if (ImGui::Button("Apply##ColorThemes", ImVec2(50, 16)))
	{
		switch (iCurrentSelectedTheme)
		{
		case 0: Themes::DefaultColorTheme(); break;
		case 1: Themes::SimplifiedWhiteRed(); break;
		case 2: Themes::SimplifiedNightRed(); break;
		case 3: Themes::SimplifiedWhiteOrange(); break;
		case 4: Themes::SimplifiedNightOrange(); break;
		case 5: Themes::SimplifiedWhiteCyan(); break;
		case 6: Themes::SimplifiedNightCyan();
		}
	}

	ImGui::PopStyleColor();
	ImGui::Child::End();
	w.ChildWidth -= ImGui::GetSameLinePadding()* 0.5F;
	ImGui::EndChild();
	ImGui::Tab::End();
}

void Menu::Tab::Settings()
{
	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];
	static char szFileContents[104448];

	RefreshSettings();

	ImGui::Spacing(4);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
	ImGui::BeginChild("##SettingsElements", ImVec2(440, 460));
	ImGui::PopStyleColor();
	ImGui::BeginGroup();
	ImGui::Text("Settings:");
	ImGui::PushItemWidth(216);
	ImGui::Control::ListBoxArray("##SettingsList", reinterpret_cast<int*>(&iSelectedSettings), sSettingsList, 13);
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Files:");
	ImGui::PushItemWidth(216);
	ImGui::ListBox("##FilesList", reinterpret_cast<int*>(&iSelectedIniFile), COMBO_ARRAY(pcszFilesList));
	ImGui::PopItemWidth();
	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

	if (ImGui::Button("Load##Settings", ImVec2(104, 22)))
		Settings::Load();

	ImGui::SameLine();

	if (ImGui::Button("Save##Settings", ImVec2(104, 22)))
		Settings::Save();

	ImGui::Spacing();

	if (ImGui::Button("Restore##Settings", ImVec2(104, 22)))
		Settings::RestorePopupOpen();

	ImGui::SameLine();

	if (ImGui::Button("Save as##Settings", ImVec2(104, 22)))
		Settings::SaveAsPopupOpen();

	ImGui::Spacing();

	if (ImGui::Button("Remove##Settings", ImVec2(104, 22)))
		Settings::RemovePopupOpen();

	ImGui::SameLine();

	if (ImGui::Button("Rename##Settings", ImVec2(104, 22)))
		Settings::RenamePopupOpen();

	ImGui::Spacing();

	if (ImGui::Button("Open folder##Settings", ImVec2(104, 21)))
		Settings::OpenSettingsFolder();

	ImGui::SameLine();

	if (ImGui::Button("Open file##Settings", ImVec2(104, 21)))
		Settings::OpenSettingsFile();

	ImGui::PopStyleColor();
	ImGui::EndGroup();
	ImGui::Spacing(2);
	ImGui::Child::Begin("##SettingsAddons", ImVec2(0, 202));
	ImGui::Spacing(4);
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::BeginGroup();
	ImGui::Checkbox("Automatic saving settings when quit", &cvar.settings.autosave_when_quit);
	ImGui::Spacing();
	ImGui::Checkbox("Automatic saving settings when unhooking", &cvar.settings.autosave_when_unhook);
	ImGui::Spacing();
	ImGui::Checkbox("Use custom settings for first loading", &cvar.settings.firstload_custom);
	ImGui::Spacing();
	ImGui::SameLine(20);
	ImGui::PushItemWidth(190);

	if (cvar.settings.firstload_iter > static_cast<int>(sSettingsList.size()) - 1)
		cvar.settings.firstload_iter = static_cast<int>(sSettingsList.size()) - 1;

	if (strcmp(sSettingsList.at(cvar.settings.firstload_iter).c_str(), cvar.settings.firstload_custom_name))
	{
		for (size_t i = 0; i < sSettingsList.size(); i++)
			if (!strcmp(sSettingsList.at(i).c_str(), cvar.settings.firstload_custom_name))
				cvar.settings.firstload_iter = static_cast<int>(i);
	}

	if (!cvar.settings.firstload_custom)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50F, 0.50F, 0.50F, 1.00F));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14F, 0.14F, 0.14F, 0.75F));
	}

	ImGui::Control::ComboArray("##SettingsFirstLoad", &cvar.settings.firstload_iter, sSettingsList);
	ImGui::PopItemWidth();

	strcpy(cvar.settings.firstload_custom_name, cvar.settings.firstload_custom ?
		sSettingsList.at(cvar.settings.firstload_iter).c_str() : "Default");

	if (!cvar.settings.firstload_custom)
	{
		const auto id = ImGui::GetCurrentWindow()->GetID("##SettingsFirstLoad");

		if (ImGui::IsPopupOpen(id))
			ImGui::ClosePopup(id);

		ImGui::PopStyleColor(2);
	}

	ImGui::Spacing(2);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

	if (ImGui::Button("Refresh", ImVec2(104, 22)))
		g_Settings.RefreshGlobals();

	ImGui::SameLine();

	if (ImGui::Button("Apply", ImVec2(104, 22)))
		g_Settings.ApplyingGlobals();

	ImGui::PopStyleColor();
	ImGui::Spacing(2);
	ImGui::EndGroup();
	ImGui::Child::End();

	if (ImGui::ModalPopup::Begin("Save as##Modal", w.ModalSize))
		ModalPopup::SettingsSaveAs();

	if (ImGui::ModalPopup::Begin("Rename##Modal", w.ModalSize))
		ModalPopup::SettingsRename();

	if (ImGui::ModalPopup::Begin("Remove##Modal", w.ModalSize))
		ModalPopup::SettingsRemove();

	if (ImGui::ModalPopup::Begin("Restore##Modal", w.ModalSize))
		ModalPopup::SettingsRestore();

	ImGui::EndChild();

	if (bUpdateFileContent)
	{
		RtlSecureZeroMemory(szFileContents, sizeof(szFileContents));

		std::string file_path_str;
		file_path_str.append(g_Globals.m_sSettingsPath);
		file_path_str.append(sSettingsList.at(iSelectedSettings));
		file_path_str.append("\\");
		file_path_str.append(pcszFilesList[iSelectedIniFile]);

		if (g_Utils.FileExists(file_path_str.c_str()))
			ReadFromFile(file_path_str.c_str(), szFileContents);
	}

	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Spacing(4);
	ImGui::PushFont(ProggyClean_13px);
	ImGui::InputTextMultiline("##FileContent", szFileContents, sizeof(szFileContents), ImVec2(378, 439), ImGuiInputTextFlags_AllowTabInput);
	ImGui::PopFont();
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::SameLine(ImGui::GetSameLinePadding());
	ImGui::PushItemWidth(714);
	ImGui::InputText("##SettingsPath", const_cast<char*>(g_Globals.m_sSettingsPath.c_str()),
		g_Globals.m_sSettingsPath.size() + 1, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());

	if (ImGui::Button("Open path##Settings", ImVec2(104, 15)))
		Settings::OpenSettingsPath();

	ImGui::PopStyleColor();
}

static void DrawTitle()
{
	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];
	auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, w.Title);
	auto current_time = std::chrono::system_clock::now();
	const auto current_time_t = std::chrono::system_clock::to_time_t(current_time);
	const char* pcsztime = std::ctime(&current_time_t);

	ImGui::PushFont(Verdana_16px);
	ImGui::Spacing(2);
	ImGui::SameLine(6.0F);
	ImGui::Text("Online: %s", g_Globals.m_sOnlineUsers.c_str());
	ImGui::SameLine((w.WindowSize.x - text_size.x) * 0.5F);
	ImGui::Text(w.Title);
	text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, pcsztime);
	ImGui::SameLine(w.WindowSize.x - text_size.x - 6.0F);
	ImGui::Text("%s", pcsztime);
	ImGui::Separator();
	ImGui::PopFont();
}

void Menu::DrawMenu()
{
	ApplyWindowParameters();
	ApplyColorsChanges();

	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(w.WindowSize);

	if (ImGui::Begin("##menu", static_cast<bool*>(0), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders))
	{
		DrawTitle();
		Tab::Labels();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::BeginChild("##Controls");
		ImGui::PopStyleColor();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1));

		switch (Tab::GetCurrentTab())
		{
		case Tab_RageBot: Tab::RageBot(); break;
		case Tab_LegitBot: Tab::LegitBot(); break;
		case Tab_Visuals: Tab::Visuals(); break;
		case Tab_Misc: Tab::Misc(); break;
		case Tab_Kreedz: Tab::Kreedz(); break;
		case Tab_GUI: Tab::GUI(); break;
		case Tab_Settings: Tab::Settings();
		}

		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
	}
}

void Menu::DrawCursor()
{
	static bool bShowCursorState = false;
	static int nFixCenterCursor = 0;

	auto& io = ImGui::GetIO();
	const bool bIsDrawing = g_DrawGUI.IsDrawing() && g_DrawGUI.GetWindowByName("menu").IsVisible();

	if (io.MouseDrawCursor = bIsDrawing)
	{
		g_pISurface->SetCursor(dc_arrow);
		g_pISurface->UnlockCursor();

		if (!cvar.hide_from_obs || !g_pIGameUI->IsGameUIActive())
		{
			if (!bShowCursorState)
			{
				ShowCursor(false);
				bShowCursorState = true;
			}

			if (nFixCenterCursor > 0)
			{
				nFixCenterCursor--;
				g_pISurface->SurfaceSetCursorPos(g_Screen.iWidth / 2, g_Screen.iHeight / 2);
			}
		}
	}
	else
	{
		nFixCenterCursor = 2;

		if (!g_pIGameUI->IsGameUIActive())
		{
			g_pISurface->SetCursor(dc_none);
			g_pISurface->LockCursor();
		}

		if (bShowCursorState)
		{
			ShowCursor(true);
			bShowCursorState = false;
		}
	}
}

void Menu::DrawMenuFade()
{
	static auto flFrames = 0.0F;
	const float flWidth = static_cast<float>(g_Screen.iWidth);
	const float flHeight = static_cast<float>(g_Screen.iHeight);
	const float flDuration = 18.0F;
	const float flMaxAlpha = g_pIRunGameEngine->IsInGame() ? 0.25F : 0.75F;

	if (g_DrawGUI.IsFadeout())
	{
		flFrames += 1.0F * (100.0F / ImGui::GetIO().Framerate);

		if (flFrames <= flDuration)
		{
			const float flAlpha = g_Utils.fInterp(1.0F, flFrames, flDuration, g_DrawGUI.IsDrawing() ?
				0.0F : flMaxAlpha, g_DrawGUI.IsDrawing() ? flMaxAlpha : 0.0F);

			g_Drawing.AddRectFilled(ImVec2(), ImVec2(flWidth, flHeight), ImVec4(0.00F, 0.00F, 0.00F, flAlpha));
		}
		else
		{
			const float flAlpha = g_DrawGUI.IsDrawing() ? flMaxAlpha : 0.0F;

			g_Drawing.AddRectFilled(ImVec2(), ImVec2(flWidth, flHeight), ImVec4(0.00F, 0.00F, 0.00F, flAlpha));
			g_DrawGUI.FadeStopping();

			if (flFrames > 0.0F)
				flFrames = 0.0F;
		}
	}
	else if (g_DrawGUI.IsDrawing())
		g_Drawing.AddRectFilled(ImVec2(), ImVec2(flWidth, flHeight), ImVec4(0.00F, 0.00F, 0.00F, flMaxAlpha));
}

void Menu::DrawPlayerList()
{
	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(600, 500));

	if (ImGui::Begin("##playerlist", static_cast<bool*>(0), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders))
	{
		const char* szTitle = "Player list";
		const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];
		auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, szTitle);

		ImGui::PushFont(Verdana_16px);
		ImGui::Spacing(2);
		ImGui::SameLine((ImGui::GetWindowWidth() - text_size.x) * 0.5F);
		ImGui::Text(szTitle);
		const auto close_button_pos = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - 11.0F, ImGui::GetWindowPos().y + 11.0F);
		if (ImGui::CloseButton(ImGui::GetID("##playerlist"), close_button_pos, 8.0F))
			g_DrawGUI.GetWindowByName("player list").Hide();
		ImGui::Separator();
		ImGui::PopFont();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::BeginChild("##Frame");
		ImGui::PopStyleColor();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1));

		static const char* const playerlist_type[] = { "None", "Whitelist", "Priority" };

		ImGui::Tab::Begin(&cvar.player_list, "##PlayerList");
		ImGui::Child::Begin("##ChildList", ImVec2(576, 420));
		ImGui::Control::PushCondition(cvar.player_list);

		for (int i = 1; i <= MAX_CLIENTS; i++)
		{
			if (i == g_Local.m_iIndex)
				continue;

			auto* const pPlayer = g_World.GetPlayer(i);

			if (!pPlayer)
				continue;

			if (!pPlayer->m_bIsConnected)
				continue;

			ImVec4 color_text;

			switch (pPlayer->m_iTeam)
			{
			case TERRORIST: color_text = ImVec4(1.00f, 0.25f, 0.25f, 1.00f); break;
			case CT:		color_text = ImVec4(0.25f, 0.25f, 1.00f, 1.00f); break;
			default:		color_text = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			}

			ImGui::Control::PushTextColor(color_text);
			ImGui::Control::Combo(&pPlayer->m_iPlayerListType, pPlayer->m_szPrintName, COMBO_ARRAY(playerlist_type));
			ImGui::Control::PopTextColor();
		}

		ImGui::Control::PopCondition();
		ImGui::Child::End();
		ImGui::Tab::End();

		ImGui::Spacing(4);

		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
	}
}