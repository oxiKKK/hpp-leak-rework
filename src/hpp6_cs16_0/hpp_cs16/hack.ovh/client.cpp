#include "main.h"

CHookData HookDataPreS_DynamicSound;
CHookData HookDataR_DrawEntitiesOnList;

cl_clientfunc_t g_Client, *g_pClient = nullptr;
cl_enginefunc_t g_Engine, *g_pEngine = nullptr;
engine_studio_api_t g_Studio, *g_pStudio = nullptr;
StudioModelRenderer_t g_StudioModelRenderer, *g_pStudioModelRenderer = nullptr;
IRunGameEngine* g_pIRunGameEngine = nullptr;
IGameConsole* g_pConsole = nullptr;
vgui::ISurface* g_pISurface = nullptr;
IGameUI* g_pGameUI = nullptr;
playermove_t* pmove = nullptr;
R_DrawEntitiesOnList_t g_pR_DrawEntitiesOnList = nullptr;
PreS_DynamicSound_t g_pPreS_DynamicSound = nullptr;
StudioModelRenderer_d pCStudioModelRenderer = nullptr;
client_state_t *client_state = nullptr;
client_static_t *client_static = nullptr;
extra_player_info_t *g_PlayerExtraInfo = nullptr;

CGlobalsVars g_Globals;
SCREENINFO g_Screen;
CCvars cvar;
net_status_s g_Status;

void ResolveSoundIndex(int a_iIndex, Vector a_vecSoundOrigin, int& a_iResolvedIndex)
{
	if (a_iResolvedIndex > 0 && a_iResolvedIndex <= MAX_CLIENTS)
		return;

	if (g_SoundResolver.size() > 0)
	{
		for (auto& resolver : g_SoundResolver)
		{
			if (resolver.m_iOriginalIndex == a_iIndex)
			{
				a_iResolvedIndex = resolver.m_iResolvedIndex;
				break;
			}
		}
	}

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		const auto pPlayer = &g_Player.at(i - 1);

		if (pPlayer == nullptr)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS)
			continue;

		if (a_vecSoundOrigin.Distance(pPlayer->m_vecOrigin) < 64)
		{
			if (g_SoundResolver.size() > 0)
			{
				for (auto j = 0; j < g_SoundResolver.size(); ++j)
				{
					if (g_SoundResolver[j].m_iOriginalIndex == a_iIndex)
					{
						g_SoundResolver[j].m_iResolvedIndex = i;
						a_iResolvedIndex = g_SoundResolver[j].m_iResolvedIndex;
						return;
					}
				}
			}

			CSoundResolver new_sound;
			new_sound.m_iOriginalIndex = a_iIndex;
			new_sound.m_iResolvedIndex = i;
			g_SoundResolver.push_back(new_sound);
			a_iResolvedIndex = new_sound.m_iResolvedIndex;
			return;
		}
	}
}

bool IsNotFakeSounds(int index, DWORD channel)
{
	const auto* pLocal = g_pEngine->GetLocalPlayer();

	if (pLocal == nullptr)
		return false;

	if (index == 0 || index == -1)
		return false;

	if (index == pLocal->index)
		return false;

	if (channel == 0)
		return false;

	if (g_Local.m_bIsDead && pLocal->curstate.iuser1 == OBS_IN_EYE && pLocal->curstate.iuser2 == index)
		return false;

	return true;
}

