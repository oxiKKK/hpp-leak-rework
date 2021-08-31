#include "framework.h"

bool CChamsHitPosition::Draw(cl_entity_s* pGameEntity)
{
	if (!IsPlayer(pGameEntity->index))
		return false;

	SetupRenderer(pGameEntity);
	DrawHitPosition(pGameEntity->index);
	RestoreRenderer(pGameEntity);

	return true;
}

bool CChamsHitPosition::IsPlayer(int index)
{
	if (g_Local->m_bIsDead)
		return false;

	if (g_Player[index]->m_bIsLocal)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	return true;
}

void CChamsHitPosition::CreateHitPositionPlayers()
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

		if (g_Player[pGameEntity->index]->m_bIsInPVS && !g_Player[pGameEntity->index]->m_bIsDead)
		{
			if (g_HitRegister->m_iIndex == pGameEntity->index && client_state->time - g_HitRegister->m_flTimeStamp < 0.05f)
			{
				if (m_HitPositionData[pGameEntity->index].empty() || m_HitPositionData[pGameEntity->index].back().timestamp != g_HitRegister->m_flTimeStamp)
				{
					m_HitPositionData[pGameEntity->index].push_back(HitPositionData{ *pGameEntity, *pPlayerInfo, g_HitRegister->m_flTimeStamp });

					Vector backtrack;

					if (Game::GetBacktrackOrigin(pGameEntity, backtrack))
						m_HitPositionData[pGameEntity->index].back().entity.origin = backtrack;
				}
			}
		}

		for (auto& hitposition : m_HitPositionData[pGameEntity->index])
		{
			if (client_state->time - hitposition.timestamp < cvars::visuals.colored_models_players_hit_position_time)
			{
				const auto backup_gameentity = *pGameEntity;
				const auto backup_playerinfo = *pPlayerInfo;

				*pGameEntity = hitposition.entity;
				*pPlayerInfo = hitposition.playerinfo;

				pGameEntity->trivial_accept = HIT_POSITION_MARKER;
				pGameEntity->curstate.animtime = FLT_MAX;
				pGameEntity->curstate.weaponmodel = 0;

				StudioDrawPlayer(pGameEntity);

				*pGameEntity = backup_gameentity;
				*pPlayerInfo = backup_playerinfo;
			}
			else
				m_HitPositionData[pGameEntity->index].erase(m_HitPositionData[pGameEntity->index].begin());
		}
	}
}

void CChamsHitPosition::DrawHitPosition(int index)
{
	SetRenderType(cvars::visuals.colored_models_players_hit_position);

	memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_players_hit_position_color, sizeof(g_pGlobals->m_flRenderColor));

	SetHidden();

	StudioRenderFinal();

	SetVisible();

	StudioRenderFinal();
}