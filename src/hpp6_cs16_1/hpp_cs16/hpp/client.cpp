#include "main.h"

static CHookData pres_dynamic_sound;
static CHookData r_draw_entities_on_list;
static CHookData cl_record_hud_command;
static CHookData initiate_game_connection;

cl_clientfunc_t g_Client, *g_pClient = nullptr;
cl_enginefunc_t g_Engine, *g_pEngine = nullptr;
engine_studio_api_t g_Studio, *g_pStudio = nullptr;
StudioModelRenderer_t g_StudioModelRenderer, *g_pStudioModelRenderer = nullptr;
IRunGameEngine* g_pIRunGameEngine = nullptr;
IGameConsole* g_pConsole = nullptr;
vgui::ISurface* g_pISurface = nullptr;
IGameUI* g_pIGameUI = nullptr;
playermove_t* pmove = nullptr;
StudioModelRenderer_d pCStudioModelRenderer = nullptr;
client_state_t *client_state = nullptr;
client_static_t *client_static = nullptr;
extra_player_info_t *g_PlayerExtraInfo = nullptr;
PreS_DynamicSound_t g_pPreS_DynamicSound = nullptr;
R_DrawEntitiesOnList_t g_pR_DrawEntitiesOnList = nullptr;
char **keybindings = nullptr;
particle_t **particles = nullptr;
ISteamClient *g_pISteamClient = nullptr;
ISteamScreenshots *g_pISteamScreenshots = nullptr;
ISteamUtils *g_pISteamUtils = nullptr;
ISteamFriends *g_pISteamFriends = nullptr;
ISteamUser *g_pISteamUser = nullptr;
CL_RecordHUDCommand_t g_pCL_RecordHUDCommand = nullptr;
InitiateGameConnection_t g_pInitiateGameConnection = nullptr;

SCREENINFO g_Screen;
net_status_s g_Status;

std::deque<my_sound_t> g_Sounds;
std::deque<sound_resolver_t> g_SoundResolver;
std::deque<CIncomingSequence> sequences;

CKreedz g_Kreedz;

static void FirstLoadSettings()
{
	g_Settings.RefreshGlobals();

	if (cvar.settings.firstload_custom)
	{
		Menu::RefreshSettings();

		for (auto& settings : Menu::sSettingsList)
		{
			if (settings.compare(cvar.settings.firstload_custom_name))
				continue;

			g_Settings.Load(settings.c_str());
			break;
		}
	}
	else
		g_Settings.Load("Default");
}

static void HUD_Frame(double time)
{
	__try
	{
		if (!g_Globals.m_bUnloadLibrary)
		{
			g_Engine.pfnGetScreenInfo(&g_Screen);
			g_Engine.pNetAPI->Status(&g_Status);

			g_Globals.m_flFrameTime = time;

			if (client_static->state == ca_active)
			{
				if (cvar.kreedz.active && !g_Miscellaneous.m_bFakeWalkActive)
				{
					g_Kreedz.HUD_Frame();
				}

				g_Miscellaneous.NameStealer();
			}

			g_Miscellaneous.Brightness();

			g_World.Reset();
		}

		g_Client.HUD_Frame(time);
	}
	__except (g_Utils.ExceptionFilter(__FUNCTION__, GetExceptionCode(), GetExceptionInformation()))
	{
	}

	g_Client.HUD_Frame(time);
}

static void Initialize(double time)
{
	g_Screen.iSize = sizeof(SCREENINFO);

	g_Engine.pfnGetScreenInfo(&g_Screen);
	g_Engine.pNetAPI->Status(&g_Status);

	g_Globals.m_flFrameTime = time;

	FirstLoadSettings();

	if (!g_pConsole->IsConsoleVisible())
		g_pConsole->Activate();

	g_pConsole->DPrintf("Hello %s!\n", g_pISteamFriends ? g_pISteamFriends->GetPersonaName() : g_Engine.pfnGetCvarString("name"));
	g_pConsole->DPrintf("Cheat injected :3\n");
	g_pConsole->DPrintf("Site: hpp.ovh\n");
	g_pConsole->DPrintf("Build: %s\n", __DATE__);

#ifndef DISABLE_SOCKETING
	websocket->send("addme");
#endif
	g_pClient->HUD_Frame = HUD_Frame;
	g_Client.HUD_Frame(time);
}

static void InterpolationTime(usercmd_t *cmd)
{
	static cvar_s *cl_updaterate = nullptr;
	static cvar_s *ex_interp = nullptr;

	if (cl_updaterate == nullptr)
		cl_updaterate = g_Engine.pfnGetCvarPointer("cl_updaterate");

	if (ex_interp == nullptr)
		ex_interp = g_Engine.pfnGetCvarPointer("ex_interp");

	auto update_interval = 0.1;

	if (cl_updaterate)
	{
		if (cl_updaterate->value < 10.f)
			g_Engine.Cvar_SetValue("cl_updaterate", 10.f);

		if ((int)cl_updaterate->value >= 10)
			update_interval = 1.0 / (double)cl_updaterate->value;
	}

	if (ex_interp)
	{
		if (ex_interp->value > 0.1f)
			g_Engine.Cvar_SetValue("ex_interp", 0.1f);
		else if (ex_interp->value < 0.001)
			g_Engine.Cvar_SetValue("ex_interp", 0.001f);

		if (update_interval > ex_interp->value)
			g_Engine.Cvar_SetValue("ex_interp", (float)update_interval);

		cmd->lerp_msec = ex_interp->value * 1000.0;
	}

	if (g_Miscellaneous.m_bPositionAdjustmentActive)
	{
		cmd->lerp_msec = g_Miscellaneous.m_sPositionAdjustmentLerp;
		g_Miscellaneous.m_sPositionAdjustmentLerpVisual = cmd->lerp_msec;
	}
	else
		g_Miscellaneous.m_sPositionAdjustmentLerpVisual = -1;

	cmd->lerp_msec = ImClamp(0, cmd->lerp_msec, 100);

	g_Globals.m_sLastLerpMsec = cmd->lerp_msec;

	g_Miscellaneous.m_bPositionAdjustmentActive = false;
	g_Miscellaneous.m_sPositionAdjustmentLerp = 0;
}