void PreS_DynamicSound_Hooked(int entid, DWORD entchannel, char* szSoundFile, float* pflOrigin, float flVolume, float flAttenuation, int iTimeOff, int iPitch)
{
	if (g_pIRunGameEngine->IsInGame())
	{
		if (IsNotFakeSounds(entid, entchannel))
		{
			const bool playerSounds = strstr(szSoundFile, "player") || strstr(szSoundFile, "weapons");

			if (playerSounds)
			{
				ResolveSoundIndex(entid, pflOrigin, entid);

				if (entid > 0 && entid <= MAX_CLIENTS)
				{
					auto* pPlayer = &g_Player.at(entid - 1);

					if (pPlayer)
					{
						if (!pPlayer->m_bIsInPVS && !strstr(szSoundFile, "c4_beep") && !strstr(szSoundFile, "flashbang") && !strstr(szSoundFile, "he_bounce") && !strstr(szSoundFile, "grenade_hit") && !strstr(szSoundFile, "sg_explode") && !strstr(szSoundFile, "debris"))
						{
							pPlayer->m_dwHistory = GetTickCount();
							pPlayer->m_vecOrigin = pflOrigin;
						}

						if (strstr(szSoundFile, "bhit_helmet"))
						{
							pPlayer->m_iArmorType = ARMOR_VESTHELM;
							pPlayer->m_iHealth -= 80;
						}
						else if (strstr(szSoundFile, "bhit_kevlar"))
						{
							pPlayer->m_iArmorType = ARMOR_KEVLAR;
							pPlayer->m_iHealth -= 20;
						}
						else if (strstr(szSoundFile, "bhit_flesh"))
						{
							pPlayer->m_iArmorType = ARMOR_NONE;
							pPlayer->m_iHealth -= 30;
						}
						else if (strstr(szSoundFile, "headshot"))
						{
							pPlayer->m_iHealth -= 80;
						}
						else if (strstr(szSoundFile, "die") || strstr(szSoundFile, "death"))
						{
							pPlayer->m_iHealth = 0;
							pPlayer->m_iArmorType = ARMOR_NONE;
						}
					}
				}
			}

			my_sound_t sound = { pflOrigin, szSoundFile, GetTickCount(), entid };
			g_Sounds.push_back(sound);
		}
	}

	((PreS_DynamicSound_t)HookDataPreS_DynamicSound.origFunc)(entid, entchannel, szSoundFile, pflOrigin, flVolume, flAttenuation, iTimeOff, iPitch);
}

void R_DrawEntitiesOnList_Hooked()
{
	int cl_numvisedicts = *g_Globals.cl_numvisedicts;
	/*if (g_Globals.m_bDrawEntitiesOriginal)
	{
		((R_DrawEntitiesOnList_t)HookDataR_DrawEntitiesOnList.origFunc)();
		return;
	}

	int cl_numvisedicts = *g_Globals.cl_numvisedicts;

	for (int i = 0; i < cl_numvisedicts; i++)
	{
		if (g_Globals.cl_visedicts[i])
		{
			g_Globals.cl_visedicts[i]->origin.z += 32;
			g_Globals.cl_visedicts[i]->curstate.origin.z += 32;
		}
	}*/

	

	/*if (cl_numvisedicts + 1 < MAX_PACKET_ENTITIES && !g_Globals.m_bUnloading)
	{
		if (!g_Globals.send_command)
		{
			cl_entity_t pPlayer = *g_Engine.GetLocalPlayer();

			pPlayer.angles = pPlayer.curstate.angles = g_Misc.m_vRealAngles;

			/*pCustomPlayer->angles = g_Misc.m_vRealAngles;
			pCustomPlayer->angles.x /= 9;
			//Fix rotation
			pCustomPlayer->curstate = {};
			pCustomPlayer->curstate.angles = pCustomPlayer->angles;
			pCustomPlayer->curstate.sequence = 1;
			pCustomPlayer->curstate.health = 1337;//ghost local player mark*/

		/*	g_Globals.cl_visedicts[cl_numvisedicts] = &pPlayer;

			*g_Globals.cl_numvisedicts = cl_numvisedicts + 1;

			cl_numvisedicts++;
		}
	}*/

	((R_DrawEntitiesOnList_t)HookDataR_DrawEntitiesOnList.origFunc)();
}

int StudioCheckBBox() { return 1; }

void V_CalcRefdef(ref_params_s* pparams)
{
	g_Local.m_vecForward = pparams->forward;
	g_Local.m_vecRight = pparams->right;
	g_Local.m_vecUp = pparams->up;

	if (cvar.remove_visual_recoil && g_Weapon.IsCurrentWeaponGun())
		pparams->punchangle.Clear();

	g_Client.V_CalcRefdef(pparams);

	g_Misc.ThirdPerson(pparams);
}

