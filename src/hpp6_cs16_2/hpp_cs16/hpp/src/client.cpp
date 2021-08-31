#include "framework.h"

static CHookData pres_dynamic_sound;
static CHookData cl_drift_interpolation_amount;
static CHookData cl_record_hud_command;
static CHookData host_filter_time;
static CHookData scr_update_screen;
static CHookData initiate_game_connection;
static CHookData cl_runusercmd;
static CHookData net_send_packet;
static CHookData in_mouse_move;
static VHookTable vmt_surface;

CHookData r_draw_entities_on_list;

cl_clientfunc_t g_Client, *g_pClient;
cl_enginefunc_t g_Engine, *g_pEngine;
engine_studio_api_t g_Studio, *g_pStudio;
r_studio_interface_t g_StudioAPI, *g_pStudioAPI;
StudioModelRenderer_t g_StudioModelRenderer, *g_pStudioModelRenderer;
playermove_s* pmove;
client_state_t* client_state;
client_static_t* client_static;
StudioModelRenderer_d pCStudioModelRenderer = 0;
svc_func_t* g_pClientEngineMsgs;

IRunGameEngine* g_pIRunGameEngine;
IGameConsole* g_pConsole;
IGameUI* g_pIGameUI;
IGameUIFuncs* g_pIGameUIFuncs;
ISurface* g_pISurface;
IEngineVGui* g_pIEngineVGui;
IPanel* g_pIPanel;
ISteamClient* g_pISteamClient;
ISteamScreenshots* g_pISteamScreenshots;
ISteamUtils* g_pISteamUtils;
ISteamFriends* g_pISteamFriends;
ISteamUser* g_pISteamUser;

static void PreS_DynamicSound(int index, DWORD channel, char* pszSoundFile, float* pOrigin, float volume, float attenuation, int timeoff, int pitch)
{
	if (!pres_dynamic_sound.IsHooked())
		return;

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		if (g_pSound->IsValidSound(pszSoundFile, pOrigin, index, channel))
		{
			int resolved_index;

			g_pSound->GetResolvedIndex(index, pOrigin, resolved_index);

			if (g_pSound->IsPlayerSound(pszSoundFile) || g_pSound->IsWeaponSound(pszSoundFile))
			{
				if (cvars::visuals.esp_player_sounds)
				{
					if (g_pSound->IsPlayerSound(pszSoundFile) && g_pSound->IsFilterSound(pszSoundFile) && !g_pSound->IsHitSound(pszSoundFile))
						g_PlayerSounds.push_back(player_sound_t{ pOrigin, client_state->time, resolved_index });
				}

				if (g_Entity[resolved_index]->m_bIsPlayer)
				{
					if (cvars::visuals.esp_dormant_update_by_sound && g_pSound->IsFilterSound(pszSoundFile))
						g_pSound->UpdateDormantPlayer(resolved_index, pOrigin);

					g_pSound->UpdatePlayerHealth(resolved_index, pszSoundFile);
					g_pSound->UpdatePlayerWeapon(resolved_index, pszSoundFile);
				}
			}
		}
	}

	((PreS_DynamicSound_t)pres_dynamic_sound.m_OrigFunc)(index, channel, pszSoundFile, pOrigin, volume, attenuation, timeoff, pitch);
}

void CL_RecordHUDCommand(const char* cmdname)
{
	if (!cl_record_hud_command.IsHooked())
		return;

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected() && !IS_NULLPTR(g_pMiscellaneous) && client_static->demorecording && cmdname && strlen(cmdname))
	{
		const std::string cmd = cmdname;

		if (cmd == std::string("+attack"))
		{
			if (g_pMiscellaneous->m_iHudCommands & IN_ATTACK)
				return;

			g_pMiscellaneous->m_iHudCommands |= IN_ATTACK;
		}
		else if (cmd == std::string("-attack"))
		{
			if (~g_pMiscellaneous->m_iHudCommands & IN_ATTACK)
				return;

			g_pMiscellaneous->m_iHudCommands &= ~IN_ATTACK;
		}
		else if (cmd == std::string("+attack2"))
		{
			if (g_pMiscellaneous->m_iHudCommands & IN_ATTACK2)
				return;

			g_pMiscellaneous->m_iHudCommands |= IN_ATTACK2;
		}
		else if (cmd == std::string("-attack2"))
		{
			if (~g_pMiscellaneous->m_iHudCommands & IN_ATTACK2)
				return;

			g_pMiscellaneous->m_iHudCommands &= ~IN_ATTACK2;
		}

#if !defined(LICENSING) || defined(DEBUG)
		g_pConsole->DPrintf("> %s: %s.\n", __FUNCTION__, cmdname);
#endif
	}

	((CL_RecordHUDCommand_t)cl_record_hud_command.m_OrigFunc)(cmdname);
}

