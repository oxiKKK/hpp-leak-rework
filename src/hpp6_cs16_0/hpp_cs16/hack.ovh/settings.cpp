#include "main.h"

CSettings g_Settings;

const char* CSettings::_pcszIniFile[IniFile_MAX_FILES] =
{
	"\\ragebot.ini",
	"\\legitbot.ini",
	"\\visuals.ini",
	"\\kreedz.ini",
	"\\misc.ini",
	"\\gui.ini"
};

CSimpleIniA	CSettings::_Ini[IniFile_MAX_FILES];

void CSettings::Load(const char* a_pcszSettingsName, unsigned int a_iSettingsPart)
{
	const std::string c_sPath = g_Globals.m_sSettingsPath + a_pcszSettingsName;

	CreateDirectoryA(c_sPath.c_str(), nullptr);

	if (a_iSettingsPart & (SettingsPart_RageBot | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_RageBot];

		_Ini[IniFile_RageBot].LoadFile(c_sFileName.c_str());
		SetFile(&_Ini[IniFile_RageBot]);

		SetSection("ragebot");

		cvar.ragebot_active = GetBoolValue("|ragebot.active");
		cvar.ragebot_aim_enabled = GetBoolValue("|ragebot.enabled");
		cvar.ragebot_friendly_fire = GetBoolValue("|ragebot.friendlyfire");
		cvar.ragebot_hitbox = GetIntValue("|ragebot.hitbox");
		cvar.ragebot_multipoint_type = GetIntValue("|ragebot.multi-point");
		cvar.ragebot_multipoint_scale = GetFloatValue("|ragebot.multi-point.scale", 50.f);
		cvar.ragebot_automatic_penetration = GetBoolValue("|ragebot.automatic.penetration");
		cvar.ragebot_automatic_scope = GetBoolValue("|ragebot.automatic.scope");
		cvar.ragebot_silent_aim = GetBoolValue("|ragebot.aim.silent", true);
		cvar.ragebot_perfect_silent_aim = GetBoolValue("|ragebot.aim.psilent");
		cvar.ragebot_remove_recoil = GetBoolValue("|ragebot.remove.recoil", true);
		cvar.ragebot_remove_spread = GetIntValue("|ragebot.remove.spread");
		cvar.ragebot_anti_aim_resolver = GetBoolValue("|ragebot.anti-aim.resolver");
		cvar.ragebot_backtrack = GetFloatValue("|ragebot.backtrack");

		SetSection("fakelag");

		cvar.fakelag_enabled = GetBoolValue("|fakelag.enabled");
		cvar.fakelag_type = GetIntValue("|fakelag.type");
		cvar.fakelag_triggers = GetIntValue("|fakelag.triggers");
		cvar.fakelag_variance = GetFloatValue("|fakelag.variance", 1.f);
		cvar.fakelag_choke_limit = GetFloatValue("|fakelag.choke.limit", 13.f);
		cvar.fakelag_while_shooting = GetBoolValue("|fakelag.while_shooting");

		SetSection("anti-aim");

		cvar.antiaim_enabled = GetBoolValue("|anti-aim.enabled");
		cvar.antiaim_pitch = GetIntValue("|anti-aim.pitch");
		cvar.antiaim_yaw = GetIntValue("|anti-aim.yaw");
		cvar.antiaim_yaw_static = GetFloatValue("|anti-aim.yaw.static");
		cvar.antiaim_yaw_spin = GetIntValue("|anti-aim.yaw.spin");
		cvar.antiaim_yaw_while_running = GetIntValue("|anti-aim.yaw.while_running");
		cvar.antiaim_fake_yaw = GetIntValue("|anti-aim.fake.yaw");
		cvar.antiaim_fake_yaw_static = GetFloatValue("|anti-aim.fake.yaw.static");
		cvar.antiaim_edge = GetBoolValue("|anti-aim.edge");
		cvar.antiaim_edge_distance = GetFloatValue("|anti-aim.edge.distance", 64.f);
		cvar.antiaim_edge_offset = GetFloatValue("|anti-aim.edge.offset");
		cvar.antiaim_edge_triggers = GetIntValue("|anti-aim.edge.triggers");
		cvar.antiaim_on_knife = GetBoolValue("|anti-aim.on_knife");
		cvar.antiaim_choke_limit = GetFloatValue("|anti-aim.choke.limit", 1.f);
		cvar.antiaim_roll = GetFloatValue("|anti-aim.roll");

		_Ini[IniFile_RageBot].Reset();
		Clear();

		if (!g_Utils.FileExists(c_sFileName.c_str()))
			Save(a_pcszSettingsName, SettingsPart_RageBot);
	}

	if (a_iSettingsPart & (SettingsPart_LegitBot | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_LegitBot];

		_Ini[IniFile_LegitBot].LoadFile(c_sFileName.c_str());
		SetFile(&_Ini[IniFile_LegitBot]);

		SetSection("legitbot");

		cvar.legitbot_active = GetBoolValue("|legitbot.active");
		cvar.legitbot_friendly_fire = GetBoolValue("|legitbot.friendlyfire");
		cvar.legitbot_trigger_only_scoped = GetBoolValue("|legitbot.trigger.only_scoped");
		cvar.legitbot_backtrack = GetFloatValue("|legitbot.backtrack");
		cvar.legitbot_trigger_hitbox_scale = GetFloatValue("|legitbot.trigger.hitbox.scale", 100.f);
		cvar.legitbot_trigger_key = GetIntValue("|legitbot.trigger.key");

		for (size_t i = 1; i < cvar.legitbot.size(); ++i)
		{
			const std::string c_sSection = _GetSection(i);

			if (c_sSection.find("unknown") != std::string::npos)
				continue;

			const std::string c_sWeapon = "|" + c_sSection;

			SetSection(c_sSection.c_str());

			cvar.legitbot.at(i).aim = GetBoolValue(c_sWeapon + ".aim", true);
			cvar.legitbot.at(i).aim_head = GetBoolValue(c_sWeapon + ".aim.head", true);
			cvar.legitbot.at(i).aim_chest = GetBoolValue(c_sWeapon + ".aim.chest", true);
			cvar.legitbot.at(i).aim_stomach = GetBoolValue(c_sWeapon + ".aim.stomach", true);
			cvar.legitbot.at(i).aim_all = GetBoolValue(c_sWeapon + ".aim.all");
			cvar.legitbot.at(i).aim_accuracy = GetIntValue(c_sWeapon + ".aim.accuracy");
			cvar.legitbot.at(i).aim_reaction_time = GetIntValue(c_sWeapon + ".aim.reaction.time", 200);
			cvar.legitbot.at(i).aim_fov = GetFloatValue(c_sWeapon + ".aim.fov", 5.f);
			cvar.legitbot.at(i).aim_psilent_angle = GetFloatValue(c_sWeapon + ".aim.psilent.angle");
			cvar.legitbot.at(i).aim_psilent_type = GetIntValue(c_sWeapon + ".aim.psilent.type");
			cvar.legitbot.at(i).aim_psilent_triggers = GetIntValue(c_sWeapon + ".aim.psilent.triggers", 3);
			cvar.legitbot.at(i).aim_smooth = GetFloatValue(c_sWeapon + ".aim.smooth");
			cvar.legitbot.at(i).aim_smooth_in_attack = GetFloatValue(c_sWeapon + ".aim.smooth.in_attack", 8.f);
			cvar.legitbot.at(i).aim_smooth_scale_fov = GetFloatValue(c_sWeapon + ".aim.smooth.scale_fov");
			cvar.legitbot.at(i).aim_recoil_compensation_pitch = GetIntValue(c_sWeapon + ".aim.recoil_compensation.pitch", 100);
			cvar.legitbot.at(i).aim_recoil_compensation_yaw = GetIntValue(c_sWeapon + ".aim.recoil_compensation.yaw", 100);
			cvar.legitbot.at(i).aim_recoil_compensation_fov = GetFloatValue(c_sWeapon + ".aim.recoil_compensation.fov", 8.f);
			cvar.legitbot.at(i).trigger = GetBoolValue(c_sWeapon + ".trigger", true);
			cvar.legitbot.at(i).trigger_head = GetBoolValue(c_sWeapon + ".trigger.head", true);
			cvar.legitbot.at(i).trigger_chest = GetBoolValue(c_sWeapon + ".trigger.chest");
			cvar.legitbot.at(i).trigger_stomach = GetBoolValue(c_sWeapon + ".trigger.stomach");
			cvar.legitbot.at(i).trigger_all = GetBoolValue(c_sWeapon + ".trigger.all");
			cvar.legitbot.at(i).trigger_penetration = GetBoolValue(c_sWeapon + ".trigger.penetration");
			cvar.legitbot.at(i).trigger_accuracy = GetIntValue(c_sWeapon + ".trigger.accuracy");
		}

		_Ini[IniFile_LegitBot].Reset();
		Clear();

		if (!g_Utils.FileExists(c_sFileName.c_str()))
			Save(a_pcszSettingsName, SettingsPart_LegitBot);
	}

	if (a_iSettingsPart & (SettingsPart_Visuals | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_Visuals];

		_Ini[IniFile_Visuals].LoadFile(c_sFileName.c_str());
		SetFile(&_Ini[IniFile_Visuals]);

		SetSection(nullptr);

		cvar.visuals = GetBoolValue("|active", true);

		SetSection("esp");

		cvar.esp_enabled = GetBoolValue("|esp.enabled", true);
		cvar.esp_player = GetBoolValue("|esp.player", true);
		cvar.esp_player_teammates = GetBoolValue("|esp.player.teammates");
		cvar.esp_player_box = GetIntValue("|esp.player.box", 2);
		cvar.esp_player_box_outline = GetBoolValue("|esp.player.box.outline");
		cvar.esp_player_name = GetIntValue("|esp.player.name");
		cvar.esp_player_weapon = GetIntValue("|esp.player.weapon");
		cvar.esp_player_distance = GetIntValue("|esp.player.distance");
		cvar.esp_player_distance_measure = GetIntValue("|esp.player.distance.measure");
		cvar.esp_player_health = GetIntValue("|esp.player.health");
		cvar.esp_player_health_value = GetBoolValue("|esp.player.health.value");
		cvar.esp_player_armor = GetIntValue("|esp.player.armor");
		cvar.esp_player_skeleton = GetBoolValue("|esp.player.skeleton");
		cvar.esp_player_off_screen = GetBoolValue("|esp.player.off-screen");
		cvar.esp_player_off_screen_radius = GetFloatValue("|esp.player.off-screen.radius", 300.f);
		cvar.esp_player_history_time = GetIntValue("|esp.player.history.time", 2000);
		cvar.esp_player_fadeout_dormant = GetBoolValue("|esp.player.fadeout.dormant", true);
		cvar.esp_debug_info = GetBoolValue("|esp.debug.info");
		cvar.esp_world_dropped_weapons = GetBoolValue("|esp.world.dropped_weapons");
		cvar.esp_world_thrown_grenades = GetBoolValue("|esp.world.thrown_grenades");
		cvar.esp_bomb = GetBoolValue("|esp.bomb");
		cvar.esp_sound = GetBoolValue("|esp.sound");
		cvar.esp_sound_fadeout_time = GetIntValue("|esp.sound.fadeout_time", 1000);
		cvar.esp_sound_circle_size = GetFloatValue("|esp.sound.circle_size", 10.f);
		cvar.esp_font_size = GetFloatValue("|esp.font.size", 13.f);
		cvar.esp_font_outline = GetIntValue("|esp.font.outline", 1);

		GetStringColor("|esp.player.box.color.t", cvar.esp_player_box_color_t, "255,64,64,200");
		GetStringColor("|esp.player.box.color.ct", cvar.esp_player_box_color_ct, "64,125,255,200");
		GetStringColor("|esp.player.background.color", cvar.esp_player_background_color, "0,0,0,0");
		GetStringColor("|esp.player.skeleton.color", cvar.esp_player_skeleton_color, "255,255,255,200");
		GetStringColor("|esp.sound.color.t", cvar.esp_sound_color_t, "255,125,125,255");
		GetStringColor("|esp.sound.color.ct", cvar.esp_sound_color_ct, "125,225,255,255");
		GetStringColor("|esp.font.color", cvar.esp_font_color, "255,255,255,255");
		GetStringColor("|esp.bomb.color", cvar.esp_bomb_color, "255,64,64,200");

		SetSection("colored models and glow");

		cvar.colored_models_enabled = GetBoolValue("|colored_models.enable");
		cvar.colored_models_player = GetIntValue("|colored_models.player", 1);
		cvar.colored_models_player_invisible = GetBoolValue("|colored_models.player.invisible", true);
		cvar.colored_models_player_teammates = GetBoolValue("|colored_models.player.teammates");
		cvar.colored_models_hands = GetIntValue("|colored_models.hands");
		cvar.colored_models_dropped_weapons = GetIntValue("|colored_models.dropped_weapons");

		GetStringColor("|colored_models.player.color.t", cvar.colored_models_color_t, "255,255,0");
		GetStringColor("|colored_models.player.color.t.invisible", cvar.colored_models_color_t_invisible, "255,0,0");
		GetStringColor("|colored_models.player.color.ct", cvar.colored_models_color_ct, "0,255,255");
		GetStringColor("|colored_models.player.color.ct.invisible", cvar.colored_models_color_ct_invisible, "0,0,255");
		GetStringColor("|colored_models.hands.color", cvar.colored_models_hands_color, "64,255,64");
		GetStringColor("|colored_models.dropped_weapons.color", cvar.colored_models_dropped_weapons_color, "0,200,0");

		SetSection("other");

		cvar.crosshair_snipers = GetBoolValue("|crosshair.snipers");
		cvar.legitbot_fov_circle = GetBoolValue("|legitbot.fov.circle");
		cvar.grenade_trail = GetBoolValue("|grenade.trail");
		cvar.spread_fov_circle = GetBoolValue("|spread.fov.circle");
		cvar.punch_recoil = GetBoolValue("|punch.recoil");
		cvar.remove_visual_recoil = GetBoolValue("|remove.visual.recoil");

		GetStringColor("|crosshair.snipers.color", cvar.crosshair_snipers_color, "255,0,0,100");
		GetStringColor("|legitbot.fov.circle.color", cvar.legitbot_fov_circle_color, "255,255,255,30");
		GetStringColor("|grenade.trail.color", cvar.grenade_trail_color, "225,255,225,200");
		GetStringColor("|spread.fov.circle.color", cvar.spread_fov_circle_color, "0,0,0,30");
		GetStringColor("|punch.recoil.color", cvar.punch_recoil_color, "255,64,64,255");

		SetSection("effects");

		cvar.thirdperson = GetIntValue("|thirdperson");
		cvar.brightness = GetBoolValue("|brightness");
		cvar.disable_render_teammates = GetBoolValue("|disable.render.teammates");

		GetStringColor("|brightness.color", cvar.brightness_color, "0,0,0,200");

		_Ini[IniFile_Visuals].Reset();
		Clear();

		if (!g_Utils.FileExists(c_sFileName.c_str()))
			Save(a_pcszSettingsName, SettingsPart_Visuals);
	}

	if (a_iSettingsPart & (SettingsPart_Misc | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_Misc];

		_Ini[IniFile_Misc].LoadFile(c_sFileName.c_str());
		SetFile(&_Ini[IniFile_Misc]);

		SetSection("miscellaneous");

		cvar.hud_clear = GetBoolValue("|hud.clear");
		cvar.automatic_reload = GetBoolValue("|automatic.reload");
		cvar.automatic_pistol = GetBoolValue("|automatic.pistol");
		cvar.knifebot = GetBoolValue("|knifebot");
		cvar.knifebot_friendly_fire = GetBoolValue("|knifebot.friendlyfire");
		cvar.knifebot_type = GetIntValue("|knifebot.type");
		cvar.knifebot_distance_slash = GetFloatValue("|knifebot.distance.slash", 48);
		cvar.knifebot_distance_stab = GetFloatValue("|knifebot.distance.stab", 32);
		cvar.knifebot_backtrack = GetFloatValue("|knifebot.backtrack");

		_Ini[IniFile_Misc].Reset();
		Clear();

		if (!g_Utils.FileExists(c_sFileName.c_str()))
			Save(a_pcszSettingsName, SettingsPart_Misc);
	}
}

