#include "framework.h"

std::map<std::string, pfnUserMsgHook> g_ClientUserMsgsMap;

void InitClientUserMsgMap()
{
	PClientUserMsg pClientUserMsgs = g_pClientUserMsgs;

	while (pClientUserMsgs)
	{
		g_ClientUserMsgsMap[pClientUserMsgs->name] = pClientUserMsgs->pfn;
		pClientUserMsgs = pClientUserMsgs->next;
	}
}

static bool HookUserMsg(const std::string& name, const pfnUserMsgHook& pfn)
{
	PClientUserMsg pClientUserMsgs = g_pClientUserMsgs;

	while (pClientUserMsgs)
	{
		if (!name.compare(pClientUserMsgs->name))
		{		
			pClientUserMsgs->pfn = pfn;
			return true;
		}

		pClientUserMsgs = pClientUserMsgs->next;
	}

	Utils::TraceLog(V("> %s: failed to hook %s.\n"), V(__FUNCTION__), name.c_str());

	return false;
}

static bool UnHookUserMsg(const std::string& name)
{
	PClientUserMsg pClientUserMsgs = g_pClientUserMsgs;

	while (pClientUserMsgs)
	{
		if (!name.compare(pClientUserMsgs->name))
		{
			pClientUserMsgs->pfn = g_ClientUserMsgsMap[name];
			return true;
		}

		pClientUserMsgs = pClientUserMsgs->next;
	}

	Utils::TraceLog(V("> %s: failed to unhook %s.\n"), V(__FUNCTION__), name.c_str());

	return false;
}

