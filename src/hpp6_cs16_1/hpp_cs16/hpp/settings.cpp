#include "main.h"

CSettings g_Settings;

enum FileList
{
	Ini_Globals,
	Ini_RageBot,
	Ini_LegitBot,
	Ini_Visuals,
	Ini_Kreedz,
	Ini_Misc,
	Ini_GUI,
	Ini_MaxFiles
};

enum Operations
{
	LoadSettings,
	SaveSettings
};

static const char* g_pcszIniFiles[Ini_MaxFiles] =
{
	"globals.ini",
	"\\ragebot.ini",
	"\\legitbot.ini",
	"\\visuals.ini",
	"\\kreedz.ini",
	"\\misc.ini",
	"\\gui.ini"
};

static CSimpleIniA g_Ini;
static int g_Operation;
static const char* g_Section;

static inline void SetSection(const char* pcszSection)
{
	g_Section = pcszSection;
}

const char* GetSectionByIndex(int nWeaponIndex)
{
	switch (nWeaponIndex)
	{
	case WEAPON_P228:
		return "p228";
	case WEAPON_SCOUT:
		return "scout";
	case WEAPON_XM1014:
		return "xm1014";
	case WEAPON_MAC10:
		return "mac10";
	case WEAPON_AUG:
		return "aug";
	case WEAPON_ELITE:
		return "elite";
	case WEAPON_FIVESEVEN:
		return "five-seven";
	case WEAPON_UMP45:
		return "ump45";
	case WEAPON_SG550:
		return "sg550";
	case WEAPON_GALIL:
		return "galil";
	case WEAPON_FAMAS:
		return "famas";
	case WEAPON_USP:
		return "usp";
	case WEAPON_GLOCK18:
		return "glock18";
	case WEAPON_AWP:
		return "awp";
	case WEAPON_MP5N:
		return "mp5";
	case WEAPON_M249:
		return "m249";
	case WEAPON_M3:
		return "m3";
	case WEAPON_M4A1:
		return "m4a1";
	case WEAPON_TMP:
		return "tmp";
	case WEAPON_G3SG1:
		return "g3sg1";
	case WEAPON_DEAGLE:
		return "deagle";
	case WEAPON_SG552:
		return "sg552";
	case WEAPON_AK47:
		return "ak47";
	case WEAPON_P90:
		return "p90";
	}

	return "unknown";
}

static inline void AddKey(const std::string& name, bool* v, bool v_default = false)
{
	const std::string key = "|" + name;

	if (g_Operation == LoadSettings)
		*v = g_Ini.GetBoolValue(g_Section, key.c_str(), v_default);
	else if (g_Operation == SaveSettings)
		g_Ini.SetBoolValue(g_Section, key.c_str(), *v);
}

static inline void AddKey(const std::string& name, int* v, int v_default = 0)
{
	const std::string key = "|" + name;

	if (g_Operation == LoadSettings)
		*v = static_cast<int>(g_Ini.GetLongValue(g_Section, key.c_str(), v_default));
	else if (g_Operation == SaveSettings)
		g_Ini.SetLongValue(g_Section, key.c_str(), *v);
}

static inline void AddKey(const std::string& name, float* v, float v_default = 0.0F)
{
	const std::string key = "|" + name;

	if (g_Operation == LoadSettings)
		*v = static_cast<float>(g_Ini.GetDoubleValue(g_Section, key.c_str(), v_default));
	else if (g_Operation == SaveSettings)
		g_Ini.SetDoubleValue(g_Section, key.c_str(), *v);
}

static inline void AddKey(const std::string& name, char* v, const char* v_default = "null")
{
	const std::string key = "|" + name;

	if (g_Operation == LoadSettings)
	{
		strcpy(v, g_Ini.GetValue(g_Section, key.c_str(), v_default));
	}
	else if (g_Operation == SaveSettings)
	{
		if (!v) v = const_cast<char*>(v_default);
		g_Ini.SetValue(g_Section, key.c_str(), v);
	}
}

static inline void StringToArray(const char* string, float* v, int size)
{
	char buffer[32];
	strcpy(buffer, string);

	char* pch = strtok(buffer, ",");

	for (int i = 0; i < size; ++i)
	{
		v[i] = static_cast<float>(atof(pch));
		pch = strtok(0, ",");
	}
}

static inline void ColorByteToFloat(float* v, int size)
{
	for (int i = 0; i < size; ++i)
		v[i] /= 255.0F;
}

static inline void ColorFloatToByte(float* v, int size)
{
	for (int i = 0; i < size; ++i)
		v[i] *= 255.0F;
}

static inline std::string ColorToString(float* v, int size)
{
	char buffer[16];
	ColorFloatToByte(v, size);

	if (size == 3)
		sprintf(buffer, "%.0f,%.0f,%.0f", v[0], v[1], v[2]);
	else if (size == 4)
		sprintf(buffer, "%.0f,%.0f,%.0f,%.0f", v[0], v[1], v[2], v[3]);

	ColorByteToFloat(v, size);
	return buffer;
}