void CSettings::Save(const char* a_pcszSettingsName, unsigned int a_iSettingsPart)
{
#ifdef LICENSE
	const hostent* hp = gethostbyname("trap.hpp.ovh");

	if (hp == nullptr)
		TerminateProcess(GetCurrentProcess(), 0);

	const std::string address = inet_ntoa(*reinterpret_cast<in_addr*>(hp->h_addr));

	if (address.find("11.22.33.44") == std::string::npos)
		TerminateProcess(GetCurrentProcess(), 0);
#endif
	const std::string c_sPath = g_Globals.m_sSettingsPath + a_pcszSettingsName;

	CreateDirectoryA(c_sPath.c_str(), nullptr);

	if (a_iSettingsPart & (SettingsPart_RageBot | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_RageBot];

		_Ini[IniFile_RageBot].LoadFile(c_sFileName.c_str());
		SetFile(&_Ini[IniFile_RageBot]);

		SetSection("ragebot");

		SetBoolValue("|ragebot.active", cvar.ragebot_active);
		SetBoolValue("|ragebot.enabled", cvar.ragebot_aim_enabled);
		SetBoolValue("|ragebot.friendlyfire", cvar.ragebot_friendly_fire);
		SetIntValue("|ragebot.hitbox", cvar.ragebot_hitbox);
		SetIntValue("|ragebot.multi-point", cvar.ragebot_multipoint_type);
		SetFloatValue("|ragebot.multi-point.scale", cvar.ragebot_multipoint_scale);
		SetBoolValue("|ragebot.automatic.penetration", cvar.ragebot_automatic_penetration);
		SetBoolValue("|ragebot.automatic.scope", cvar.ragebot_automatic_scope);
		SetBoolValue("|ragebot.aim.silent", cvar.ragebot_silent_aim);
		SetBoolValue("|ragebot.aim.psilent", cvar.ragebot_perfect_silent_aim);
		SetBoolValue("|ragebot.remove.recoil", cvar.ragebot_remove_recoil);
		SetIntValue("|ragebot.remove.spread", cvar.ragebot_remove_spread);
		SetBoolValue("|ragebot.anti-aim.resolver", cvar.ragebot_anti_aim_resolver);
		SetFloatValue("|ragebot.backtrack", cvar.ragebot_backtrack);

		SetSection("fakelag");

		SetBoolValue("|fakelag.enabled", cvar.fakelag_enabled);
		SetIntValue("|fakelag.type", cvar.fakelag_type);
		SetIntValue("|fakelag.triggers", cvar.fakelag_triggers);
		SetFloatValue("|fakelag.variance", cvar.fakelag_variance);
		SetFloatValue("|fakelag.choke.limit", cvar.fakelag_choke_limit);
		SetBoolValue("|fakelag.while_shooting", cvar.fakelag_while_shooting);

		SetSection("anti-aim");

		SetBoolValue("|anti-aim.enabled", cvar.antiaim_enabled);
		SetIntValue("|anti-aim.pitch", cvar.antiaim_pitch);
		SetIntValue("|anti-aim.yaw", cvar.antiaim_yaw);
		SetFloatValue("|anti-aim.yaw.static", cvar.antiaim_yaw_static);
		SetIntValue("|anti-aim.yaw.spin", cvar.antiaim_yaw_spin);
		SetIntValue("|anti-aim.yaw.while_running", cvar.antiaim_yaw_while_running);
		SetIntValue("|anti-aim.fake.yaw", cvar.antiaim_fake_yaw);
		SetFloatValue("|anti-aim.fake.yaw.static", cvar.antiaim_fake_yaw_static);
		SetBoolValue("|anti-aim.edge", cvar.antiaim_edge);
		SetFloatValue("|anti-aim.edge.distance", cvar.antiaim_edge_distance);
		SetFloatValue("|anti-aim.edge.offset", cvar.antiaim_edge_offset);
		SetIntValue("|anti-aim.edge.triggers", cvar.antiaim_edge_triggers);
		SetBoolValue("|anti-aim.on_knife", cvar.antiaim_on_knife);
		SetFloatValue("|anti-aim.choke.limit", cvar.antiaim_choke_limit);
		SetFloatValue("|anti-aim.roll", cvar.antiaim_roll);

		_Ini[IniFile_RageBot].SaveFile(c_sFileName.c_str());
		_Ini[IniFile_RageBot].Reset();
		Clear();
	}

	if (a_iSettingsPart & (SettingsPart_LegitBot | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_LegitBot];

		_Ini[IniFile_LegitBot].LoadFile(c_sFileName.c_str());

		SetFile(&_Ini[IniFile_LegitBot]);
		SetSection("legitbot");

		SetBoolValue("|legitbot.active", cvar.legitbot_active);
		SetBoolValue("|legitbot.friendlyfire", cvar.legitbot_friendly_fire);
		SetBoolValue("|legitbot.trigger.only_scoped", cvar.legitbot_trigger_only_scoped);
		SetFloatValue("|legitbot.backtrack", cvar.legitbot_backtrack);
		SetFloatValue("|legitbot.trigger.hitbox.scale", cvar.legitbot_trigger_hitbox_scale);
		SetIntValue("|legitbot.trigger.key", cvar.legitbot_trigger_key);

		for (size_t i = 1; i < cvar.legitbot.size(); ++i)
		{
			const std::string c_sSection = _GetSection(i);

			if (c_sSection.find("unknown") != std::string::npos)
				continue;

			const std::string c_sWeapon = "|" + c_sSection;

			SetSection(c_sSection.c_str());

			SetBoolValue(c_sWeapon + ".aim", cvar.legitbot.at(i).aim);
			SetBoolValue(c_sWeapon + ".aim.head", cvar.legitbot.at(i).aim_head);
			SetBoolValue(c_sWeapon + ".aim.chest", cvar.legitbot.at(i).aim_chest);
			SetBoolValue(c_sWeapon + ".aim.stomach", cvar.legitbot.at(i).aim_stomach);
			SetBoolValue(c_sWeapon + ".aim.all", cvar.legitbot.at(i).aim_all);
			SetIntValue(c_sWeapon + ".aim.accuracy", cvar.legitbot.at(i).aim_accuracy);
			SetIntValue(c_sWeapon + ".aim.reaction.time", cvar.legitbot.at(i).aim_reaction_time);
			SetFloatValue(c_sWeapon + ".aim.fov", cvar.legitbot.at(i).aim_fov);
			SetFloatValue(c_sWeapon + ".aim.psilent.angle", cvar.legitbot.at(i).aim_psilent_angle);
			SetIntValue(c_sWeapon + ".aim.psilent.type", cvar.legitbot.at(i).aim_psilent_type);
			SetIntValue(c_sWeapon + ".aim.psilent.triggers", cvar.legitbot.at(i).aim_psilent_triggers);
			SetFloatValue(c_sWeapon + ".aim.smooth", cvar.legitbot.at(i).aim_smooth);
			SetFloatValue(c_sWeapon + ".aim.smooth.in_attack", cvar.legitbot.at(i).aim_smooth_in_attack);
			SetFloatValue(c_sWeapon + ".aim.smooth.scale_fov", cvar.legitbot.at(i).aim_smooth_scale_fov);
			SetIntValue(c_sWeapon + ".aim.recoil_compensation.pitch", cvar.legitbot.at(i).aim_recoil_compensation_pitch);
			SetIntValue(c_sWeapon + ".aim.recoil_compensation.yaw", cvar.legitbot.at(i).aim_recoil_compensation_yaw);
			SetFloatValue(c_sWeapon + ".aim.recoil_compensation.fov", cvar.legitbot.at(i).aim_recoil_compensation_fov);
			SetBoolValue(c_sWeapon + ".trigger", cvar.legitbot.at(i).trigger);
			SetBoolValue(c_sWeapon + ".trigger.head", cvar.legitbot.at(i).trigger_head);
			SetBoolValue(c_sWeapon + ".trigger.chest", cvar.legitbot.at(i).trigger_chest);
			SetBoolValue(c_sWeapon + ".trigger.stomach", cvar.legitbot.at(i).trigger_stomach);
			SetBoolValue(c_sWeapon + ".trigger.all", cvar.legitbot.at(i).trigger_all);
			SetBoolValue(c_sWeapon + ".trigger.penetration", cvar.legitbot.at(i).trigger_penetration);
			SetIntValue(c_sWeapon + ".trigger.accuracy", cvar.legitbot.at(i).trigger_accuracy);
		}

		_Ini[IniFile_LegitBot].SaveFile(c_sFileName.c_str());
		_Ini[IniFile_LegitBot].Reset();
		Clear();
	}

	if (a_iSettingsPart & (SettingsPart_Visuals | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_Visuals];

		_Ini[IniFile_Visuals].LoadFile(c_sFileName.c_str());
		SetFile(&_Ini[IniFile_Visuals]);

		SetSection(nullptr);

		SetBoolValue("|active", cvar.visuals);

		SetSection("esp");

		SetBoolValue("|esp.enabled", cvar.esp_enabled);
		SetBoolValue("|esp.player", cvar.esp_player);
		SetBoolValue("|esp.player.teammates", cvar.esp_player_teammates);
		SetIntValue("|esp.player.box", cvar.esp_player_box);
		SetBoolValue("|esp.player.box.outline", cvar.esp_player_box_outline);
		SetStringColor("|esp.player.box.color.t", cvar.esp_player_box_color_t);
		SetStringColor("|esp.player.box.color.ct", cvar.esp_player_box_color_ct);
		SetIntValue("|esp.player.name", cvar.esp_player_name);
		SetIntValue("|esp.player.weapon", cvar.esp_player_weapon);
		SetIntValue("|esp.player.distance", cvar.esp_player_distance);
		SetIntValue("|esp.player.distance.measure", cvar.esp_player_distance_measure);
		SetStringColor("|esp.player.background.color", cvar.esp_player_background_color);
		SetIntValue("|esp.player.health", cvar.esp_player_health);
		SetBoolValue("|esp.player.health.value", cvar.esp_player_health_value);
		SetIntValue("|esp.player.armor", cvar.esp_player_armor);
		SetBoolValue("|esp.player.skeleton", cvar.esp_player_skeleton);
		SetStringColor("|esp.player.skeleton.color", cvar.esp_player_skeleton_color);
		SetBoolValue("|esp.player.off-screen", cvar.esp_player_off_screen);
		SetFloatValue("|esp.player.off-screen.radius", cvar.esp_player_off_screen_radius);
		SetIntValue("|esp.player.history.time", cvar.esp_player_history_time);
		SetBoolValue("|esp.player.fadeout.dormant", cvar.esp_player_fadeout_dormant);
		SetBoolValue("|esp.debug.info", cvar.esp_debug_info);
		SetBoolValue("|esp.world.dropped_weapons", cvar.esp_world_dropped_weapons);
		SetBoolValue("|esp.world.thrown_grenades", cvar.esp_world_thrown_grenades);
		SetBoolValue("|esp.bomb", cvar.esp_bomb);
		SetStringColor("|esp.bomb.color", cvar.esp_bomb_color);
		SetBoolValue("|esp.sound", cvar.esp_sound);
		SetIntValue("|esp.sound.fadeout_time", cvar.esp_sound_fadeout_time);
		SetFloatValue("|esp.sound.circle_size", cvar.esp_sound_circle_size);
		SetStringColor("|esp.sound.color.t", cvar.esp_sound_color_t);
		SetStringColor("|esp.sound.color.ct", cvar.esp_sound_color_ct);
		SetFloatValue("|esp.font.size", cvar.esp_font_size);
		SetStringColor("|esp.font.color", cvar.esp_font_color);
		SetIntValue("|esp.font.outline", cvar.esp_font_outline);

		SetSection("colored models and glow");

		SetBoolValue("|colored_models.enable", cvar.colored_models_enabled);
		SetIntValue("|colored_models.player", cvar.colored_models_player);
		SetBoolValue("|colored_models.player.invisible", cvar.colored_models_player_invisible);
		SetBoolValue("|colored_models.player.teammates", cvar.colored_models_player_teammates);
		SetStringColor("|colored_models.player.color.t", cvar.colored_models_color_t);
		SetStringColor("|colored_models.player.color.t.invisible", cvar.colored_models_color_t_invisible);
		SetStringColor("|colored_models.player.color.ct", cvar.colored_models_color_ct);
		SetStringColor("|colored_models.player.color.ct.invisible", cvar.colored_models_color_ct_invisible);
		SetIntValue("|colored_models.hands", cvar.colored_models_hands);
		SetStringColor("|colored_models.hands.color", cvar.colored_models_hands_color);
		SetIntValue("|colored_models.dropped_weapons", cvar.colored_models_dropped_weapons);
		SetStringColor("|colored_models.dropped_weapons.color", cvar.colored_models_dropped_weapons_color);

		SetSection("other");

		SetBoolValue("|crosshair.snipers", cvar.crosshair_snipers);
		SetStringColor("|crosshair.snipers.color", cvar.crosshair_snipers_color);
		SetBoolValue("|legitbot.fov.circle", cvar.legitbot_fov_circle);
		SetStringColor("|legitbot.fov.circle.color", cvar.legitbot_fov_circle_color);
		SetBoolValue("|grenade.trail", cvar.grenade_trail);
		SetStringColor("|grenade.trail.color", cvar.grenade_trail_color);
		SetBoolValue("|spread.fov.circle", cvar.spread_fov_circle);
		SetStringColor("|spread.fov.circle.color", cvar.spread_fov_circle_color);
		SetBoolValue("|punch.recoil", cvar.punch_recoil);
		SetStringColor("|punch.recoil.color", cvar.punch_recoil_color);
		SetBoolValue("|remove.visual.recoil", cvar.remove_visual_recoil);

		SetSection("effects");

		SetIntValue("|thirdperson", cvar.thirdperson);
		SetIntValue("|brightness", cvar.brightness);
		SetStringColor("|brightness.color", cvar.brightness_color);
		SetBoolValue("|disable.render.teammates", cvar.disable_render_teammates);

		_Ini[IniFile_Visuals].SaveFile(c_sFileName.c_str());
		_Ini[IniFile_Visuals].Reset();
		Clear();
	}

	if (a_iSettingsPart & (SettingsPart_Misc | SettingsPart_All))
	{
		const std::string c_sFileName = c_sPath + _pcszIniFile[IniFile_Misc];

		_Ini[IniFile_Misc].LoadFile(c_sFileName.c_str());

		SetFile(&_Ini[IniFile_Misc]);
		SetSection("miscellaneous");

		SetBoolValue("|hud.clear", cvar.hud_clear);
		SetBoolValue("|automatic.reload", cvar.automatic_reload);
		SetBoolValue("|automatic.pistol", cvar.automatic_pistol);
		SetBoolValue("|knifebot", cvar.knifebot);
		SetBoolValue("|knifebot.friendlyfire", cvar.knifebot_friendly_fire);
		SetIntValue("|knifebot.type", cvar.knifebot_type);
		SetFloatValue("|knifebot.distance.slash", cvar.knifebot_distance_slash);
		SetFloatValue("|knifebot.distance.stab", cvar.knifebot_distance_stab);
		SetFloatValue("|knifebot.backtrack", cvar.knifebot_backtrack);

		_Ini[IniFile_Misc].SaveFile(c_sFileName.c_str());
		_Ini[IniFile_Misc].Reset();
		Clear();
	}
}

