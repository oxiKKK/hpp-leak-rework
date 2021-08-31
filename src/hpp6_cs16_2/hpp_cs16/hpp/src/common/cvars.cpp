#include "framework.h"

std::map<std::string, cvar_t*> g_ClientCvarsMap;

namespace cvars
{
	cvars_ragebot ragebot;
	cvars_legitbot legitbot;
	cvars_weapons weapons[WEAPON_MAX_COUNT];
	cvars_visuals visuals;
	cvars_kreedz kreedz;
	cvars_misc misc;
}

static bool WeaponFilter(int index)
{
	switch (index)
	{
	case WEAPON_KNIFE:
	case WEAPON_HEGRENADE:
	case WEAPON_SMOKEGRENADE:
	case WEAPON_C4:
	case WEAPON_FLASHBANG:
		return false;
	}

	return true;
}

static void FileRageHandle()
{
	g_pSettings->Section("rage");
	g_pSettings->Key("rage.active", cvars::ragebot.active);
	g_pSettings->Key("rage.friendly_fire", cvars::ragebot.friendly_fire);
	g_pSettings->Key("rage.aim.maximum_fov", cvars::ragebot.raim_fov, 180.f);
	g_pSettings->Key("rage.aim.target.selection.highest_damage", cvars::ragebot.raim_target_selection[0], true);
	g_pSettings->Key("rage.aim.target.selection.closest_by_fov", cvars::ragebot.raim_target_selection[1], true);
	g_pSettings->Key("rage.aim.target.selection.hitbox_weights", cvars::ragebot.raim_target_selection[2], true);
	g_pSettings->Key("rage.aim.target.selection.weight.damage", cvars::ragebot.raim_target_weight_damage, 40);
	g_pSettings->Key("rage.aim.target.selection.weight.fov", cvars::ragebot.raim_target_weight_fov, 30);
	g_pSettings->Key("rage.aim.target.selection.weight.head", cvars::ragebot.raim_target_weight_head, 50);
	g_pSettings->Key("rage.aim.target.selection.weight.neck", cvars::ragebot.raim_target_weight_neck, 40);
	g_pSettings->Key("rage.aim.target.selection.weight.chest", cvars::ragebot.raim_target_weight_chest, 30);
	g_pSettings->Key("rage.aim.target.selection.weight.stomach", cvars::ragebot.raim_target_weight_stomach, 20);
	g_pSettings->Key("rage.aim.target.selection.weight.arms", cvars::ragebot.raim_target_weight_arms, 10);
	g_pSettings->Key("rage.aim.target.selection.weight.legs", cvars::ragebot.raim_target_weight_legs, 10);
	g_pSettings->Key("rage.aim.automatic.fire", cvars::ragebot.raim_auto_fire, true);
	g_pSettings->Key("rage.aim.automatic.scope", cvars::ragebot.raim_auto_scope);
	g_pSettings->Key("rage.aim.type", cvars::ragebot.raim_type, 2);
	g_pSettings->Key("rage.aim.remove.recoil", cvars::ragebot.raim_remove_recoil, true);
	g_pSettings->Key("rage.aim.remove.spread", cvars::ragebot.raim_remove_spread, NOSPREAD_PITCH_YAW_ROLL);
	g_pSettings->Key("rage.aim.tapping_mode", cvars::ragebot.raim_tapping_mode);
	g_pSettings->Key("rage.aim.delayshot.unlag", cvars::ragebot.raim_delayshot[0], true);
	g_pSettings->Key("rage.aim.delayshot.history", cvars::ragebot.raim_delayshot[1]);
	g_pSettings->Key("rage.aim.force_body.key", cvars::ragebot.raim_force_body_key);
	g_pSettings->Key("rage.aim.low_fps_mitigations.value", cvars::ragebot.raim_low_fps_value, 100.f);
	g_pSettings->Key("rage.aim.low_fps_mitigations.traces_only_if_can_attack", cvars::ragebot.raim_low_fps_mitigations[0], true);
	g_pSettings->Key("rage.aim.low_fps_mitigations.dont_trace_arms_points", cvars::ragebot.raim_low_fps_mitigations[1], true);
	g_pSettings->Key("rage.aim.low_fps_mitigations.dont_trace_legs_points", cvars::ragebot.raim_low_fps_mitigations[2], true);
	g_pSettings->Key("rage.resolver.pitch", cvars::ragebot.raim_resolver_pitch, 1);
	g_pSettings->Key("rage.resolver.yaw", cvars::ragebot.raim_resolver_yaw);

	for (int weapon_idx = 1; weapon_idx < WEAPON_MAX_COUNT; weapon_idx++)
	{
		if (!WeaponFilter(weapon_idx))
			continue;

		std::string wpn = Game::GetWeaponName(weapon_idx);

		if (!wpn.compare("unknown"))
			continue;

		g_pSettings->Section(wpn);
		g_pSettings->Key(wpn + ".aim.enabled", cvars::weapons[weapon_idx].raim_enabled, true);
		g_pSettings->Key(wpn + ".aim.hitboxes.head", cvars::weapons[weapon_idx].raim_hitboxes[0], true);
		g_pSettings->Key(wpn + ".aim.hitboxes.neck", cvars::weapons[weapon_idx].raim_hitboxes[1]);
		g_pSettings->Key(wpn + ".aim.hitboxes.chest", cvars::weapons[weapon_idx].raim_hitboxes[2]);
		g_pSettings->Key(wpn + ".aim.hitboxes.stomach", cvars::weapons[weapon_idx].raim_hitboxes[3]);
		g_pSettings->Key(wpn + ".aim.hitboxes.arms", cvars::weapons[weapon_idx].raim_hitboxes[4]);
		g_pSettings->Key(wpn + ".aim.hitboxes.legs", cvars::weapons[weapon_idx].raim_hitboxes[5]);
		g_pSettings->Key(wpn + ".aim.head.scale", cvars::weapons[weapon_idx].raim_head_scale);
		g_pSettings->Key(wpn + ".aim.neck.scale", cvars::weapons[weapon_idx].raim_neck_scale);
		g_pSettings->Key(wpn + ".aim.chest.scale", cvars::weapons[weapon_idx].raim_chest_scale);
		g_pSettings->Key(wpn + ".aim.stomach.scale", cvars::weapons[weapon_idx].raim_stomach_scale);
		g_pSettings->Key(wpn + ".aim.arms.scale", cvars::weapons[weapon_idx].raim_arms_scale);
		g_pSettings->Key(wpn + ".aim.legs.scale", cvars::weapons[weapon_idx].raim_legs_scale);
		g_pSettings->Key(wpn + ".aim.automatic.penetration", cvars::weapons[weapon_idx].raim_auto_penetration);
		g_pSettings->Key(wpn + ".aim.automatic.penetration.minimum_damage", cvars::weapons[weapon_idx].raim_auto_penetration_min_damage);
		g_pSettings->Key(wpn + ".aim.autostop", cvars::weapons[weapon_idx].raim_autostop);
		g_pSettings->Key(wpn + ".aim.autostop.crouch", cvars::weapons[weapon_idx].raim_autostop_crouch);
	}

	g_pSettings->Section("anti-aimbot");
	g_pSettings->Key("anti-aimbot.enabled", cvars::ragebot.aa_enabled);
	g_pSettings->Key("anti-aimbot.teammates", cvars::ragebot.aa_teammates);
	g_pSettings->Key("anti-aimbot.conditions.on_knife", cvars::ragebot.aa_conditions[0], true);
	g_pSettings->Key("anti-aimbot.conditions.on_grenades", cvars::ragebot.aa_conditions[1], true);
	g_pSettings->Key("anti-aimbot.conditions.on_freeze_period", cvars::ragebot.aa_conditions[2], true);
	g_pSettings->Key("anti-aimbot.side", cvars::ragebot.aa_side);
	g_pSettings->Key("anti-aimbot.side.key", cvars::ragebot.aa_side_key);
	g_pSettings->Key("anti-aimbot.side.switch_when_take_damage", cvars::ragebot.aa_side_switch_when_take_damage);
	g_pSettings->Key("anti-aimbot.at_targets", cvars::ragebot.aa_at_targets, 1);
	g_pSettings->Key("anti-aimbot.stand.pitch", cvars::ragebot.aa_stand_pitch);
	g_pSettings->Key("anti-aimbot.stand.yaw", cvars::ragebot.aa_stand_yaw);
	g_pSettings->Key("anti-aimbot.stand.yaw.static", cvars::ragebot.aa_stand_yaw_static);
	g_pSettings->Key("anti-aimbot.stand.desync", cvars::ragebot.aa_stand_desync);
	g_pSettings->Key("anti-aimbot.stand.desync.helper", cvars::ragebot.aa_stand_desync_helper);
	g_pSettings->Key("anti-aimbot.move.pitch", cvars::ragebot.aa_move_pitch);
	g_pSettings->Key("anti-aimbot.move.yaw", cvars::ragebot.aa_move_yaw);
	g_pSettings->Key("anti-aimbot.roll", cvars::ragebot.aa_roll);
	g_pSettings->Key("anti-aimbot.roll.static", cvars::ragebot.aa_roll_static);
	g_pSettings->Key("anti-aimbot.untrusted_checkes", cvars::ragebot.aa_untrusted_checks);

	g_pSettings->Section("fakelag");
	g_pSettings->Key("fakelag.enabled", cvars::ragebot.fakelag_enabled);
	g_pSettings->Key("fakelag.while_shooting", cvars::ragebot.fakelag_while_shooting);
	g_pSettings->Key("fakelag.on_enemy_in_pvs", cvars::ragebot.fakelag_on_enemy_in_pvs);
	g_pSettings->Key("fakelag.on_peek", cvars::ragebot.fakelag_on_peek);
	g_pSettings->Key("fakelag.triggers.in_move", cvars::ragebot.fakelag_triggers[0], true);
	g_pSettings->Key("fakelag.triggers.in_air", cvars::ragebot.fakelag_triggers[1], true);
	g_pSettings->Key("fakelag.type", cvars::ragebot.fakelag_type);
	g_pSettings->Key("fakelag.choke_limit", cvars::ragebot.fakelag_choke_limit, 15);
}

