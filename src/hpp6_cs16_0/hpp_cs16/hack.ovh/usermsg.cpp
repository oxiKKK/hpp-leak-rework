#include "main.h"

UserMsg g_pClientUserMsgs = nullptr;

std::map<std::string, pfnUserMsgHook> g_ClientUserMsgsMap;

void HookUserMessages()
{
	HookUserMsg("ResetHUD", MSG_ResetHUD);
	HookUserMsg("BombDrop", MSG_BombDrop);
	HookUserMsg("Battery", MSG_Battery);
	HookUserMsg("Health", MSG_Health);
	HookUserMsg("DeathMsg", MSG_DeathMsg);
	HookUserMsg("SetFOV", MSG_SetFOV);
	HookUserMsg("TeamInfo", MSG_TeamInfo);
}

void UnHookUserMessages()
{
	UnHookUserMsg("ResetHUD");
	UnHookUserMsg("BombDrop");
	UnHookUserMsg("Battery");
	UnHookUserMsg("Health");
	UnHookUserMsg("DeathMsg");
	UnHookUserMsg("SetFOV");
	UnHookUserMsg("TeamInfo");
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

	const int iIndex = READ_BYTE();
	const char* pcszTeam = READ_STRING();

	int *piTeam = nullptr;

	const auto pGameEntity = g_Engine.GetLocalPlayer();

	if (pGameEntity && iIndex == pGameEntity->index)
		piTeam = &g_Local.m_iTeam;
	else if (iIndex > 0 && iIndex <= MAX_CLIENTS)
		piTeam = &g_Player[iIndex - 1].m_iTeam;

	if (piTeam)
	{
		if (!_stricmp(pcszTeam, "SPECTATOR"))
			*piTeam = SPECTATOR;
		if (!_stricmp(pcszTeam, "TERRORIST"))
			*piTeam = TERRORIST;
		else if (!_stricmp(pcszTeam, "CT"))
			*piTeam = CT;
		else
			*piTeam = UNASSIGNED;
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_BombDrop(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_World.m_Bomb.m_vOrigin.x = READ_COORD();
	g_World.m_Bomb.m_vOrigin.y = READ_COORD();
	g_World.m_Bomb.m_vOrigin.z = READ_COORD();

	g_World.m_Bomb.m_iFlag = READ_BYTE();

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int MSG_ResetHUD(const char* pszName, int iSize, void* pbuf)
{
	g_World.m_Bomb.m_iFlag = BOMB_FLAG_DROPPED;

	for (auto i = 1; i <= g_Engine.GetMaxClients(); ++i)
	{
		auto *pPlayer = &g_Player[i - 1];

		if (!pPlayer)
			continue;

		if (pPlayer->m_iIndex == g_Local.m_iIndex)
			continue;

		pPlayer->m_iArmorType = ARMOR_NONE;
		pPlayer->m_iHealth = 100;
	}

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

	int KillerID = READ_BYTE();
	int VictimID = READ_BYTE();
	int IsHeadshot = READ_BYTE();
	char* TruncatedWeaponName = READ_STRING();

	if (KillerID != VictimID && KillerID == g_Local.m_iIndex)
		g_Aimbot.m_flLastTimeKilled = GetTickCount();

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
		if (!strcmp(pClientUserMsgs->szName, pszMsgName))
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