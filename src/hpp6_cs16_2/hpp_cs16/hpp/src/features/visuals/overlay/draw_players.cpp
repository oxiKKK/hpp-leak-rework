#include "framework.h"

CDrawPlayers::CDrawPlayers()
	: m_pOutFOV(std::make_unique<CDrawPlayersOutFOV>())
	, m_pSound(std::make_unique<CDrawPlayersSound>())
{
}

void CDrawPlayers::Overlay()
{
	if (g_Local->m_iObserverState >= OBS_MAP_FREE)
		return;

	if (cvars::visuals.esp_player_sounds)
		m_pSound->Draw();

	ImGui::PushFont(g_pFontList[std::clamp(cvars::visuals.esp_font_size, 1, 25)]);

	if (cvars::visuals.esp_player)
	{
		auto& selection_players = GetSelectionPlayers([](int index)
		{
			if (g_Player[index]->m_bIsLocal)
				return false;

			if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
				return false;

			if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
				return false;

			if (!cvars::visuals.esp_player_players[1] && g_Player[index]->m_iTeamNum != g_Local->m_iTeamNum)
				return false;

			if (!cvars::visuals.esp_player_players[2] && g_Player[index]->m_iTeamNum == g_Local->m_iTeamNum)
				return false;

			if (Game::ObservedPlayer(index))
				return false;

			if (!g_Player[index]->m_bIsInPVS)
			{
				if (client_state->time - g_Player[index]->m_flHistory > cvars::visuals.esp_dormant_time)
					return false;
			}

			if (g_Player[index]->m_bIsDead)
				return false;

			return true;
		});

		if (cvars::visuals.esp_player_players[0] && g_pGlobals->m_bIsInThirdPerson && !g_Local->m_bIsDead)
			selection_players.push_back(SelectionData{ 0.f, g_Local->m_iEntIndex });

		for (auto& player : selection_players)
		{
			if (CalcScreen(player.index))
			{
				if (cvars::visuals.esp_player_box)
					BoundBox(player.index);

				if (cvars::visuals.esp_player_armor)
					ArmorBar(player.index);

				if (cvars::visuals.esp_player_health)
					HealthBar(player.index);

				if (cvars::visuals.esp_player_name)
					Name(player.index);

				if (cvars::visuals.esp_player_weapon_text)
					WeaponText(player.index);

				if (cvars::visuals.esp_player_money)
					Money(player.index);

				if (cvars::visuals.esp_player_distance)
					Distance(player.index);

				if (cvars::visuals.esp_player_actions)
					Actions(player.index);

				if (cvars::visuals.esp_player_weapon_icon)
					WeaponIcon(player.index);

				if (cvars::visuals.esp_player_has_c4)
					HasC4(player.index);

				if (cvars::visuals.esp_player_has_defusal_kits)
					HasDefusalKits(player.index);
			}

			if (cvars::visuals.esp_player_skeleton)
				Skeleton(player.index);

			if (cvars::visuals.esp_player_skeleton_backtrack)
				SkeletonBacktrack(player.index);

			if (cvars::visuals.esp_player_hitboxes)
				Hitboxes(player.index);
		}

		if (cvars::visuals.esp_player_hitboxes_hit_position)
			HitboxesHitPosition();
	}

	if (cvars::visuals.esp_player_out_of_fov)
		m_pOutFOV->Draw();

	ImGui::PopFont();
}