static void FileLegitHandle()
{
	g_pSettings->Section("legit");
	g_pSettings->Key("legit.active", cvars::legitbot.active);
	g_pSettings->Key("legit.friendly_fire", cvars::legitbot.friendly_fire);
	g_pSettings->Key("legit.position_adjustment", cvars::legitbot.position_adjustment);
	g_pSettings->Key("legit.desync.helper", cvars::legitbot.desync_helper);
	g_pSettings->Key("legit.target_switch_delay", cvars::legitbot.target_switch_delay, 200);
	g_pSettings->Key("legit.aim.automatic.scope", cvars::legitbot.aim_auto_scope);
	g_pSettings->Key("legit.aim.smooth.independence_fps", cvars::legitbot.aim_smooth_independence_fps, true);
	g_pSettings->Key("legit.aim.block_attack_after_kill", cvars::legitbot.aim_block_attack_after_kill);
	g_pSettings->Key("legit.aim.dont_shoot_in_shield", cvars::legitbot.aim_dont_shoot_in_shield);
	g_pSettings->Key("legit.aim.demochecker_bypass", cvars::legitbot.aim_demochecker_bypass);
	g_pSettings->Key("legit.aim.recoil.standalone", cvars::legitbot.aim_recoil_standalone);
	g_pSettings->Key("legit.aim.key", cvars::legitbot.aim_key);
	g_pSettings->Key("legit.aim.psilent.key", cvars::legitbot.aim_psilent_key);
	g_pSettings->Key("legit.trigger.only_scoped", cvars::legitbot.trigger_only_scoped);
	g_pSettings->Key("legit.trigger.accurate_traces", cvars::legitbot.trigger_accurate_traces);
	g_pSettings->Key("legit.trigger.hitbox_scale", cvars::legitbot.trigger_hitbox_scale, 100.f);
	g_pSettings->Key("legit.trigger.key", cvars::legitbot.trigger_key);

	for (int weapon_idx = 1; weapon_idx < WEAPON_MAX_COUNT; weapon_idx++)
	{
		if (!WeaponFilter(weapon_idx))
			continue;

		std::string wpn = Game::GetWeaponName(weapon_idx);

		if (!wpn.compare("unknown"))
			continue;

		g_pSettings->Section(wpn);
		g_pSettings->Key(wpn + ".aim.enabled", cvars::weapons[weapon_idx].aim_enabled, true);
		g_pSettings->Key(wpn + ".aim.hitboxes.head", cvars::weapons[weapon_idx].aim_hitboxes[0], true);
		g_pSettings->Key(wpn + ".aim.hitboxes.neck", cvars::weapons[weapon_idx].aim_hitboxes[1], true);
		g_pSettings->Key(wpn + ".aim.hitboxes.chest", cvars::weapons[weapon_idx].aim_hitboxes[2], true);
		g_pSettings->Key(wpn + ".aim.hitboxes.stomach", cvars::weapons[weapon_idx].aim_hitboxes[3], true);
		g_pSettings->Key(wpn + ".aim.hitboxes.arms", cvars::weapons[weapon_idx].aim_hitboxes[4]);
		g_pSettings->Key(wpn + ".aim.hitboxes.legs", cvars::weapons[weapon_idx].aim_hitboxes[5]);
		g_pSettings->Key(wpn + ".aim.maximum_fov", cvars::weapons[weapon_idx].aim_fov, 2.5f);
		g_pSettings->Key(wpn + ".aim.smooth.auto", cvars::weapons[weapon_idx].aim_smooth_auto);
		g_pSettings->Key(wpn + ".aim.smooth.in_attack", cvars::weapons[weapon_idx].aim_smooth_in_attack, 5.f);
		g_pSettings->Key(wpn + ".aim.smooth.scale_fov", cvars::weapons[weapon_idx].aim_smooth_scale_fov);
		g_pSettings->Key(wpn + ".aim.recoil.maximum_fov", cvars::weapons[weapon_idx].aim_recoil_fov, 3.f);
		g_pSettings->Key(wpn + ".aim.recoil.smooth", cvars::weapons[weapon_idx].aim_recoil_smooth, 6.5f);
		g_pSettings->Key(wpn + ".aim.recoil.pitch", cvars::weapons[weapon_idx].aim_recoil_pitch, 100.f);
		g_pSettings->Key(wpn + ".aim.recoil.yaw", cvars::weapons[weapon_idx].aim_recoil_yaw, 100.f);
		g_pSettings->Key(wpn + ".aim.recoil.start", cvars::weapons[weapon_idx].aim_recoil_start);
		g_pSettings->Key(wpn + ".aim.psilent.angle", cvars::weapons[weapon_idx].aim_psilent_angle);
		g_pSettings->Key(wpn + ".aim.psilent.type", cvars::weapons[weapon_idx].aim_psilent_type, 1);
		g_pSettings->Key(wpn + ".aim.psilent.tapping_mode", cvars::weapons[weapon_idx].aim_psilent_tapping_mode, true);
		g_pSettings->Key(wpn + ".aim.psilent.triggers.standing", cvars::weapons[weapon_idx].aim_psilent_triggers[0]);
		g_pSettings->Key(wpn + ".aim.psilent.triggers.on_land", cvars::weapons[weapon_idx].aim_psilent_triggers[1]);
		g_pSettings->Key(wpn + ".aim.psilent.triggers.in_air", cvars::weapons[weapon_idx].aim_psilent_triggers[2], true);
		g_pSettings->Key(wpn + ".aim.maximum_lock-on_time", cvars::weapons[weapon_idx].aim_maximum_lock_on_time);
		g_pSettings->Key(wpn + ".aim.delay.before.aiming", cvars::weapons[weapon_idx].aim_delay_before_aiming);
		g_pSettings->Key(wpn + ".aim.delay.before.firing", cvars::weapons[weapon_idx].aim_delay_before_firing);
		g_pSettings->Key(wpn + ".aim.automatic.penetration", cvars::weapons[weapon_idx].aim_auto_penetration);
		g_pSettings->Key(wpn + ".aim.automatic.penetration.minimum_damage", cvars::weapons[weapon_idx].aim_auto_penetration_min_damage);
		g_pSettings->Key(wpn + ".aim.automatic.fire", cvars::weapons[weapon_idx].aim_auto_fire);
		g_pSettings->Key(wpn + ".aim.accuracy_boost", cvars::weapons[weapon_idx].aim_accuracy_boost);
		g_pSettings->Key(wpn + ".trigger.enabled", cvars::weapons[weapon_idx].trigger_enabled, true);
		g_pSettings->Key(wpn + ".trigger.hitboxes.head", cvars::weapons[weapon_idx].trigger_hitboxes[0], true);
		g_pSettings->Key(wpn + ".trigger.hitboxes.neck", cvars::weapons[weapon_idx].trigger_hitboxes[1], true);
		g_pSettings->Key(wpn + ".trigger.hitboxes.chest", cvars::weapons[weapon_idx].trigger_hitboxes[2], true);
		g_pSettings->Key(wpn + ".trigger.hitboxes.stomach", cvars::weapons[weapon_idx].trigger_hitboxes[3], true);
		g_pSettings->Key(wpn + ".trigger.hitboxes.arms", cvars::weapons[weapon_idx].trigger_hitboxes[4]);
		g_pSettings->Key(wpn + ".trigger.hitboxes.legs", cvars::weapons[weapon_idx].trigger_hitboxes[5]);
		g_pSettings->Key(wpn + ".trigger.automatic.penetration", cvars::weapons[weapon_idx].trigger_auto_penetration);
		g_pSettings->Key(wpn + ".trigger.automatic.penetration.minimum_damage", cvars::weapons[weapon_idx].trigger_auto_penetration_min_damage);
		g_pSettings->Key(wpn + ".trigger.accuracy_boost", cvars::weapons[weapon_idx].trigger_accuracy_boost, 2);
	}
}