void CSettings::Restore(const char* a_pcszSettingsName, bool a_bSetAsDefault)
{
	const std::string c_sPath = g_Globals.m_sSettingsPath + a_pcszSettingsName;
	const std::string c_sPathRageBot = c_sPath + _pcszIniFile[IniFile_RageBot];
	const std::string c_sPathLegitBot = c_sPath + _pcszIniFile[IniFile_LegitBot];
	const std::string c_sPathVisuals = c_sPath + _pcszIniFile[IniFile_Visuals];
	const std::string c_sPathKreedz = c_sPath + _pcszIniFile[IniFile_Kreedz];
	const std::string c_sPathMisc = c_sPath + _pcszIniFile[IniFile_Misc];
	const std::string c_sPathGUI = c_sPath + _pcszIniFile[IniFile_GUI];

	remove(c_sPathRageBot.c_str());
	remove(c_sPathLegitBot.c_str());
	remove(c_sPathVisuals.c_str());
	remove(c_sPathKreedz.c_str());
	remove(c_sPathMisc.c_str());
	remove(c_sPathGUI.c_str());

	a_bSetAsDefault
		? Load(a_pcszSettingsName)
		: Save(a_pcszSettingsName);
}

bool CSettings::Delete(const char* a_szSettingsName)
{
	if (!a_szSettingsName)
		return false;

	const DWORD dwAttributes = GetFileAttributesA(a_szSettingsName);

	if (dwAttributes == INVALID_FILE_ATTRIBUTES)
		return true;

	if (~dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return false;

	SetLastError(0);

	if (RemoveDirectoryA(a_szSettingsName))
		return true;

	if (GetLastError() != ERROR_DIR_NOT_EMPTY)
		return false;

	if (strlen(a_szSettingsName) + strlen("\\*.*") + 1 > MAX_PATH)
		return false;

	char szPath[MAX_PATH] = { 0 };
	sprintf(&szPath[0], "%s\\*.*", a_szSettingsName);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFileA(&szPath[0], &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	strcpy(&szPath[0], a_szSettingsName);

	do {
		if (!strcmp(&FindFileData.cFileName[0], "."))
			continue;

		if (!strcmp(&FindFileData.cFileName[0], ".."))
			continue;

		if (strlen(&szPath[0]) + strlen("") + strlen(&FindFileData.cFileName[0]) + 1 > MAX_PATH)
			continue;

		char szCurrentFile[MAX_PATH] = { 0 };
		sprintf(&szCurrentFile[0], "%s\\%s", &szPath[0], &FindFileData.cFileName[0]);

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
				FindFileData.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributesA(&szCurrentFile[0], FindFileData.dwFileAttributes);
			}

			Delete(&szCurrentFile[0]);
		}
		else
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) || (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
				SetFileAttributesA(&szCurrentFile[0], FILE_ATTRIBUTE_NORMAL);

			DeleteFileA(&szCurrentFile[0]);
		}

	} while (FindNextFileA(hFind, &FindFileData));

	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	if (GetLastError() != ERROR_NO_MORE_FILES)
		return false;

	return RemoveDirectoryA(a_szSettingsName);
}