static int CL_DriftInterpolationAmount(int interp)
{
	if (!cl_drift_interpolation_amount.IsHooked())
		return 0;

	int lerp_msec = ((CL_DriftInterpolationAmount_t)cl_drift_interpolation_amount.m_OrigFunc)(interp);

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		if (!IS_NULLPTR(g_pMiscellaneous) && g_pMiscellaneous->m_bPositionAdjustmentActive)
		{
			lerp_msec = g_pMiscellaneous->m_flPositionAdjustmentInterpAmount * 1000.0;

			g_pMiscellaneous->m_bPositionAdjustmentActive = false;
		}
	}

	//g_Engine.Con_NPrintf(0, "lerp_msec: %i", lerp_msec);

	return lerp_msec;
}

static int Host_FilterTime(float passedTime)
{
	if (!host_filter_time.IsHooked())
		return 0;

	auto ret = ((Host_FilterTime_t)host_filter_time.m_OrigFunc)(passedTime);

	if (!g_pGlobals->m_bIsUnloadingLibrary)
	{
		//speedhack if minus,  any frametime and etc
	}

	return ret;
}

static void SCR_UpdateScreen()
{
	if (!scr_update_screen.IsHooked())
		return;

	if (!g_pGlobals->m_bIsUnloadingLibrary)
	{
		/*if (cvar.norefresh)
		{
			ÍÈÕÓß ÍÅ ÐÅÍÄÈÌ Â ÊÑ ÒÓÏÀ ÏÐßÌ ÍÈÕÓß ÏËÞÑ ÌÈËËÈÀÐÄ ÔÏÑ ÑÎÑÀÒÜ ÅÁÒÀ ÃÅÉÁ ÍÀÕÓÉ ÕÓÅÏÓÒÀËÎ
				return
		}*/
	}

	((SCR_UpdateScreen_t)scr_update_screen.m_OrigFunc)();
}

static void CL_CreateMove(float frametime, usercmd_s* cmd, int active)
{
	g_Client.CL_CreateMove(frametime, cmd, active);
	
	Game::SendCommand(true);

	if (!g_pGlobals->m_bIsUnloadingLibrary && g_pGlobals->m_bIsInGame && strcmp(g_Engine.PhysInfo_ValueForKey("pi"), V("xvi")))
	{
		g_Local.Update(frametime, cmd);

		if (g_Local->m_bIsConnected)
		{
			g_Sequences.Update();
			g_Player.Update();
			g_Entity.Update();

			if (!client_static->demoplayback)
			{
				g_pMiscellaneous->FakeLatency();

				if (!g_Local->m_bIsDead)
				{
					if (g_pLegitBot.get())
						g_pLegitBot->Run(cmd);

					if (g_pRageBot.get())
						g_pRageBot->Run(cmd);

					if (g_pKnifeBot.get())
						g_pKnifeBot->Run(cmd);

					g_pMiscellaneous->AutoReload(cmd);
					g_pMiscellaneous->AutoPistol(cmd);

					if (g_pRageBot.get())
					{
						g_pRageBot->AntiAimbot(cmd);
						g_pRageBot->FakeLag(cmd);
					}

					g_pMiscellaneous->ChokedCommandsCounter();

					g_pMiscellaneous->NameStealer();

					g_HitRegister.Update(cmd);
				}

				g_pMiscellaneous->RecordHUDCommands(cmd);
			}
		}
	}
}

static void MemoryController()
{
	if (g_pIRunGameEngine->IsInGame() && g_Engine.GetClientMaxspeed())
	{
		if (!g_pGlobals->m_bIsInGame)
		{
			g_Local.AllocateMemory();
			g_Player.AllocateMemory();
			g_Entity.AllocateMemory();
			g_Weapon.AllocateMemory();
			g_HitRegister.AllocateMemory();

			g_Sequences.Clear();
			g_HitRegister.Clear();

			g_ToggleStatus = ToggleStatusData{};

			MemoryAdjust(g_pMiscellaneous);
			MemoryAdjust(g_pSound);

			g_pGlobals->m_bIsInGame = true;
			g_pGlobals->m_nNewCommands = (client_static->build_num < 5971) ? 16 : MAX_TOTAL_CMDS;
		}

		MemoryAdjust(g_pLegitBot, cvars::legitbot.active);
		MemoryAdjust(g_pKnifeBot, cvars::misc.kb_enabled);
		MemoryAdjust(g_pRageBot, cvars::ragebot.active);
		MemoryAdjust(g_pVisuals, cvars::visuals.active);
	}
	else
	{
		if (g_pGlobals->m_bIsInGame)
		{
			g_Local.FreeMemory();
			g_Player.FreeMemory();
			g_Entity.FreeMemory();
			g_Weapon.FreeMemory();
			g_HitRegister.FreeMemory();

			g_Sequences.Clear();
			g_HitRegister.Clear();

			g_ToggleStatus = ToggleStatusData{};

			MemoryAdjust(g_pLegitBot, false);
			MemoryAdjust(g_pRageBot, false);
			MemoryAdjust(g_pKnifeBot, false);
			MemoryAdjust(g_pMiscellaneous, false);
			MemoryAdjust(g_pVisuals, false);
			MemoryAdjust(g_pSound, false);

			g_PlayerSounds.clear();

			g_pGlobals->m_bIsInGame = false;
		}
	}
};