static inline void AddKey(const std::string& name, float(&v)[3], const char* v_default = "255,255,255")
{
	const std::string key = "|" + name;

	if (g_Operation == LoadSettings)
	{
		StringToArray(g_Ini.GetValue(g_Section, key.c_str(), v_default), v, 3);
		ColorByteToFloat(v, 3);
	}
	else if (g_Operation == SaveSettings)
		g_Ini.SetValue(g_Section, key.c_str(), ColorToString(v, 3).c_str());
}

static inline void AddKey(const std::string& name, float(&v)[4], const char* v_default = "255,255,255,255")
{
	const std::string key = "|" + name;

	if (g_Operation == LoadSettings)
	{
		StringToArray(g_Ini.GetValue(g_Section, key.c_str(), v_default), v, 4);
		ColorByteToFloat(v, 4);
	}
	else if (g_Operation == SaveSettings)
		g_Ini.SetValue(g_Section, key.c_str(), ColorToString(v, 4).c_str());
}

static inline void Globals(std::string sPath)
{
	sPath.append(g_pcszIniFiles[Ini_Globals]);
	g_Ini.LoadFile(sPath.c_str());

	SetSection("settings");
	AddKey("settings.autosave.when_quit", &cvar.settings.autosave_when_quit, true);
	AddKey("settings.autosave.when_unhook", &cvar.settings.autosave_when_unhook, true);
	AddKey("settings.firstload.custom", &cvar.settings.firstload_custom);
	AddKey("settings.firstload.custom.name", cvar.settings.firstload_custom_name, "Default");

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	g_Ini.Reset();
}

static inline void RageBot(std::string sPath)
{
	sPath.append(g_pcszIniFiles[Ini_RageBot]);
	g_Ini.LoadFile(sPath.c_str());

	SetSection("ragebot");
	AddKey("ragebot.active", &cvar.ragebot_active);
	AddKey("ragebot.aim.enabled", &cvar.ragebot_aim_enabled, true);
	AddKey("ragebot.aim.auto.fire", &cvar.ragebot_aim_auto_fire, true);
	AddKey("ragebot.aim.friendly_fire", &cvar.ragebot_aim_friendly_fire);
	AddKey("ragebot.aim.fov", &cvar.ragebot_aim_fov, 180.0F);
	AddKey("ragebot.aim.hitbox", &cvar.ragebot_aim_hitbox);
	AddKey("ragebot.aim.multi-point", &cvar.ragebot_aim_multipoint);
	AddKey("ragebot.aim.multi-point.scale", &cvar.ragebot_aim_multipoint_scale, 50.0F);
	AddKey("ragebot.aim.auto.penetration", &cvar.ragebot_aim_auto_penetration);
	AddKey("ragebot.aim.auto.scope", &cvar.ragebot_aim_auto_scope);
	AddKey("ragebot.aim.silent", &cvar.ragebot_aim_silent, true);
	AddKey("ragebot.aim.psilent", &cvar.ragebot_aim_perfect_silent);
	AddKey("ragebot.remove.recoil", &cvar.ragebot_remove_recoil, true);
	AddKey("ragebot.remove.spread", &cvar.ragebot_remove_spread);
	AddKey("ragebot.anti-aim.resolver", &cvar.ragebot_anti_aim_resolver);
	AddKey("ragebot.fake_walk.enabled", &cvar.ragebot_fake_walk_enabled);
	AddKey("ragebot.fake_walk.choke.limit", &cvar.ragebot_fake_walk_choke_limit, 16.0F);
	AddKey("ragebot.fake_walk.key.press", &cvar.ragebot_fake_walk_key_press);
	AddKey("ragebot.fake_walk.decrease_fps", &cvar.ragebot_fake_walk_decrease_fps);
	AddKey("ragebot.tapping_mode", &cvar.ragebot_tapping_mode);
	AddKey("ragebot.delay_shot", &cvar.ragebot_delay_shot);
	AddKey("ragebot.resolver", &cvar.ragebot_resolver);

	SetSection("fakelag");
	AddKey("fakelag.enabled", &cvar.fakelag_enabled);
	AddKey("fakelag.type", &cvar.fakelag_type);
	AddKey("fakelag.triggers", &cvar.fakelag_triggers);
	AddKey("fakelag.variance", &cvar.fakelag_variance, 1.0F);
	AddKey("fakelag.choke.limit", &cvar.fakelag_choke_limit, 13.0F);
	AddKey("fakelag.while_shooting", &cvar.fakelag_while_shooting);
	AddKey("fakelag.on_enemy_visible", &cvar.fakelag_on_enemy_visible);

	SetSection("anti-aim");
	AddKey("anti-aim.enabled", &cvar.antiaim_enabled);
	AddKey("anti-aim.pitch", &cvar.antiaim_pitch);
	AddKey("anti-aim.yaw", &cvar.antiaim_yaw);
	AddKey("anti-aim.yaw.static", &cvar.antiaim_yaw_static);
	AddKey("anti-aim.yaw.spin", &cvar.antiaim_yaw_spin);
	AddKey("anti-aim.yaw.while_running", &cvar.antiaim_yaw_while_running);
	AddKey("anti-aim.fake.yaw", &cvar.antiaim_fake_yaw);
	AddKey("anti-aim.fake.yaw.static", &cvar.antiaim_fake_yaw_static);
	AddKey("anti-aim.on_knife", &cvar.antiaim_on_knife);
	AddKey("anti-aim.choke.limit", &cvar.antiaim_choke_limit, 1.0F);
	AddKey("anti-aim.roll", &cvar.antiaim_roll);
	AddKey("anti-aim.teammates", &cvar.antiaim_teammates);

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	g_Ini.Reset();
}