static void CL_CreateMove(float frametime, usercmd_s* cmd, int active)
{
	g_Client.CL_CreateMove(frametime, cmd, active);

	__try
	{
		g_Globals.m_flFrameTime = frametime;

		cmd::get_pointer(cmd);

		g_Globals.m_bSendCommand = true;
		g_Utils.AdjustSpeed(1.0);

		if (/*g_Globals.m_iGameBuild > 8245 || */g_Engine.PhysInfo_ValueForKey("pi") == "xvi")
		{
			g_Globals.m_bUnloadLibrary = true;
			return;
		}

		if (!g_Globals.m_bUnloadLibrary && client_static->state == ca_active)
		{
			g_Utils.UpdateSequences();
			g_World.Update();
			g_Weapon.Update();

			if (!client_static->demoplayback)
			{
				if (cvar.maximize_window_after_respawn)
					g_Miscellaneous.MaximizeWindowAfterRespawn();

				state.visuals_panic || g_Globals.m_bSnapshot || g_Globals.m_bScreenshot
					? g_Miscellaneous.CustomRenderFov(DEFAULT_FOV)
					: g_Miscellaneous.CustomRenderFov(cvar.custom_render_fov);

				g_Miscellaneous.FakeLatency();

				if (!g_Local.m_bIsDead)
				{
					if (!g_DrawGUI.IsDrawing())
					{
						if (cvar.kreedz.active && !g_Miscellaneous.m_bFakeWalkActive)
						{
							g_Kreedz.CL_CreateMove();
						}

						g_Aimbot.Run(cmd);

						g_Miscellaneous.AutoReload(cmd);
						g_Miscellaneous.AutoPistol(cmd);

						g_NoSpread.Run(cmd);

						g_Knifebot.Run(cmd);

						g_Miscellaneous.FakeLag(cmd);
						g_Miscellaneous.AntiAim(cmd);
						g_Miscellaneous.FakeWalk(cmd);
					}
				}

				g_Miscellaneous.HUDCommands(cmd);
			}
		}

		g_Utils.SendCommand(g_Globals.m_bSendCommand);

		InterpolationTime(cmd);

		cmd::release_pointer();
	}
	__except (g_Utils.ExceptionFilter(__FUNCTION__, GetExceptionCode(), GetExceptionInformation()))
	{
	}
}

static void HUD_PostRunCmd(local_state_s* from, local_state_s* to, usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_Client.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);

	g_Weapon.ItemPreFrame(to, cmd, runfuncs, time, random_seed);
}

static int StudioCheckBBox()
{
	auto ret = g_Studio.StudioCheckBBox();

	if (ret != TRUE)
	{
		if (cvar.ragebot_active)
			return TRUE;
		else if (cvar.knifebot_enabled && (cvar.knifebot_aim_silent || cvar.knifebot_aim_perfect_silent) && cvar.knifebot_fov > 45.f)
			return TRUE;

		for (auto i = 1; i <= MAX_CLIENTS; ++i)
		{
			if (i == g_Local.m_iIndex)
				continue;

			const auto pPlayer = g_World.GetPlayer(i);

			if (!pPlayer)
				continue;

			if (!cvar.knifebot_friendly_fire && !cvar.ragebot_aim_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
				continue;

			if (pPlayer->m_bIsDead)
				continue;

			if (!pPlayer->m_bIsInPVS && !g_Miscellaneous.m_bFakeLatencyActive)
				continue;

			auto fov = g_Local.m_vecForward.AngleBetween(pPlayer->m_vecOrigin - g_Local.m_vecEye);

			if (fov > 45.f)
				return TRUE;
		}
	}

	return ret;
}

static inline bool IsFilteredSound(int index, DWORD channel, const cl_entity_s* ent)
{
	return index > 0 && channel != CHAN_AUTO && index != ent->index;
}

static inline bool IsUnnecessarySound(const char* pcszSoundFile)
{
	return strstr(pcszSoundFile, "c4_beep") || strstr(pcszSoundFile, "flashbang") ||
		strstr(pcszSoundFile, "he_bounce") || strstr(pcszSoundFile, "grenade_hit") ||
		strstr(pcszSoundFile, "sg_explode") || strstr(pcszSoundFile, "debris");
}

static inline void GetResolvedSoundIndex(int index, Vector origin, int& resolved)
{
	resolved = index;

	if (resolved <= MAX_CLIENTS)
		return;

	if (!g_SoundResolver.empty())
	{
		for (auto& resolver : g_SoundResolver)
		{
			if (resolver.m_iOriginalIndex == index)
			{
				resolved = resolver.m_iResolvedIndex;
				break;
			}
		}
	}

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (g_Local.m_iIndex == i)
			continue;

		const auto pPlayer = g_World.GetPlayer(i);

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS)
			continue;

		if (origin.Distance(pPlayer->m_vecOrigin) < 64.f)
		{
			if (!g_SoundResolver.empty())
			{
				for (auto j = 0; j < g_SoundResolver.size(); ++j)
				{
					if (g_SoundResolver[j].m_iOriginalIndex == index)
					{
						g_SoundResolver[j].m_iResolvedIndex = i;
						resolved = g_SoundResolver[j].m_iResolvedIndex;
						return;
					}
				}
			}

			sound_resolver_t sound = { index, i };
			g_SoundResolver.push_back(sound);

			resolved = sound.m_iResolvedIndex;
			return;
		}
	}
}