void CDrawPlayers::LineSight()
{
	for (int i = 1; i <= client_state->maxclients; i++)
	{
		if (g_Player[i]->m_bIsLocal)
			continue;

		if (g_Player[i]->m_iTeamNum == TEAM_UNASSIGNED)
			continue;

		if (g_Player[i]->m_iTeamNum == TEAM_SPECTATOR)
			continue;

		if (!cvars::visuals.esp_player_players[1] && g_Player[i]->m_iTeamNum != g_Local->m_iTeamNum)
			continue;

		if (!cvars::visuals.esp_player_players[2] && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			continue;

		if (!g_Player[i]->m_bIsInPVS)
			continue;

		if (g_Player[i]->m_bIsDead)
			continue;

		if (Game::ObservedPlayer(i))
			continue;

		Vector vecForward;

		g_Player[i]->m_QAngles.AngleVectors(&vecForward, NULL, NULL);

		Vector vecStart = g_Player[i]->m_vecHitbox[HITBOX_HEAD];
		Vector vecDelta = vecForward * cvars::visuals.esp_player_line_of_sight;

		pmtrace_t tr;

		g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
		g_Engine.pEventAPI->EV_PlayerTrace(vecStart, vecStart + vecDelta, PM_GLASS_IGNORE, -1, &tr);

		const float* col = cvars::visuals.esp_player_line_of_sight_color;

		static int beam_index = g_Engine.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");

		g_Engine.pEfxAPI->R_BeamPoints(vecStart, tr.endpos, beam_index, 0.001f, 1.f, 0, 128, 0, 0, 0, col[0], col[1], col[2]);
	}
}

ImRect CDrawPlayers::GetRect(const Vector2D& ScreenTop, const Vector2D& ScreenBot)
{
	ImVec2 min, max;

	max.y = ceil(ScreenBot.y - ScreenTop.y);
	max.x = ceil(max.y * 0.5f);
	min.y = ceil(ScreenTop.y);
	min.x = ceil(ScreenBot.x - max.x * 0.5f);

	return ImRect(min, min + max);
}

static FontFlags_t GetFontFlags(int pos)
{
	return pos > PosRight ? FontFlags_CenterX | FontFlags_Shadow : FontFlags_Shadow;
}

static float GetSpacingBar()
{
	return cvars::visuals.esp_player_box_outline ? 3.f : 1.f;
}

static float GetNextlineText(int index, int& pos)
{
	float nextline = 0.f;

	if (strnlen_s(g_Player[index]->m_szPrintName, MAX_PLAYER_NAME_LENGTH))
	{
		if (&pos == &cvars::visuals.esp_player_name)
			return nextline;
	
		if (pos == cvars::visuals.esp_player_name)
			nextline += GImGui->Font->FontSize;
	}

	if (strnlen_s(g_Player[index]->m_szWeaponModelName, MAX_MODEL_NAME))
	{
		if (&pos == &cvars::visuals.esp_player_weapon_text)
			return nextline;

		if (pos == cvars::visuals.esp_player_weapon_text)
			nextline += GImGui->Font->FontSize;
	}

	if (g_Player[index]->m_iMoney > 0)
	{
		if (&pos == &cvars::visuals.esp_player_money)
			return nextline;

		if (pos == cvars::visuals.esp_player_money)
			nextline += GImGui->Font->FontSize;
	}

	if (!g_Player[index]->m_bIsLocal)
	{
		if (&pos == &cvars::visuals.esp_player_distance)
			return nextline;

		if (pos == cvars::visuals.esp_player_distance)
			nextline += GImGui->Font->FontSize;
	}

	if (&pos == &cvars::visuals.esp_player_weapon_icon)
		return nextline;

	if (pos == cvars::visuals.esp_player_weapon_icon)
		nextline += GImGui->Font->FontSize + 3.f;

	if (g_Player[index]->m_iSequence & (SEQUENCE_SHOOT | SEQUENCE_RELOAD | SEQUENCE_THROW | SEQUENCE_ARM_C4))
	{
		if (&pos == &cvars::visuals.esp_player_actions)
			return nextline;

		if (pos == cvars::visuals.esp_player_actions)
			nextline += GImGui->Font->FontSize;
	}

	return nextline;
}

static float GetSpacingText(int index, int pos)
{
	float spacing = cvars::visuals.esp_player_box_outline ? 3.f : 2.f;

	if (pos == cvars::visuals.esp_player_health)
		spacing += GetSpacingBar() + 2.f;

	if (pos == cvars::visuals.esp_player_armor)
	{
		if (g_Player[index]->m_iArmorType || g_Player[index]->m_iArmor)
			spacing += GetSpacingBar() + 2.f;
	}

	return spacing;
}

ImVec2 CDrawPlayers::GetPosition(int index, int& pos, std::string text, ImVec2 text_size)
{
	if (!text_size.x || !text_size.y)
		text_size = GImGui->Font->CalcTextSizeA(GImGui->Font->FontSize, FLT_MAX, 0, text.c_str());

	const float nextline = GetNextlineText(index, pos);
	const float spacing = GetSpacingText(index, pos);

	ImVec2 position;

	switch (pos)
	{
	case PosLeft:
		position.x = m_rect.Min.x - spacing - text_size.x;
		position.y = m_rect.Min.y + nextline - 3.f;
		break;
	case PosRight:
		position.x = m_rect.Max.x + spacing + 1.f;
		position.y = m_rect.Min.y + nextline - 3.f;
		break;
	case PosTop:
		position.x = m_rect.Min.x + m_rect.GetWidth() * 0.5f;
		position.y = m_rect.Min.y - spacing - text_size.y - nextline;
		break;
	case PosBot:
		position.x = m_rect.Min.x + m_rect.GetWidth() * 0.5f;
		position.y = m_rect.Max.y + spacing + nextline - 2.f;
	}

	if (&pos == &cvars::visuals.esp_player_weapon_icon || 
		&pos == &cvars::visuals.esp_player_has_c4 || 
		&pos == &cvars::visuals.esp_player_has_defusal_kits)
	{
		switch (pos)
		{
		case PosLeft:	position -= ImVec2(2.f, -2.f); break;
		case PosRight:  position += ImVec2(2.f, 2.f); break;
		case PosTop:	position -= ImVec2(text_size.x * 0.5f, 1.f); break;
		case PosBot:	position -= ImVec2(text_size.x * 0.5f, -2.f);
		}
	}

	return position;
}

ImColor CDrawPlayers::GetTeamColor(int index)
{
	switch (g_Player[index]->m_iTeamNum)
	{
	case TEAM_TERRORIST: return cvars::visuals.esp_player_box_color_t;
	case TEAM_CT:		 return cvars::visuals.esp_player_box_color_ct;
	}

	return ImColor();
}

bool CDrawPlayers::CalcScreen(int index)
{
	Vector mins, maxs;

	GetBBMaxsMins(index, mins, maxs);

	Vector2D ScreenTop, ScreenBot;

	const bool ret = Game::WorldToScreen(maxs, ScreenTop) && Game::WorldToScreen(mins, ScreenBot);

	if (g_Player[index]->m_bIsInPVS)
	{
		for (int hitboxnum = 0; hitboxnum < HITBOX_MAX - 1; hitboxnum++)
		{
			for (auto& hitboxpoint : g_Player[index]->m_vecHitboxPoints[hitboxnum])
			{
				Vector2D ScreenPoint;

				if (Game::WorldToScreen(hitboxpoint, ScreenPoint))
					ScreenTop.y = min(ScreenTop.y, ScreenPoint.y);
			}
		}
	}

	if (ret)
		m_rect = GetRect(ScreenTop, ScreenBot);

	return ret;
}

void CDrawPlayers::BoundBox(int index)
{
	ImColor color = GetTeamColor(index);

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	switch (cvars::visuals.esp_player_box)
	{
	case BoxDefault: g_pRenderer->AddRect(m_rect.Min, m_rect.Max, color, cvars::visuals.esp_player_box_outline); break;
	case BoxCorner:  g_pRenderer->AddRectCorner(m_rect.Min, m_rect.Max, color, cvars::visuals.esp_player_box_outline); break;
	case BoxRound:   g_pRenderer->AddRect(m_rect.Min, m_rect.Max, color, cvars::visuals.esp_player_box_outline, 3.f);
	}

	if (cvars::visuals.esp_player_box_filled)
	{
		color.value.w = cvars::visuals.esp_player_box_filled / 100.f;

		FadeoutDormant(index, color, g_Player[index]->m_flHistory);

		const float padding = cvars::visuals.esp_player_box_outline ? 2.f : 1.f;

		g_pRenderer->AddRectFilled(m_rect.Min + padding, m_rect.Max - padding + 1, color, cvars::visuals.esp_player_box == BoxRound ? 3.f : 0.f);
	}
}

void CDrawPlayers::DrawBar(int index, int pos, float value, const ImVec4& color, float spacing, bool percentage, ImColor color_text)
{
	value = std::clamp(value, 0.f, 100.f);

	const float size = ceil((pos <= PosRight ? m_rect.GetHeight() : m_rect.GetWidth()) / 100.f * value);

	constexpr auto thickness = 2.f;

	ImColor color_background = ImColor(0.25f, 0.25, 0.25f, color.w);
	ImColor color_outline = ImColor(0.00f, 0.00f, 0.00f, color.w);

	FadeoutDormant(index, color_text, g_Player[index]->m_flHistory);

	float a;

	switch (pos)
	{
	case PosLeft:	a = m_rect.Min.x - spacing - thickness; break;
	case PosRight:	a = m_rect.Max.x + spacing + 1.f; break;
	case PosTop:	a = m_rect.Min.y - spacing - thickness; break;
	case PosBot:	a = m_rect.Max.y + spacing + 1.f;
	}

	switch (pos)
	{
	case PosLeft:
	case PosRight:
		g_pRenderer->AddRectFilled(ImVec2(a, m_rect.Min.y), ImVec2(a + thickness, m_rect.Max.y + 1), color_background);
		g_pRenderer->AddRectFilled(ImVec2(a, m_rect.Max.y + 1), ImVec2(a + thickness, m_rect.Max.y - size), color);

		if (cvars::visuals.esp_player_box_outline)
			g_pRenderer->AddRect(ImVec2(a, m_rect.Min.y) - 1, ImVec2(a + thickness, m_rect.Max.y + 1), color_outline);

		if (percentage && value > 0.f && value < 100.f)
		{
			std::string text = std::to_string(static_cast<int>(value));

			ImVec2 text_size = GImGui->Font->CalcTextSizeA(GImGui->Font->FontSize, FLT_MAX, 0, text.c_str());

			g_pRenderer->AddText(ImVec2(a + thickness * 0.5f, m_rect.Max.y - size - text_size.y), color_text, FontFlags_CenterX | FontFlags_Shadow, text.c_str());
		}
		break;

	case PosTop:
	case PosBot:
		g_pRenderer->AddRectFilled(ImVec2(m_rect.Min.x, a), ImVec2(m_rect.Max.x + 1, a + thickness), color_background);
		g_pRenderer->AddRectFilled(ImVec2(m_rect.Min.x, a), ImVec2(m_rect.Min.x + 1 + size, a + thickness), color);

		if (cvars::visuals.esp_player_box_outline)
			g_pRenderer->AddRect(ImVec2(m_rect.Min.x, a) - 1, ImVec2(m_rect.Max.x + 1, a + thickness), color_outline);

		if (percentage && value > 0.f && value < 100.f)
		{
			std::string text = std::to_string(static_cast<int>(value));

			ImVec2 text_size = GImGui->Font->CalcTextSizeA(GImGui->Font->FontSize, FLT_MAX, 0, text.c_str());

			g_pRenderer->AddText(ImVec2(m_rect.Min.x + size - 1, a + thickness * 0.5f), color_text, FontFlags_CenterY | FontFlags_Shadow, text.c_str());
		}
	}
}

void CDrawPlayers::HealthBar(int index)
{
	float health_color[4];

	Game::GetColorHealthBased(g_Player[index]->m_iHealth, health_color);

	ImColor color_team = GetTeamColor(index);
	ImColor color_health = health_color;

	color_health.value.w = color_team.value.w;

	FadeoutDormant(index, color_health, g_Player[index]->m_flHistory);
	DrawBar(index, cvars::visuals.esp_player_health, static_cast<float>(g_Player[index]->m_iHealth), color_health, 
		GetSpacingBar(), cvars::visuals.esp_player_health_percentage, cvars::visuals.esp_player_health_percentage_color);
}

void CDrawPlayers::ArmorBar(int index)
{
	float armor = static_cast<float>(g_Player[index]->m_iArmor);

	if (!armor)
	{
		switch (g_Player[index]->m_iArmorType)
		{
		case ARMOR_VESTHELM: armor = 100.f; break;
		case ARMOR_KEVLAR:	 armor = 50.f; break;
		case ARMOR_NONE:	 return;
		}
	}

	float spacing = GetSpacingBar();

	if (cvars::visuals.esp_player_armor == cvars::visuals.esp_player_health)
		spacing += GetSpacingBar() + 2.f;

	ImColor color_team = GetTeamColor(index);
	ImColor color_armor = cvars::visuals.esp_player_armor_color;

	color_armor.value.w = color_team.value.w;

	FadeoutDormant(index, color_armor, g_Player[index]->m_flHistory);
	DrawBar(index, cvars::visuals.esp_player_armor, armor, color_armor, spacing);
}

void CDrawPlayers::Name(int index)
{
	if (!strnlen_s(g_Player[index]->m_szPrintName, sizeof(MAX_PLAYER_NAME_LENGTH)))
		return;

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_name, g_Player[index]->m_szPrintName);

	ImColor color = cvars::visuals.esp_player_name_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddText(position, color, GetFontFlags(cvars::visuals.esp_player_name), "%s", g_Player[index]->m_szPrintName);
}