static void FileVisualsHandle()
{
	g_pSettings->Section("visuals");
	g_pSettings->Key("active", cvars::visuals.active);
	g_pSettings->Key("panic.key", cvars::visuals.panic_key, bind_t{ K_F6, KEY_ON_TOGGLE });
	g_pSettings->Key("copyright", cvars::visuals.copyright);
	g_pSettings->Key("copyright.position", cvars::visuals.copyright_position, "100,100");
	g_pSettings->Key("antiscreen", cvars::visuals.antiscreen, true);
	g_pSettings->Key("streamer_mode", cvars::visuals.streamer_mode);

	g_pSettings->Section("esp");
	g_pSettings->Key("dormant.time", cvars::visuals.esp_dormant_time, 2.f);
	g_pSettings->Key("dormant.fadeout", cvars::visuals.esp_dormant_fadeout, true);
	g_pSettings->Key("dormant.update.by_sound", cvars::visuals.esp_dormant_update_by_sound, true);
	g_pSettings->Key("interpolate.history", cvars::visuals.esp_interpolate_history);
	g_pSettings->Key("font.size", cvars::visuals.esp_font_size, 10);
	g_pSettings->Key("player", cvars::visuals.esp_player, true);
	g_pSettings->Key("player.local", cvars::visuals.esp_player_players[0]);
	g_pSettings->Key("player.enemies", cvars::visuals.esp_player_players[1], true);
	g_pSettings->Key("player.teammates", cvars::visuals.esp_player_players[2]);
	g_pSettings->Key("player.box", cvars::visuals.esp_player_box, BoxDefault);
	g_pSettings->Key("player.box.outline", cvars::visuals.esp_player_box_outline, true);
	g_pSettings->Key("player.box.filled", cvars::visuals.esp_player_box_filled);
	g_pSettings->Key("player.box.color.t", cvars::visuals.esp_player_box_color_t, "255,64,64,145");
	g_pSettings->Key("player.box.color.ct", cvars::visuals.esp_player_box_color_ct, "64,64,255,145");
	g_pSettings->Key("player.health", cvars::visuals.esp_player_health, PosLeft);
	g_pSettings->Key("player.health.percentage", cvars::visuals.esp_player_health_percentage);
	g_pSettings->Key("player.health.percentage.color", cvars::visuals.esp_player_health_percentage_color, "255,255,255,255");
	g_pSettings->Key("player.armor", cvars::visuals.esp_player_armor);
	g_pSettings->Key("player.armor.color", cvars::visuals.esp_player_armor_color, "255,255,255,255");
	g_pSettings->Key("player.name", cvars::visuals.esp_player_name);
	g_pSettings->Key("player.name.color", cvars::visuals.esp_player_name_color, "255,255,255,255");
	g_pSettings->Key("player.weapon.text", cvars::visuals.esp_player_weapon_text);
	g_pSettings->Key("player.weapon.text.color", cvars::visuals.esp_player_weapon_text_color, "255,255,255,255");
	g_pSettings->Key("player.weapon.icon", cvars::visuals.esp_player_weapon_icon);
	g_pSettings->Key("player.weapon.icon.color", cvars::visuals.esp_player_weapon_icon_color, "255,255,255,255");
	g_pSettings->Key("player.money", cvars::visuals.esp_player_money);
	g_pSettings->Key("player.money.color", cvars::visuals.esp_player_money_color, "255,255,255,255");
	g_pSettings->Key("player.distance", cvars::visuals.esp_player_distance);
	g_pSettings->Key("player.distance.measurement", cvars::visuals.esp_player_distance_measurement, InMeters);
	g_pSettings->Key("player.distance.color", cvars::visuals.esp_player_distance_color, "255,255,255,255");
	g_pSettings->Key("player.actions", cvars::visuals.esp_player_actions);
	g_pSettings->Key("player.actions.color", cvars::visuals.esp_player_actions_color, "255,255,255,255");
	g_pSettings->Key("player.actions.bar", cvars::visuals.esp_player_actions_bar);
	g_pSettings->Key("player.actions.bar.color", cvars::visuals.esp_player_actions_bar_color, "100,255,100,255");
	g_pSettings->Key("player.has.c4", cvars::visuals.esp_player_has_c4, PosLeft);
	g_pSettings->Key("player.has.c4.color", cvars::visuals.esp_player_has_c4_color, "255,255,255,255");
	g_pSettings->Key("player.has.defusal_kits", cvars::visuals.esp_player_has_defusal_kits, PosLeft);
	g_pSettings->Key("player.has.defusal_kits.color", cvars::visuals.esp_player_has_defusal_kits_color, "255,255,255,255");
	g_pSettings->Key("player.line_of_sight", cvars::visuals.esp_player_line_of_sight);
	g_pSettings->Key("player.line_of_sight.color", cvars::visuals.esp_player_line_of_sight_color, "255,255,255,255");
	g_pSettings->Key("player.hitboxes", cvars::visuals.esp_player_hitboxes);
	g_pSettings->Key("player.hitboxes.color", cvars::visuals.esp_player_hitboxes_color, "255,255,255,255");
	g_pSettings->Key("player.hitboxes.hit_position", cvars::visuals.esp_player_hitboxes_hit_position);
	g_pSettings->Key("player.hitboxes.hit_position.time", cvars::visuals.esp_player_hitboxes_hit_position_time, 3.5f);
	g_pSettings->Key("player.hitboxes.hit_position.color", cvars::visuals.esp_player_hitboxes_hit_position_color, "255,80,180,100");
	g_pSettings->Key("player.hitboxes.hit_position.color2", cvars::visuals.esp_player_hitboxes_hit_position_color2, "100,255,100,100");
	g_pSettings->Key("player.skeleton", cvars::visuals.esp_player_skeleton);
	g_pSettings->Key("player.skeleton.color", cvars::visuals.esp_player_skeleton_color, "255,255,255,255");
	g_pSettings->Key("player.skeleton.backtrack", cvars::visuals.esp_player_skeleton_backtrack);
	g_pSettings->Key("player.skeleton.backtrack.color", cvars::visuals.esp_player_skeleton_backtrack_color, "64,165,255,255");
	g_pSettings->Key("glow", cvars::visuals.esp_player_glow);
	g_pSettings->Key("glow.local", cvars::visuals.esp_player_glow_players[0]);
	g_pSettings->Key("glow.enemies", cvars::visuals.esp_player_glow_players[1], true);
	g_pSettings->Key("glow.teammates", cvars::visuals.esp_player_glow_players[2]);
	g_pSettings->Key("glow.amount", cvars::visuals.esp_player_glow_amount, 5);
	g_pSettings->Key("glow.color.health_based", cvars::visuals.esp_player_glow_color_health_based);
	g_pSettings->Key("glow.color.t", cvars::visuals.esp_player_glow_color_t, "255,64,64,255");
	g_pSettings->Key("glow.color.ct", cvars::visuals.esp_player_glow_color_ct, "64,64,255,255");
	g_pSettings->Key("sounds", cvars::visuals.esp_player_sounds);
	g_pSettings->Key("sounds.local", cvars::visuals.esp_player_sounds_players[0]);
	g_pSettings->Key("sounds.enemies", cvars::visuals.esp_player_sounds_players[1], true);
	g_pSettings->Key("sounds.teammates", cvars::visuals.esp_player_sounds_players[2]);
	g_pSettings->Key("sounds.time", cvars::visuals.esp_player_sounds_time, 0.8f);
	g_pSettings->Key("sounds.circle.radius", cvars::visuals.esp_player_sounds_circle_radius, 10.f);
	g_pSettings->Key("sounds.color.t", cvars::visuals.esp_player_sounds_color_t, "255,64,64,255");
	g_pSettings->Key("sounds.color.ct", cvars::visuals.esp_player_sounds_color_ct, "64,64,255,255");
	g_pSettings->Key("out_of_fov", cvars::visuals.esp_player_out_of_fov, true);
	g_pSettings->Key("out_of_fov.draw_type", cvars::visuals.esp_player_out_of_fov_draw_type, RHOMBUS);
	g_pSettings->Key("out_of_fov.enemies", cvars::visuals.esp_player_out_of_fov_players[0], true);
	g_pSettings->Key("out_of_fov.teammates", cvars::visuals.esp_player_out_of_fov_players[1]);
	g_pSettings->Key("out_of_fov.name", cvars::visuals.esp_player_out_of_fov_additional_info[0]);
	g_pSettings->Key("out_of_fov.weapon", cvars::visuals.esp_player_out_of_fov_additional_info[1]);
	g_pSettings->Key("out_of_fov.distance", cvars::visuals.esp_player_out_of_fov_additional_info[2]);
	g_pSettings->Key("out_of_fov.size", cvars::visuals.esp_player_out_of_fov_size, 10.f);
	g_pSettings->Key("out_of_fov.radius", cvars::visuals.esp_player_out_of_fov_radius, 60.f);
	g_pSettings->Key("out_of_fov.aspect_ratio", cvars::visuals.esp_player_out_of_fov_aspect_ratio, true);
	g_pSettings->Key("out_of_fov.color.t", cvars::visuals.esp_player_out_of_fov_color_t, "255,64,64,255");
	g_pSettings->Key("out_of_fov.color.ct", cvars::visuals.esp_player_out_of_fov_color_ct, "64,64,255,255");
	g_pSettings->Key("out_of_fov.impulse_alpha", cvars::visuals.esp_player_out_of_fov_impulse_alpha, true);
	g_pSettings->Key("out_of_fov.outline", cvars::visuals.esp_player_out_of_fov_outline);

	g_pSettings->Section("colored_models");
	g_pSettings->Key("dormant.time", cvars::visuals.colored_models_dormant_time, 0.5f);
	g_pSettings->Key("dormant.fadeout", cvars::visuals.colored_models_dormant_fadeout, true);
	g_pSettings->Key("paint_players_weapons", cvars::visuals.colored_models_paint_players_weapons);
	g_pSettings->Key("players", cvars::visuals.colored_models_players, FLAT);
	g_pSettings->Key("players.wireframe", cvars::visuals.colored_models_players_wireframe);
	g_pSettings->Key("players.local", cvars::visuals.colored_models_players_players[0]);
	g_pSettings->Key("players.enemies", cvars::visuals.colored_models_players_players[1], true);
	g_pSettings->Key("players.teammates", cvars::visuals.colored_models_players_players[2]);
	g_pSettings->Key("players.behind_wall", cvars::visuals.colored_models_players_behind_wall, true);
	g_pSettings->Key("players.color.health_based", cvars::visuals.colored_models_players_color_health_based);
	g_pSettings->Key("players.color.t.hide", cvars::visuals.colored_models_players_color_t_hide, "255,80,80,255");
	g_pSettings->Key("players.color.t.vis", cvars::visuals.colored_models_players_color_t_vis, "255,255,100,255");
	g_pSettings->Key("players.color.ct.hide", cvars::visuals.colored_models_players_color_ct_hide, "80,127,255,255");
	g_pSettings->Key("players.color.ct.vis", cvars::visuals.colored_models_players_color_ct_vis, "80,255,80,255");
	g_pSettings->Key("players.on_the_dead", cvars::visuals.colored_models_players_on_the_dead);
	g_pSettings->Key("players.on_the_dead.color", cvars::visuals.colored_models_players_on_the_dead_color, "20,20,20,255");
	g_pSettings->Key("players.hit_position", cvars::visuals.colored_models_players_hit_position);
	g_pSettings->Key("players.hit_position.time", cvars::visuals.colored_models_players_hit_position_time, 3.5f);
	g_pSettings->Key("players.hit_position.color", cvars::visuals.colored_models_players_hit_position_color, "255,80,180,127");
	g_pSettings->Key("players.desync_aa", cvars::visuals.colored_models_players_desync_aa);
	g_pSettings->Key("players.desync_aa.color", cvars::visuals.colored_models_players_desync_aa_color, "195,160,255,255");
	g_pSettings->Key("backtrack", cvars::visuals.colored_models_backtrack);
	g_pSettings->Key("backtrack.wireframe", cvars::visuals.colored_models_backtrack_wireframe);
	g_pSettings->Key("backtrack.enemies", cvars::visuals.colored_models_backtrack_players[0], true);
	g_pSettings->Key("backtrack.teammates", cvars::visuals.colored_models_backtrack_players[1]);
	g_pSettings->Key("backtrack.behind_wall", cvars::visuals.colored_models_backtrack_behind_wall);
	g_pSettings->Key("backtrack.color.t.hide", cvars::visuals.colored_models_backtrack_color_t_hide, "255,255,255,127");
	g_pSettings->Key("backtrack.color.t.vis", cvars::visuals.colored_models_backtrack_color_t_vis, "255,255,255,127");
	g_pSettings->Key("backtrack.color.ct.hide", cvars::visuals.colored_models_backtrack_color_ct_hide, "255,255,255,127");
	g_pSettings->Key("backtrack.color.ct.vis", cvars::visuals.colored_models_backtrack_color_ct_vis, "255,255,255,127");
	g_pSettings->Key("hands", cvars::visuals.colored_models_hands);
	g_pSettings->Key("hands.wireframe", cvars::visuals.colored_models_hands_wireframe);
	g_pSettings->Key("hands.color", cvars::visuals.colored_models_hands_color, "64,200,255,255");
	g_pSettings->Key("hands.color.rainbow", cvars::visuals.colored_models_hands_color_rainbow);
	g_pSettings->Key("hands.color.rainbow.speed", cvars::visuals.colored_models_hands_color_rainbow_speed, 0.1f);
	g_pSettings->Key("dlight", cvars::visuals.colored_models_dlight);
	g_pSettings->Key("dlight.local", cvars::visuals.colored_models_dlight_players[0]);
	g_pSettings->Key("dlight.enemies", cvars::visuals.colored_models_dlight_players[1], true);
	g_pSettings->Key("dlight.teammates", cvars::visuals.colored_models_dlight_players[2]);
	g_pSettings->Key("dlight.origin", cvars::visuals.colored_models_dlight_origin);
	g_pSettings->Key("dlight.radius", cvars::visuals.colored_models_dlight_radius, 50.f);
	g_pSettings->Key("dlight.minlight", cvars::visuals.colored_models_dlight_minlight);
	g_pSettings->Key("dlight.fading_lighting", cvars::visuals.colored_models_dlight_fading_lighting);
	g_pSettings->Key("dlight.fading_lighting.speed", cvars::visuals.colored_models_dlight_fading_lighting_speed, 1.f);
	g_pSettings->Key("dlight.color", cvars::visuals.colored_models_dlight_color, "80,0,255,255");
	g_pSettings->Key("elight", cvars::visuals.colored_models_elight);
	g_pSettings->Key("elight.local", cvars::visuals.colored_models_elight_players[0]);
	g_pSettings->Key("elight.enemies", cvars::visuals.colored_models_elight_players[1], true);
	g_pSettings->Key("elight.teammates", cvars::visuals.colored_models_elight_players[2]);
	g_pSettings->Key("elight.radius", cvars::visuals.colored_models_elight_radius, 50.f);
	g_pSettings->Key("elight.color", cvars::visuals.colored_models_elight_color, "255,127,80,255");

	g_pSettings->Section("other");
	g_pSettings->Key("local.aa_side_arrows", cvars::visuals.esp_other_local_aa_side_arrows);
	g_pSettings->Key("local.sniper_crosshair", cvars::visuals.esp_other_local_sniper_crosshair);
	g_pSettings->Key("local.recoil.point", cvars::visuals.esp_other_local_recoil_point);
	g_pSettings->Key("local.recoil.point.color", cvars::visuals.esp_other_local_recoil_point_color, "255,64,64,255");
	g_pSettings->Key("local.spread.point", cvars::visuals.esp_other_local_spread_point);
	g_pSettings->Key("local.spread.point.color", cvars::visuals.esp_other_local_spread_point_color, "255,255,0,255");
	g_pSettings->Key("local.spread.circle", cvars::visuals.esp_other_local_spread_circle);
	g_pSettings->Key("local.spread.circle.color", cvars::visuals.esp_other_local_spread_circle_color, "255,150,55,100");
	g_pSettings->Key("local.spread.circle.color2", cvars::visuals.esp_other_local_spread_circle_color2, "0,0,0,0");
	g_pSettings->Key("local.aim.fov", cvars::visuals.esp_other_local_aim_fov);
	g_pSettings->Key("local.aim.fov.color", cvars::visuals.esp_other_local_aim_fov_color, "255,255,255,30");
	g_pSettings->Key("local.aim.fov.color2", cvars::visuals.esp_other_local_aim_fov_color2, "0,0,0,0");
	g_pSettings->Key("local.toggle_status", cvars::visuals.esp_other_local_toggle_status, true);
	g_pSettings->Key("local.toggle_status.color", cvars::visuals.esp_other_local_toggle_status_color, "64,255,64,255");
	g_pSettings->Key("local.toggle_status.color2", cvars::visuals.esp_other_local_toggle_status_color2, "255,64,64,255");

	g_pSettings->Section("effects");
	g_pSettings->Key("effects.hud_clear", cvars::visuals.effects_hud_clear);
	g_pSettings->Key("effects.thirdperson", cvars::visuals.effects_thirdperson);
	g_pSettings->Key("effects.thirdperson.key", cvars::visuals.effects_thirdperson_key);
	g_pSettings->Key("effects.custom.render.fov", cvars::visuals.effects_custom_render_fov, DEFAULT_FOV);
	g_pSettings->Key("remove.scope", cvars::visuals.remove_scope);
}

