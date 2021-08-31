#include "framework.h"

std::vector<player_sound_t> g_PlayerSounds;

void CDrawPlayersSound::Draw()
{
	while (!g_PlayerSounds.empty() && client_state->time - g_PlayerSounds.front().time >= cvars::visuals.esp_player_sounds_time)
		g_PlayerSounds.erase(g_PlayerSounds.begin());

	for (auto& sound : g_PlayerSounds)
	{
		if (!Game::ObservedPlayer(sound.index))
			DrawSound(sound);
	}
}

ImColor CDrawPlayersSound::GetSoundColor(int index)
{
	if (g_Entity[index]->m_bIsPlayer)
	{
		switch (g_Player[index]->m_iTeamNum)
		{
		case TEAM_TERRORIST: return cvars::visuals.esp_player_sounds_color_t;
		case TEAM_CT:		 return cvars::visuals.esp_player_sounds_color_ct;
		}
	}
	else
	{
		switch (g_Local->m_iTeamNum)
		{
		case TEAM_TERRORIST: return cvars::visuals.esp_player_sounds_color_ct;
		case TEAM_CT:		 return cvars::visuals.esp_player_sounds_color_t;
		}
	}

	return ImColor();
}

void CDrawPlayersSound::DrawSound(const player_sound_t& sound)
{
	Vector origin = sound.origin;
	origin.z -= 36.f;

	if (g_Entity[sound.index]->m_bIsPlayer)
	{
		if (g_Player[sound.index]->m_bIsLocal)
		{
			if (!cvars::visuals.esp_player_sounds_players[0] || !g_pGlobals->m_bIsInThirdPerson)
				return;
		}
		else
		{
			if (!cvars::visuals.esp_player_sounds_players[1] && g_Player[sound.index]->m_iTeamNum != g_Local->m_iTeamNum)
				return;

			if (!cvars::visuals.esp_player_sounds_players[2] && g_Player[sound.index]->m_iTeamNum == g_Local->m_iTeamNum)
				return;
		}
	}

	Vector2D tmp;

	if (!Game::WorldToScreen(origin, tmp))
		return;

	const double s1 = sound.time;
	const double s3 = s1 + cvars::visuals.esp_player_sounds_time;

	constexpr auto step = IM_PI * 0.075f;
	constexpr auto max = IM_PI * 3.f;

	Vector prevpoint;

	for (float lat = 0.f; lat <= max; lat += step)
	{
		const float sin1 = sin(lat);
		const float cos1 = cos(lat);
		const float sin3 = sin(0.f);
		const float cos3 = cos(0.f);

		Vector point = origin + Vector(sin1 * cos3, cos1, sin1 * sin3) * static_cast<float>(
			Math::Interp(s1, client_state->time, s3, cvars::visuals.esp_player_sounds_circle_radius, 0.0));

		ImColor color = GetSoundColor(sound.index);
		color.value.w = static_cast<float>(Math::Interp(s1, client_state->time, s3, 1.0, 0.0));

		Vector2D ScreenPoint, ScreenPrevious;

		if (Game::WorldToScreen(point, ScreenPoint) && Game::WorldToScreen(prevpoint, ScreenPrevious) && lat)
			g_pRenderer->AddLine(ScreenPrevious, ScreenPoint, color);

		prevpoint = point;
	}
}