void CDrawPlayers::WeaponText(int index)
{
	if (!strnlen_s(g_Player[index]->m_szWeaponModelName, MAX_MODEL_NAME))
		return;

	const int weapon_index = Game::GetWeaponIndex(g_Player[index]->m_szWeaponModelName);

	if (!weapon_index)
		return;

	std::string weapon_name = Game::GetWeaponName(weapon_index);

	if (weapon_name.empty())
		return;

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_weapon_text, weapon_name);

	ImColor color = cvars::visuals.esp_player_weapon_text_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddText(position, color, GetFontFlags(cvars::visuals.esp_player_weapon_text), weapon_name.c_str());
}

void CDrawPlayers::WeaponIcon(int index)
{
	if (!strnlen_s(g_Player[index]->m_szWeaponModelName, MAX_MODEL_NAME))
		return;

	const int weapon_index = Game::GetWeaponIndex(g_Player[index]->m_szWeaponModelName);

	if (!weapon_index)
		return;

	ImageInfo icon = g_pImageList[IconsEspWpnsOffset + weapon_index];

	ImVec2 size = icon.size / (icon.size.y / GImGui->Font->FontSize);

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_weapon_icon, "", size);

	ImColor color = cvars::visuals.esp_player_weapon_icon_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddImage(icon.data, position, position + size, ImVec2(), ImVec2(1, 1), color);
}