static void HUD_Frame(double frametime)
{
	auto ForceCvarValues = []()
	{
		g_ClientCvarsMap["cl_crosshair_translucent"]->value = 0.f;
		g_ClientCvarsMap["host_limitlocal"]->value = 1.f;
		g_ClientCvarsMap["cl_nosmooth"]->value = 1.f;
		g_ClientCvarsMap["cl_lc"]->value = 1.f;
		g_ClientCvarsMap["cl_lw"]->value = 1.f;	
		g_ClientCvarsMap["pausable"]->value = 0.f;

		if (g_ClientCvarsMap["gl_ztrick"])
			g_ClientCvarsMap["gl_ztrick"]->value = 0.f;

		if (g_ClientCvarsMap["gl_ztrick_old"])
			g_ClientCvarsMap["gl_ztrick_old"]->value = 0.f;
	};	

	if (!g_pGlobals->m_bIsUnloadingLibrary)
	{
		g_pGlobals->m_flFrameTime = static_cast<float>(frametime);
	
		ForceCvarValues();
		MemoryController();
		HookControllerGL();

		Game::CountGameTime();
	
		if (Game::IsConnected())
		{
			g_Local->m_flFrameTime = g_pGlobals->m_flFrameTime;

			Game::CountRoundTime();
			Game::EstimateGait();
		}
	}

	g_Client.HUD_Frame(frametime);
}

static void Initialize(double time)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary)
	{
		if (!g_pConsole->IsConsoleVisible())
			g_pConsole->Activate();

		g_pSettings = std::make_unique<CSettings>();

		InitFileHandleList();

		g_pSettings->Load("Default");

		g_pConsole->DPrintf(V("Hello %s\n"), g_pISteamFriends ? g_pISteamFriends->GetPersonaName() : g_ClientCvarsMap["name"]->string);
		g_pConsole->DPrintf(V("Cheat injected :3\n"));
		g_pConsole->DPrintf(V("Site: hpp.ovh\n"));
		g_pConsole->DPrintf(V("Build: %s\n"), V(__DATE__));

		PM_InitTextureTypes(pmove);

		g_pClient->HUD_Frame = HUD_Frame;
	}

	g_Client.HUD_Frame(time);
}

static void HUD_PostRunCmd(local_state_s* from, local_state_s* to, usercmd_s* cmd, int runfuncs, double time, unsigned random_seed)
{
	g_Client.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		g_Local->m_iRealFOV = static_cast<int>(to->client.fov);

		if (!g_Local->m_iFOV)
		{
			g_Local->m_iFOV = g_Local->m_iRealFOV ? min(g_Local->m_iRealFOV, DEFAULT_FOV) : DEFAULT_FOV;
			g_Local->m_bIsScoped = g_Local->m_iFOV < 55;
		}

		if (g_pVisuals.get())
		{
			if (cvars::visuals.remove_scope && g_Local->m_bIsScoped)
				Game::SetRenderFOV(45);

			if (!g_Local->m_bIsScoped)
			{
				if (!Game::IsTakenScreenshot() && !cvars::visuals.streamer_mode && cvars::visuals.effects_custom_render_fov > DEFAULT_FOV)
					Game::SetRenderFOV(cvars::visuals.effects_custom_render_fov);
				else
					Game::SetRenderFOV(g_Local->m_iFOV);
			}
		}

		g_Weapon.Update(from, to, cmd, time, random_seed);
	}
}

static void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, tempent_s** ppTempEntFree, tempent_s** ppTempEntActive,
	int(*Callback_AddVisibleEntity)(cl_entity_s* pEntity), void(*Callback_TempEntPlaySound)(tempent_s* pTemp, float damp))
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected() && !Game::IsTakenScreenshot() && !cvars::visuals.streamer_mode)
	{
		if (g_pVisuals.get())
		{
			g_pVisuals->Light();
			g_pVisuals->LineSight();
		}
	}

	g_Client.HUD_TempEntUpdate(frametime, client_time, cl_gravity, ppTempEntFree, ppTempEntActive, Callback_AddVisibleEntity, Callback_TempEntPlaySound);
}

