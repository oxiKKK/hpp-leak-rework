class CCvars;
class CCvars 
{
public:
	//ragebot
	bool	ragebot_active;
	bool	ragebot_aim_enabled;
	bool	ragebot_friendly_fire;
	bool	ragebot_automatic_penetration;
	bool	ragebot_silent_aim;
	bool	ragebot_perfect_silent_aim;
	bool	ragebot_remove_recoil;
	bool	ragebot_automatic_scope;
	bool	ragebot_anti_aim_resolver;
	bool	ragebot_multipoint;
	float	ragebot_multipoint_scale;
	int		ragebot_remove_spread;
	int		ragebot_multipoint_type;
	int		ragebot_hitbox;
	float	ragebot_backtrack;	

	//anti-aimbot
	bool	antiaim_enabled;
	int		antiaim_pitch;
	int		antiaim_yaw;
	int		antiaim_yaw_while_running;
	int		antiaim_fake_yaw;
	bool	antiaim_edge;
	float	antiaim_edge_distance;
	int		antiaim_edge_triggers;
	float	antiaim_edge_offset;
	float	antiaim_yaw_static;
	float	antiaim_fake_yaw_static;
	int		antiaim_yaw_spin;
	bool	antiaim_on_knife;
	float	antiaim_choke_limit;
	float	antiaim_roll;

	//fake lag
	bool	fakelag_enabled;
	int		fakelag_triggers;
	int		fakelag_type;
	float	fakelag_variance;
	float	fakelag_choke_limit;
	bool	fakelag_while_shooting;

	//visuals
	bool	visuals;

	bool	esp_enabled;
	bool	esp_player;
	bool	esp_player_teammates;
	int		esp_player_box;
	bool	esp_player_box_outline;
	float	esp_player_box_color_t[4];
	float	esp_player_box_color_ct[4];
	int		esp_player_name;
	int		esp_player_weapon;
	int		esp_player_distance;
	int		esp_player_distance_measure;
	float	esp_player_background_color[4];
	int		esp_player_health;
	bool	esp_player_health_value;
	int		esp_player_armor;
	bool	esp_player_skeleton;
	float	esp_player_skeleton_color[4];
	bool	esp_player_off_screen;
	float	esp_player_off_screen_radius;
	int		esp_player_history_time;
	bool	esp_player_fadeout_dormant;
	bool	esp_debug_info;
	bool	esp_world_dropped_weapons;
	bool	esp_world_thrown_grenades;
	bool	esp_bomb;
	float	esp_bomb_color[4];
	bool	esp_sound;
	int		esp_sound_fadeout_time;
	float	esp_sound_circle_size;
	float	esp_sound_color_t[4];
	float	esp_sound_color_ct[4];
	float	esp_font_size;
	int		esp_font_outline;
	float	esp_font_color[4];

	bool	grenade_trail;
	float	grenade_trail_color[4];

	bool	brightness;
	float	brightness_color[4];

	int		thirdperson;

	bool	crosshair_snipers;
	float	crosshair_snipers_color[4];

	bool	disable_render_teammates;

	bool	colored_models_enabled;
	int		colored_models_player;
	bool	colored_models_player_invisible;
	bool	colored_models_player_teammates;
	float	colored_models_color_t[3];
	float	colored_models_color_t_invisible[3];
	char*	colored_models_color_ct_string;
	float	colored_models_color_ct[3];
	float	colored_models_color_ct_invisible[3];
	int		colored_models_hands;
	float	colored_models_hands_color[3];
	int		colored_models_dropped_weapons;
	float	colored_models_dropped_weapons_color[3];

	bool	legitbot_fov_circle;
	float	legitbot_fov_circle_color[4];

	bool	spread_fov_circle;
	float	spread_fov_circle_color[4];

	bool	punch_recoil;
	float	punch_recoil_color[4];

	bool	remove_visual_recoil;

	//kreedz
	int		kreedz;
	bool	ragekreedz_bunnyhop;

	//misc
	bool	automatic_reload;
	bool	automatic_pistol;
	bool	hud_clear;

	//knifebot
	bool	knifebot;
	bool	knifebot_friendly_fire;
	int		knifebot_type;
	float	knifebot_backtrack;
	float	knifebot_distance_stab;
	float	knifebot_distance_slash;
	float	knifebot_fov;

	//legitbot
	bool	legitbot_active;
	bool	legitbot_friendly_fire;
	bool	legitbot_trigger_only_scoped;
	float	legitbot_backtrack;
	float	legitbot_trigger_hitbox_scale;
	int		legitbot_trigger_key;

	struct legitbot
	{
		bool	trigger;
		bool	trigger_head;
		bool	trigger_chest;
		bool	trigger_stomach;
		bool	trigger_all;
		bool	trigger_penetration;
		int		trigger_accuracy;

		bool	aim;
		bool	aim_head;
		bool	aim_chest;
		bool	aim_stomach;
		bool	aim_all;
		int		aim_accuracy;
		int		aim_reaction_time;
		float	aim_fov;
		float	aim_psilent_angle;
		int		aim_psilent_type;
		int		aim_psilent_triggers;
		float	aim_smooth;
		float	aim_smooth_in_attack;
		float	aim_smooth_scale_fov;
		int		aim_recoil_compensation_pitch;
		int		aim_recoil_compensation_yaw;
		float	aim_recoil_compensation_fov;
	};
	
	std::array<legitbot, MAX_WEAPONS> legitbot;
};
extern CCvars cvar;

bool HookCommand(const char *pszName, xcommand_t pfn);
bool UnHookCommand(const char *pszName);
void HookCommands();
void UnHookCommands();
void CL_TakeSnapshot();
void CL_TakeScreenshot();
void AntiScreen();