#include "framework.h"

enum GeneralFeaturesList { GENERAL_AIM, GENERAL_TRIGGER };

static const char* GeneralFeatures[] = { "AIM", "TRIGGER" };

extern void WeaponCombo(int selected_weapon_group, int* selected_weapon);
extern void ChangeToCurrentWeapon(int* selected_weapon_group, int* selected_weapon);
extern void GetCurrentWeaponId(int selected_weapon_group, int selected_weapon, int* current_weapon_id);
extern void ChangeValuesWholeGroup(int selected_weapon_group, int current_weapon_id);

namespace General
{
	static void DrawAim(const std::unique_ptr<CMenuGui>& m_pGui)
	{
		const char* aim_recoil_standalone[] = { "None", "Default", "Return angles" };

		m_pGui->SetID("##legit_general_aim");
		m_pGui->Checkbox(&cvars::legitbot.aim_auto_scope, "Automatic scope");
		m_pGui->Checkbox(&cvars::legitbot.aim_smooth_independence_fps, "Smooth independence fps", "ENG: Turns off smoothing angles dependency on FPS.\nRUS: Выключает зависимость сглаживания от FPS.");
		m_pGui->Slider(&cvars::legitbot.target_switch_delay, "Reaction time", 0, 2000, "%i ms");
		m_pGui->Slider(&cvars::legitbot.aim_block_attack_after_kill, "Block attack after kill", 0, 2000, "%i ms");
		m_pGui->Checkbox(&cvars::legitbot.aim_dont_shoot_in_shield, "Don't shoot in shield");
		m_pGui->Checkbox(&cvars::legitbot.aim_demochecker_bypass, "Demochecker bypass", "ENG: Sets only the correct angles.\nRUS: Устанавливает только правильные углы.");
		m_pGui->Combo(&cvars::legitbot.aim_recoil_standalone, "Recoil comp. standalone", aim_recoil_standalone, IM_ARRAYSIZE(aim_recoil_standalone));
		////m_pGui->Key(&cvars::legitbot.aim_psilent_key, "Perfect silent key");
		
	}

	static void DrawTrigger(const std::unique_ptr<CMenuGui>& m_pGui)
	{
		m_pGui->SetID("##legit_general_trigger");
		m_pGui->Slider(&cvars::legitbot.trigger_hitbox_scale, "Hitbox scale", 1.f, 200.f, "%.0f%%", 1.f);
		m_pGui->Checkbox(&cvars::legitbot.trigger_only_scoped, "Only scoped");
		m_pGui->Checkbox(&cvars::legitbot.trigger_accurate_traces, "Accurate traces", "ENG: Does not shoot if the selected hitbox overlaps an unselected hitbox.\nRUS: Не стреляет, если выбранный хитбокс перекрывает невыбранный хитбокс.");
		
	}
}