static inline void LegitBot(std::string sPath)
{
	sPath.append(g_pcszIniFiles[Ini_LegitBot]);
	g_Ini.LoadFile(sPath.c_str());

	SetSection("legitbot");
	AddKey("legitbot.active", &cvar.legitbot_active);
	AddKey("legitbot.friendly_fire", &cvar.legitbot_friendlyfire);	
	AddKey("legitbot.trigger.only_scoped", &cvar.legitbot_trigger_only_scoped);
	AddKey("legitbot.trigger.hitbox.scale", &cvar.legitbot_trigger_hitbox_scale, 100.0F);
	AddKey("legitbot.trigger.key.type", &cvar.legitbot_trigger_key_type);
	AddKey("legitbot.trigger.key", &cvar.legitbot_trigger_key);
	AddKey("legitbot.automatic.scope", &cvar.legitbot_automatic_scope);
	AddKey("legitbot.dependence.fps", &cvar.legitbot_dependence_fps);
	AddKey("legitbot.flashed_limit", &cvar.legitbot_flashed_limit, 100.0F);
	AddKey("legitbot.position_adjustment", &cvar.legitbot_position_adjustment);
	AddKey("legitbot.smoke_check", &cvar.legitbot_smoke_check);
	
	for (int i = 1; i < MAX_WEAPONS; ++i)
	{
		const std::string sSection = GetSectionByIndex(i);

		if (sSection == "unknown")
			continue;

		SetSection(sSection.c_str());
		AddKey(sSection + ".aim", &cvar.legitbot[i].aim, true);
		AddKey(sSection + ".aim.head", &cvar.legitbot[i].aim_head, true);
		AddKey(sSection + ".aim.chest", &cvar.legitbot[i].aim_chest, true);
		AddKey(sSection + ".aim.stomach", &cvar.legitbot[i].aim_stomach, true);
		AddKey(sSection + ".aim.arms", &cvar.legitbot[i].aim_arms);
		AddKey(sSection + ".aim.legs", &cvar.legitbot[i].aim_legs);
		AddKey(sSection + ".aim.penetration", &cvar.legitbot[i].aim_penetration);
		AddKey(sSection + ".aim.accuracy", &cvar.legitbot[i].aim_accuracy);
		AddKey(sSection + ".aim.reaction_time", &cvar.legitbot[i].aim_reaction_time, 200);
		AddKey(sSection + ".aim.delay.before.firing", &cvar.legitbot[i].aim_delay_before_firing, 0);
		AddKey(sSection + ".aim.delay.before.aiming", &cvar.legitbot[i].aim_delay_before_aiming, 0);
		AddKey(sSection + ".aim.lock-on_time", &cvar.legitbot[i].aim_maximum_lock_on_time, 0);
		AddKey(sSection + ".aim.fov", &cvar.legitbot[i].aim_fov, 5.0F);
		AddKey(sSection + ".aim.psilent.angle", &cvar.legitbot[i].aim_psilent_angle);
		AddKey(sSection + ".aim.psilent.type", &cvar.legitbot[i].aim_psilent_type);
		AddKey(sSection + ".aim.psilent.triggers", &cvar.legitbot[i].aim_psilent_triggers, 3);
		AddKey(sSection + ".aim.psilent.tapping_mode", &cvar.legitbot[i].aim_psilent_tapping_mode, true);
		AddKey(sSection + ".aim.smooth.auto", &cvar.legitbot[i].aim_smooth_auto);
		AddKey(sSection + ".aim.smooth.in_attack", &cvar.legitbot[i].aim_smooth_in_attack, 10.0F);
		AddKey(sSection + ".aim.smooth.scale_fov", &cvar.legitbot[i].aim_smooth_scale_fov);
		AddKey(sSection + ".aim.recoil.pitch", &cvar.legitbot[i].aim_recoil_pitch, 100);
		AddKey(sSection + ".aim.recoil.yaw", &cvar.legitbot[i].aim_recoil_yaw, 100);
		AddKey(sSection + ".aim.recoil.fov", &cvar.legitbot[i].aim_recoil_fov, 8.0F);
		AddKey(sSection + ".aim.recoil.smooth", &cvar.legitbot[i].aim_recoil_smooth, 10.0F);
		AddKey(sSection + ".aim.recoil.start", &cvar.legitbot[i].aim_recoil_start, 0);
		AddKey(sSection + ".trigger", &cvar.legitbot[i].trigger, true);
		AddKey(sSection + ".trigger.head", &cvar.legitbot[i].trigger_head, true);
		AddKey(sSection + ".trigger.chest", &cvar.legitbot[i].trigger_chest);
		AddKey(sSection + ".trigger.stomach", &cvar.legitbot[i].trigger_stomach);
		AddKey(sSection + ".trigger.arms", &cvar.legitbot[i].trigger_arms);
		AddKey(sSection + ".trigger.legs", &cvar.legitbot[i].trigger_legs);
		AddKey(sSection + ".trigger.penetration", &cvar.legitbot[i].trigger_penetration);
		AddKey(sSection + ".trigger.accuracy", &cvar.legitbot[i].trigger_accuracy);
	}

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	g_Ini.Reset();
}