void CDrawPlayers::Money(int index)
{
	if (g_Player[index]->m_iMoney <= 0)
		return;

	std::string money = "$" + std::to_string(g_Player[index]->m_iMoney);

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_money, money);

	ImColor color = cvars::visuals.esp_player_money_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddText(position, color, GetFontFlags(cvars::visuals.esp_player_money), money.c_str());
}

std::string GetDistanceString(int index)
{
	std::string distance;

	switch (cvars::visuals.esp_player_distance_measurement)
	{
	case InUnits:  distance = std::to_string(static_cast<int>(g_Player[index]->m_flDistance - 16.f)) + " u"; break;
	case InMeters: distance = std::to_string(static_cast<int>((g_Player[index]->m_flDistance - 16.f) * 0.025f)) + " m";
	}

	return distance;
}

void CDrawPlayers::Distance(int index)
{
	if (g_Player[index]->m_bIsLocal)
		return;

	std::string distance = GetDistanceString(index);

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_distance, distance);

	ImColor color = cvars::visuals.esp_player_distance_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddText(position, color, GetFontFlags(cvars::visuals.esp_player_distance), distance.c_str());
}

void CDrawPlayers::Actions(int index)
{
	if (!g_Player[index]->m_iSequence)
		return;

	std::string action;

	if (g_Player[index]->m_iSequence & SEQUENCE_SHOOT) action = "Shooting";
	else if (g_Player[index]->m_iSequence & SEQUENCE_RELOAD) action = "Reloading";
	else if (g_Player[index]->m_iSequence & SEQUENCE_THROW) action = "Throwing";
	else if (g_Player[index]->m_iSequence & SEQUENCE_ARM_C4) action = "Planting";

	if (action.empty())
		return;

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_actions, action);

	ImColor color = cvars::visuals.esp_player_actions_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddText(position, color, GetFontFlags(cvars::visuals.esp_player_actions), action.c_str());

	if (cvars::visuals.esp_player_actions_bar)
	{
		ImColor color_team = GetTeamColor(index);
		ImColor color_actions = cvars::visuals.esp_player_actions_bar_color;

		color_actions.value.w = color_team.value.w;

		ImColor color_background = ImColor(0.25f, 0.25f, 0.25f, color_team.value.w);
		ImColor color_outline = ImColor(0.00f, 0.00f, 0.00f, color_team.value.w);

		position.y += GImGui->Font->FontSize + 2.f;

		ImVec2 text_size = GImGui->Font->CalcTextSizeA(GImGui->Font->FontSize, FLT_MAX, 0, action.c_str());
		ImVec2 a = position;
		ImVec2 b = a + ImVec2(text_size.x, 2);

		const float size = ceil(text_size.x / 255.f * g_Player[index]->m_iSequenceFrame);

		g_pRenderer->AddRectFilled(a, b, color_background);
		g_pRenderer->AddRectFilled(a, a + ImVec2(size, 2), color_actions);
		
		if (cvars::visuals.esp_player_box_outline)
			g_pRenderer->AddRect(a - 1, b, color_outline);
	}
}