static int StudioCheckBBox()
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		cl_entity_s* pGameEntity = g_Studio.GetCurrentEntity();

		if (Game::IsValidEntity(pGameEntity) && pGameEntity->index >= 1 && pGameEntity->index <= client_state->maxclients)
		{
			if (pGameEntity->trivial_accept == HIT_POSITION_MARKER)
				return 1;

			if (!g_Player[pGameEntity->index]->m_bIsDead && !pGameEntity->trivial_accept)
			{
				const auto flMinimalFOV = g_Local->m_iFOV / 2.f;

				if (!IS_NULLPTR(g_pRageBot) && (cvars::ragebot.raim_fov > flMinimalFOV || (cvars::ragebot.fakelag_enabled && cvars::ragebot.fakelag_on_enemy_in_pvs) || cvars::ragebot.aa_enabled))
				{
					if (g_Player[pGameEntity->index]->m_iTeamNum != g_Local->m_iTeamNum || cvars::ragebot.friendly_fire || cvars::ragebot.aa_teammates)
						return 1;
				}

				if (!IS_NULLPTR(g_pLegitBot) && g_Weapon.IsGun() && cvars::weapons[g_Weapon->m_iWeaponID].aim_enabled && g_pLegitBot->m_flCurrentFOV > flMinimalFOV)
				{
					if (g_Player[pGameEntity->index]->m_iTeamNum != g_Local->m_iTeamNum || cvars::legitbot.friendly_fire)
						return 1;
				}

				if (!IS_NULLPTR(g_pMiscellaneous) && cvars::misc.kb_enabled && cvars::misc.kb_fov > flMinimalFOV && g_Weapon.IsKnife())
				{
					if (g_Player[pGameEntity->index]->m_iTeamNum != g_Local->m_iTeamNum || cvars::misc.kb_friendly_fire)
						return 1;
				}

				if (!IS_NULLPTR(g_pVisuals))
				{
					if (cvars::visuals.colored_models_players_hit_position)
						return 1;

					if (cvars::visuals.esp_player && cvars::visuals.esp_player_line_of_sight)
						return 1;
				}
			}
		}
	}

	return g_Studio.StudioCheckBBox();
}

static int CL_IsThirdPerson()
{
	g_pGlobals->m_bIsInThirdPerson = g_Client.CL_IsThirdPerson();

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected() && !Game::IsTakenScreenshot())
	{
		if (g_pVisuals.get() && cvars::visuals.effects_thirdperson && !g_Local->m_bIsDead)
		{
			if (!cvars::visuals.effects_thirdperson_key.keynum || (cvars::visuals.effects_thirdperson_key.keynum && g_pVisuals->m_bThirdPersonState))
				g_pGlobals->m_bIsInThirdPerson = true;
		}
	}

	return static_cast<int>(g_pGlobals->m_bIsInThirdPerson);
}

static void V_CalcRefdef(ref_params_s* pparams)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
	}

	g_Client.V_CalcRefdef(pparams);

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		if (g_pVisuals.get() && cvars::visuals.effects_thirdperson && !Game::IsTakenScreenshot() && !g_Local->m_bIsDead)
		{
			if (!cvars::visuals.effects_thirdperson_key.keynum || (cvars::visuals.effects_thirdperson_key.keynum && g_pVisuals->m_bThirdPersonState))
			{
				Game::V_GetChaseOrigin(pparams->vieworg, cvars::visuals.effects_thirdperson, pparams->vieworg);

				g_Local->m_flDistance = g_Local->m_vecOrigin.Distance(pparams->vieworg);

				QAngle viewangles = pparams->viewangles;

				viewangles.Normalize();

				viewangles.x /= -3.f;

				cl_entity_t* pGameEntity = g_Engine.GetLocalPlayer();

				if (Game::IsValidEntity(pGameEntity))
				{
					pGameEntity->angles.x = viewangles.x;
					pGameEntity->curstate.angles.x = viewangles.x;
					pGameEntity->prevstate.angles.x = viewangles.x;
					pGameEntity->latched.prevangles.x = viewangles.x;
				}
			}
		}
	}
}

static bool HandleBind(bind_t key, int keynum, int down, bool* state, std::string name = std::string())
{
	if (key.keynum)
	{
		auto OnPress = [](int check, int keynum, int down, bool* state)
		{
			const bool ret = keynum == check;

			if (keynum == check)
				*state = !!down;

			return ret;
		};

		auto OnToggle = [](int check, int keynum, int down, bool* state, std::string name)
		{
			const bool ret = keynum == check;

			if (down && ret)
			{
				*state = !*state;

				if (!name.empty())
					g_ToggleStatus = ToggleStatusData{ name, *state, client_state->time };
			}

			return ret;
		};

		switch (key.type)
		{
		case KEY_ON_PRESS:
			return OnPress(key.keynum, keynum, down, state);
		case KEY_ON_TOGGLE:
			return OnToggle(key.keynum, keynum, down, state, name);
		}
	}

	return false;
}