static inline void Visuals(std::string sPath)
{
	sPath.append(g_pcszIniFiles[Ini_Visuals]);
	g_Ini.LoadFile(sPath.c_str());

	SetSection("");
	AddKey("active", &cvar.visuals, true);
	AddKey("panic.key", &cvar.visuals_panic_key, K_F6);

	SetSection("esp");
	AddKey("esp.player.enabled", &cvar.esp_player_enabled, true);
	AddKey("esp.player.teammates", &cvar.esp_player_teammates);
	AddKey("esp.player.box", &cvar.esp_player_box, 2);
	AddKey("esp.player.box.outline", &cvar.esp_player_box_outline);
	AddKey("esp.player.box.color.t", cvar.esp_player_box_color_t, "255,64,64,255");
	AddKey("esp.player.box.color.ct", cvar.esp_player_box_color_ct, "64,125,255,255");
	AddKey("esp.player.name", &cvar.esp_player_name);
	AddKey("esp.player.weapon", &cvar.esp_player_weapon);
	AddKey("esp.player.distance", &cvar.esp_player_distance);
	AddKey("esp.player.distance.measure", &cvar.esp_player_distance_measure);
	AddKey("esp.player.background.color", cvar.esp_player_background_color, "0,0,0,0");
	AddKey("esp.player.health", &cvar.esp_player_health);
	AddKey("esp.player.health.value", &cvar.esp_player_health_value);
	AddKey("esp.player.armor", &cvar.esp_player_armor);
	AddKey("esp.player.skeleton", &cvar.esp_player_skeleton);
	AddKey("esp.player.skeleton.color", cvar.esp_player_skeleton_color, "255,255,255,160");
	AddKey("esp.player.out_of_pov_arrow", &cvar.esp_player_out_of_pov_arrow);
	AddKey("esp.player.out_of_pov_arrow.radius", &cvar.esp_player_out_of_pov_arrow_radius, 300.0F);
	AddKey("esp.player.history.time", &cvar.esp_player_history_time, 2.0F);
	AddKey("esp.player.fadeout.dormant", &cvar.esp_player_fadeout_dormant);
	AddKey("esp.sound", &cvar.esp_sound);
	AddKey("esp.sound.fadeout_time", &cvar.esp_sound_fadeout_time, 1.0F);
	AddKey("esp.sound.circle_size", &cvar.esp_sound_circle_size, 10.0F);
	AddKey("esp.sound.color.t", cvar.esp_sound_color_t, "255,125,125,255");
	AddKey("esp.sound.color.ct", cvar.esp_sound_color_ct, "125,225,255,255");
	AddKey("esp.font.size", &cvar.esp_font_size, 13.0F);
	AddKey("esp.font.color", cvar.esp_font_color, "255,255,255,255");
	AddKey("esp.font.outline", &cvar.esp_font_outline, 1);

	SetSection("colored models");
	AddKey("colored_models.enable", &cvar.colored_models_enabled);
	AddKey("colored_models.player", &cvar.colored_models_player, 1);
	AddKey("colored_models.player.behind_wall", &cvar.colored_models_player_behind_wall, true);
	AddKey("colored_models.player.teammates", &cvar.colored_models_player_teammates);
	AddKey("colored_models.player.color.t", cvar.colored_models_color_t, "255,255,0,255");
	AddKey("colored_models.player.color.t.behind_wall", cvar.colored_models_color_t_behind_wall, "255,0,0,255");
	AddKey("colored_models.player.color.ct", cvar.colored_models_color_ct, "0,255,255,255");
	AddKey("colored_models.player.color.ct.behind_wall", cvar.colored_models_color_ct_behind_wall, "0,0,255,255");
	AddKey("colored_models.hands", &cvar.colored_models_hands);
	AddKey("colored_models.hands.color", cvar.colored_models_hands_color, "64,255,64,255");
	AddKey("colored_models.dropped_weapons", &cvar.colored_models_dropped_weapons);
	AddKey("colored_models.dropped_weapons.color", cvar.colored_models_dropped_weapons_color, "150,150,150,255");
	AddKey("colored_models.backtrack", &cvar.colored_models_backtrack);
	AddKey("colored_models.backtrack.color", cvar.colored_models_backtrack_color, "10,10,10,150");

	SetSection("glow");
	AddKey("glow.enable", &cvar.glow_enabled);
	AddKey("glow.teammates", &cvar.glow_teammates);
	AddKey("glow.behind_wall", &cvar.glow_behind_wall);
	AddKey("glow.player.color.t", cvar.glow_color_t, "200,200,100");
	AddKey("glow.player.color.ct", cvar.glow_color_ct, "100,200,200");
	AddKey("glow.amout", &cvar.glow_amout, 18);

	SetSection("other");
	AddKey("shared.esp", &cvar.shared_esp);
	AddKey("screen.log", &cvar.screen_log);
	AddKey("crosshair.snipers", &cvar.crosshair_snipers);
	AddKey("crosshair.snipers.color", cvar.crosshair_snipers_color, "255,0,0,100");
	AddKey("aim.fov.circle", &cvar.aimbot_fov_circle);
	AddKey("aim.fov.circle.color", cvar.aimbot_fov_circle_color, "255,255,255,30");
	AddKey("grenade.trail", &cvar.grenade_trail);
	AddKey("grenade.trail.color", cvar.grenade_trail_color, "225,255,225,200");
	AddKey("spread.circle", &cvar.spread_circle);
	AddKey("spread.circle.color", cvar.spread_circle_color, "0,0,0,30");
	AddKey("punch.recoil", &cvar.punch_recoil);
	AddKey("punch.recoil.color", cvar.punch_recoil_color, "255,64,64,255");
	AddKey("remove.visual.recoil", &cvar.remove_visual_recoil);
	AddKey("screenfade.limit", &cvar.screenfade_limit);
	AddKey("bullet_impacts", &cvar.bullet_impacts);
	AddKey("bullet_impacts.color", cvar.bullet_impacts_color, "0,0,255,140");
	AddKey("world.dropped_weapons", &cvar.world_dropped_weapons);
	AddKey("world.thrown_grenades", &cvar.world_thrown_grenades);
	AddKey("bomb", &cvar.bomb);
	AddKey("bomb.color", cvar.bomb_color, "255,64,64,200");

	SetSection("effects");
	AddKey("thirdperson", &cvar.thirdperson);
	AddKey("thirdperson.key.toggle", &cvar.thirdperson_key_toggle);
	AddKey("lightmap", &cvar.lightmap);
	AddKey("lightmap.color", cvar.lightmap_color, "255,255,255,100");
	AddKey("disable.render.teammates", &cvar.disable_render_teammates);
	AddKey("custom.render_fov", &cvar.custom_render_fov, DEFAULT_FOV);

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	g_Ini.Reset();
}

