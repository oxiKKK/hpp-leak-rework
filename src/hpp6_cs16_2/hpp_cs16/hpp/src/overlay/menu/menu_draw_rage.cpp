#include "framework.h"

enum AntiAimFeaturesList { ANTIAIM_GLOBALS, ANTIAIM_STAND, ANTIAIM_MOVE };

static const char* AntiAimFeatures[] = { "GLOBALS", "STAND", "MOVE" };

extern void WeaponCombo(int selected_weapon_group, int* selected_weapon);
extern void ChangeToCurrentWeapon(int* selected_weapon_group, int* selected_weapon);
extern void GetCurrentWeaponId(int selected_weapon_group, int selected_weapon, int* current_weapon_id);
extern void ChangeValuesWholeGroup(int selected_weapon_group, int current_weapon_id);

namespace AntiAim
{
	static void DrawGlobals(const std::unique_ptr<CMenuGui>& m_pGui)
	{
		const char* aa_roll[] = { "None", "Sideways 50°", "Sideways 90°", "Sideways 180°", "Static" };
	
		m_pGui->SetID("##rage_antiaimbot_globals");
		m_pGui->Checkbox(&cvars::ragebot.aa_enabled, "Enabled");
		//m_pGui->PushCondition(cvars::ragebot.aa_enabled);
		m_pGui->Checkbox(&cvars::ragebot.aa_teammates, "Teammates");
		//m_pGui->Checkbox(&cvars::ragebot.aa_at_targets, "At targets");
		//m_pGui->Checkbox(&cvars::ragebot.aa_on_knife, "On knife");
		//m_pGui->Checkbox(&cvars::ragebot.aa_on_grenades, "On grenades");
		//m_pGui->PushTextColor(ImVec4(1.00f, 1.00f, 0.00f, GImGui->Style.Colors[ImGuiCol_Text].w));
		m_pGui->Combo(&cvars::ragebot.aa_roll, "Roll", aa_roll, IM_ARRAYSIZE(aa_roll), "ENG: It works only on special servers. (Example: hvh.hpp.ovh:27015)\nRUS: Работает только на специальных серверах (Например: hvh.hpp.ovh:27015).");
		//m_pGui->PushCondition(cvars::ragebot.aa_roll == 4);
		m_pGui->Slider(&cvars::ragebot.aa_roll_static, "Roll static", -180.f, 180.f, "%.1f°", 1.f);
		//m_pGui->PopCondition();
		//m_pGui->PopTextColor();
		m_pGui->Checkbox(&cvars::ragebot.aa_untrusted_checks, "Untrusted checks", "ENG: Use only trusted angles.\nRUS: Использовать только надежные углы.");
		//m_pGui->PopCondition();
			
	}

	static void DrawStand(const std::unique_ptr<CMenuGui>& m_pGui)
	{
		const char* aa_side[] = { "Left", "Right" };
		const char* aa_stand_pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" };
		const char* aa_stand_yaw[] = { "None", "Backwards", "Sideways 90°", "Sideways 140°", "Static", "Local view", "Backwards desyns", };
		const char* aa_stand_desync[] = { "None", "Default", "Max delta" };

		m_pGui->SetID("##rage_antiaimbot_stand");
		//m_pGui->PushCondition(cvars::ragebot.aa_enabled);
		m_pGui->Combo(&cvars::ragebot.aa_stand_pitch, "Pitch", aa_stand_pitch, IM_ARRAYSIZE(aa_stand_pitch));
		m_pGui->Combo(&cvars::ragebot.aa_stand_yaw, "Yaw", aa_stand_yaw, IM_ARRAYSIZE(aa_stand_yaw));
		//m_pGui->PushCondition(cvars::ragebot.aa_stand_yaw == 4);
		m_pGui->Slider(&cvars::ragebot.aa_stand_yaw_static, "Yaw static", -180.f, 180.f, "%.1f°", 1.f);
		//m_pGui->PopCondition();
		m_pGui->Combo(&cvars::ragebot.aa_stand_desync, "Desync", aa_stand_desync, IM_ARRAYSIZE(aa_stand_desync));
		m_pGui->Checkbox(&cvars::ragebot.aa_stand_desync_helper, "Desync helper");
		m_pGui->Combo(&cvars::ragebot.aa_side, "Side", aa_side, IM_ARRAYSIZE(aa_side));
		//m_pGui->KeyOnlyToggle(&cvars::ragebot.aa_side_key, "Side switch key");
		m_pGui->Checkbox(&cvars::ragebot.aa_side_switch_when_take_damage, "Side switch when take damage");
		//m_pGui->PopCondition();
		
	}