void CDrawPlayers::HasC4(int index)
{
	if (!g_Player[index]->m_bHasC4)
		return;

	ImageInfo icon = g_pImageList[IconsEspWpnsOffset + WEAPON_C4];

	ImVec2 size = icon.size / (icon.size.y / GImGui->Font->FontSize);

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_has_c4, "", size);

	ImColor color = cvars::visuals.esp_player_has_c4_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddImage(icon.data, position, position + size, ImVec2(), ImVec2(1, 1), color);
}

void CDrawPlayers::HasDefusalKits(int index)
{
	if (!g_Player[index]->m_bHasDefusalKits)
		return;

	ImageInfo icon = g_pImageList[DefuserIcon];

	ImVec2 size = icon.size / (icon.size.y / GImGui->Font->FontSize);

	ImVec2 position = GetPosition(index, cvars::visuals.esp_player_has_defusal_kits, "", size);

	ImColor color = cvars::visuals.esp_player_has_defusal_kits_color;

	FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	g_pRenderer->AddImage(icon.data, position, position + size, ImVec2(), ImVec2(1, 1), color);
}

static void LineByOrigin(const Vector& from, const Vector& to, const ImColor& color)
{
	Vector2D ScreenBone1, ScreenBone2;

	if (Game::WorldToScreen(from, ScreenBone1) && Game::WorldToScreen(to, ScreenBone2))
		g_pRenderer->AddLine(ScreenBone1, ScreenBone2, color);
}