std::string CSettings::_GetSection(unsigned int a_nWeaponIndex)
{
	switch (a_nWeaponIndex)
	{
	case WEAPON_P228:		return "p228";
	case WEAPON_SCOUT:		return "scout";
	case WEAPON_XM1014:		return "xm1014";
	case WEAPON_MAC10:		return "mac10";
	case WEAPON_AUG:		return "aug";
	case WEAPON_ELITE:		return "elite";
	case WEAPON_FIVESEVEN:	return "five-seven";
	case WEAPON_UMP45:		return "ump45";
	case WEAPON_SG550:		return "sg550";
	case WEAPON_GALIL:		return "galil";
	case WEAPON_FAMAS:		return "famas";
	case WEAPON_USP:		return "usp";
	case WEAPON_GLOCK18:	return "glock18";
	case WEAPON_AWP:		return "awp";
	case WEAPON_MP5N:		return "mp5";
	case WEAPON_M249:		return "m249";
	case WEAPON_M3:			return "m3";
	case WEAPON_M4A1:		return "m4a1";
	case WEAPON_TMP:		return "tmp";
	case WEAPON_G3SG1:		return "g3sg1";
	case WEAPON_DEAGLE:		return "deagle";
	case WEAPON_SG552:		return "sg552";
	case WEAPON_AK47:		return "ak47";
	case WEAPON_P90:		return "p90";
	}

	return "unknown";
}


