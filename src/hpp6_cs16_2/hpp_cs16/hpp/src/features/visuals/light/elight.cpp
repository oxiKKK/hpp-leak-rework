#include "framework.h"

void CElight::Draw()
{
	for (int i = 1; i <= client_state->maxclients; i++)
	{
		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!Game::IsValidEntity(pGameEntity))
			continue;

		if (!IsPlayer(pGameEntity->index))
			continue;

		DrawLight(pGameEntity);
	}
}

bool CElight::IsPlayer(int index)
{
	if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	if (g_Player[index]->m_bIsLocal)
	{
		if (!cvars::visuals.colored_models_elight_players[0] || !g_pGlobals->m_bIsInThirdPerson)
			return false;
	}
	else
	{
		if (!cvars::visuals.colored_models_elight_players[1] && g_Player[index]->m_iTeamNum != g_Local->m_iTeamNum)
			return false;

		if (!cvars::visuals.colored_models_elight_players[2] && g_Player[index]->m_iTeamNum == g_Local->m_iTeamNum)
			return false;
	}

	if (!g_Player[index]->m_bIsInPVS)
	{
		if (client_state->time - g_Player[index]->m_flLastTimeInPVS > cvars::visuals.colored_models_dormant_time)
			return false;
	}

	if (g_Player[index]->m_bIsDead)
		return false;

	if (Game::ObservedPlayer(index))
		return false;

	return true;
}

void CElight::DrawLight(cl_entity_s* pGameEntity)
{
	dlight_t* dl = g_Engine.pEfxAPI->CL_AllocElight(pGameEntity->index);

	dl->origin = pGameEntity->origin;
	dl->radius = cvars::visuals.colored_models_elight_radius;
	dl->die = static_cast<float>(client_state->time + 0.01);
	dl->color.r = static_cast<byte>(cvars::visuals.colored_models_elight_color[0] * 255.f);
	dl->color.g = static_cast<byte>(cvars::visuals.colored_models_elight_color[1] * 255.f);
	dl->color.b = static_cast<byte>(cvars::visuals.colored_models_elight_color[2] * 255.f);
}