int CL_IsThirdPerson(void)
{
	if (cvar.thirdperson && !g_Local.m_bIsDead && cvar.visuals && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
		return 1;

	return g_Client.CL_IsThirdPerson();
}

void HUD_Frame(double time)
{
	static bool bReset = false;

	if (!bReset && client_static->state != ca_active)
	{
		g_World.Clear();
		g_SoundResolver.clear();
		bReset = true;
	}
	else if (bReset && client_static->state == ca_active)
		bReset = false;

	g_Engine.pfnGetScreenInfo(&g_Screen);
	g_Engine.pNetAPI->Status(&g_Status);

	g_Visuals.Brightness();

	g_Client.HUD_Frame(time);
}

void HUD_Frame_init(double time)
{
	g_Screen.iSize = sizeof(SCREENINFO);
	g_Engine.pfnGetScreenInfo(&g_Screen);
	g_Engine.pNetAPI->Status(&g_Status);

	g_Settings.Load("Default");

	if (!g_pConsole->IsConsoleVisible())
		g_pConsole->Activate();

	g_pConsole->DPrintf("Cheat injected :3\n");
	g_pConsole->DPrintf("Site: hpp.ovh\n");
	g_pConsole->DPrintf("Build: %s\n", __DATE__);

	g_World.Clear();
	g_SoundResolver.clear();

	g_Local.m_iFOV = DEFAULT_FOV;

	g_pClient->HUD_Frame = HUD_Frame;
	g_Client.HUD_Frame(time);
}

void CL_CreateMove(float frametime, usercmd_s* cmd, int active)
{
	g_Client.CL_CreateMove(frametime, cmd, active);

	g_Globals.send_command = true;
	g_Globals.backtrack = 0;

	g_World.Update(frametime, cmd);

	if (!g_Local.m_bIsDead)
	{
		g_Weapon.SimulatePostFrame(cmd);

		g_Aimbot.Trigger(cmd);

		g_Aimbot.Run(cmd);

		g_Misc.AutoReload(cmd);
		g_Misc.AutoPistol(cmd);

		g_Weapon.ItemPostFrame(cmd); // do weapon stuff

		if (cvar.ragebot_active && cvar.ragebot_aim_enabled && cvar.ragebot_remove_spread && cmd->buttons & IN_ATTACK && g_Weapon.CanAttack())
		{
			QAngle QAngles(cmd->viewangles), QNewAngles;
			g_NoSpread.GetSpreadOffset(g_Weapon.m_iRandomSeed, 1, QAngles, QNewAngles, cvar.ragebot_remove_spread);
			cmd->viewangles = QNewAngles;
		}

		g_Misc.FakeLag(cmd);
		g_Misc.AntiAim(cmd);

		g_KnifeBot.Run(cmd);

		g_Kreedz.Run(cmd);
	}

	g_Utils.SendCommand(g_Globals.send_command);
	g_Misc.AddFakeLatency(g_Globals.backtrack);
}

int HUD_Key_Event(int down, int keynum, const char* pszCurrentBinding)
{
	if (down != 0)
	{
		if (cvar.legitbot_trigger_key && cvar.legitbot_trigger_key == keynum)
		{
			g_Aimbot.m_bTriggerStatus = !g_Aimbot.m_bTriggerStatus;
			return 0;
		}
	}

	return g_Client.HUD_Key_Event(down, keynum, pszCurrentBinding);
}

void HUD_PostRunCmd(local_state_s* from, local_state_s* to, usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_Client.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);

	g_Weapon.ItemPreFrame(to, cmd, runfuncs, time, random_seed);
}

void StudioSetRemapColors(int top, int bottom)
{
	auto pGameEntity = g_Studio.GetCurrentEntity();

	if(pGameEntity)
		g_Utils.GetHitboxes(pGameEntity);

	g_Studio.StudioSetRemapColors(top, bottom);
}

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