std::string CIni::_ColorToString(float(&a_flColor)[4])
{
	char szBuffer[16] = { 0 };

	_ColorFloatToByte(a_flColor);
	sprintf(&szBuffer[0], "%.0f,%.0f,%.0f,%.0f", a_flColor[0], a_flColor[1], a_flColor[2], a_flColor[3]);
	_ColorByteToFloat(a_flColor);

	return &szBuffer[0];
}

std::string CIni::_ColorToString(float(&a_flColor)[3])
{
	char szBuffer[16] = { 0 };

	_ColorFloatToByte(a_flColor);
	sprintf(&szBuffer[0], "%.0f,%.0f,%.0f", a_flColor[0], a_flColor[1], a_flColor[2]);
	_ColorByteToFloat(a_flColor);

	return &szBuffer[0];
}

void CIni::_ColorByteToFloat(float(&a_flColor)[4]) noexcept
{
	a_flColor[0] /= 255.f;
	a_flColor[1] /= 255.f;
	a_flColor[2] /= 255.f;
	a_flColor[3] /= 255.f;
}

void CIni::_ColorByteToFloat(float(&a_flColor)[3]) noexcept
{
	a_flColor[0] /= 255.f;
	a_flColor[1] /= 255.f;
	a_flColor[2] /= 255.f;
}

