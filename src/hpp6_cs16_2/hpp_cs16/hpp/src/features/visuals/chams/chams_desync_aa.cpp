#include "framework.h"

bool CChamsDesyncAA::Draw(cl_entity_s* pGameEntity)
{
	if (!IsPlayer())
		return false;

	SetupRenderer(pGameEntity);

	if (!cvars::visuals.streamer_mode)
	{
		pGameEntity->curstate.renderfx = kRenderFxGlowShell;
		pGameEntity->curstate.renderamt = -2;
	}

	DrawDesyncAA();
	RestoreRenderer(pGameEntity);

	return true;
}

bool CChamsDesyncAA::IsPlayer()
{
	if (g_Local->m_bIsDead)
		return false;

	if (g_Local->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Local->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	if (!g_pGlobals->m_bIsInThirdPerson)
		return false;
	
	return true;
}

void CChamsDesyncAA::CreateDesyncAA()
{
	if (!IsPlayer())
		return;

	cl_entity_s* pGameEntity = g_Engine.GetLocalPlayer();

	if (!Game::IsValidEntity(pGameEntity))
		return;

	player_info_s* pPlayerInfo = g_Studio.PlayerInfo(pGameEntity->curstate.number - 1);

	if (!pPlayerInfo)
		return;

	const auto backup_gameentity = *pGameEntity;
	const auto backup_playerinfo = *pPlayerInfo;

	if (g_pMiscellaneous->m_iChokedCommands)
	{
		*pGameEntity = m_DesyncData.entity;
		*pPlayerInfo = m_DesyncData.playerinfo;
	}
	else
	{
		m_DesyncData.entity = *pGameEntity;
		m_DesyncData.playerinfo = *pPlayerInfo;
	}

	pGameEntity->trivial_accept = LOCAL_DESYNC_MARKER;
	pGameEntity->curstate.weaponmodel = 0;

	StudioDrawPlayer(pGameEntity);

	*pGameEntity = backup_gameentity;
	*pPlayerInfo = backup_playerinfo;
}

void CChamsDesyncAA::DrawDesyncAA()
{
	SetRenderType(cvars::visuals.colored_models_players_desync_aa);

	memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_players_desync_aa_color, sizeof(g_pGlobals->m_flRenderColor));

	StudioRenderFinal();
}