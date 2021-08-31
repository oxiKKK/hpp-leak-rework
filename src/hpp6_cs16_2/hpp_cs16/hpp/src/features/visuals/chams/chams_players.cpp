#include "framework.h"

bool CChamsPlayers::Draw(cl_entity_s* pGameEntity)
{
	if (!IsPlayer(pGameEntity->index))
		return false;

	if (!cvars::visuals.colored_models_paint_players_weapons && IsWeaponModelRender(pGameEntity))
		return false;

	SetupRenderer(pGameEntity);
	DrawPlayer(pGameEntity->index);	
	RestoreRenderer(pGameEntity);

	return true;
}

bool CChamsPlayers::IsPlayer(int index)
{
	if (!g_Entity[index]->m_bIsPlayer)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	if (g_Player[index]->m_bIsLocal)
	{
		if (!cvars::visuals.colored_models_players_players[0] || !g_pGlobals->m_bIsInThirdPerson)
			return false;
	}
	else
	{
		if (!cvars::visuals.colored_models_players_players[1] && g_Player[index]->m_iTeamNum != g_Local->m_iTeamNum)
			return false;

		if (!cvars::visuals.colored_models_players_players[2] && g_Player[index]->m_iTeamNum == g_Local->m_iTeamNum)
			return false;
	}

	if (!g_Player[index]->m_bIsInPVS)
	{
		if (client_state->time - g_Player[index]->m_flLastTimeInPVS > cvars::visuals.colored_models_dormant_time)
			return false;
	}

	if (g_Player[index]->m_bIsDead && !cvars::visuals.colored_models_players_on_the_dead)
		return false;

	return true;
}

void CChamsPlayers::CreatePlayers()
{
	for (int i = 1; i <= client_state->maxclients; i++)
	{
		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!Game::IsValidEntity(pGameEntity))
			continue;

		if (!IsPlayer(pGameEntity->index))
			continue;

		if (!g_Player[pGameEntity->index]->m_bIsInPVS)
			continue;

		player_info_s* pPlayerInfo = g_Studio.PlayerInfo(pGameEntity->curstate.number - 1);

		if (!pPlayerInfo)
			continue;

		const auto backup_gameentity = *pGameEntity;
		const auto backup_playerinfo = *pPlayerInfo;

		pGameEntity->trivial_accept = PLAYER_MARKER;

		StudioDrawPlayer(pGameEntity);

		*pGameEntity = backup_gameentity;
		*pPlayerInfo = backup_playerinfo;
	}
}

void CChamsPlayers::CreateDormantPlayers()
{
	for (int i = 1; i <= client_state->maxclients; i++)
	{
		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!Game::IsValidEntity(pGameEntity))
			continue;

		if (!IsPlayer(pGameEntity->index))
			continue;

		if (g_Player[pGameEntity->index]->m_bIsInPVS)
			continue;

		player_info_s* pPlayerInfo = g_Studio.PlayerInfo(pGameEntity->curstate.number - 1);

		if (!pPlayerInfo)
			continue;

		const auto backup_gameentity = *pGameEntity;
		const auto backup_playerinfo = *pPlayerInfo;

		pGameEntity->trivial_accept = PLAYER_MARKER;

		StudioDrawPlayer(pGameEntity);

		*pGameEntity = backup_gameentity;
		*pPlayerInfo = backup_playerinfo;
	}
}

static void SetHealthBasedColor(int index)
{
	float flHealthColor[4];

	Game::GetColorHealthBased(g_Player[index]->m_iHealth, flHealthColor);

	memcpy(g_pGlobals->m_flRenderColor, flHealthColor, sizeof(g_pGlobals->m_flRenderColor));
}

void CChamsPlayers::DrawPlayer(int index)
{
	SetRenderType(cvars::visuals.colored_models_players, cvars::visuals.colored_models_players_wireframe);

	if (cvars::visuals.colored_models_players_behind_wall && !g_Player[index]->m_bIsDead)
	{
		SetHidden();

		if (cvars::visuals.colored_models_players_color_health_based == ColorHealthBased_Always)
		{
			SetHealthBasedColor(index);
		}
		else
		{
			switch (g_Player[index]->m_iTeamNum)
			{
			case TEAM_TERRORIST: memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_players_color_t_hide, sizeof(g_pGlobals->m_flRenderColor)); break;
			case TEAM_CT:		 memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_players_color_ct_hide, sizeof(g_pGlobals->m_flRenderColor));
			}
		}

		DormantColor(index, g_pGlobals->m_flRenderColor);

		StudioRenderFinal();
	}

	SetVisible();

	if (g_Player[index]->m_bIsDead)
	{
		SetRenderType(cvars::visuals.colored_models_players_on_the_dead);

		memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_players_on_the_dead_color, sizeof(g_pGlobals->m_flRenderColor));
	}
	else
	{
		if (cvars::visuals.colored_models_players_color_health_based)
		{
			SetHealthBasedColor(index);
		}
		else
		{
			switch (g_Player[index]->m_iTeamNum)
			{
			case TEAM_TERRORIST: memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_players_color_t_vis, sizeof(g_pGlobals->m_flRenderColor)); break;
			case TEAM_CT:		 memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_players_color_ct_vis, sizeof(g_pGlobals->m_flRenderColor));
			}
		}
	}

	StudioRenderFinal();
}