void CIni::_ColorFloatToByte(float(&a_flColor)[4]) noexcept
{
	a_flColor[0] *= 255.f;
	a_flColor[1] *= 255.f;
	a_flColor[2] *= 255.f;
	a_flColor[3] *= 255.f;
}

void CIni::_ColorFloatToByte(float(&a_flColor)[3]) noexcept
{
	a_flColor[0] *= 255.f;
	a_flColor[1] *= 255.f;
	a_flColor[2] *= 255.f;
}

void CIni::_StringToArray(const char* a_pcszString, float(&a_flVariable)[3]) noexcept
{
	char szBuffer[32] = { 0 };
	strcpy(&szBuffer[0], a_pcszString);

	char* pszBuffer = strtok(&szBuffer[0], ",");

	a_flVariable[0] = static_cast<float>(atof(pszBuffer));
	pszBuffer = strtok(nullptr, ",");

	a_flVariable[1] = static_cast<float>(atof(pszBuffer));
	pszBuffer = strtok(nullptr, ",");

	a_flVariable[2] = static_cast<float>(atof(pszBuffer));
	pszBuffer = strtok(nullptr, ",");
}

void CIni::_StringToArray(const char* a_pcszString, float(&a_flVariable)[4]) noexcept
{
	char szBuffer[32] = { 0 };
	strcpy(&szBuffer[0], a_pcszString);

	char* pszBuffer = strtok(&szBuffer[0], ",");

	a_flVariable[0] = static_cast<float>(atof(pszBuffer));
	pszBuffer = strtok(nullptr, ",");

	a_flVariable[1] = static_cast<float>(atof(pszBuffer));
	pszBuffer = strtok(nullptr, ",");

	a_flVariable[2] = static_cast<float>(atof(pszBuffer));
	pszBuffer = strtok(nullptr, ",");

	a_flVariable[3] = static_cast<float>(atof(pszBuffer));
	pszBuffer = strtok(nullptr, ",");
}

