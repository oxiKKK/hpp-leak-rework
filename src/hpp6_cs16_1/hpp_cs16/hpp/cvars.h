class CStates
{
public:
	CStates();

	bool	visuals_panic;

	bool	trigger_active;
	bool	bunnyhop_active;
	bool	gstrafe_active;
	bool	gstrafe_standup_active;
	bool	jumpbug_active;
	bool	edgebug_active;
	bool	strafe_active;
	bool	knifebot_active;
	bool	fakewalk_active;
	bool	fakelatency_active;
	bool	thirdperson_active;
};

class CCvars 
{
public:
	CCvars();

	struct settings_s
	{
		bool	autosave_when_quit;
		bool	autosave_when_unhook;
		bool	firstload_custom;
		char	firstload_custom_name[40];
		int		firstload_iter;
	} settings;
	
	// ragebot
	bool	ragebot_active;
	bool	ragebot_aim_enabled;
	bool	ragebot_aim_auto_fire;
	bool	ragebot_aim_friendly_fire;
	float	ragebot_aim_fov;
	int		ragebot_aim_hitbox;
	int		ragebot_aim_multipoint;
	float	ragebot_aim_multipoint_scale;
	bool	ragebot_aim_auto_penetration;
	bool	ragebot_aim_auto_scope;
	bool	ragebot_aim_silent;
	bool	ragebot_aim_perfect_silent;
	bool	ragebot_remove_recoil;
	int		ragebot_remove_spread;
	bool	ragebot_anti_aim_resolver;
	bool	ragebot_fake_walk_enabled;
	int		ragebot_fake_walk_key_press;
	int		ragebot_fake_walk_choke_limit;
	bool	ragebot_fake_walk_decrease_fps;
	bool	ragebot_tapping_mode;
	bool	ragebot_delay_shot;
	int		ragebot_resolver;
	
	// anti-aimbot
	bool	antiaim_enabled;
	int		antiaim_pitch;
	int		antiaim_yaw;
	int		antiaim_yaw_while_running;
	int		antiaim_fake_yaw;
	float	antiaim_yaw_static;
	float	antiaim_fake_yaw_static;
	int		antiaim_yaw_spin;
	bool	antiaim_on_knife;
	float	antiaim_choke_limit;
	float	antiaim_roll;
	bool	antiaim_teammates;

	// fake lag
	bool	fakelag_enabled;
	int		fakelag_type;
	int		fakelag_triggers;	
	float	fakelag_variance;
	int		fakelag_choke_limit;
	bool	fakelag_while_shooting;
	bool	fakelag_on_enemy_visible;

	//visuals
	bool	visuals;
	int		visuals_panic_key;

	bool	esp_player_enabled;
	//bool	esp_player;
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
	bool	esp_player_out_of_pov_arrow;
	float	esp_player_out_of_pov_arrow_radius;
	float	esp_player_history_time;
	bool	esp_player_fadeout_dormant;
	bool	debug_visuals;
	bool	world_dropped_weapons;
	bool	world_thrown_grenades;
	bool	bomb;
	float	bomb_color[4];
	bool	esp_sound;
	float	esp_sound_fadeout_time;
	float	esp_sound_circle_size;
	float	esp_sound_color_t[4];
	float	esp_sound_color_ct[4];
	float	esp_font_size;
	int		esp_font_outline;
	float	esp_font_color[4];

	bool	grenade_trail;
	float	grenade_trail_color[4];

	bool	lightmap;
	float	lightmap_color[4];

	int		thirdperson;
	int		thirdperson_key_toggle;

	bool	crosshair_snipers;
	float	crosshair_snipers_color[4];

	bool	disable_render_teammates;

	bool	colored_models_enabled;
	int		colored_models_player;
	bool	colored_models_player_behind_wall;
	bool	colored_models_player_teammates;
	float	colored_models_color_t[4];
	float	colored_models_color_t_behind_wall[4];
	float	colored_models_color_ct[4];
	float	colored_models_color_ct_behind_wall[4];
	int		colored_models_hands;
	float	colored_models_hands_color[4];
	int		colored_models_dropped_weapons;
	float	colored_models_dropped_weapons_color[4];
	bool	colored_models_backtrack;
	float	colored_models_backtrack_color[4];

	bool	glow_enabled;
	bool	glow_behind_wall;
	bool	glow_teammates;
	float	glow_color_t[3];
	float	glow_color_ct[3];
	int		glow_amout;

	bool	aimbot_fov_circle;
	float	aimbot_fov_circle_color[4];

	bool	spread_circle;
	float	spread_circle_color[4];

	bool	punch_recoil;
	float	punch_recoil_color[4];

	bool	remove_visual_recoil;
	float	screenfade_limit;

	bool	bullet_impacts;
	float	bullet_impacts_color[4];

	bool	screen_log;