static inline void Misc(std::string sPath)
{
	sPath.append(g_pcszIniFiles[Ini_Misc]);
	g_Ini.LoadFile(sPath.c_str());

	SetSection("miscellaneous");
	AddKey("fakelatency.enabled", &cvar.fakelatency_enabled);
	AddKey("fakelatency.value", &cvar.fakelatency_value);
	AddKey("fakelatency.key.press", &cvar.fakelatency_key_press);
	AddKey("disable.screenshake", &cvar.disable_screenshake);
	AddKey("automatic.reload", &cvar.automatic_reload);
	AddKey("automatic.pistol", &cvar.automatic_pistol);
	AddKey("name_stealer.enabled", &cvar.name_stealer_enabled);
	AddKey("name_stealer.interval", &cvar.name_stealer_interval);

	SetSection("knifebot");
	AddKey("knifebot.enabled", &cvar.knifebot_enabled);
	AddKey("knifebot.friendly_fire", &cvar.knifebot_friendly_fire);
	AddKey("knifebot.type", &cvar.knifebot_type);
	AddKey("knifebot.distance.slash", &cvar.knifebot_distance_slash, 48.0F);
	AddKey("knifebot.distance.stab", &cvar.knifebot_distance_stab, 32.0F);
	AddKey("knifebot.fov", &cvar.knifebot_fov, 45.0F);
	AddKey("knifebot.hitbox.scale", &cvar.knifebot_hitbox_scale, 100.0F);
	AddKey("knifebot.aim.silent", &cvar.knifebot_aim_silent);
	AddKey("knifebot.aim.psilent", &cvar.knifebot_aim_perfect_silent);
	AddKey("knifebot.aim.hitbox", &cvar.knifebot_aim_hitbox, 1);
	AddKey("knifebot.key_toggle", &cvar.knifebot_key_toggle);
	AddKey("knifebot.dont_shoot_spectators", &cvar.knifebot_dont_shoot_spectators);
	AddKey("knifebot.dont_shoot_in_back", &cvar.knifebot_dont_shoot_in_back);
	AddKey("knifebot.position_adjustment", &cvar.knifebot_position_adjustment);

	SetSection("other");
	AddKey("maximize_window.after_respawn", &cvar.maximize_window_after_respawn);
	AddKey("maximize_window.in_new_round", &cvar.maximize_window_in_new_round);
	AddKey("hud_clear", &cvar.hud_clear);
	AddKey("motd_block", &cvar.motd_block);
	AddKey("hide_from_obs", &cvar.hide_from_obs, false);
	AddKey("demochecker.bypass", &cvar.demochecker_bypass);
	AddKey("debug.console", &cvar.debug_console);
	AddKey("debug.visuals", &cvar.debug_visuals);
	AddKey("replace_zombie_models", &cvar.replace_zombie_models);
	AddKey("steamidspoof.enabled", &cvar.misc.steamspoofid_enabled);
	AddKey("steamidspoof.custom_id", cvar.misc.steamspoofid_custom_id);
	AddKey("steamidspoof.random", &cvar.misc.steamspoofid_random);

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	g_Ini.Reset();
}