static int HUD_Key_Event(int down, int keynum, const char* pszCurrentBinding)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		if (HandleBind(cvars::visuals.panic_key, keynum, down, &cvars::visuals.active, "GLOBAL VISUALS"))
			return 0;

		if (HandleBind(cvars::misc.kb_key, keynum, down, &cvars::misc.kb_enabled, "KNIFEBOT"))
			return 0;

		if (HandleBind(cvars::misc.fakelatency_key, keynum, down, &cvars::misc.fakelatency, "FAKE LATENCY"))
			return 0;

		if (g_pLegitBot.get())
		{
			if (HandleBind(cvars::legitbot.aim_key, keynum, down, &g_pLegitBot->m_bAimState, "LEGITBOT AIM"))
				return 0;

			if (HandleBind(cvars::legitbot.aim_psilent_key, keynum, down, &g_pLegitBot->m_bAimPerfectSilentState, "LEGITBOT PSILENT"))
				return 0;

			if (HandleBind(cvars::legitbot.trigger_key, keynum, down, &g_pLegitBot->m_bTriggerState, "LEGITBOT TRIGGER"))
				return 0;
		}

		if (g_pRageBot.get())
		{
			if (HandleBind(cvars::ragebot.raim_force_body_key, keynum, down, &g_pRageBot->m_bForceBody, "FORCE BODY AIM"))
				return 0;

			if (HandleBind(cvars::ragebot.aa_side_key, keynum, down, reinterpret_cast<bool*>(&cvars::ragebot.aa_side)))
				return 0;
		}

		if (g_pVisuals.get())
		{
			if (HandleBind(cvars::visuals.effects_thirdperson_key, keynum, down, &g_pVisuals->m_bThirdPersonState, "THIRDPERSON"))
				return 0;
		}
	}

	return g_Client.HUD_Key_Event(down, keynum, pszCurrentBinding);
}

#pragma region StudioRenderModel

void __declspec(naked) oStudioRenderFinal()
{
	_asm
	{
		mov ecx, pCStudioModelRenderer
		jmp g_StudioModelRenderer.StudioRenderFinal;
	}
}

void __declspec(naked) oStudioRenderModel()
{
	_asm
	{
		mov ecx, pCStudioModelRenderer
		jmp g_StudioModelRenderer.StudioRenderModel;
	}
}

void __declspec(naked) oStudioSetUpTransform()
{
	_asm
	{
		mov ecx, pCStudioModelRenderer
		jmp g_StudioModelRenderer.StudioSetUpTransform;
	}
}

void StudioRenderModel()
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		cl_entity_s* pGameEntity = pCStudioModelRenderer->m_pCurrentEntity;

		if (pGameEntity && pGameEntity->model && pGameEntity->index > 0 && pGameEntity->index <= client_state->max_edicts)
		{
			if (pGameEntity->trivial_accept == LOCAL_PLAYER_FOR_GAITYAW)
				return;

			if (!pGameEntity->trivial_accept)
			{
				Game::GetHitboxes(pGameEntity);

				if (g_Entity[pGameEntity->index]->m_bIsPlayer && pCStudioModelRenderer->m_pPlayerInfo)
				{
					if (g_Player[pGameEntity->index]->m_bIsLocal)
					{
						g_Local->m_flGaitYaw = pCStudioModelRenderer->m_pPlayerInfo->gaityaw;
					}
					else
					{
						g_Player[pGameEntity->index]->m_flGaitYaw = pCStudioModelRenderer->m_pPlayerInfo->gaityaw;
					}
				}
			}

			if (!Game::IsTakenScreenshot() && g_pVisuals.get())
			{
				bool bSkipRenderModel = false;

				if (!cvars::visuals.streamer_mode)
				{
					if (g_pVisuals->Glow(pGameEntity))
						bSkipRenderModel = true;
				}

				if (g_pVisuals->Chams(pGameEntity))
					bSkipRenderModel = true;

				if (bSkipRenderModel) 
					return;
			}
		}
	}

	oStudioRenderModel();
}