void CIni::_StringToArray(const char* a_pcszString, int(&a_iVariable)[2]) noexcept
{
	char szBuffer[32] = { 0 };
	strcpy(&szBuffer[0], a_pcszString);

	char* pszBuffer = strtok(&szBuffer[0], ",");

	a_iVariable[0] = atoi(pszBuffer);
	pszBuffer = strtok(nullptr, ",");

	a_iVariable[1] = atoi(pszBuffer);
	pszBuffer = strtok(nullptr, ",");
}

void CIni::SetFile(CSimpleIniA* a_Ini) noexcept
{
	if (a_Ini != nullptr)
		this->_Ini = a_Ini;
}

void CIni::SetSection(const char* a_pcszSection) noexcept
{
	if (a_pcszSection != nullptr)
		this->_pcszSection = a_pcszSection;
}

bool CIni::GetBoolValue(std::string a_sKey, bool a_bDefault)
{
	return this->_Ini->GetBoolValue(_pcszSection, a_sKey.c_str(), a_bDefault);
}

int CIni::GetIntValue(std::string a_sKey, int a_iDefault)
{
	return gsl::narrow_cast<int>(this->_Ini->GetLongValue(_pcszSection, a_sKey.c_str(), a_iDefault));
}

float CIni::GetFloatValue(std::string a_sKey, float a_flDefault)
{
	return static_cast<float>(this->_Ini->GetDoubleValue(_pcszSection, a_sKey.c_str(), a_flDefault));
}