	bool	shared_esp;

	struct kreedz_s
	{
		bool	active;

		int		bunnyhop_key;
		bool	bunnyhop_scroll_emulation;
		bool	bunnyhop_scroll_helper;
		int		bunnyhop_correct_type;
		int		bunnyhop_correct_value;
		int		bunnyhop_frames_on_ground[3];
		int		bunnyhop_ground_equal;
		float	bunnyhop_standup_distance;
		bool	bunnyhop_break_jump_animation;	
		bool	bunnyhop_notouch_ground_illusion;
	
		int		gstrafe_key;
		int		gstrafe_standup_key;
		bool	gstrafe_scroll_emulation;
		float 	gstrafe_slowdown_scale;
		float	gstrafe_slowdown_angle;
		int		gstrafe_correct_type;
		int		gstrafe_correct_value;
		int		gstrafe_frames_on_ground[3];
		int		gstrafe_ground_equal;
		bool	gstrafe_jump_animation;

		int		jumpbug_key;
		int		jumpbug_auto_damage_min;
		bool	jumpbug_auto_damage_fatal;
		bool	jumpbug_slowdown_velocity;

		int		edgebug_key;
		float	edgebug_efficiency;
		float	edgebug_auto_efficiency;
		int		edgebug_height_frames;

		int		strafe_invisible_key;
		int		strafe_invisible_direction;
		int		strafe_invisible_skipframes;
		float	strafe_invisible_fps_helper;
	} kreedz;

	//misc
	bool	hide_from_obs;
	bool	automatic_reload;
	bool	automatic_pistol;
	bool	hud_clear;
	bool	motd_block;
	bool	disable_screenshake;
	bool	debug_console;
	bool	demochecker_bypass;
	bool	fakelatency_enabled;
	float	fakelatency_value;
	int		fakelatency_key_press;
	bool	name_stealer_enabled;
	float	name_stealer_interval;
	bool	replace_zombie_models;
	int		custom_render_fov;
	bool	maximize_window_after_respawn;
	bool	maximize_window_in_new_round;
	bool	player_list;

	struct misc_s
	{
		bool	steamspoofid_enabled;
		char	steamspoofid_custom_id[10];
		bool	steamspoofid_random;
	} misc;

	//knifebot
	bool	knifebot_enabled;
	bool	knifebot_friendly_fire;
	int		knifebot_type;
	float	knifebot_distance_stab;
	float	knifebot_distance_slash;
	float	knifebot_fov;
	float	knifebot_hitbox_scale;
	bool	knifebot_aim_silent;
	bool	knifebot_aim_perfect_silent;
	int		knifebot_aim_hitbox;
	int		knifebot_key_toggle;
	bool	knifebot_dont_shoot_spectators;
	bool	knifebot_position_adjustment;
	bool	knifebot_dont_shoot_in_back;

	//legitbot
	bool	legitbot_active;
	bool	legitbot_friendlyfire;
	bool	legitbot_trigger_only_scoped;
	float	legitbot_trigger_hitbox_scale;
	int		legitbot_trigger_key_type;
	int		legitbot_trigger_key;
	bool	legitbot_automatic_scope;
	bool	legitbot_dependence_fps;
	float	legitbot_flashed_limit;
	bool	legitbot_position_adjustment;
	bool	legitbot_smoke_check;

	struct legitbot
	{
		bool	trigger;
		bool	trigger_head;
		bool	trigger_chest;
		bool	trigger_stomach;
		bool	trigger_arms;
		bool	trigger_legs;
		bool	trigger_penetration;
		int		trigger_accuracy;

		bool	aim;
		bool	aim_head;
		bool	aim_chest;
		bool	aim_stomach;
		bool	aim_arms;
		bool	aim_legs;
		bool	aim_penetration;
		int		aim_accuracy;
		int		aim_reaction_time;
		float	aim_delay_before_firing;
		float	aim_delay_before_aiming;
		float	aim_maximum_lock_on_time;
		float	aim_fov;
		float	aim_psilent_angle;
		int		aim_psilent_type;
		int		aim_psilent_triggers;
		bool	aim_psilent_tapping_mode;
		float	aim_smooth_auto;
		float	aim_smooth_in_attack;
		float	aim_smooth_scale_fov;
		int		aim_recoil_start;
		int		aim_recoil_pitch;
		int		aim_recoil_yaw;
		float	aim_recoil_fov;
		float	aim_recoil_smooth;
	} legitbot[MAX_WEAPONS];

	float gui[ImGuiCol_COUNT][4];
};

extern CCvars cvar;
extern CStates state;

bool HookCommand(const char *pszName, xcommand_t pfn);
bool UnHookCommand(const char *pszName);
bool HookCommands(void);
void UnHookCommands(void);
void AntiScreen();

extern std::map<std::string, xcommand_t> g_ClientCommandsMap;