static void DrawSkeleton(const Vector* hitbox, const ImColor& color)
{
	LineByOrigin(hitbox[HITBOX_STOMACH], hitbox[HITBOX_LOWER_CHEST], color);
	LineByOrigin(hitbox[HITBOX_LOWER_CHEST], hitbox[HITBOX_UPPER_CHEST], color);
	LineByOrigin(hitbox[HITBOX_UPPER_CHEST], hitbox[HITBOX_HEAD], color);

	LineByOrigin(hitbox[HITBOX_STOMACH], hitbox[HITBOX_LEFT_FOOT], color);
	LineByOrigin(hitbox[HITBOX_LEFT_FOOT], hitbox[HITBOX_LEFT_CALF], color);
	LineByOrigin(hitbox[HITBOX_LEFT_CALF], hitbox[HITBOX_LEFT_THIGH], color);

	LineByOrigin(hitbox[HITBOX_STOMACH], hitbox[HITBOX_RIGHT_FOOT], color);
	LineByOrigin(hitbox[HITBOX_RIGHT_FOOT], hitbox[HITBOX_RIGHT_CALF], color);
	LineByOrigin(hitbox[HITBOX_RIGHT_CALF], hitbox[HITBOX_RIGHT_THIGH], color);

	LineByOrigin(hitbox[HITBOX_UPPER_CHEST], hitbox[HITBOX_LEFT_UPPER_ARM], color);
	LineByOrigin(hitbox[HITBOX_LEFT_UPPER_ARM], hitbox[HITBOX_LEFT_HAND], color);
	LineByOrigin(hitbox[HITBOX_LEFT_HAND], hitbox[HITBOX_LEFT_FOREARM], color);
	LineByOrigin(hitbox[HITBOX_LEFT_FOREARM], hitbox[HITBOX_LEFT_WRIST], color);

	LineByOrigin(hitbox[HITBOX_UPPER_CHEST], hitbox[HITBOX_RIGHT_UPPER_ARM], color);
	LineByOrigin(hitbox[HITBOX_RIGHT_UPPER_ARM], hitbox[HITBOX_RIGHT_HAND], color);
	LineByOrigin(hitbox[HITBOX_RIGHT_HAND], hitbox[HITBOX_RIGHT_FOREARM], color);
	LineByOrigin(hitbox[HITBOX_RIGHT_FOREARM], hitbox[HITBOX_RIGHT_WRIST], color);
};

