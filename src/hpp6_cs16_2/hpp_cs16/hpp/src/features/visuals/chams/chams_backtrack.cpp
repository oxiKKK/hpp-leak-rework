#include "framework.h"

bool CChamsBacktrack::Draw(cl_entity_s* pGameEntity)
{
	if (!IsPlayer(pGameEntity->index))
		return false;

	SetupRenderer(pGameEntity);
	DrawBacktrack(pGameEntity->index);
	RestoreRenderer(pGameEntity);

	return true;
}

bool CChamsBacktrack::IsPlayer(int index)
{
	if (g_Local->m_bIsDead)
		return false;

	if (g_Player[index]->m_bIsLocal)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	if (!cvars::visuals.colored_models_backtrack_players[0] && g_Player[index]->m_iTeamNum != g_Local->m_iTeamNum)
		return false;

	if (!cvars::visuals.colored_models_backtrack_players[1] && g_Player[index]->m_iTeamNum == g_Local->m_iTeamNum)
		return false;

	if (!g_Player[index]->m_bIsInPVS)
	{
		if (client_state->time - g_Player[index]->m_flLastTimeInPVS > cvars::visuals.colored_models_dormant_time)
			return false;
	}

	if (g_Player[index]->m_bIsDead)
		return false;

	return true;
}

void CChamsBacktrack::CreateBacktrackPlayers()
{
	for (int i = 1; i <= client_state->maxclients; i++)
	{
		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!Game::IsValidEntity(pGameEntity))
			continue;

		if (!IsPlayer(pGameEntity->index))
			continue;

		player_info_s* pPlayerInfo = g_Studio.PlayerInfo(pGameEntity->curstate.number - 1);

		if (!pPlayerInfo)
			continue;

		Vector backtrack;

		if (!Game::GetBacktrackOrigin(pGameEntity, backtrack))
			continue;

		if (pGameEntity->origin == backtrack)
			continue;

		const auto backup_gameentity = *pGameEntity;
		const auto backup_playerinfo = *pPlayerInfo;

		pGameEntity->trivial_accept = BACKTRACK_MARKER;
		pGameEntity->origin = backtrack;
		pGameEntity->curstate.weaponmodel = 0;

		pPlayerInfo->prevgaitorigin = pGameEntity->origin - Game::PredictPlayer(pGameEntity->index);

		StudioDrawPlayer(pGameEntity);

		*pGameEntity = backup_gameentity;
		*pPlayerInfo = backup_playerinfo;
	}
}

void CChamsBacktrack::DrawBacktrack(int index)
{
	SetRenderType(cvars::visuals.colored_models_backtrack, cvars::visuals.colored_models_backtrack_wireframe);

	if (cvars::visuals.colored_models_backtrack_behind_wall)
	{
		SetHidden();

		switch (g_Player[index]->m_iTeamNum)
		{
		case TEAM_TERRORIST: memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_backtrack_color_t_hide, sizeof(g_pGlobals->m_flRenderColor)); break;
		case TEAM_CT:		 memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_backtrack_color_ct_hide, sizeof(g_pGlobals->m_flRenderColor));
		}

		DormantColor(index, g_pGlobals->m_flRenderColor);

		StudioRenderFinal();
	}

	SetVisible();

	switch (g_Player[index]->m_iTeamNum)
	{
	case TEAM_TERRORIST: memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_backtrack_color_t_vis, sizeof(g_pGlobals->m_flRenderColor)); break;
	case TEAM_CT:		 memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_backtrack_color_ct_vis, sizeof(g_pGlobals->m_flRenderColor));
	}

	StudioRenderFinal();
}