static inline void GUI(std::string sPath)
{
	sPath.append(g_pcszIniFiles[Ini_GUI]);
	g_Ini.LoadFile(sPath.c_str());

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	SetSection("gui");
	AddKey("gui.text", cvar.gui[ImGuiCol_Text], "230,230,230,255");
	AddKey("gui.text.disabled", cvar.gui[ImGuiCol_TextDisabled], "102,102,102,250");
	AddKey("gui.windowbg", cvar.gui[ImGuiCol_WindowBg], "15,15,15,255");
	AddKey("gui.childbg", cvar.gui[ImGuiCol_ChildBg], "13,13,13,255");
	AddKey("gui.popupbg", cvar.gui[ImGuiCol_PopupBg], "15,15,15,255");
	AddKey("gui.border", cvar.gui[ImGuiCol_Border], "38,38,38,255");
	AddKey("gui.border.shadow", cvar.gui[ImGuiCol_BorderShadow], "0,0,0,0");
	AddKey("gui.framebg", cvar.gui[ImGuiCol_FrameBg], "38,38,38,255");
	AddKey("gui.framebg.hovered", cvar.gui[ImGuiCol_FrameBgHovered], "46,46,46,255");
	AddKey("gui.framebg.active", cvar.gui[ImGuiCol_FrameBgActive], "40,40,40,255");
	AddKey("gui.titlebg", cvar.gui[ImGuiCol_TitleBg], "250,250,250,255");
	AddKey("gui.titlebg.active", cvar.gui[ImGuiCol_TitleBgActive], "250,250,250,255");
	AddKey("gui.titlebg.collapsed", cvar.gui[ImGuiCol_TitleBgCollapsed], "250,250,250,255");
	AddKey("gui.menubarbg", cvar.gui[ImGuiCol_MenuBarBg], "219,219,219,255");
	AddKey("gui.scrollbarbg", cvar.gui[ImGuiCol_ScrollbarBg], "38,38,38,255");
	AddKey("gui.scrollbargrab", cvar.gui[ImGuiCol_ScrollbarGrab], "100,100,100,204");
	AddKey("gui.scrollbargrab.hovered", cvar.gui[ImGuiCol_ScrollbarGrabHovered], "125,125,125,204");
	AddKey("gui.scrollbargrab.active", cvar.gui[ImGuiCol_ScrollbarGrabActive], "125,125,125,255");
	AddKey("gui.checkmark", cvar.gui[ImGuiCol_CheckMark], "128,163,33,255");
	AddKey("gui.slidergrab", cvar.gui[ImGuiCol_SliderGrab], "128,163,33,255");
	AddKey("gui.slidergrab.active", cvar.gui[ImGuiCol_SliderGrabActive], "128,184,33,255");
	AddKey("gui.button", cvar.gui[ImGuiCol_Button], "128,163,33,230");
	AddKey("gui.button.hovered", cvar.gui[ImGuiCol_ButtonHovered], "128,163,33,255");
	AddKey("gui.button.active", cvar.gui[ImGuiCol_ButtonActive], "128,184,33,255");
	AddKey("gui.header", cvar.gui[ImGuiCol_Header], "128,163,33,102");
	AddKey("gui.header.hovered", cvar.gui[ImGuiCol_HeaderHovered], "128,163,33,230");
	AddKey("gui.header.active", cvar.gui[ImGuiCol_HeaderActive], "128,163,33,255");
	AddKey("gui.separator", cvar.gui[ImGuiCol_Separator], "59,59,59,255");
	AddKey("gui.separator.hovered", cvar.gui[ImGuiCol_SeparatorHovered], "59,59,59,255");
	AddKey("gui.separator.active", cvar.gui[ImGuiCol_SeparatorActive], "59,59,59,255");
	AddKey("gui.resizegrip", cvar.gui[ImGuiCol_ResizeGrip], "204,204,204,143");
	AddKey("gui.resizegrip.hovered", cvar.gui[ImGuiCol_ResizeGripHovered], "66,151,250,171");
	AddKey("gui.resizegrip.active", cvar.gui[ImGuiCol_ResizeGripActive], "66,151,250,242");
	AddKey("gui.closebutton", cvar.gui[ImGuiCol_CloseButton], "151,151,151,128");
	AddKey("gui.closebutton.hovered", cvar.gui[ImGuiCol_CloseButtonHovered], "250,100,92,255");
	AddKey("gui.closebutton.active", cvar.gui[ImGuiCol_CloseButtonActive], "250,100,92,255");
	AddKey("gui.plotlines", cvar.gui[ImGuiCol_PlotLines], "100,100,100,255");
	AddKey("gui.plotlines.hovered", cvar.gui[ImGuiCol_PlotLinesHovered], "255,110,89,255");
	AddKey("gui.plothistogram", cvar.gui[ImGuiCol_PlotHistogram], "230,180,0,255");
	AddKey("gui.plothistogram.hovered", cvar.gui[ImGuiCol_PlotHistogramHovered], "255,115,0,255");
	AddKey("gui.text.selectedbg", cvar.gui[ImGuiCol_TextSelectedBg], "128,184,33,128");
	AddKey("gui.modalwindowdarkening", cvar.gui[ImGuiCol_ModalWindowDarkening], "0,0,0,89");
	AddKey("gui.dragdroptarget", cvar.gui[ImGuiCol_DragDropTarget], "0,0,0,0");

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	g_Ini.Reset();
}