void CMenu::DrawLegit()
{
	//static int selected_weapon_group, hovered_weapon_group;
	//static int selected_weapon[IM_ARRAYSIZE(WpnLabels)];
	//static int selected_general, hovered_general;
	//static int current_weapon_id;
	//static bool auto_change_current_weapon;

	//ImVec2 child_size;

	//child_size.x = Math::TranslateToEvenGreatest(GImGui->CurrentWindow->Size.x - WINDOW_PADDING_X * 3.f) * 0.5f;
	//child_size.y = (GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_X) * 0.53f;

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

	//if (cvars::legitbot.active && cvars::ragebot.active)
	//	cvars::ragebot.active = false;

	//m_pGui->BeginChild("General", child_size);
	//m_pGui->SetID("##legit_general");
	//m_pGui->Checkbox(&cvars::legitbot.active, "Enabled", "Global legitbot on/off.");
	//m_pGui->Checkbox(&cvars::legitbot.friendly_fire, "Friendly fire");
	////m_pGui->PushTextColor(ImVec4(1.00f, 1.00f, 0.00f, GImGui->Style.Colors[ImGuiCol_Text].w));
	//m_pGui->Checkbox(&cvars::legitbot.position_adjustment, "Position adjustment", "ENG: Exploit allowing you to shoot at multiple positions of the target.\nRUS: Эксплоит позволяющий стрелять в несколько позиций цели.");
	////m_pGui->PopTextColor();
	//m_pGui->Checkbox(&cvars::legitbot.desync_helper, "Desync helper");

	//ImGui::Spacing();

	////m_pGui->TabFeaturesList(GeneralFeatures, IM_ARRAYSIZE(GeneralFeatures), &selected_general, &hovered_general, features_list_item_width);
	////m_pGui->PushCondition(cvars::legitbot.active);

	//switch (selected_general)
	//{
	//case GENERAL_AIM:	  General::DrawAim(m_pGui); break;
	//case GENERAL_TRIGGER: General::DrawTrigger(m_pGui);
	//}

	////m_pGui->PopCondition();
	//
	//m_pGui->EndChild();

	//ImGui::Spacing(WINDOW_PADDING_Y - 1);

	//child_size.y = GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_X;

	//const char* trigger_accuracy_boost[] = { "None", "Recoil", "Recoil / Spread" };

	//m_pGui->BeginChild("Triggerbot", child_size);
	//m_pGui->SetID("##legit_triggerbot");
	////m_pGui->PushCondition(cvars::legitbot.active);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].trigger_enabled, "Enabled");
	//////m_pGui->Key(&cvars::legitbot.trigger_key, "Trigger key");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].trigger_enabled);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].trigger_head, "Head");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].trigger_chest, "Chest");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].trigger_stomach, "Stomach");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].trigger_arms, "Arms");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].trigger_legs, "Legs");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].trigger_head || cvars::weapons[current_weapon_id].trigger_chest || cvars::weapons[current_weapon_id].trigger_stomach || cvars::weapons[current_weapon_id].trigger_legs || cvars::weapons[current_weapon_id].trigger_arms);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].trigger_auto_penetration, "Automatic penetration");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].trigger_auto_penetration);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].trigger_auto_penetration_min_damage, "Automatic penetration min. damage", 1, 100, "%i hp");
	////m_pGui->PopCondition();
	//m_pGui->Combo(&cvars::weapons[current_weapon_id].trigger_accuracy, "Accuracy boost", trigger_accuracy_boost, IM_ARRAYSIZE(trigger_accuracy_boost));
	////m_pGui->PopCondition(3);
	//
	//m_pGui->EndChild();

	//ImGui::EndGroup();
	//ImGui::SameLine(0, WINDOW_PADDING_X);

	//child_size.y = GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_X;

	//const char* aim_psilent_type[] = { "Manual", "Automatic fire" };
	//const char* aim_psilent_triggers[] = { "Standing", "On land", "In air", "Always" };
	//const char* aim_accuracy_boost[] = { "None", "Bound box (Recoil)", "Bound box (Recoil / Spread)", "Recoil", "Recoil / Spread" };

	//m_pGui->BeginChild("Aimbot", child_size);
	//m_pGui->SetID("##legit_aimbot");
	////m_pGui->PushCondition(cvars::legitbot.active);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_enabled, "Enabled");
	//////m_pGui->Key(&cvars::legitbot.aim_key, "Aim key");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].aim_enabled);
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_head, "Head");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_chest, "Chest");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_stomach, "Stomach");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_arms, "Arms");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_legs, "Legs");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].aim_head || cvars::weapons[current_weapon_id].aim_chest || cvars::weapons[current_weapon_id].aim_stomach || cvars::weapons[current_weapon_id].aim_legs || cvars::weapons[current_weapon_id].aim_arms);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_fov, "Maximum FOV", 0.f, 180.f, "%.1f°", 1.f);
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].aim_fov > 0.f);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_smooth_auto, "Smooth automatic", 0.f, 300.f, cvars::weapons[current_weapon_id].aim_smooth_auto > 0.f ? "%.1f" : "disabled", 2.f);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_smooth_in_attack, "Smooth in attack", 1.f, 300.f, "%.1f", 2.f);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_smooth_scale_fov, "Smooth scale - FOV", 0.f, 100.f, "%.0f%%", 1.f);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_recoil_fov, "Recoil comp. maximum FOV", 0.f, 180.f, "%.1f°", 1.f);
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].aim_recoil_fov > 0.f);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_recoil_smooth, "Recoil comp. smooth", 1.f, 300.f, "%.1f", 2.f);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_recoil_pitch, "Recoil comp. pitch", 1.f, 100.f, "%.0f%%", 1.f, "Vertical.");
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_recoil_yaw, "Recoil comp. yaw", 1.f, 100.f, "%.0f%%", 1.f, "Horizontal.");
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_recoil_start, "Recoil comp. start", 0, 15, cvars::weapons[current_weapon_id].aim_recoil_start ? "%i bullet" : "if there is recoil");
	////m_pGui->PopCondition();
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].aim_psilent_angle > 0.f);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_psilent_angle, "Perfect silent maximum angle", 0.f, 1.f, "%.1f°", 1.f);
	//m_pGui->Combo(&cvars::weapons[current_weapon_id].aim_psilent_type, "Perfect silent type", aim_psilent_type, IM_ARRAYSIZE(aim_psilent_type));
	//m_pGui->Combo(&cvars::weapons[current_weapon_id].aim_psilent_triggers, "Perfect silent triggers", aim_psilent_triggers, IM_ARRAYSIZE(aim_psilent_triggers));
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_psilent_tapping_mode, "Perfect silent tapping mode");
	////m_pGui->PopCondition();
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_maximum_lock_on_time, "Maximum lock-on time", 0, 10000, "%i ms");
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_delay_before_aiming, "Delay before aiming", 0, 1000, "%i ms");
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_delay_before_firing, "Delay before firing", 0, 1000, "%i ms");
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_auto_penetration, "Automatic penetration");
	////m_pGui->PushCondition(cvars::weapons[current_weapon_id].aim_auto_penetration);
	//m_pGui->Slider(&cvars::weapons[current_weapon_id].aim_auto_penetration_min_damage, "Automatic penetration min. damage", 1, 100, "%i hp");
	////m_pGui->PopCondition();
	//m_pGui->Checkbox(&cvars::weapons[current_weapon_id].aim_auto_fire, "Automatic fire");
	//m_pGui->Combo(&cvars::weapons[current_weapon_id].aim_accuracy, "Accuracy boost", aim_accuracy_boost, IM_ARRAYSIZE(aim_accuracy_boost));
	////m_pGui->PopCondition(4);
	//
	//m_pGui->EndChild();

	//if (!selected_weapon[selected_weapon_group] && m_pGui->IsChangedValue())
	//	ChangeValuesWholeGroup(selected_weapon_group, current_weapon_id);

	//m_pGui->EndCheckChangeValue();
}