void CIni::GetStringColor(std::string a_sKey, float(&a_flColor)[4], const char* a_pcszDefault)
{
	_StringToArray(this->_Ini->GetValue(_pcszSection, a_sKey.c_str(), a_pcszDefault), a_flColor);
	_ColorByteToFloat(a_flColor);
}

void CIni::GetStringColor(std::string a_sKey, float(&a_flColor)[3], const char* a_pcszDefault)
{
	_StringToArray(this->_Ini->GetValue(_pcszSection, a_sKey.c_str(), a_pcszDefault), a_flColor);
	_ColorByteToFloat(a_flColor);
}

void CIni::SetBoolValue(std::string a_sKey, bool a_bValue)
{
	this->_Ini->SetBoolValue(_pcszSection, a_sKey.c_str(), a_bValue);
}

void CIni::SetIntValue(std::string a_sKey, int a_iValue)
{
	this->_Ini->SetLongValue(_pcszSection, a_sKey.c_str(), gsl::narrow_cast<long>(a_iValue));
}

void CIni::SetFloatValue(std::string a_sKey, float a_flValue)
{
	this->_Ini->SetDoubleValue(_pcszSection, a_sKey.c_str(), static_cast<double>(a_flValue));
}

void CIni::SetStringColor(std::string a_sKey, float(&a_flColor)[4])
{
	this->_Ini->SetValue(_pcszSection, a_sKey.c_str(), _ColorToString(a_flColor).c_str());
}

void CIni::SetStringColor(std::string a_sKey, float(&a_flColor)[3])
{
	this->_Ini->SetValue(_pcszSection, a_sKey.c_str(), _ColorToString(a_flColor).c_str());
}

void CIni::Clear() noexcept
{
	this->_Ini = nullptr;
	this->_pcszSection = nullptr;
}