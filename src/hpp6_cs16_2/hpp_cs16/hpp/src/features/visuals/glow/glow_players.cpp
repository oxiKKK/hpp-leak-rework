#include "framework.h"

extern void oStudioRenderFinal();

static void SetRenderColor(color24& rendercolor, float* color)
{
	for (size_t i = 0; i < sizeof(color24); i++)
		((byte*)&rendercolor)[i] = static_cast<byte>(color[i] * 255.f);
}

bool CGlowPlayers::Draw(cl_entity_s* pGameEntity)
{
	if (pGameEntity->trivial_accept)
		return false;

	if (!IsPlayer(pGameEntity->index))
		return false;

	if (CRenderModels::IsWeaponModelRender(pGameEntity))
		return false;

	g_Studio.SetForceFaceFlags(STUDIO_NF_CHROME);

	const float amount = cvars::visuals.esp_player_glow_amount / 5.f;
	const float amount_distance_based = round(amount * sqrt(g_Player[pGameEntity->index]->m_flDistance));

	pGameEntity->curstate.rendermode = kRenderTransTexture;
	pGameEntity->curstate.renderfx = kRenderFxGlowShell;
	pGameEntity->curstate.renderamt = static_cast<int>(amount_distance_based);

	if (cvars::visuals.esp_player_glow_color_health_based)
	{
		float color_health_based[4];

		Game::GetColorHealthBased(g_Player[pGameEntity->index]->m_iHealth, color_health_based);

		SetRenderColor(pGameEntity->curstate.rendercolor, color_health_based);
	}
	else
	{
		switch (g_Player[pGameEntity->index]->m_iTeamNum)
		{
		case TEAM_TERRORIST: SetRenderColor(pGameEntity->curstate.rendercolor, cvars::visuals.esp_player_glow_color_t); break;
		case TEAM_CT:        SetRenderColor(pGameEntity->curstate.rendercolor, cvars::visuals.esp_player_glow_color_ct);
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDepthFunc(GL_GREATER);
	glDisable(GL_DEPTH_TEST);

	oStudioRenderFinal();

	g_Studio.SetForceFaceFlags(0);

	pGameEntity->curstate.rendermode = kRenderNormal;
	pGameEntity->curstate.renderfx = kRenderFxNone;
	pGameEntity->curstate.renderamt = 0;

	oStudioRenderFinal();

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	return false;
}

bool CGlowPlayers::IsPlayer(int index)
{
	if (!g_Entity[index]->m_bIsPlayer)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	if (g_Player[index]->m_bIsLocal)
	{
		if (!cvars::visuals.esp_player_glow_players[0] || !g_pGlobals->m_bIsInThirdPerson)
			return false;
	}
	else
	{
		if (!cvars::visuals.esp_player_glow_players[1] && g_Player[index]->m_iTeamNum != g_Local->m_iTeamNum)
			return false;

		if (!cvars::visuals.esp_player_glow_players[2] && g_Player[index]->m_iTeamNum == g_Local->m_iTeamNum)
			return false;
	}

	if (!g_Player[index]->m_bIsInPVS)
		return false;

	if (g_Player[index]->m_bIsDead)
		return false;

	return true;
}