static void StudioSetUpTransform()
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		cl_entity_t* pGameEntity = pCStudioModelRenderer->m_pCurrentEntity;

		if (pGameEntity && pGameEntity->index >= 1 && pGameEntity->index <= client_state->maxclients)
		{
			if (g_Player[pGameEntity->index]->m_bIsLocal && g_pGlobals->m_bIsInThirdPerson)
			{
				if (pGameEntity->trivial_accept == LOCAL_DESYNC_MARKER && g_pMiscellaneous->m_iChokedCommands < 2)
				{
					pGameEntity->angles[1] = pGameEntity->curstate.angles[1] = g_pGlobals->m_flGaitYaw;
					pGameEntity->curstate.blending[0] = (byte)g_pGlobals->m_flBlendYaw;
				}
			}

			if (!IS_NULLPTR(g_pRageBot))
			{ 
				if ((cvars::ragebot.raim_resolver_pitch == 1 && !g_Player[pGameEntity->index]->m_bIsLocal) || (g_Player[pGameEntity->index]->m_bIsLocal && g_pGlobals->m_bIsInThirdPerson))
				{
					static const float flPitchBreakingPoint = 45.f;

					float flPitch = (g_Player[pGameEntity->index]->m_bIsLocal) ? g_pRageBot->m_QAntiAimbotAngle.x : g_Player[pGameEntity->index]->m_QAngles.x;

					int iBlend = flPitch / -3.f;

					if (iBlend > flPitchBreakingPoint || iBlend < -flPitchBreakingPoint)
					{
						pGameEntity->angles[0] = pGameEntity->curstate.angles[0] = flPitch / -6.f;
						pGameEntity->curstate.blending[1] = iBlend;
					}
					else
					{
						pGameEntity->angles[0] = pGameEntity->curstate.angles[0] = 0.f;
						pGameEntity->curstate.blending[1] = 255 * (flPitchBreakingPoint - iBlend) / (2 * flPitchBreakingPoint);
					}
				}
			}
		}
	}

	oStudioSetUpTransform();
}

static void __declspec(naked) StudioRenderModel_Gate()
{
	_asm
	{
		mov pCStudioModelRenderer, ecx
		jmp StudioRenderModel
	}
}

static void __declspec(naked) StudioSetUpTransform_Gate(int trivial_accept)
{
	_asm
	{
		mov pCStudioModelRenderer, ecx
		jmp StudioSetUpTransform
	}
}

#pragma endregion

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK hkWndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_KEYDOWN)
	{
		if (wParam == VK_INSERT && g_pMenu.get())
			g_pMenu->Toggle();

		if (wParam == VK_END)
			g_pGlobals->m_bIsUnloadingLibrary = true;
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		if (g_pMenu.get() && g_pMenu->IsOpened())
			return false;
	}

	return CallWindowProc(g_pGlobals->m_pWndProc, hWnd, message, wParam, lParam);
}

static void HUD_ProcessPlayerState(entity_state_s* dst, const entity_state_s* src)
{
	g_Client.HUD_ProcessPlayerState(dst, src);

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		if (!g_Player[dst->number]->m_bIsLocal && !g_Player[dst->number]->m_bIsDead)
		{
			if (g_Player[dst->number]->m_iTeamNum != g_Local->m_iTeamNum && g_Player[dst->number]->m_iTeamNum != TEAM_UNASSIGNED && g_Player[dst->number]->m_iTeamNum != TEAM_SPECTATOR)
			{
				dst->solid = SOLID_BBOX;
			}
			else if (g_Player[dst->number]->m_iTeamNum == g_Local->m_iTeamNum)
			{
				if (g_Weapon.IsGun())
				{
					if (cvars::legitbot.active)
					{
						if ((cvars::weapons[g_Weapon->m_iWeaponID].aim_enabled || cvars::weapons[g_Weapon->m_iWeaponID].trigger_enabled) && cvars::legitbot.friendly_fire)
							dst->solid = SOLID_BBOX;
					}
					else if (cvars::ragebot.active)
					{
						if (cvars::weapons[g_Weapon->m_iWeaponID].raim_enabled && cvars::ragebot.friendly_fire)
							dst->solid = SOLID_BBOX;
					}
				}
				else if (g_Weapon.IsKnife())
				{
					if (cvars::misc.kb_enabled && cvars::misc.kb_friendly_fire)
						dst->solid = SOLID_BBOX;
				}
			}
		}
	}
}

static int pfnVGUI2DrawCharacterAdditive(int x, int y, int ch, int r, int g, int b, unsigned int hfont)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected() && cvars::visuals.effects_hud_clear && cvars::visuals.active && !Game::IsTakenScreenshot())
		return TRUE;

	return g_Engine.pfnVGUI2DrawCharacterAdditive(x, y, ch, r, g, b, hfont);
}

static void R_DrawEntitiesOnList()
{
	if (!r_draw_entities_on_list.IsHooked())
		return;

	((R_DrawEntitiesOnList_t)r_draw_entities_on_list.m_OrigFunc)();

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected() && !Game::IsTakenScreenshot() && !cvars::visuals.streamer_mode)
	{
		if (g_pVisuals.get())
			g_pVisuals->CreateEntities();

		if (!g_pGlobals->m_bIsInThirdPerson && !g_Local->m_bIsDead && !IS_NULLPTR(g_pLegitBot) && cvars::legitbot.desync_helper)
		{
			cl_entity_s* pGameEntity = g_Engine.GetLocalPlayer();

			if (pGameEntity)
			{
				player_info_s* pPlayerInfo = g_Studio.PlayerInfo(pGameEntity->curstate.number - 1);

				if (pPlayerInfo)
				{
					const auto backup_gameentity = *pGameEntity;

					pGameEntity->trivial_accept = LOCAL_PLAYER_FOR_GAITYAW;

					CRenderModels::StudioDrawPlayer(pGameEntity);

					*pGameEntity = backup_gameentity;

					g_Local->m_flGaitYaw = pPlayerInfo->gaityaw;
				}
			}
		}
	}
}

