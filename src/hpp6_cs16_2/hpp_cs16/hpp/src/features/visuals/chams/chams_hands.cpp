#include "framework.h"

bool CChamsHands::Draw(cl_entity_s* pGameEntity)
{
	if (!IsHands(pGameEntity))
		return false;

	SetupRenderer(pGameEntity);
	DrawHands();
	RestoreRenderer(pGameEntity);

	return true;
}

bool CChamsHands::IsHands(cl_entity_s* pGameEntity)
{
	if (g_pGlobals->m_bIsInThirdPerson)
		return false;

	if (pGameEntity != g_Engine.GetViewModel())
		return false;

	return true;
}

void CChamsHands::DrawHands()
{
	SetRenderType(cvars::visuals.colored_models_hands, cvars::visuals.colored_models_hands_wireframe);

	if (cvars::visuals.colored_models_hands_color_rainbow)
	{
		static float rainbow_color[4];

		Utils::RainbowColor(cvars::visuals.colored_models_hands_color_rainbow_speed, 1.00f, rainbow_color);

		memcpy(g_pGlobals->m_flRenderColor, rainbow_color, sizeof(g_pGlobals->m_flRenderColor));
	}
	else
		memcpy(g_pGlobals->m_flRenderColor, cvars::visuals.colored_models_hands_color, sizeof(g_pGlobals->m_flRenderColor));

	StudioRenderFinal();
}