#include "framework.h"

hud_player_info_t CPlayerInfoList::GetPlayerInfo(int index)
{
	hud_player_info_t info;
	g_Engine.pfnGetPlayerInfo(index, &info);
	return info;
}

char* CPlayerInfoList::m_pszName(int index)
{
	return GetPlayerInfo(index).name;
}

short CPlayerInfoList::m_nPing(int index)
{
	return GetPlayerInfo(index).ping;
}

byte CPlayerInfoList::m_nPacketLoss(int index)
{
	return GetPlayerInfo(index).packetloss;
}

uint64 CPlayerInfoList::m_nSteamID(int index)
{
	return GetPlayerInfo(index).m_nSteamID;
}

CPlayerInfoList g_PlayerInfoList;