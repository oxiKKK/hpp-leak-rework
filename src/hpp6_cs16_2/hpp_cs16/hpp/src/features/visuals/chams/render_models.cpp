#include "framework.h"

void CRenderModels::SetupRenderer(cl_entity_s* pGameEntity)
{
	pGameEntity->curstate.rendermode = kRenderTransTexture;

	pGameEntity->curstate.renderfx = cvars::visuals.streamer_mode ? kRenderFxGlowShell : kRenderFxNone;
	pGameEntity->curstate.renderamt = cvars::visuals.streamer_mode ? 5 : 0;

	g_Studio.SetForceFaceFlags(0);
}

void CRenderModels::RestoreRenderer(cl_entity_s* pGameEntity)
{
	SetRenderType(TEXTURE);

	pGameEntity->trivial_accept = 0;
	pGameEntity->curstate.rendermode = kRenderNormal;
}

void CRenderModels::SetRenderType(RenderType_t renderType, bool wireframe)
{
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

	switch (renderType)
	{
	case FLAT:
	case DARKENED:
	case LIGHTED:
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	case TEXTURE:
		glEnable(GL_TEXTURE_2D);
	}

	g_pGlobals->m_iRenderType = renderType;
}

void CRenderModels::SetVisible()
{
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
}

void CRenderModels::SetHidden()
{
	glDepthFunc(GL_GREATER);
	glDisable(GL_DEPTH_TEST);
}

void CRenderModels::DormantColor(int index, float* flRenderColor)
{
	if (!cvars::visuals.colored_models_dormant_time)
		return;

	if (g_Player[index]->m_bIsInPVS)
		return;

	if (cvars::visuals.colored_models_dormant_fadeout)
	{
		flRenderColor[3] = static_cast<float>(Math::Interp(g_Player[index]->m_flLastTimeInPVS, client_state->time,
			g_Player[index]->m_flLastTimeInPVS + cvars::visuals.colored_models_dormant_time, flRenderColor[3], 0));
	}
	else
		flRenderColor[0] = flRenderColor[1] = flRenderColor[2] = 0.50f;
}

bool CRenderModels::IsWeaponModelRender(cl_entity_s* pGameEntity)
{
	model_t* pWeaponModel = client_state->model_precache[pGameEntity->curstate.weaponmodel];

	if (pCStudioModelRenderer->m_pStudioHeader == g_Studio.Mod_Extradata(pWeaponModel))
		return true;

	return false;
}

void CRenderModels::StudioDrawPlayer(cl_entity_s* pGameEntity)
{
	*(PDWORD)(g_pGlobals->m_dwCurrentEntity) = (DWORD)(pGameEntity);

	g_Studio.SetupRenderer(kRenderNormal);

	g_StudioAPI.StudioDrawPlayer(STUDIO_RENDER, &pGameEntity->curstate);
	
	g_Studio.RestoreRenderer();
}

extern void oStudioRenderFinal();

void CRenderModels::StudioRenderFinal()
{
	g_pGlobals->m_bRenderModels = true;

	oStudioRenderFinal();

	g_pGlobals->m_bRenderModels = false;
}