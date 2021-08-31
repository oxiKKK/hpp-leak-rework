#include "framework.h"

bool CPlayerExtraInfo::GetAddress(PVOID address)
{
	m_ptr = reinterpret_cast<uintptr_t>(address);
	return static_cast<bool>(m_ptr);
}

short CPlayerExtraInfo::m_nFrags(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].frags 
		: ((extra_player_info_old_t*)m_ptr)[index].frags;
}

short CPlayerExtraInfo::m_nDeaths(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].deaths 
		: ((extra_player_info_old_t*)m_ptr)[index].deaths;
}

int CPlayerExtraInfo::m_bHasC4(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].has_c4 
		: ((extra_player_info_old_t*)m_ptr)[index].has_c4;
}

int CPlayerExtraInfo::m_bVip(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].vip 
		: ((extra_player_info_old_t*)m_ptr)[index].vip;
}

Vector CPlayerExtraInfo::m_vecOrigin(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].origin 
		: ((extra_player_info_old_t*)m_ptr)[index].origin;
}

short CPlayerExtraInfo::m_PlayerClass(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].playerclass 
		: ((extra_player_info_old_t*)m_ptr)[index].playerclass;
}

short CPlayerExtraInfo::m_TeamNum(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].teamnumber 
		: ((extra_player_info_old_t*)m_ptr)[index].teamnumber;
}

char* CPlayerExtraInfo::m_pszTeamName(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].teamname 
		: ((extra_player_info_old_t*)m_ptr)[index].teamname;
}

bool CPlayerExtraInfo::m_bDead(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].dead 
		: ((extra_player_info_old_t*)m_ptr)[index].dead;
}

int CPlayerExtraInfo::m_nHealth(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].health 
		: -1;
}

int CPlayerExtraInfo::m_nMoney(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].money 
		: -1;
}

int CPlayerExtraInfo::m_bHasDefusalKits(int index)
{
	return g_pGlobals->m_iGameBuild >= 8212 
		? ((extra_player_info_t*)m_ptr)[index].has_defusekits 
		: -1;
}

CPlayerExtraInfo g_PlayerExtraInfo;