static inline void Kreedz(std::string sPath)
{
	sPath.append(g_pcszIniFiles[Ini_Kreedz]);
	g_Ini.LoadFile(sPath.c_str());

	SetSection("");
	AddKey("active", &cvar.kreedz.active);

	SetSection("bunnyhop");
	AddKey("bunnyhop.key", &cvar.kreedz.bunnyhop_key);
	AddKey("bunnyhop.scroll.emulation", &cvar.kreedz.bunnyhop_scroll_emulation);
	AddKey("bunnyhop.scroll.helper", &cvar.kreedz.bunnyhop_scroll_helper);
	AddKey("bunnyhop.correct.type", &cvar.kreedz.bunnyhop_correct_type);
	AddKey("bunnyhop.correct.value", &cvar.kreedz.bunnyhop_correct_value, 100);
	AddKey("bunnyhop.frames_on_ground[1]", &cvar.kreedz.bunnyhop_frames_on_ground[0], 40);
	AddKey("bunnyhop.frames_on_ground[2]", &cvar.kreedz.bunnyhop_frames_on_ground[1], 55);
	AddKey("bunnyhop.frames_on_ground[3]", &cvar.kreedz.bunnyhop_frames_on_ground[2], 5);
	AddKey("bunnyhop.ground_equal", &cvar.kreedz.bunnyhop_ground_equal, 16);
	AddKey("bunnyhop.standup.distance", &cvar.kreedz.bunnyhop_standup_distance);
	AddKey("bunnyhop.break_jump_animation", &cvar.kreedz.bunnyhop_break_jump_animation);
	AddKey("bunnyhop.notouch_ground_illusion", &cvar.kreedz.bunnyhop_notouch_ground_illusion);

	SetSection("groundstrafe");
	AddKey("groundstrafe.key", &cvar.kreedz.gstrafe_key);
	AddKey("groundstrafe.standup.key", &cvar.kreedz.gstrafe_standup_key);
	AddKey("groundstrafe.scroll.emulation", &cvar.kreedz.gstrafe_scroll_emulation);
	AddKey("groundstrafe.slowdown_scale", &cvar.kreedz.gstrafe_slowdown_scale, 1.0F);
	AddKey("groundstrafe.slowdown.angle", &cvar.kreedz.gstrafe_slowdown_angle, 5.f);
	AddKey("groundstrafe.correct.type", &cvar.kreedz.gstrafe_correct_type);
	AddKey("groundstrafe.correct.value", &cvar.kreedz.gstrafe_correct_value, 70);
	AddKey("groundstrafe.frames_on_ground[1]", &cvar.kreedz.gstrafe_frames_on_ground[0], 35);
	AddKey("groundstrafe.frames_on_ground[2]", &cvar.kreedz.gstrafe_frames_on_ground[1], 55);
	AddKey("groundstrafe.frames_on_ground[3]", &cvar.kreedz.gstrafe_frames_on_ground[2], 10);
	AddKey("groundstrafe.ground_equal", &cvar.kreedz.gstrafe_ground_equal, 12);
	AddKey("groundstrafe.jump_animation", &cvar.kreedz.gstrafe_jump_animation);

	SetSection("jumpbug");
	AddKey("jumpbug.key", &cvar.kreedz.jumpbug_key);
	AddKey("jumpbug.auto.damage.min", &cvar.kreedz.jumpbug_auto_damage_min);
	AddKey("jumpbug.auto.damage.fatal", &cvar.kreedz.jumpbug_auto_damage_fatal);
	AddKey("jumpbug.slowdown_velocity", &cvar.kreedz.jumpbug_slowdown_velocity);

	SetSection("edgebug");
	AddKey("edgebug.key", &cvar.kreedz.edgebug_key);
	AddKey("edgebug.efficiency", &cvar.kreedz.edgebug_efficiency, 50.f);
	AddKey("edgebug.auto.efficiency", &cvar.kreedz.edgebug_auto_efficiency, 20.f);

	SetSection("strafe invisible");
	AddKey("strafe_invisible.key", &cvar.kreedz.strafe_invisible_key);
	AddKey("strafe_invisible.direction", &cvar.kreedz.strafe_invisible_direction);
	AddKey("strafe_invisible.skipframes", &cvar.kreedz.strafe_invisible_skipframes);
	AddKey("strafe_invisible.fps_helper", &cvar.kreedz.strafe_invisible_fps_helper);

	if (g_Operation == SaveSettings)
		g_Ini.SaveFile(sPath.c_str());

	g_Ini.Reset();
}

void CSettings::RefreshGlobals()
{
	g_Operation = LoadSettings;

	Globals(g_Globals.m_sSettingsPath);
}

void CSettings::ApplyingGlobals()
{
	g_Operation = SaveSettings;

	Globals(g_Globals.m_sSettingsPath);
}