	static void DrawMove(const std::unique_ptr<CMenuGui>& m_pGui)
	{
		const char* aa_move_pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" };
		const char* aa_move_yaw[] = { "None", "Backwards", "Local view", "Gait sideways 120°" };

		m_pGui->SetID("##rage_antiaimbot_move");
		//m_pGui->PushCondition(cvars::ragebot.aa_enabled);
		m_pGui->Combo(&cvars::ragebot.aa_move_pitch, "Pitch", aa_move_pitch, IM_ARRAYSIZE(aa_move_pitch));
		m_pGui->Combo(&cvars::ragebot.aa_move_yaw, "Yaw", aa_move_yaw, IM_ARRAYSIZE(aa_move_yaw));
		//m_pGui->PopCondition();
		
	}
}

void CMenu::DrawRage()
{
	//static int selected_weapon_group, hovered_weapon_group;
	//static int selected_weapon[IM_ARRAYSIZE(WpnLabels)];
	//static int selected_antiaim, hovered_antiaim;
	//static int current_weapon_id;
	//static bool auto_change_current_weapon;

	//ImVec2 child_size;

	//child_size.x = Math::TranslateToEvenGreatest(GImGui->CurrentWindow->Size.x - WINDOW_PADDING_X * 3.f) * 0.5f;
	//child_size.y = (GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_X) * 0.50f;

	//const float features_list_item_width = child_size.x - WINDOW_PADDING_X * 2.f;

	//ImGui::SameLine(WINDOW_PADDING_X);

	//m_pGui->TabWeaponList(WpnLabels, IM_ARRAYSIZE(WpnLabels), &selected_weapon_group, &hovered_weapon_group);

	//ImGui::SameLine(0, WINDOW_PADDING_X);
	//ImGui::BeginGroup();
	//ImGui::SetNextItemWidth(GImGui->CurrentWindow->Size.x - ImGui::GetCursorPosX() - WINDOW_PADDING_X);

	//WeaponCombo(selected_weapon_group, &selected_weapon[selected_weapon_group]);

	//ImGui::Checkbox("Automatically change to current weapon", &auto_change_current_weapon);

	//if (auto_change_current_weapon)
	//	ChangeToCurrentWeapon(&selected_weapon_group, &selected_weapon[selected_weapon_group]);

	//ImGui::EndGroup();

	//GetCurrentWeaponId(selected_weapon_group, selected_weapon[selected_weapon_group], &current_weapon_id);

	//m_pGui->BeginCheckChangeValue();

	//ImGui::Spacing(WINDOW_PADDING_Y);
	//ImGui::SameLine(0, WINDOW_PADDING_X);
	//ImGui::BeginGroup();

	//const char* raim_type[] = { "Visible", "Visible (Recoil / Spread)", "Silent", "Perfect silent" };
	//const char* raim_auto_scope[] = { "None", "No unzoom", "Unzoom" };
	//const char* raim_remove_spread[] = { "Pitch / Yaw", "Pitch / Roll", "Pitch / Yaw / Roll" };
	//const char* raim_resolver_pitch[] = { "None", "Breakpoint 90°" };
	//const char* raim_resolver_yaw[] = { "None", "Desyns bruteforce" };

	//if (cvars::ragebot.active && cvars::legitbot.active)
	//	cvars::legitbot.active = false;

	//m_pGui->BeginChild("General", child_size);
	//m_pGui->SetID("##rage_general");
	//m_pGui->Checkbox(&cvars::ragebot.active, "Enabled");
	////m_pGui->PushCondition(cvars::ragebot.active);
	//m_pGui->Slider(&cvars::ragebot.raim_fov, "Maximum FOV", 0.f, 180.f, "%.1f°", 1.f);
	////m_pGui->PushCondition(cvars::ragebot.raim_fov > 0.f);
	//m_pGui->Checkbox(&cvars::ragebot.friendly_fire, "Friendly fire");
	//m_pGui->Checkbox(&cvars::ragebot.raim_auto_fire, "Automatic fire");
	//m_pGui->Combo(&cvars::ragebot.raim_type, "Aim type", raim_type, IM_ARRAYSIZE(raim_type));
	//m_pGui->Combo(&cvars::ragebot.raim_remove_spread, "Remove spread", raim_remove_spread, IM_ARRAYSIZE(raim_remove_spread));
	//m_pGui->Combo(&cvars::ragebot.raim_auto_scope, "Automatic scope", raim_auto_scope, IM_ARRAYSIZE(raim_auto_scope));
	////m_pGui->Checkbox(&cvars::ragebot.raim_delayshot, "Delay shot");
	//m_pGui->Checkbox(&cvars::ragebot.raim_tapping_mode, "Tapping mode");
	//m_pGui->Checkbox(&cvars::ragebot.raim_optimize_fps, "Optimize FPS");
	////m_pGui->Key(&cvars::ragebot.raim_force_body_key, "Force body aim");
	//m_pGui->Combo(&cvars::ragebot.raim_resolver_pitch, "Pitch resolver", raim_resolver_pitch, IM_ARRAYSIZE(raim_resolver_pitch));
	////m_pGui->Combo(&cvars::ragebot.raim_resolver_yaw, "Yaw resolver", resolver_yaw, IM_ARRAYSIZE(resolver_yaw));
	////m_pGui->PopCondition(2);
	//
	//m_pGui->EndChild();

	//ImGui::Spacing(WINDOW_PADDING_Y - 1);

	//child_size.y = GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_X;

	//m_pGui->BeginChild("Anti-Aimbot", child_size);
	////m_pGui->TabFeaturesList(AntiAimFeatures, IM_ARRAYSIZE(AntiAimFeatures), &selected_antiaim, &hovered_antiaim, features_list_item_width);
	////m_pGui->PushCondition(cvars::ragebot.active);
	//
	//switch (selected_antiaim)
	//{
	//case ANTIAIM_GLOBALS:	AntiAim::DrawGlobals(m_pGui); break;
	//case ANTIAIM_STAND:	AntiAim::DrawStand(m_pGui); break;
	//case ANTIAIM_MOVE:  AntiAim::DrawMove(m_pGui);
	//}

	////m_pGui->PopCondition();
	//m_pGui->EndChild();

	//ImGui::EndGroup();
	//ImGui::SameLine(0, WINDOW_PADDING_X);
	//ImGui::BeginGroup();

	//child_size.y = (GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_X) * 0.65f;

	//m_pGui->BeginChild("Aimbot", child_size);
	//m_pGui->SetID("##rage_aimbot");
	////m_pGui->PushCondition(cvars::ragebot.active && cvars::ragebot.raim_fov > 0.f);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].raim_enabled, "Enabled");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].raim_enabled);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].raim_head, "Head");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].raim_head);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].raim_head_scale, "Head points scale", 0.f, 100.f, "%.0f%%", 1.f);
	////m_pGui->PopCondition();
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].raim_chest, "Chest");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].raim_chest);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].raim_chest_scale, "Chest points scale", 0.f, 100.f, "%.0f%%", 1.f);
	////m_pGui->PopCondition();
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].raim_stomach, "Stomach");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].raim_stomach);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].raim_stomach_scale, "Stomach points scale", 0.f, 100.f, "%.0f%%", 1.f);
	////m_pGui->PopCondition();
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].raim_arms, "Arms");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].raim_arms);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].raim_arms_scale, "Arms points scale", 0.f, 100.f, "%.0f%%", 1.f);
	////m_pGui->PopCondition();
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].raim_legs, "Legs");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].raim_legs);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].raim_legs_scale, "Legs points scale", 0.f, 100.f, "%.0f%%", 1.f);
	////m_pGui->PopCondition();
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].raim_head || cvars::weapons[current_weapon_id].raim_chest || cvars::weapons[current_weapon_id].raim_stomach || cvars::weapons[current_weapon_id].raim_legs || cvars::weapons[current_weapon_id].raim_arms);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].raim_auto_penetration, "Automatic penetration");
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].raim_auto_penetration_min_damage, "Automatic penetration min. damage", 1, 100, "%i hp");
	////m_pGui->PopCondition(3);
	//
	//m_pGui->EndChild();

	//ImGui::Spacing(WINDOW_PADDING_Y - 1);

	//child_size.y = GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_X;

	//const char* fakelag_triggers[] = { "On land", "In air", "On land + In air" };
	//const char* fakelag_type[] = { "Maximum", "Break lag compensation" };

	//m_pGui->BeginChild("Fakelag", child_size);
	//m_pGui->SetID("##rage_fakelag");
	////m_pGui->PushCondition(cvars::ragebot.active);
	//m_pGui->Checkbox(&cvars::ragebot.fakelag_enabled, "Enabled");
	////m_pGui->PushCondition(cvars::ragebot.fakelag_enabled);
	//m_pGui->Combo(&cvars::ragebot.fakelag_type, "Type", fakelag_type, IM_ARRAYSIZE(fakelag_type));
	////m_pGui->Combo(&cvars::ragebot.fakelag_triggers, "Triggers", fakelag_triggers, IM_ARRAYSIZE(fakelag_triggers));
	//m_pGui->Slider(&cvars::ragebot.fakelag_choke_limit, "Choke limit: ", 1, MAX_TOTAL_CMDS, "%i cmd", "Server max commands: " + std::to_string(g_pGlobals->m_nNewCommands));
	//m_pGui->Checkbox(&cvars::ragebot.fakelag_while_shooting, "While shooting");
	//m_pGui->Checkbox(&cvars::ragebot.fakelag_on_enemy_in_pvs, "On enemy in PVS");
	////m_pGui->PushCondition(!cvars::ragebot.raim_optimize_fps);
	//m_pGui->Checkbox(&cvars::ragebot.fakelag_on_peek, "On peek");
	////m_pGui->PopCondition();
	////m_pGui->PopCondition(2);
	//
	//m_pGui->EndChild();

	//ImGui::EndGroup();

	//if (!selected_weapon[selected_weapon_group] && m_pGui->IsChangedValue())
	//	ChangeValuesWholeGroup(selected_weapon_group, current_weapon_id);

	//m_pGui->EndCheckChangeValue();
}