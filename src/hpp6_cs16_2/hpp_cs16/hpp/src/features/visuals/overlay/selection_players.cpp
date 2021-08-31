#include "framework.h"

std::vector<SelectionData>& CSelectionPlayers::GetSelectionPlayers(std::function<bool(int)> FilterPlayerFn)
{
	static std::vector<SelectionData> selection_players;

	if (!selection_players.empty())
		selection_players.clear();

	for (int i = 1; i <= client_state->maxclients; i++)
	{
		if (FilterPlayerFn(i))
			PushPlayer(selection_players, i);
	}

	SortSelectionPlayers(selection_players);
	return selection_players;
}

void CSelectionPlayers::PushPlayer(std::vector<SelectionData>& data, int index)
{
	data.push_back(SelectionData{ g_Player[index]->m_flDistance, index });
}

void CSelectionPlayers::SortSelectionPlayers(std::vector<SelectionData>& data)
{
	std::vector<SelectionData> sort_selection_players;

	for (size_t start_idx = 0; start_idx < data.size(); start_idx++)
	{
		float max_dist = 0.f;
		int sort_idx = 0;

		for (size_t current_idx = 0; current_idx < data.size(); current_idx++)
		{
			bool skip_player = false;

			for (auto& player : sort_selection_players)
			{
				if (player.index == data[current_idx].index)
				{
					skip_player = true;
					break;
				}
			}

			if (skip_player)
				continue;

			if (data[current_idx].dist > max_dist)
			{
				max_dist = data[current_idx].dist;
				sort_idx = data[current_idx].index;
			}
		}

		if (!max_dist || !sort_idx)
			continue;

		sort_selection_players.push_back(SelectionData{ max_dist, sort_idx });
	}

	data = sort_selection_players;
}

void CSelectionPlayers::InterpolateOrigin(int index, Vector& origin)
{
	if (!g_Player[index]->m_bSoundUpdated)
		return;

	if (g_Player[index]->m_flUpdateIntervalTime > 0.5f)
		return;

	if (g_Player[index]->m_vecOrigin.Distance(g_Player[index]->m_vecPrevOrigin) > 128.f)
		return;

	if (client_state->time > g_Player[index]->m_flHistory + g_Player[index]->m_flUpdateIntervalTime)
		return;

	origin.x = static_cast<float>(Math::Interp(g_Player[index]->m_flHistory, client_state->time, g_Player[index]->m_flHistory + 
		g_Player[index]->m_flUpdateIntervalTime, g_Player[index]->m_vecPrevOrigin.x, g_Player[index]->m_vecOrigin.x));

	origin.y = static_cast<float>(Math::Interp(g_Player[index]->m_flHistory, client_state->time, g_Player[index]->m_flHistory + 
		g_Player[index]->m_flUpdateIntervalTime, g_Player[index]->m_vecPrevOrigin.y, g_Player[index]->m_vecOrigin.y));

	origin.z = static_cast<float>(Math::Interp(g_Player[index]->m_flHistory, client_state->time, g_Player[index]->m_flHistory + 
		g_Player[index]->m_flUpdateIntervalTime, g_Player[index]->m_vecPrevOrigin.z, g_Player[index]->m_vecOrigin.z));
}

void CSelectionPlayers::GetBBMaxsMins(int index, Vector& mins, Vector& maxs)
{
	Vector origin = g_Player[index]->m_vecOrigin;

	if (cvars::visuals.esp_interpolate_history)
		InterpolateOrigin(index, origin);

	mins = origin;
	maxs = g_Player[index]->m_bIsInPVS ? g_Player[index]->m_vecHitbox[HITBOX_HEAD] : mins;

	mins.z += g_Player[index]->m_vecBoundBoxMins.z - 1.f;
	maxs.z += g_Player[index]->m_bIsInPVS ? 9.f : g_Player[index]->m_vecBoundBoxMaxs.z - 7.f;
}

void CSelectionPlayers::FadeoutDormant(int index, ImColor& color, float history_time)
{
	if (!cvars::visuals.esp_dormant_time)
		return;

	if (g_Player[index]->m_bIsInPVS)
		return;

	if (cvars::visuals.esp_dormant_fadeout)
	{
		color.value.w = static_cast<float>(Math::Interp(history_time, client_state->time,
			history_time + cvars::visuals.esp_dormant_time, color.value.w, 0));
	}
	else
		color = ImColor(0.50f, 0.50f, 0.50f, color.value.w);
}