void StudioRenderModel()
{
	auto pGameEntity = g_Studio.GetCurrentEntity();

	if (pGameEntity && pGameEntity->index > 0 && pGameEntity->index <= MAX_CLIENTS && pGameEntity->index != g_Local.m_iIndex)
	{
		auto pPlayer = &g_Player[pGameEntity->index - 1];

		if (pPlayer)
		{
			if (pPlayer->m_iTeam == UNASSIGNED && g_PlayerExtraInfo[pGameEntity->index].teamnumber)
				pPlayer->m_iTeam = gsl::narrow_cast<int>(g_PlayerExtraInfo[pGameEntity->index].teamnumber);
		}
	}

	if (!g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
		g_Visuals.ColoredModels();
	else
		oStudioRenderModel();
}

void __declspec(naked) StudioRenderModel_Gate()
{
	_asm
	{
		mov pCStudioModelRenderer, ecx
		jmp StudioRenderModel
	}
}

int HUD_GetHullBounds(int hullnum, float* /*mins*/, float* maxs)
{
	if (hullnum == 1)
		maxs[2] = 32.f; //https://gist.github.com/WPMGPRoSToTeMa/2e6e0454654f9e4ca22ee3e987051b57
	return 1;
}

void HUD_PlayerMoveInit(playermove_s* ppmove)
{
	PM_InitTextureTypes(ppmove);

	return g_Client.HUD_PlayerMoveInit(ppmove);
}

int pfnDrawUnicodeCharacter(int x, int y, short number, int r, int g, int b, unsigned long hfont)
{
	if (cvar.hud_clear && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
		return 1;

	return g_Engine.pfnDrawUnicodeCharacter(x, y, number, r, g, b, hfont);
}

bool HookEngine(void)
{
	g_pStudio->StudioCheckBBox = StudioCheckBBox;
	g_pStudio->StudioSetRemapColors = StudioSetRemapColors;

	g_pClient->V_CalcRefdef = V_CalcRefdef;
	g_pClient->HUD_Frame = HUD_Frame_init;
	g_pClient->HUD_PlayerMoveInit = HUD_PlayerMoveInit;
	g_pClient->CL_CreateMove = CL_CreateMove;
	g_pClient->HUD_PostRunCmd = HUD_PostRunCmd;
	g_pClient->CL_IsThirdPerson = CL_IsThirdPerson;
	g_pClient->HUD_Key_Event = HUD_Key_Event;
	g_pClient->HUD_GetHullBounds = HUD_GetHullBounds;

	g_pEngine->pfnDrawUnicodeCharacter = pfnDrawUnicodeCharacter;

	g_pStudioModelRenderer->StudioRenderModel = StudioRenderModel_Gate;

	if (g_pPreS_DynamicSound && g_pR_DrawEntitiesOnList)
	{
		HookDataPreS_DynamicSound.Hook((FARPROC)g_pPreS_DynamicSound, (FARPROC)PreS_DynamicSound_Hooked);
		HookDataR_DrawEntitiesOnList.Hook((FARPROC)g_pR_DrawEntitiesOnList, (FARPROC)R_DrawEntitiesOnList_Hooked);

		HookDataPreS_DynamicSound.Rehook();
		HookDataR_DrawEntitiesOnList.Rehook();
	}
	else
		return false;

	if (!HookOpenGL())
		return false;

	//HookEngineMessages();
	HookUserMessages();
	HookCommands();

	return true;
}

void UnHooks(bool a_bFreeLibrary)
{
	g_Globals.m_bUnloading = true;

	Sleep(500);

	g_pConsole->DPrintf("UnHooking!\n");

	g_World.Clear();

	ImGui_ImplSdlGL2_Shutdown();
	SetWindowLongA(g_hWnd, GWL_WNDPROC, (LRESULT)g_WndProc_o);

	HookDataPreS_DynamicSound.Unhook();
	HookDataR_DrawEntitiesOnList.Unhook();

	*g_pStudio = g_Studio;
	*g_pClient = g_Client;
	*g_pEngine = g_Engine;
	*g_pStudioModelRenderer = g_StudioModelRenderer;

	g_Utils.SendCommand(true);
	g_Misc.AddFakeLatency(0);

	//UnHookEngineMessages();
	UnHookUserMessages();
	UnHookCommands();
	UnHookOpenGL();

	if (a_bFreeLibrary)
	{
		TraceLog("> %s: FreeLibraryAndExitThread.\n", __FUNCTION__);
		Sleep(2500);
		FreeLibraryAndExitThread(g_Globals.m_hModule, 0);
	}
}