static int MSG_MOTD(const char* pszName, int iSize, void* pbuf)
{
	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_TeamInfo(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		const int index = reader.ReadByte();

		if (index > 0 && index <= client_state->maxclients && !g_Player[index]->m_bIsLocal)
			g_Player[index]->m_iTeamNum = Game::GetTeamNum(reader.ReadString());
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_BombDrop(const char* pszName, int iSize, void* pbuf)
{
	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_TextMsg(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		reader.ReadByte();

		if (strstr(reader.ReadString(), "#Bomb_Planted"))
		{
			for (int i = 1; i <= client_state->maxclients; i++)
			{
				if (g_Player[i]->m_bIsLocal)
					continue;

				if (g_Player[i]->m_bIsInPVS)
					continue;

				if (strstr(g_Player[i]->m_szWeaponModelName, "c4"))
					memset(g_Player[i]->m_szWeaponModelName, 0, MAX_MODEL_NAME);

				g_Player[i]->m_bHasC4 = false;
			}
		}
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_ResetHUD(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		g_HitRegister->m_iHits = 0;
		g_HitRegister->m_iMisses = 0;

		for (int i = 1; i <= client_state->maxclients; i++)
		{
			if (g_Player[i]->m_bIsLocal)
				continue;

			g_Player[i]->m_iHealth = Game::GetRespawnHealth();
			g_Player[i]->m_bHasC4 = false;		
			g_Player[i]->m_iMoney = 0;
			g_Player[i]->m_iSequence = SEQUENCE_IDLE;
			g_Player[i]->m_iSequenceFrame = 0;

			if (g_Player[i]->m_bIsInPVS)
				continue;

			memset(g_Player[i]->m_szWeaponModelName, 0, MAX_MODEL_NAME);
		}
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_Battery(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		g_Local->m_iArmor = reader.ReadShort();
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_ArmorType(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		g_Local->m_iArmorType = reader.ReadByte() + 1;
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_Health(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		g_Local->m_iHealth = reader.ReadByte();
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_DeathMsg(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		const int killer = reader.ReadByte();
		const int victim = reader.ReadByte();

		if (killer > 0 && killer <= MAX_CLIENTS && victim > 0 && victim <= MAX_CLIENTS)
		{
			if (!g_Player[victim]->m_bIsLocal)
			{
				g_Player[victim]->m_iArmorType = ARMOR_NONE;
				g_Player[victim]->m_iHealth = 0;
				g_Player[victim]->m_iSequence = 0;
				g_Player[victim]->m_bHasC4 = false;
				g_Player[victim]->m_bHasDefusalKits = false;
			}

			if (killer != victim)
			{
				if (g_Player[killer]->m_bIsLocal)
				{
					g_Local->m_flLastKillTime = static_cast<float>(client_state->time);
				}
				else
				{
					g_Player[killer]->m_flLastKillTime = static_cast<float>(client_state->time);
				}
			}
		}
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_RoundTime(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		g_pGlobals->m_flRoundTime = static_cast<float>(reader.ReadShort());
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_Radar(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		const int index = reader.ReadByte();

		if (index > 0 && index <= client_state->maxclients && !g_Player[index]->m_bIsLocal && !g_Player[index]->m_bIsInPVS)
		{
			Vector origin;

			origin.x = reader.ReadCoord();
			origin.y = reader.ReadCoord();
			origin.z = reader.ReadCoord();

			const double timestamp = client_state->time;

			if (timestamp - g_Player[index]->m_flHistory > 0.5)
			{
				g_Player[index]->m_flHistory = static_cast<float>(timestamp);
				g_Player[index]->m_flDistance = g_Local->m_vecOrigin.Distance(origin);
				g_Player[index]->m_vecPrevOrigin = g_Player[index]->m_vecOrigin;
				g_Player[index]->m_vecOrigin = origin;
				g_Player[index]->m_bIsDead = false;

				if (g_Player[index]->m_iHealth <= 0)
					g_Player[index]->m_iHealth = Game::GetRespawnHealth();
			}
		}
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

static int MSG_SetFOV(const char* pszName, int iSize, void* pbuf)
{
	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		BufferReader reader(pszName, pbuf, iSize);

		g_Local->m_iFOV = reader.ReadByte();
		g_Local->m_iFOV = g_Local->m_iFOV ? min(g_Local->m_iFOV, DEFAULT_FOV) : DEFAULT_FOV;
		g_Local->m_bIsScoped = g_Local->m_iFOV < 55;
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

bool HookUserMessages()
{
	if (!g_pClientUserMsgs)
		return false;

	if (!HookUserMsg("TextMsg", MSG_TextMsg))
		return false;

	if (!HookUserMsg("ResetHUD", MSG_ResetHUD))
		return false;

	if (!HookUserMsg("BombDrop", MSG_BombDrop))
		return false;

	if (!HookUserMsg("Battery", MSG_Battery))
		return false;

	if (!HookUserMsg("ArmorType", MSG_ArmorType))
		return false;

	if (!HookUserMsg("Health", MSG_Health))
		return false;

	if (!HookUserMsg("DeathMsg", MSG_DeathMsg))
		return false;

	if (!HookUserMsg("TeamInfo", MSG_TeamInfo))
		return false;

	if (!HookUserMsg("MOTD", MSG_MOTD))
		return false;

	if (!HookUserMsg("RoundTime", MSG_RoundTime))
		return false;

	if (!HookUserMsg("Radar", MSG_Radar))
		return false;

	if (!HookUserMsg("SetFOV", MSG_SetFOV))
		return false;

	return true;
}

void UnHookUserMessages()
{
	if (g_pClientUserMsgs)
	{
		UnHookUserMsg("TextMsg");
		UnHookUserMsg("ResetHUD");
		UnHookUserMsg("BombDrop");
		UnHookUserMsg("Battery");
		UnHookUserMsg("ArmorType");
		UnHookUserMsg("Health");
		UnHookUserMsg("DeathMsg");
		UnHookUserMsg("TeamInfo");
		UnHookUserMsg("MOTD");
		UnHookUserMsg("RoundTime");
		UnHookUserMsg("Radar");
		UnHookUserMsg("SetFOV");
	}
}