static void PreS_DynamicSound_Hooked(int index, DWORD channel, char* pszSoundFile, float* pflOrigin, float flVolume, float flAttenuation, int iTimeOff, int iPitch)
{
	if (pszSoundFile)
	{
		if (!g_Globals.m_bUnloadLibrary && client_static->state == ca_active)
		{
			const auto pGameEntity = g_Engine.GetLocalPlayer();

			if (pGameEntity && pGameEntity->index && IsFilteredSound(index, channel, pGameEntity))
			{
				auto iResolved = -1;
				GetResolvedSoundIndex(index, pflOrigin, iResolved);

				if (strstr(pszSoundFile, "weapons/sg_explode.wav"))
				{
					bool bSmokeFound = false;

					if (index > MAX_CLIENTS && index < MAX_ENTITIES)
					{
						CBaseEntity* pEntity = g_World.GetEntity(index);

						if (pEntity->m_bIsInPVS && pEntity->m_bIsSmoke)
						{
							g_ScreenLog.Log(ImColor(IM_COL32_WHITE), "Smoke %i at x: %f y: %f z: %f", index, pflOrigin[0], pflOrigin[1], pflOrigin[2]);
							pEntity->m_vecDetonatedOrigin = pflOrigin;
							bSmokeFound = true;
						}
					}

					if (!bSmokeFound)
					{
						for (int i = 1; i < MAX_ENTITIES; ++i)
						{
							CBaseEntity* pEntity = g_World.GetEntity(i);

							if (!pEntity->m_bIsInPVS)
								continue;

							if (pEntity->m_bIsPlayer)
								continue;

							if (!pEntity->m_bIsSmoke)
								continue;

							if (pEntity->m_vecOrigin.Distance(pflOrigin) < smokeRadius)
							{
								g_ScreenLog.Log(ImColor(IM_COL32_WHITE), "Smoke %i resolved at x: %f y: %f z: %f", pEntity->m_iIndex, pflOrigin[0], pflOrigin[1], pflOrigin[2]);
								pEntity->m_vecDetonatedOrigin = pflOrigin;
								bSmokeFound = true;
								break;
							}
						}
					}
				}
				else if (strstr(pszSoundFile, "player") || strstr(pszSoundFile, "weapons"))
				{
					if (iResolved > -1 && iResolved <= MAX_CLIENTS && iResolved != g_Local.m_iIndex)
					{
						const auto pPlayer = g_World.GetPlayer(iResolved);

						if (!pPlayer->m_bIsInPVS && !IsUnnecessarySound(pszSoundFile))
						{
							pPlayer->m_flHistory = (float)client_state->time;
							pPlayer->m_vecOrigin = pflOrigin;
							pPlayer->m_bSoundUpdated = true;
						}

						if (strstr(pszSoundFile, "bhit_helmet"))
						{
							pPlayer->m_iArmorType = ARMOR_VESTHELM;
							pPlayer->m_iHealth -= 80;
						}
						else if (strstr(pszSoundFile, "bhit_kevlar"))
						{
							pPlayer->m_iArmorType = ARMOR_KEVLAR;
							pPlayer->m_iHealth -= 20;
						}
						else if (strstr(pszSoundFile, "bhit_flesh"))
						{
							pPlayer->m_iArmorType = ARMOR_NONE;
							pPlayer->m_iHealth -= 30;
						}
						else if (strstr(pszSoundFile, "headshot"))
						{
							pPlayer->m_iHealth -= 80;
						}
						else if (strstr(pszSoundFile, "die") || strstr(pszSoundFile, "death"))
						{
							pPlayer->m_iArmorType = ARMOR_NONE;
							pPlayer->m_iHealth = 0;
						}
						else if (strstr(pszSoundFile, "weapons"))
						{
							if (strstr(pszSoundFile, "ak47"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "ak47");
							else if (strstr(pszSoundFile, "deagle"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "deagle");
							else if (strstr(pszSoundFile, "elite"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "elite");
							else if (strstr(pszSoundFile, "famas"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "famas");
							else if (strstr(pszSoundFile, "fiveseven"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "fiveseven");
							else if (strstr(pszSoundFile, "g3sg1"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "g3sg1");
							else if (strstr(pszSoundFile, "galil"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "galil");
							else if (strstr(pszSoundFile, "glock18"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "glock18");
							else if (strstr(pszSoundFile, "knife"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "knife");
							else if (strstr(pszSoundFile, "m3"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "m3");
							else if (strstr(pszSoundFile, "m4a1"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "m4a1");
							else if (strstr(pszSoundFile, "m249"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "m249");
							else if (strstr(pszSoundFile, "mac10"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "mac10");
							else if (strstr(pszSoundFile, "mp5"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "mp5");
							else if (strstr(pszSoundFile, "p90"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "p90");
							else if (strstr(pszSoundFile, "p228"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "p228");
							else if (strstr(pszSoundFile, "scout"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "scout");
							else if (strstr(pszSoundFile, "sg550"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "sg550");
							else if (strstr(pszSoundFile, "sg552"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "sg552");
							else if (strstr(pszSoundFile, "tmp"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "tmp");
							else if (strstr(pszSoundFile, "ump45"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "ump45");
							else if (strstr(pszSoundFile, "usp"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "usp");
							else if (strstr(pszSoundFile, "xm1014"))
								strcpy_s(pPlayer->m_szWeaponSoundName, "xm1014");
						}
					}
				}

				my_sound_t sound = { pflOrigin, pszSoundFile, (float)client_state->time, iResolved };
				g_Sounds.push_back(sound);
			}
		}
	}

	((PreS_DynamicSound_t)pres_dynamic_sound.origFunc)(index, channel, pszSoundFile, pflOrigin, flVolume, flAttenuation, iTimeOff, iPitch);
}

static void V_CalcRefdef(ref_params_s* pparams)
{
	g_Local.m_vecForward = pparams->forward;
	g_Local.m_vecRight = pparams->right;
	g_Local.m_vecUp = pparams->up;

	auto backup_punchangle = pparams->punchangle;

	if (cvar.remove_visual_recoil)
		pparams->punchangle.Clear();

	g_Client.V_CalcRefdef(pparams);

	pparams->punchangle = backup_punchangle;

	g_Miscellaneous.ThirdPerson(pparams);
}

static void __declspec(naked) oStudioRenderFinal()
{
	_asm
	{
		mov ecx, pCStudioModelRenderer
		jmp g_StudioModelRenderer.StudioRenderFinal;
	}
}

static void __declspec(naked) oStudioRenderModel()
{
	_asm
	{
		mov ecx, pCStudioModelRenderer
		jmp g_StudioModelRenderer.StudioRenderModel;
	}
}

#define IsInvisiblePart glDepthFunc (GL_GREATER); glDisable(GL_DEPTH_TEST)
#define IsVisiblePart glDepthFunc (GL_LESS); glEnable(GL_DEPTH_TEST)
#define StudioRenderFinal() { g_Globals.m_bRenderColoredModels = true; oStudioRenderFinal(); g_Globals.m_bRenderColoredModels = false; }

__forceinline void BeginRenderType(int rendertype)
{
	switch (rendertype)
	{
	case RenderType_Wireframe:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.5f);
	case RenderType_Material:
	case RenderType_Flat:
		glDisable(GL_TEXTURE_2D);
	case RenderType_Texture:
		break;
	}

	g_Globals.m_iRenderType = rendertype;
}

__forceinline void EndRenderType()
{
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	g_Globals.m_iRenderType = 0;
}

static void StudioRenderModel(void)
{
	if (!g_Globals.m_bUnloadLibrary && client_static->state == ca_active)
	{
		const auto pGameEntity = g_Studio.GetCurrentEntity();

		if (pGameEntity && pGameEntity->model && pGameEntity->index >= 1 && pGameEntity->index < MAX_ENTITIES)
		{
			if (!g_Globals.m_bDrawMyVisibleEdicts) // Get hitboxes only from original render
			{
				g_Utils.GetHitboxes(pGameEntity);
			}

			if (cvar.hide_from_obs)
			{
				oStudioRenderModel();
				return;
			}

			if (g_Globals.m_bDrawMyVisibleEdicts && pGameEntity->baseline.playerclass != 1234) //Skip not marked entities
				return;

			if (cvar.visuals && !state.visuals_panic && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
			{
				if (pGameEntity->index >= 1 && pGameEntity->index <= MAX_CLIENTS && pGameEntity->index != g_Local.m_iIndex)
				{
					const auto pPlayer = g_World.GetPlayer(pGameEntity->index);

					if (cvar.disable_render_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
						return;

					if (cvar.colored_models_backtrack && g_Globals.m_bDrawMyVisibleEdicts && pGameEntity->baseline.playerclass == 1234 && !pPlayer->m_bIsDead)
					{
						g_Studio.SetForceFaceFlags(0);
						pGameEntity->curstate.rendermode = kRenderTransTexture;
						pGameEntity->curstate.renderfx = 0;
						pGameEntity->curstate.renderamt = 0;

						BeginRenderType(cvar.colored_models_player ? cvar.colored_models_player : RenderType_Flat);

						if (cvar.colored_models_player_behind_wall)
						{
							IsInvisiblePart;

							RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_backtrack_color, sizeof(cvar.colored_models_backtrack_color));

							StudioRenderFinal();
						}

						IsVisiblePart;

						RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_backtrack_color, sizeof(cvar.colored_models_backtrack_color));

						StudioRenderFinal();

						EndRenderType();

						pGameEntity->curstate.rendermode = 0;
						pGameEntity->baseline.playerclass = 0;
						pGameEntity->origin = pGameEntity->baseline.origin;
						return;
					}

					if (cvar.glow_enabled && cvar.glow_amout > 0 && !pPlayer->m_bIsDead)
					{
						if (!cvar.glow_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
							goto colored_models;

						g_Studio.SetForceFaceFlags(STUDIO_NF_CHROME);
						pGameEntity->curstate.rendermode = kRenderTransTexture;
						pGameEntity->curstate.renderfx = kRenderFxGlowShell;
						pGameEntity->curstate.renderamt = cvar.glow_amout;

						if (pPlayer->m_iTeam == TERRORIST)
							RtlCopyMemory(g_Globals.m_flRenderColor, cvar.glow_color_t, sizeof(cvar.glow_color_t));
						else if (pPlayer->m_iTeam == CT)
							RtlCopyMemory(g_Globals.m_flRenderColor, cvar.glow_color_ct, sizeof(cvar.glow_color_ct));
						else
							RtlSecureZeroMemory(g_Globals.m_flRenderColor, sizeof(g_Globals.m_flRenderColor));

						g_Globals.m_flRenderColor[3] = 255;

						if (cvar.glow_behind_wall)
						{
							IsInvisiblePart;

							StudioRenderFinal();

							if (!cvar.colored_models_enabled || !cvar.colored_models_player || !cvar.colored_models_player_behind_wall)
							{
								pGameEntity->curstate.rendermode = 0;
								pGameEntity->curstate.renderfx = 0;
								pGameEntity->curstate.renderamt = 0;
								g_Studio.SetForceFaceFlags(0);

								oStudioRenderFinal();

								g_Studio.SetForceFaceFlags(STUDIO_NF_CHROME);
								pGameEntity->curstate.rendermode = kRenderTransTexture;
								pGameEntity->curstate.renderfx = kRenderFxGlowShell;
								pGameEntity->curstate.renderamt = cvar.glow_amout;
							}
						}

						IsVisiblePart;

						StudioRenderFinal();

						pGameEntity->curstate.rendermode = 0;
						pGameEntity->curstate.renderfx = 0;
						pGameEntity->curstate.renderamt = 0;
						g_Studio.SetForceFaceFlags(0);
					}

				colored_models:

					if (cvar.colored_models_enabled && cvar.colored_models_player && !pPlayer->m_bIsDead)
					{
						if (!cvar.colored_models_player_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
						{
							oStudioRenderModel();
							return;
						}

						g_Studio.SetForceFaceFlags(0);
						pGameEntity->curstate.rendermode = kRenderTransTexture;
						pGameEntity->curstate.renderfx = 0;
						pGameEntity->curstate.renderamt = 0;

						BeginRenderType(cvar.colored_models_player);

						if (cvar.colored_models_player_behind_wall)
						{
							IsInvisiblePart;

							if (pPlayer->m_iTeam == TERRORIST)
								RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_t_behind_wall, sizeof(cvar.colored_models_color_t_behind_wall));
							else if (pPlayer->m_iTeam == CT)
								RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_ct_behind_wall, sizeof(cvar.colored_models_color_ct_behind_wall));
							else
							{
								RtlSecureZeroMemory(g_Globals.m_flRenderColor, sizeof(g_Globals.m_flRenderColor));
								g_Globals.m_flRenderColor[3] = 255;
							}

							StudioRenderFinal();
						}

						IsVisiblePart;

						if (pPlayer->m_iTeam == TERRORIST)
							RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_t, sizeof(cvar.colored_models_color_t));
						else if (pPlayer->m_iTeam == CT)
							RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_ct, sizeof(cvar.colored_models_color_ct));
						else
						{
							RtlSecureZeroMemory(g_Globals.m_flRenderColor, sizeof(g_Globals.m_flRenderColor));
							g_Globals.m_flRenderColor[3] = 255;
						}

						StudioRenderFinal();

						EndRenderType();

						pGameEntity->curstate.rendermode = 0;
						return;
					}
				}
				else if (cvar.colored_models_enabled && cvar.colored_models_hands && pGameEntity->index == g_Local.m_iIndex && strstr(pGameEntity->model->name, "/v_"))
				{
					BeginRenderType(cvar.colored_models_hands);

					IsVisiblePart;

					RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_hands_color, sizeof(cvar.colored_models_hands_color));

					StudioRenderFinal();

					EndRenderType();
					return;
				}
				else if (cvar.colored_models_enabled && cvar.colored_models_dropped_weapons && strstr(pGameEntity->model->name, "/w_"))
				{
					BeginRenderType(cvar.colored_models_dropped_weapons);

					IsVisiblePart;

					RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_dropped_weapons_color, sizeof(cvar.colored_models_dropped_weapons_color));

					StudioRenderFinal();

					EndRenderType();
					return;
				}
			}
		}
	}

	oStudioRenderModel();
}

static void __declspec(naked) StudioRenderModel_Gate()
{
	_asm
	{
		mov pCStudioModelRenderer, ecx
		jmp StudioRenderModel
	}
}

static void __declspec(naked) oStudioSetupBones()
{
	_asm
	{
		mov ecx, pCStudioModelRenderer
		jmp g_StudioModelRenderer.StudioSetupBones;
	}
}

static void StudioSetupBones(void)
{
	if (!g_Globals.m_bUnloadLibrary && pCStudioModelRenderer && pCStudioModelRenderer->m_pStudioHeader && pCStudioModelRenderer->m_pPlayerInfo)
	{
		if (cvar.ragebot_active && cvar.ragebot_resolver)
		{
			const auto pGameEntity = pCStudioModelRenderer->m_pCurrentEntity;

			if (pGameEntity && pGameEntity->player && pGameEntity->index != g_Local.m_iIndex && pGameEntity->index >= 1 && pGameEntity->index <= MAX_CLIENTS)
			{
				TransformMatrix matrix;

				QAngle QAngles(pGameEntity->curstate.angles);

				QAngles.y = pCStudioModelRenderer->m_pPlayerInfo->gaityaw;

				float flPitch = QAngles.x * 9;

				if (flPitch > 179)
					QAngles.x = 30;
				else if (flPitch < -179)
					QAngles.x = -30;

				g_Utils.AngleMatrix(QAngles, (*matrix));

				(*matrix)[0][3] = pGameEntity->origin[0];
				(*matrix)[1][3] = pGameEntity->origin[1];
				(*matrix)[2][3] = pGameEntity->origin[2];

				const auto pbones = (mstudiobone_t*)((byte*)pCStudioModelRenderer->m_pStudioHeader + pCStudioModelRenderer->m_pStudioHeader->boneindex);

				if (pbones)
				{
					for (auto i = 0; i < pCStudioModelRenderer->m_pStudioHeader->numbones; i++)
					{
						if (!strlen(pbones[i].name))
							continue;

						if (pbones[i].parent != -1)
							continue;

						for (auto x = 0; x < 3; x++)
						{
							for (auto j = 0; j < 4; j++)
							{
								pCStudioModelRenderer->m_protationmatrix[i][x][j] = matrix[i][x][j];
							}
						}
					}
				}
			}
		}
	}

	oStudioSetupBones();
}

static void __declspec(naked) StudioSetupBones_Gate()
{
	_asm
	{
		mov pCStudioModelRenderer, ecx
		jmp StudioSetupBones
	}
}

static void HUD_PlayerMoveInit(playermove_s* ppmove)
{
	PM_InitTextureTypes(ppmove);

	return g_Client.HUD_PlayerMoveInit(ppmove);
}

static int CL_IsThirdPerson(void)
{
	if (!g_Globals.m_bUnloadLibrary && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot && cvar.visuals && !state.visuals_panic)
	{
		if (cvar.thirdperson && !g_Local.m_bIsDead)
		{
			if ((cvar.thirdperson_key_toggle && state.thirdperson_active || !cvar.thirdperson_key_toggle))
			{
				return TRUE;
			}
		}
	}

	return g_Client.CL_IsThirdPerson();
}

static int pfnVGUI2DrawCharacterAdditive(int x, int y, int ch, int r, int g, int b, unsigned int hfont)
{
	if (cvar.hud_clear && !state.visuals_panic && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
		return TRUE;

	return g_Engine.pfnVGUI2DrawCharacterAdditive(x, y, ch, r, g, b, hfont);
}

static int HUD_GetHullBounds(int hullnum, float* /*mins*/, float* maxs)
{
	if (hullnum == 1)
		maxs[2] = 32.f; //https://gist.github.com/WPMGPRoSToTeMa/2e6e0454654f9e4ca22ee3e987051b57
	return TRUE;
}

static bool GetKeyPress(const int keyvar, const int keynum, const int down, bool* state)
{
	if (keynum == keyvar)
	{
		if (!g_Local.m_bIsDead)
		{
			*state = !!down;
			return true;
		}
		else
		{
			// It's useless to use keybind when we're dead.
			*state = false;
		}
	}

	return false;
}

static bool GetKeyToggle(const int keyvar, const int keynum, const int down, bool* state)
{
	if (down && keynum == keyvar)
	{
		*state = !*state;
		return true;
	}

	return false;
}

static int HUD_Key_Event(int down, int keynum, const char* pszCurrentBinding)
{
	if (cvar.visuals_panic_key && GetKeyToggle(cvar.visuals_panic_key, keynum, down, &state.visuals_panic))
		return 0;

	if (cvar.ragebot_active)
	{
		if (cvar.ragebot_fake_walk_enabled)
		{
			if (GetKeyPress(cvar.ragebot_fake_walk_key_press, keynum, down, &state.fakewalk_active))
				return 0;
		}
	}
	else if (cvar.legitbot_active)
	{
		if (cvar.legitbot_trigger_key)
		{
			if (cvar.legitbot_trigger_key_type == 0)
			{
				if (GetKeyPress(cvar.legitbot_trigger_key, keynum, down, &state.trigger_active))
					return 0;
			}
			else if (cvar.legitbot_trigger_key_type == 1)
			{
				if (GetKeyToggle(cvar.legitbot_trigger_key, keynum, down, &state.trigger_active))
					return 0;
			}
		}
	}

	if (cvar.fakelatency_enabled && cvar.fakelatency_value > 0)
	{
		if (GetKeyPress(cvar.fakelatency_key_press, keynum, down, &state.fakelatency_active))
			return 0;
	}

	if (cvar.knifebot_enabled && cvar.knifebot_key_toggle)
	{
		if (GetKeyToggle(cvar.knifebot_key_toggle, keynum, down, &state.knifebot_active))
			return 0;
	}

	if (cvar.thirdperson && cvar.thirdperson_key_toggle)
	{
		if (GetKeyToggle(cvar.thirdperson_key_toggle, keynum, down, &state.thirdperson_active))
			return 0;
	}

	if (cvar.kreedz.active)
	{
		if (cvar.kreedz.strafe_invisible_key && GetKeyPress(cvar.kreedz.strafe_invisible_key, keynum, down, &state.strafe_active))
			return 0;

		if (cvar.kreedz.jumpbug_key && GetKeyPress(cvar.kreedz.jumpbug_key, keynum, down, &state.jumpbug_active))
			return 0;

		if (cvar.kreedz.edgebug_key && GetKeyPress(cvar.kreedz.edgebug_key, keynum, down, &state.edgebug_active))
			return 0;

		if (cvar.kreedz.bunnyhop_key && GetKeyPress(cvar.kreedz.bunnyhop_key, keynum, down, &state.bunnyhop_active))
			return 0;

		if (cvar.kreedz.gstrafe_standup_key && GetKeyPress(cvar.kreedz.gstrafe_standup_key, keynum, down, &state.gstrafe_standup_active))
			return 0;

		if (cvar.kreedz.gstrafe_key && GetKeyPress(cvar.kreedz.gstrafe_key, keynum, down, &state.gstrafe_active))
			return 0;
	}

	return g_Client.HUD_Key_Event(down, keynum, pszCurrentBinding);
}

static void HUD_Shutdown()
{
	if (cvar.settings.autosave_when_quit)
		g_Settings.Save("Last used settings");

	g_Client.HUD_Shutdown();
}

static void R_DrawEntitiesOnList_Hooked(void)
{
	if (!g_Globals.m_bUnloadLibrary && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
	{
		if (!cvar.hide_from_obs && !state.visuals_panic)
		{
			const auto pNumEntities = g_Globals.m_piNumVisibleEntities;
			const auto pEntities = g_Globals.m_pVisibleEntities;

			if (cvar.colored_models_enabled && cvar.colored_models_backtrack)
			{
				if (g_Miscellaneous.m_bFakeLatencyActive || g_Miscellaneous.m_sPositionAdjustmentLerpVisual != -1)
				{
					auto nRender = 0;

					for (auto i = 0; i < *pNumEntities; i++)
					{
						const auto pGameEntity = pEntities[i];

						if (!pGameEntity)
							continue;

						if (!pGameEntity->index)
							continue;

						if (pGameEntity->index < 1 || pGameEntity->index > MAX_CLIENTS)
							continue;

						if (pGameEntity->index == g_Local.m_iIndex)
							continue;

						if (!pGameEntity->model)
							continue;

						const auto pPlayer = g_World.GetPlayer(pGameEntity->index);

						if (pPlayer->m_bIsDead)
							continue;

						if (cvar.disable_render_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
							continue;

						if (!cvar.colored_models_player_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
							continue;

						auto lerp = (g_Miscellaneous.m_sPositionAdjustmentLerpVisual != -1) ? g_Miscellaneous.m_sPositionAdjustmentLerpVisual : -1;

						Vector vecCorrectedOrigin(pPlayer->m_vecOrigin);

						g_Utils.LagCompensation(pPlayer->m_iIndex, lerp, vecCorrectedOrigin);

						if (vecCorrectedOrigin != pPlayer->m_vecOrigin)
						{
							pGameEntity->baseline.playerclass = 1234;
							pGameEntity->baseline.origin = pGameEntity->origin;
							pGameEntity->origin = vecCorrectedOrigin;
							nRender++;
						}
					}

					if (nRender)
					{
						g_Globals.m_bDrawMyVisibleEdicts = true;
						((R_DrawEntitiesOnList_t)r_draw_entities_on_list.origFunc)();
						g_Globals.m_bDrawMyVisibleEdicts = false;
					}
				}
			}
		}
	}

	((R_DrawEntitiesOnList_t)r_draw_entities_on_list.origFunc)();
}

LRESULT CALLBACK WndProc_Hooked(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == VK_INSERT)
		{
			g_DrawGUI.FadeRunning();
			g_DrawGUI.Toggle();
		}
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		if (g_DrawGUI.IsDrawing() && !ImGui::Control::IsKeyAssingActive())
			return false;
	}

	return CallWindowProc(g_pWndProc, hWnd, uMsg, wParam, lParam);
}

void CL_RecordHUDCommand_Hooked(const char *cmdname)
{
	std::string cmd = cmdname;

	if (cmd == std::string("+attack"))
	{
		if (g_Miscellaneous.hud_buttons & IN_ATTACK)
			return;
		g_Miscellaneous.hud_buttons |= IN_ATTACK;
	}
	else if (cmd == std::string("-attack"))
	{
		if (!(g_Miscellaneous.hud_buttons & IN_ATTACK))
			return;
		g_Miscellaneous.hud_buttons &= ~IN_ATTACK;
	}
	else if (cmd == std::string("+attack2"))
	{
		if (g_Miscellaneous.hud_buttons & IN_ATTACK2)
			return;
		g_Miscellaneous.hud_buttons |= IN_ATTACK2;
	}
	else if (cmd == std::string("-attack2"))
	{
		if (!(g_Miscellaneous.hud_buttons & IN_ATTACK2))
			return;
		g_Miscellaneous.hud_buttons &= ~IN_ATTACK2;
	}

	((CL_RecordHUDCommand_t)cl_record_hud_command.origFunc)(cmdname);
}

static int InitiateGameConnection_Hooked(void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure)
{
	if (cvar.misc.steamspoofid_enabled)
	{
		int steamid = atoi(cvar.misc.steamspoofid_custom_id);

		if (cvar.misc.steamspoofid_random)
			steamid = g_Engine.pfnRandomLong(1, 999999999);
		
		return GenerateRevEmu(pAuthBlob, steamid);
	}

	return ((InitiateGameConnection_t)initiate_game_connection.origFunc)(pAuthBlob, cbMaxAuthBlob, steamIDGameServer, unIPServer, usPortServer, bSecure);
}

BOOL InstallHooks(void)
{
	g_pConsole->DPrintf("[hpp] installing hooks...\n");

	if (!HookCommands())
		return FALSE;

	if (!HookUserMessages())
		return FALSE;

	if (g_pClientEngineMsgs)
	{
		if (!HookEngineMessages())
			return FALSE;
	}

	if (!(g_pWndProc = (WNDPROC)(SetWindowLong(g_Globals.m_hWnd, GWL_WNDPROC, (LRESULT)(WndProc_Hooked)))))
	{
		TraceLog("%s: windowproc not hooked.\n", __FUNCTION__);
		return FALSE;
	}

	if (g_pCL_RecordHUDCommand != nullptr)
	{
		cl_record_hud_command.Hook((FARPROC)g_pCL_RecordHUDCommand, (FARPROC)CL_RecordHUDCommand_Hooked);
		cl_record_hud_command.Rehook();
	}

	g_pClient->HUD_Frame = Initialize;
	g_pClient->HUD_GetHullBounds = HUD_GetHullBounds;
	g_pClient->HUD_PlayerMoveInit = HUD_PlayerMoveInit;
	g_pClient->HUD_PostRunCmd = HUD_PostRunCmd;
	g_pClient->CL_CreateMove = CL_CreateMove;
	g_pClient->CL_IsThirdPerson = CL_IsThirdPerson;
	g_pClient->V_CalcRefdef = V_CalcRefdef;
	g_pClient->HUD_Key_Event = HUD_Key_Event;
	g_pClient->HUD_Shutdown = HUD_Shutdown;

	g_pEngine->pfnVGUI2DrawCharacterAdditive = pfnVGUI2DrawCharacterAdditive;

	if (g_Utils.EnablePageWrite((DWORD)(g_pStudioModelRenderer), sizeof(g_StudioModelRenderer)))
	{
		g_pStudioModelRenderer->StudioRenderModel = StudioRenderModel_Gate;
		g_pStudioModelRenderer->StudioSetupBones = StudioSetupBones_Gate;

		g_Utils.RestorePageProtection((DWORD)(g_pStudioModelRenderer), sizeof(g_StudioModelRenderer));
	}

	g_pStudio->StudioCheckBBox = StudioCheckBBox;

	if (g_pPreS_DynamicSound != nullptr)
	{
		pres_dynamic_sound.Hook((FARPROC)g_pPreS_DynamicSound, (FARPROC)PreS_DynamicSound_Hooked);
		pres_dynamic_sound.Rehook();
	}

	if (g_pR_DrawEntitiesOnList != nullptr)
	{
		r_draw_entities_on_list.Hook((FARPROC)g_pR_DrawEntitiesOnList, (FARPROC)R_DrawEntitiesOnList_Hooked);
		r_draw_entities_on_list.Rehook();
	}

	if (g_pInitiateGameConnection != nullptr)
	{
		initiate_game_connection.Hook((FARPROC)g_pInitiateGameConnection, (FARPROC)InitiateGameConnection_Hooked);
		initiate_game_connection.Rehook();
	}

	if (!HookOpenGL())
		return FALSE;

	g_Engine.pfnClientCmd("-attack");
	g_Engine.pfnClientCmd("-attack2");

	return TRUE;
}

void DeleteHooks()
{
	g_pConsole->DPrintf("[hpp] preparing to unload...\n");

	if (cvar.settings.autosave_when_unhook)
	{
		g_Settings.Save("Last used settings");
		g_pConsole->DPrintf("[hpp] settings automatically saved.\n");
	}

	CloseSocket();

	Sleep(100);

	g_Miscellaneous.CustomRenderFov(DEFAULT_FOV);

	g_pConsole->DPrintf("[hpp] deleting hooks...\n");

	pres_dynamic_sound.Unhook();
	r_draw_entities_on_list.Unhook();
	cl_record_hud_command.Unhook();
	initiate_game_connection.Unhook();

	UnHookCommands();
	UnHookUserMessages();
	UnHookEngineMessages();

	if (g_pWndProc)
		SetWindowLong(g_Globals.m_hWnd, GWL_WNDPROC, (LRESULT)(g_pWndProc));

	if (g_pStudio)
		*g_pStudio = g_Studio;

	if (g_pClient)
		*g_pClient = g_Client;

	if (g_pEngine)
		*g_pEngine = g_Engine;

	if (g_pStudioModelRenderer)
	{
		if (g_Utils.EnablePageWrite((DWORD)(g_pStudioModelRenderer), sizeof(g_StudioModelRenderer)))
		{
			*g_pStudioModelRenderer = g_StudioModelRenderer;

			g_Utils.RestorePageProtection((DWORD)(g_pStudioModelRenderer), sizeof(g_StudioModelRenderer));
		}
	}

	if (g_Offsets.m_dwInterpolationTime)
		g_Utils.memwrite(g_Offsets.m_dwInterpolationTime, (uintptr_t)(g_Offsets.m_puszPatchInterpolationTimeBackup), sizeof(g_Offsets.m_puszPatchInterpolationTimeBackup));

	UnHookOpenGL();

	Sleep(100);

	ImGui_ImplSdlGL2_Shutdown();

	Sleep(1000);

	g_pConsole->DPrintf("[hpp] free library...\n");

	FreeLibraryAndExitThread(g_Globals.m_hModule, 0);
}