static void DrawHitbox(const Vector* hitbox_points, const ImColor& color)
{
	LineByOrigin(hitbox_points[0], hitbox_points[2], color);
	LineByOrigin(hitbox_points[0], hitbox_points[3], color);
	LineByOrigin(hitbox_points[0], hitbox_points[4], color);

	LineByOrigin(hitbox_points[6], hitbox_points[1], color);
	LineByOrigin(hitbox_points[6], hitbox_points[2], color);
	LineByOrigin(hitbox_points[6], hitbox_points[4], color);

	LineByOrigin(hitbox_points[5], hitbox_points[1], color);
	LineByOrigin(hitbox_points[5], hitbox_points[3], color);
	LineByOrigin(hitbox_points[5], hitbox_points[4], color);

	LineByOrigin(hitbox_points[7], hitbox_points[1], color);
	LineByOrigin(hitbox_points[7], hitbox_points[2], color);
	LineByOrigin(hitbox_points[7], hitbox_points[3], color);
}

void CDrawPlayers::Skeleton(int index)
{
	ImColor color = cvars::visuals.esp_player_skeleton_color;

	FadeoutDormant(index, color, g_Player[index]->m_flLastTimeInPVS);

	DrawSkeleton(g_Player[index]->m_vecHitbox, color);
}