static int InitiateGameConnection(void* pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure)
{
	if (!initiate_game_connection.IsHooked())
		return FALSE;

	if (cvars::misc.steamid_spoofer)
	{
		revEmuTicket_t revEmuTicket;

		std::srand(time(0));

		for (size_t i = 0; i < 7; i++) 
			revEmuTicket.hash[i] = RevEmu::HashSymbolTable[std::rand() % sizeof(RevEmu::HashSymbolTable)];

		revEmuTicket.hash[7] = '\0';
		revEmuTicket.version = 'J';
		revEmuTicket.highPartAuthID = RevEmu::Hash((const char*)revEmuTicket.hash) & 0x7FFFFFFF;
		revEmuTicket.signature = 'rev';
		revEmuTicket.secondSignature = 0;
		revEmuTicket.authID = RevEmu::Hash((const char*)revEmuTicket.hash) << 1;
		revEmuTicket.thirdSignature = 0x01100001;
		memcpy(pAuthBlob, &revEmuTicket, sizeof(revEmuTicket));

		return sizeof(revEmuTicket);
	}

	return ((InitiateGameConnection_t)initiate_game_connection.m_OrigFunc)(pAuthBlob, cbMaxAuthBlob, steamIDGameServer, unIPServer, usPortServer, bSecure);
}

void CL_RunUsercmd(local_state_t* from, local_state_t* to, usercmd_t* u, qboolean runfuncs, double* time, unsigned int random_seed)
{
	if (!cl_runusercmd.IsHooked())
		return;

	return ((CL_RunUsercmd_t)cl_runusercmd.m_OrigFunc)(from, to, u, runfuncs, time, random_seed);
}

int NET_SendPacket(netsrc_t sock, size_t length, const void* data, netadr_t to)
{
	if (!net_send_packet.IsHooked())
		return FALSE;

	auto ret = ((NET_SendPacket_t)net_send_packet.m_OrigFunc)(sock, length, data, to);

	return ret;
}

static model_s* SetupPlayerModel(int number)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		static const char t_model[] = "models/player/terror/terror.mdl";
		static const char ct_model[] = "models/player.mdl";

		int index = number + 1;

		if (g_Player[index]->m_bReplaceModel || cvars::misc.replace_models_with_original)
		{
			int modelindex = 0;

			if (g_Player[index]->m_iTeamNum == TEAM_TERRORIST)
				modelindex = g_Engine.pEventAPI->EV_FindModelIndex(t_model);
			else
				modelindex = g_Engine.pEventAPI->EV_FindModelIndex(ct_model);

			if (modelindex)
				return client_state->model_precache[modelindex];
			else
			{
				model_t* model = g_Engine.CL_LoadModel(ct_model, NULL);

				if (model)
					return model;
			}
		}
	}

	return g_Studio.SetupPlayerModel(number);
}

static void pfnFillRGBA(int x, int y, int width, int height, int r, int g, int b, int a)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected() && !Game::IsTakenScreenshot() && g_pVisuals.get())
	{
		const auto from = ImVec2(float(x), float(y));
		const auto to = ImVec2(float(width), float(height));
		const auto color = ImColor(r ? 255: 0, g ? 255 : 0, b ? 255 : 0, (r + g + b + a) / 4);

		g_pRenderer->PushRectFilled(from, from + to, color);
	}

	return g_Engine.pfnFillRGBA(x, y, width, height, r, g, b, a);
}

static void pfnFillRGBABlend(int x, int y, int width, int height, int r, int g, int b, int a)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected() && !Game::IsTakenScreenshot() && g_pVisuals.get())
	{
		const auto from = ImVec2(float(x), float(y));
		const auto to = ImVec2(float(width), float(height));
		const auto color = ImColor(r, g, b, a);

		g_pRenderer->PushRectFilled(from, from + to, color);
	}

	return g_Engine.pfnFillRGBABlend(x, y, width, height, r, g, b, a);
}

static void IN_MouseMove(float frametime, usercmd_t* cmd)
{
	//if (!IS_NULLPTR(g_pMenu) && g_pMenu->IsOpened())
	//	return;

	((IN_MouseMove_t)in_mouse_move.m_OrigFunc)(frametime, cmd);

}

static void __stdcall CalculateMouseVisible()
{
	vmt_surface.UnHook();

	if (!g_pMenu.get() || !g_pMenu->IsOpened())
		g_pISurface->CalculateMouseVisible();

	vmt_surface.ReHook();
}

