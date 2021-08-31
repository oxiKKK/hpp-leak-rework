#include "framework.h"

static float impulse_radius = 0.f;

static void ImpulseRadius()
{
	static bool plus_or_minus;
	
	if (impulse_radius <= 0.f || impulse_radius >= 1.f)
		plus_or_minus = !plus_or_minus;

	const float speed = cvars::visuals.colored_models_dlight_fading_lighting_speed;

	impulse_radius += plus_or_minus ? speed * g_Local->m_flFrameTime : -speed * g_Local->m_flFrameTime;
	impulse_radius = std::clamp(impulse_radius, 0.f, 1.f);
}

void CDlight::Draw()
{
	ImpulseRadius();

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

bool CDlight::IsPlayer(int index)
{
	if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	if (g_Player[index]->m_bIsLocal)
	{
		if (!cvars::visuals.colored_models_dlight_players[0] || !g_pGlobals->m_bIsInThirdPerson)
			return false;
	}
	else
	{
		if (!cvars::visuals.colored_models_dlight_players[1] && g_Player[index]->m_iTeamNum != g_Local->m_iTeamNum)
			return false;

		if (!cvars::visuals.colored_models_dlight_players[2] && g_Player[index]->m_iTeamNum == g_Local->m_iTeamNum)
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

void CDlight::DrawLight(cl_entity_s* pGameEntity)
{
	dlight_t* dl = g_Engine.pEfxAPI->CL_AllocDlight(pGameEntity->index);

	dl->origin = pGameEntity->origin;

	switch (cvars::visuals.colored_models_dlight_origin)
	{
	case DLIGHT_LEGS: dl->origin.z += g_Player[pGameEntity->index]->m_vecBoundBoxMins.z; break;
	case DLIGHT_HEAD: dl->origin.z += g_Player[pGameEntity->index]->m_vecBoundBoxMaxs.z;
	}

	dl->radius = cvars::visuals.colored_models_dlight_radius;

	if (cvars::visuals.colored_models_dlight_fading_lighting)
		dl->radius *= impulse_radius;

	dl->die = static_cast<float>(client_state->time + 0.01);
	dl->color.r = static_cast<byte>(cvars::visuals.colored_models_dlight_color[0] * 255.f);
	dl->color.g = static_cast<byte>(cvars::visuals.colored_models_dlight_color[1] * 255.f);
	dl->color.b = static_cast<byte>(cvars::visuals.colored_models_dlight_color[2] * 255.f);
	dl->minlight = cvars::visuals.colored_models_dlight_minlight;
}