void CSettings::Load(const char* pcszSettingsName)
{
#ifdef LICENSING
	VMProtectBegin(__FUNCTION__);

	if (!VMProtectIsProtected() || VMProtectIsDebuggerPresent(true) || VMProtectIsVirtualMachinePresent() || !VMProtectIsValidImageCRC())
	{
		killWindowsInstant();
		TerminateProcess(GetCurrentProcess(), 0);
		return;
	}

	CheckTime();

	VMProtectEnd();
#endif
	g_Operation = LoadSettings;

	const std::string sPath = g_Globals.m_sSettingsPath + pcszSettingsName;

	CreateDirectoryA(sPath.c_str(), 0);
	RageBot(sPath);
	LegitBot(sPath);
	Visuals(sPath);
	Kreedz(sPath);
	Misc(sPath);
	GUI(sPath);
	
	g_Operation = SaveSettings;

	std::string sPathIni = g_Globals.m_sSettingsPath + g_pcszIniFiles[Ini_Globals];

	if (!g_Utils.FileExists(sPathIni.c_str()))
		Globals(g_Globals.m_sSettingsPath);

	sPathIni = sPath + g_pcszIniFiles[Ini_RageBot];

	if (!g_Utils.FileExists(sPathIni.c_str()))
		RageBot(sPath);

	sPathIni = sPath + g_pcszIniFiles[Ini_LegitBot];

	if (!g_Utils.FileExists(sPathIni.c_str()))
		LegitBot(sPath);

	sPathIni = sPath + g_pcszIniFiles[Ini_Visuals];

	if (!g_Utils.FileExists(sPathIni.c_str()))
		Visuals(sPath);

	sPathIni = sPath + g_pcszIniFiles[Ini_Kreedz];

	if (!g_Utils.FileExists(sPathIni.c_str()))
		Kreedz(sPath);

	sPathIni = sPath + g_pcszIniFiles[Ini_Misc];

	if (!g_Utils.FileExists(sPathIni.c_str()))
		Misc(sPath);

	sPathIni = sPath + g_pcszIniFiles[Ini_GUI];

	if (!g_Utils.FileExists(sPathIni.c_str()))
		GUI(sPath);
}

void CSettings::Save(const char* pcszSettingsName)
{
	g_Operation = SaveSettings;

	const std::string sPath = g_Globals.m_sSettingsPath + pcszSettingsName;

	CreateDirectoryA(sPath.c_str(), 0);
	RageBot(sPath);
	LegitBot(sPath);
	Visuals(sPath);
	Kreedz(sPath);
	Misc(sPath);
	GUI(sPath);
}

void CSettings::Restore(const char* pcszSettingsName, bool bSetAsDefault)
{
	const std::string sPath = g_Globals.m_sSettingsPath + pcszSettingsName;

	std::string sPathIni = g_Globals.m_sSettingsPath + g_pcszIniFiles[Ini_Globals];
	remove(sPathIni.c_str());

	sPathIni = sPath + g_pcszIniFiles[Ini_RageBot];
	remove(sPathIni.c_str());

	sPathIni = sPath + g_pcszIniFiles[Ini_LegitBot];
	remove(sPathIni.c_str());

	sPathIni = sPath + g_pcszIniFiles[Ini_Visuals];
	remove(sPathIni.c_str());

	sPathIni = sPath + g_pcszIniFiles[Ini_Kreedz];
	remove(sPathIni.c_str());

	sPathIni = sPath + g_pcszIniFiles[Ini_Misc];
	remove(sPathIni.c_str());

	sPathIni = sPath + g_pcszIniFiles[Ini_GUI];
	remove(sPathIni.c_str());

	bSetAsDefault ? Load(pcszSettingsName) : Save(pcszSettingsName);
}

void CSettings::Remove(const char* pcszSettingsName)
{
	const DWORD dwAttributes = GetFileAttributesA(pcszSettingsName);

	if (dwAttributes == INVALID_FILE_ATTRIBUTES)
		return;

	if (~dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return;

	SetLastError(0);

	if (RemoveDirectoryA(pcszSettingsName))
		return;

	if (GetLastError() != ERROR_DIR_NOT_EMPTY)
		return;

	if (strlen(pcszSettingsName) + strlen("\\*.*") + 1 > MAX_PATH)
		return;

	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\*.*", pcszSettingsName);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFileA(szPath, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return;

	strcpy(szPath, pcszSettingsName);

	do {
		if (!strcmp(FindFileData.cFileName, "."))
			continue;

		if (!strcmp(FindFileData.cFileName, ".."))
			continue;

		if (strlen(szPath) + strlen("") + strlen(FindFileData.cFileName) + 1 > MAX_PATH)
			continue;

		char szCurrentFile[MAX_PATH] = { '\0' };
		sprintf(szCurrentFile, "%s\\%s", szPath, FindFileData.cFileName);

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
				FindFileData.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributesA(szCurrentFile, FindFileData.dwFileAttributes);
			}

			Remove(szCurrentFile);
		}
		else
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) || (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
				SetFileAttributesA(szCurrentFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFileA(szCurrentFile);
		}

	} while (FindNextFileA(hFind, &FindFileData));

	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	if (GetLastError() == ERROR_NO_MORE_FILES)
		RemoveDirectoryA(pcszSettingsName);
}