bool SetupHooks()
{
#if !defined(LICENSING) || defined(DEBUG)
	g_pConsole->DPrintf("> %s: called\n", __FUNCTION__);
#endif
	g_pGlobals->m_pWndProc = (WNDPROC)(SetWindowLong(g_pGlobals->m_hWnd, GWL_WNDPROC, (LONG)hkWndProcHandler));

	if (!g_pGlobals->m_pWndProc)
		return false;
	
	InitClientCvarsMap();
	InitClientUserMsgMap();

	if (vmt_surface.HookTable((DWORD)(g_pISurface)))
		vmt_surface.HookIndex(83, CalculateMouseVisible);

	if (!HookCommands())
		return false;

	if (!HookUserMessages())
		return false;

	if (!HookEngineMessages())
		return false;

	if (!HookOpenGL())
		return false;
	
	g_pClient->HUD_Frame = Initialize;
	g_pClient->HUD_PostRunCmd = HUD_PostRunCmd;
	g_pClient->HUD_Key_Event = HUD_Key_Event;
	g_pClient->CL_CreateMove = CL_CreateMove;
	g_pClient->CL_IsThirdPerson = CL_IsThirdPerson;
	g_pClient->V_CalcRefdef = V_CalcRefdef;
	g_pClient->HUD_ProcessPlayerState = HUD_ProcessPlayerState;
	g_pClient->HUD_TempEntUpdate = HUD_TempEntUpdate;

	g_pEngine->pfnVGUI2DrawCharacterAdditive = pfnVGUI2DrawCharacterAdditive;
	g_pEngine->pfnFillRGBA = pfnFillRGBA;
	g_pEngine->pfnFillRGBABlend = pfnFillRGBABlend;

	g_pStudio->StudioCheckBBox = StudioCheckBBox;
	g_pStudio->SetupPlayerModel = SetupPlayerModel;

	g_pStudioModelRenderer->StudioRenderModel = StudioRenderModel_Gate;
	g_pStudioModelRenderer->StudioSetUpTransform = StudioSetUpTransform_Gate;

	MAKE_HOOK(pres_dynamic_sound, g_pPreS_DynamicSound, PreS_DynamicSound);
	MAKE_HOOK(cl_drift_interpolation_amount, g_pCL_DriftInterpolationAmount, CL_DriftInterpolationAmount);
	MAKE_HOOK(cl_record_hud_command, g_pCL_RecordHUDCommand, CL_RecordHUDCommand);
	MAKE_HOOK(host_filter_time, g_pHost_FilterTime, Host_FilterTime);
	MAKE_HOOK(scr_update_screen, g_pSCR_UpdateScreen, SCR_UpdateScreen);
	MAKE_HOOK(r_draw_entities_on_list, g_pR_DrawEntitiesOnList, R_DrawEntitiesOnList);
	MAKE_HOOK(initiate_game_connection, g_pInitiateGameConnection, InitiateGameConnection);
	MAKE_HOOK(cl_runusercmd, g_pCL_RunUsercmd, CL_RunUsercmd);
	MAKE_HOOK(net_send_packet, g_pNET_SendPacket, NET_SendPacket);
	MAKE_HOOK(in_mouse_move, g_pIN_MouseMove, IN_MouseMove);
	
	g_pGlobals->m_bIsGameHooked = true;

	return true;
}

void DeleteHooks()
{
#if !defined(LICENSING) || defined(DEBUG)
	if (g_pConsole)
		g_pConsole->DPrintf("> %s: called\n", __FUNCTION__);
#endif
	if (g_pGlobals->m_bIsInGame)
		Game::SetRenderFOV(g_Local->m_iFOV);

	vmt_surface.UnHook();

	if (g_pGlobals->m_pWndProc)
		SetWindowLong(g_pGlobals->m_hWnd, GWL_WNDPROC, (LONG)g_pGlobals->m_pWndProc);

	RESET_HOOK(scr_update_screen);
	RESET_HOOK(host_filter_time);
	RESET_HOOK(pres_dynamic_sound);
	RESET_HOOK(cl_drift_interpolation_amount);
	RESET_HOOK(cl_record_hud_command);
	RESET_HOOK(r_draw_entities_on_list);
	RESET_HOOK(initiate_game_connection);
	RESET_HOOK(cl_runusercmd);
	RESET_HOOK(net_send_packet);
	RESET_HOOK(in_mouse_move);

	UnHookCommands();
	UnHookUserMessages();
	UnHookEngineMessages();
	UnHookOpenGL();

	if (g_pStudio)
		*g_pStudio = g_Studio;

	if (g_pClient)
		*g_pClient = g_Client;

	if (g_pEngine)
		*g_pEngine = g_Engine;

	if (g_pStudioModelRenderer)
		*g_pStudioModelRenderer = g_StudioModelRenderer;

	g_pGlobals->m_bIsGameHooked = false;
}