void CDrawPlayers::SkeletonBacktrack(int index)
{
	if (g_Player[index]->m_bIsLocal)
		return;

	cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(index);

	if (!Game::IsValidEntity(pGameEntity))
		return;

	Vector backtrack;

	if (!Game::GetBacktrackOrigin(pGameEntity, backtrack))
		return;

	if (!cvars::visuals.esp_player_skeleton || (cvars::visuals.esp_player_skeleton && g_Player[index]->m_vecOrigin != backtrack))
	{
		ImColor color = cvars::visuals.esp_player_skeleton_backtrack_color;

		FadeoutDormant(index, color, g_Player[index]->m_flLastTimeInPVS);

		Vector hitboxes[HITBOX_MAX];

		memcpy(hitboxes, g_Player[index]->m_vecHitbox, sizeof(hitboxes));

		for (auto& hitbox : hitboxes)
			hitbox = backtrack + hitbox - g_Player[index]->m_vecOrigin;

		DrawSkeleton(hitboxes, color);
	}
}

void CDrawPlayers::Hitboxes(int index)
{
	ImColor color = cvars::visuals.esp_player_hitboxes_color;

	FadeoutDormant(index, color, g_Player[index]->m_flLastTimeInPVS);

	switch (cvars::visuals.esp_player_hitboxes)
	{
	case DrawOnlyHead:
		DrawHitbox(g_Player[index]->m_vecHitboxPoints[HITBOX_HEAD], color);
		break;
	case DrawAllHitboxes:
		for (int hitboxnum = 0; hitboxnum < (g_Player[index]->m_bHasShield ? HITBOX_MAX : HITBOX_MAX - 1); hitboxnum++)
			DrawHitbox(g_Player[index]->m_vecHitboxPoints[hitboxnum], color);
	}
}

void CDrawPlayers::HitboxesHitPosition()
{
	static std::vector<HitboxesHitPositionData> hitposition_data[MAX_CLIENTS];

	auto IsPlayer = [](int index)
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
	};

	for (int i = 1; i <= client_state->maxclients; i++)
	{
		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!Game::IsValidEntity(pGameEntity))
			continue;

		if (!IsPlayer(pGameEntity->index))
			continue;

		if (g_Player[pGameEntity->index]->m_bIsInPVS && !g_Player[pGameEntity->index]->m_bIsDead)
		{
			if (g_HitRegister->m_iIndex == pGameEntity->index && client_state->time - g_HitRegister->m_flTimeStamp < 0.05f)
			{
				if (hitposition_data[pGameEntity->index].empty() || hitposition_data[pGameEntity->index].back().timestamp != g_HitRegister->m_flTimeStamp)
				{
					HitboxesHitPositionData data;	

					memcpy(&data.hitbox_points, g_Player[pGameEntity->index]->m_vecHitboxPoints, sizeof(data.hitbox_points));
					data.hitboxnum = g_HitRegister->m_iHitboxNum;
					data.timestamp = g_HitRegister->m_flTimeStamp;

					Vector backtrack;

					if (Game::GetBacktrackOrigin(pGameEntity, backtrack))
					{
						for (int hitboxnum = 0; hitboxnum < HITBOX_MAX - 1; hitboxnum++)
						{
							for (auto& hitbox_points : data.hitbox_points[hitboxnum])
								hitbox_points = backtrack + hitbox_points - g_Player[pGameEntity->index]->m_vecOrigin;
						}
					}

					hitposition_data[pGameEntity->index].push_back(data);
				}
			}
		}

		for (auto& hitposition : hitposition_data[pGameEntity->index])
		{
			if (client_state->time - hitposition.timestamp < cvars::visuals.esp_player_hitboxes_hit_position_time)
			{
				for (int hitboxnum = 0; hitboxnum < HITBOX_MAX - 1; hitboxnum++)		
				{
					DrawHitbox(hitposition.hitbox_points[hitboxnum], hitposition.hitboxnum == hitboxnum
						? cvars::visuals.esp_player_hitboxes_hit_position_color2 
						: cvars::visuals.esp_player_hitboxes_hit_position_color);
				}
			}
			else
				hitposition_data[pGameEntity->index].erase(hitposition_data[pGameEntity->index].begin());
		}
	}
}