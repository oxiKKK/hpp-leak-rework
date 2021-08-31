#include "main.h"

UserMsg g_pClientUserMsgs = nullptr;

std::map<std::string, pfnUserMsgHook> g_ClientUserMsgsMap;

bool HookUserMessages(void)
{
	if (!g_pClientUserMsgs)
		return false;

	if (!HookUserMsg("ResetHUD", MSG_ResetHUD))
		return false;
	if (!HookUserMsg("BombDrop", MSG_BombDrop))
		return false;
	if (!HookUserMsg("Battery", MSG_Battery))
		return false;
	if (!HookUserMsg("Health", MSG_Health))
		return false;
	if (!HookUserMsg("DeathMsg", MSG_DeathMsg))
		return false;
	if (!HookUserMsg("SetFOV", MSG_SetFOV))
		return false;
	if (!HookUserMsg("TeamInfo", MSG_TeamInfo))
		return false;
	if (!HookUserMsg("MOTD", MSG_MOTD))
		return false;
	if (!HookUserMsg("ScoreAttrib", MSG_ScoreAttrib))
		return false;

	return true;
}

void UnHookUserMessages(void)
{
	if (!g_pClientUserMsgs)
		return;

	UnHookUserMsg("ResetHUD");
	UnHookUserMsg("BombDrop");
	UnHookUserMsg("Battery");
	UnHookUserMsg("Health");
	UnHookUserMsg("DeathMsg");
	UnHookUserMsg("SetFOV");
	UnHookUserMsg("TeamInfo");
	UnHookUserMsg("MOTD");
	UnHookUserMsg("ScoreAttrib");
}

int MSG_ScoreAttrib(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	auto PlayerID = READ_BYTE();
	auto Flags = READ_BYTE();

	if (PlayerID >= 1 && PlayerID <= MAX_CLIENTS)
	{
		const auto pPlayer = g_World.GetPlayer(PlayerID);

		if (pPlayer)
		{
			pPlayer->m_iScoreAttribFlags = Flags;
		}
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_MOTD(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int flag = READ_BYTE();
	char* text = READ_STRING();

	if (cvar.motd_block)
	{
		if (cvar.debug_console)
		{
			g_pConsole->DPrintf(text);

			if (flag == 1)
				g_pConsole->DPrintf("\n");
		}

		return 1;
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_SetFOV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_Local.m_iFOV = READ_BYTE();

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_TeamInfo(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	const auto index = READ_BYTE();
	const auto pczszTeam = READ_STRING();

	const auto pGameEntity = g_Engine.GetLocalPlayer();

	int* iTeamPtr = nullptr;

	if (pGameEntity && pGameEntity->model && pGameEntity->index)
	{
		if (pGameEntity->index == index)
			iTeamPtr = &g_Local.m_iTeam;
		else if (index >= 1 && index <= MAX_CLIENTS)
			iTeamPtr = &g_World.GetPlayer(index)->m_iTeam;

		if (iTeamPtr)
		{
			if (!_stricmp(pczszTeam, "SPECTATOR"))
				*iTeamPtr = SPECTATOR;
			if (!_stricmp(pczszTeam, "TERRORIST"))
				*iTeamPtr = TERRORIST;
			else if (!_stricmp(pczszTeam, "CT"))
				*iTeamPtr = CT;
			else
				*iTeamPtr = UNASSIGNED;
		}
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_BombDrop(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_World.m_vecBombOrigin.x = READ_COORD();
	g_World.m_vecBombOrigin.y = READ_COORD();
	g_World.m_vecBombOrigin.z = READ_COORD();

	g_World.m_iBombFlag = READ_BYTE();

	if (g_World.m_iBombFlag == BOMB_FLAG_PLANTED)
		g_World.m_dbBombPlantedTime = client_state->time;

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_ResetHUD(const char* pszName, int iSize, void* pbuf)
{
	const auto pPlayer = g_World.GetPlayer();

	for (auto i = 0; i < MAX_CLIENTS - 1; ++i)
	{
		pPlayer[i].m_iArmorType = ARMOR_NONE;
		pPlayer[i].m_iHealth = 100;
	}

	g_World.m_iBombFlag = BOMB_FLAG_DROPPED;
	g_World.m_vecBombOrigin.Clear();

	if (cvar.maximize_window_in_new_round)
		SendMessageA(g_Globals.m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_Battery(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_Local.m_iArmor = READ_BYTE();

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_Health(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_Local.m_iHealth = READ_BYTE();

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_DeathMsg(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	const auto KillerID = READ_BYTE();
	const auto VictimID = READ_BYTE();
	const auto IsHeadshot = READ_BYTE();
	const char* pczszTruncatedWeaponName = READ_STRING();

	if (KillerID != VictimID && KillerID == g_Local.m_iIndex)
		g_Aimbot.m_dbLastTimeKilled = client_state->time;

	if (KillerID == g_Local.m_iIndex)
	{
		if (KillerID == VictimID)
		{
			const char* pszVictim = (VictimID == g_Local.m_iIndex)
				? g_Engine.pfnGetCvarString("name")
				: g_World.GetPlayer(VictimID)->m_szPrintName;

			g_ScreenLog.Log(ImColor(200, 255, 255, 255), "%s died", pszVictim);
		}
		else
		{
			const char* pszKiller = (KillerID == g_Local.m_iIndex)
				? g_Engine.pfnGetCvarString("name")
				: g_World.GetPlayer(KillerID)->m_szPrintName;

			const char* pszVictim = (VictimID == g_Local.m_iIndex)
				? g_Engine.pfnGetCvarString("name")
				: g_World.GetPlayer(VictimID)->m_szPrintName;

			g_ScreenLog.Log(ImColor(200, 255, 255, 255), IsHeadshot ? "%s killed %s with a headshot from %s" : "%s killed %s from %s", pszKiller, pszVictim, pczszTruncatedWeaponName);
		}
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

bool HookUserMsg(const char *pszMsgName, pfnUserMsgHook pfn)
{
	auto pClientUserMsgs = g_pClientUserMsgs;
	while (pClientUserMsgs)
	{
		if (!strcmp(pClientUserMsgs->szName, pszMsgName))
		{
			g_ClientUserMsgsMap[pszMsgName] = pClientUserMsgs->pfn;
			pClientUserMsgs->pfn = pfn;
			return true;
		}

		pClientUserMsgs = pClientUserMsgs->next;
	}

	TraceLog("> %s: failed to hook %s.\n", __FUNCTION__, pszMsgName);

	return false;
}

bool UnHookUserMsg(const char *pszMsgName)
{
	auto pClientUserMsgs = g_pClientUserMsgs;
	while (pClientUserMsgs)
	{
		if (!strcmp(pClientUserMsgs->szName, pszMsgName) && g_ClientUserMsgsMap[pszMsgName] != nullptr)
		{
			pClientUserMsgs->pfn = g_ClientUserMsgsMap[pszMsgName];
			g_ClientUserMsgsMap[pszMsgName] = nullptr;
			return true;
		}

		pClientUserMsgs = pClientUserMsgs->next;
	}

	TraceLog("> %s: failed to unhook %s.\n", __FUNCTION__, pszMsgName);

	return false;
}