static void FileKreedzHandle()
{
	g_pSettings->Section("kreedz");
	g_pSettings->Key("kreedz.active", cvars::kreedz.active);
}

static void FileMiscHandle()
{
	g_pSettings->Section("misc");
	g_pSettings->Key("automatic.reload", cvars::misc.automatic_reload);
	g_pSettings->Key("automatic.pistol", cvars::misc.automatic_pistol);
	g_pSettings->Key("fakelatency.enabled", cvars::misc.fakelatency);
	g_pSettings->Key("fakelatency.amount", cvars::misc.fakelatency_amount);
	g_pSettings->Key("fakelatency.key", cvars::misc.fakelatency_key);
	g_pSettings->Key("namestealer.enabled", cvars::misc.namestealer);
	g_pSettings->Key("namestealer.interval", cvars::misc.namestealer_interval);
	g_pSettings->Key("replace_models_with_original", cvars::misc.replace_models_with_original);
	g_pSettings->Key("steamid_spoofer", cvars::misc.steamid_spoofer);

	g_pSettings->Section("knifebot");
	g_pSettings->Key("knifebot.enabled", cvars::misc.kb_enabled);
	g_pSettings->Key("knifebot.key", cvars::misc.kb_key);
	g_pSettings->Key("knifebot.friendly_fire", cvars::misc.kb_friendly_fire);
	g_pSettings->Key("knifebot.maximum_fov", cvars::misc.kb_fov, 180.f);
	g_pSettings->Key("knifebot.attack_type", cvars::misc.kb_attack_type, 1);
	g_pSettings->Key("knifebot.aim.type", cvars::misc.kb_aim_type);
	g_pSettings->Key("knifebot.aim.hitbox.head", cvars::misc.kb_aim_hitbox[0], true);
	g_pSettings->Key("knifebot.aim.hitbox.neck", cvars::misc.kb_aim_hitbox[1], true);
	g_pSettings->Key("knifebot.aim.hitbox.chest", cvars::misc.kb_aim_hitbox[2], true);
	g_pSettings->Key("knifebot.aim.hitbox.stomach", cvars::misc.kb_aim_hitbox[3], true);
	g_pSettings->Key("knifebot.aim.hitbox.arms", cvars::misc.kb_aim_hitbox[4], true);
	g_pSettings->Key("knifebot.aim.hitbox.legs", cvars::misc.kb_aim_hitbox[5], true);
	g_pSettings->Key("knifebot.swing.distance", cvars::misc.kb_swing_distance, 48.f);
	g_pSettings->Key("knifebot.stab.distance", cvars::misc.kb_stab_distance, 32.f);
	g_pSettings->Key("knifebot.position_adjustment", cvars::misc.kb_position_adjustment);
	g_pSettings->Key("knifebot.conditions.dont_shoot.spectators", cvars::misc.kb_conditions[0], true);
	g_pSettings->Key("knifebot.conditions.dont_shoot.in_back", cvars::misc.kb_conditions[1]);
	g_pSettings->Key("knifebot.conditions.dont_shoot.in_shield", cvars::misc.kb_conditions[2], true);
}

void InitClientCvarsMap()
{
	cvar_t* pcvarslist = nullptr;

	do 
	{
		if (!pcvarslist)
			pcvarslist = g_pEngine->pfnGetCvarList();

		g_ClientCvarsMap[pcvarslist->name] = pcvarslist;
	} while (pcvarslist = pcvarslist->next);
}

void InitFileHandleList()
{
	g_pSettings->PushHandleFile(SettingsFileData{ FileRageHandle, g_IniFile[FILE_RAGE_INI] });
	g_pSettings->PushHandleFile(SettingsFileData{ FileLegitHandle, g_IniFile[FILE_LEGIT_INI] });
	g_pSettings->PushHandleFile(SettingsFileData{ FileVisualsHandle, g_IniFile[FILE_VISUALS_INI] });
	g_pSettings->PushHandleFile(SettingsFileData{ FileKreedzHandle, g_IniFile[FILE_KREEDZ_INI] });
	g_pSettings->PushHandleFile(SettingsFileData{ FileMiscHandle, g_IniFile[FILE_MISC_INI] });
}