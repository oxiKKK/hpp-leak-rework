#include "framework.h"

static float impulse_alpha = 0.00f;

static void ImpulseAlpha()
{
	static bool plus_or_minus;

	if (impulse_alpha <= 0.20f || impulse_alpha >= 1.25f)
		plus_or_minus = !plus_or_minus;

	impulse_alpha += plus_or_minus ? 0.016f / 0.01f * g_Local->m_flFrameTime : -0.016f / 0.01f * g_Local->m_flFrameTime;
	impulse_alpha = std::clamp(impulse_alpha, 0.20f, 1.25f);
}

static bool FilterPlayer(int index)
{
	if (g_Player[index]->m_bIsLocal)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_UNASSIGNED)
		return false;

	if (g_Player[index]->m_iTeamNum == TEAM_SPECTATOR)
		return false;

	if (!cvars::visuals.esp_player_out_of_fov_players[0] && g_Player[index]->m_iTeamNum != g_Local->m_iTeamNum)
		return false;

	if (!cvars::visuals.esp_player_out_of_fov_players[1] && g_Player[index]->m_iTeamNum == g_Local->m_iTeamNum)
		return false;

	if (Game::ObservedPlayer(index))
		return false;

	if (!g_Player[index]->m_bIsInPVS && client_state->time - g_Player[index]->m_flHistory > cvars::visuals.esp_dormant_time)
		return false;

	if (g_Player[index]->m_bIsDead)
		return false;

	return true;
}

void CDrawPlayersOutFOV::Draw()
{
	const auto& selection_players = GetSelectionPlayers(FilterPlayer);

	if (selection_players.empty())
		return;

	ImpulseAlpha();

	for (auto& player : selection_players)
	{
		if (CalcScreen(player.index))
			DrawOutFOV(player.index);
	}
}

bool CDrawPlayersOutFOV::CalcScreen(int index)
{
	Vector mins, maxs;

	GetBBMaxsMins(index, mins, maxs);

	Vector2D ScreenTop, ScreenBot;

	return !(Game::WorldToScreen(maxs, ScreenTop) && Game::WorldToScreen(mins, ScreenBot));
}

ImColor CDrawPlayersOutFOV::GetTeamColor(int index)
{
	switch (g_Player[index]->m_iTeamNum)
	{
	case TEAM_TERRORIST: return cvars::visuals.esp_player_out_of_fov_color_t;
	case TEAM_CT:		 return cvars::visuals.esp_player_out_of_fov_color_ct;
	}

	return ImColor();
}

static float GetAngleYaw(const Vector2D& screen, const Vector2D& screen_center)
{
	QAngle angles;

	Math::VectorAngles(Vector(screen.x - screen_center.x, screen_center.y - screen.y, 0), angles);

	return DEG2RAD(angles.Negate().y);
}

static void CalcTrianglePoints(ImVec2(&points)[3], const Vector2D& position, float yaw, float size)
{
	points[0] = ImVec2(position.x - size, position.y - size);
	points[1] = ImVec2(position.x + size + 2, position.y);
	points[2] = ImVec2(position.x - size, position.y + size);

	ImVec2 points_center = ImVec2(points[0].x + points[1].x + points[2].x, points[0].y + points[1].y + points[2].y) / 3.f;

	for (auto& point : points)
	{
		point -= points_center;

		ImVec2 temp = point;

		const float theta = yaw;
		const float c = cos(theta);
		const float s = sin(theta);

		point.x = temp.x * c - temp.y * s;
		point.y = temp.x * s + temp.y * c;

		point += points_center;
	}
}

void CDrawPlayersOutFOV::DrawOutFOV(int index)
{
	Vector origin = g_Player[index]->m_vecOrigin;

	if (cvars::visuals.esp_interpolate_history)
		InterpolateOrigin(index, origin);

	Vector2D screen, screen_center, radius, point;

	Game::WorldToScreen(origin, screen);

	screen_center.x = GImGui->IO.DisplaySize.x * 0.5f;
	screen_center.y = GImGui->IO.DisplaySize.y * 0.5f;

	const float yaw = GetAngleYaw(screen, screen_center);
	const float padding = cvars::visuals.esp_player_out_of_fov_draw_type == ARROWS ? 
		cvars::visuals.esp_player_out_of_fov_size * 2.f : cvars::visuals.esp_player_out_of_fov_size;

	radius.x = cvars::visuals.esp_player_out_of_fov_aspect_ratio ? screen_center.x : screen_center.y;
	radius.y = screen_center.y;
	radius = (radius - padding) / 100.f * cvars::visuals.esp_player_out_of_fov_radius;

	point.x = screen_center.x + radius.x * cos(yaw);
	point.y = screen_center.y + radius.y * sin(yaw);

	ImColor color = GetTeamColor(index);
	
	if (cvars::visuals.esp_player_out_of_fov_impulse_alpha)
		color.value.w = impulse_alpha;
	else
		FadeoutDormant(index, color, g_Player[index]->m_flHistory);

	ImColor color_outline = ImColor(0.00f, 0.00f, 0.00f, color.value.w);

	if (cvars::visuals.esp_player_out_of_fov_draw_type == ARROWS)
	{
		ImVec2 points[3];

		CalcTrianglePoints(points, point, yaw, cvars::visuals.esp_player_out_of_fov_size);
		g_pRenderer->AddTriangleFilled(points[0], points[1], points[2], color);

		if (cvars::visuals.esp_player_out_of_fov_outline)
		{
			CalcTrianglePoints(points, point, yaw, cvars::visuals.esp_player_out_of_fov_size + 1);
			g_pRenderer->AddTriangle(points[0], points[1], points[2], color_outline, 2.f);
		}
	}
	else
	{
		if (cvars::visuals.esp_player_out_of_fov_draw_type == CIRCLES)
		{
			g_pRenderer->AddCircleFilled(ImVec2(point.x, point.y), cvars::visuals.esp_player_out_of_fov_size, color, 50);

			if (cvars::visuals.esp_player_out_of_fov_outline)
				g_pRenderer->AddCircle(ImVec2(point.x, point.y), cvars::visuals.esp_player_out_of_fov_size + 1, color_outline, 50);
		}
		else if (cvars::visuals.esp_player_out_of_fov_draw_type == RHOMBUS)
		{
			g_pRenderer->AddCircleFilled(ImVec2(point.x, point.y), cvars::visuals.esp_player_out_of_fov_size, color, 4);

			if (cvars::visuals.esp_player_out_of_fov_outline)
				g_pRenderer->AddCircle(ImVec2(point.x, point.y), cvars::visuals.esp_player_out_of_fov_size + 1, color_outline, 4);
		}

		if (cvars::visuals.esp_player_out_of_fov_additional_info[0])
			DrawName(index, point);

		if (cvars::visuals.esp_player_out_of_fov_additional_info[1])
			DrawWeapon(index, point);		

		if (cvars::visuals.esp_player_out_of_fov_additional_info[2])
			DrawDistance(index, point);
	}
}

static float GetNextlineText(int index, bool& cond)
{
	float nextline = 0.f;

	if (&cond == &cvars::visuals.esp_player_out_of_fov_additional_info[0])
		return nextline;

	if (cond == cvars::visuals.esp_player_out_of_fov_additional_info[0])
		nextline += GImGui->Font->FontSize;

	if (strnlen_s(g_Player[index]->m_szWeaponModelName, MAX_MODEL_NAME))
	{
		if (&cond == &cvars::visuals.esp_player_out_of_fov_additional_info[1])
			return nextline;

		if (cond == cvars::visuals.esp_player_out_of_fov_additional_info[1])
			nextline += GImGui->Font->FontSize;
	}

	if (&cond == &cvars::visuals.esp_player_out_of_fov_additional_info[2])
		return nextline;

	if (cond == cvars::visuals.esp_player_out_of_fov_additional_info[2])
		nextline += GImGui->Font->FontSize;

	return nextline;
}

static ImVec2 GetPosition(int index, const Vector2D& point, bool& cond)
{
	ImVec2 position = ImVec2(point.x, point.y);

	position.y += cvars::visuals.esp_player_out_of_fov_size;
	position.y += GetNextlineText(index, cond) + 2.f;

	return position;
}

void CDrawPlayersOutFOV::DrawName(int index, const Vector2D& point)
{
	ImVec2 position = GetPosition(index, point, cvars::visuals.esp_player_out_of_fov_additional_info[0]);

	ImColor color_text = cvars::visuals.esp_player_name_color;

	FadeoutDormant(index, color_text, g_Player[index]->m_flHistory);

	if (strnlen_s(g_Player[index]->m_szPrintName, sizeof(MAX_PLAYER_NAME_LENGTH)))
		g_pRenderer->AddText(position, color_text, FontFlags_CenterX | FontFlags_Shadow, "%s", g_Player[index]->m_szPrintName);
}

void CDrawPlayersOutFOV::DrawWeapon(int index, const Vector2D& point)
{
	if (!strnlen_s(g_Player[index]->m_szWeaponModelName, MAX_MODEL_NAME))
		return;

	const int weaponindex = Game::GetWeaponIndex(g_Player[index]->m_szWeaponModelName);

	if (!weaponindex)
		return;

	std::string weaponname = Game::GetWeaponName(weaponindex);

	if (weaponname.empty())
		return;

	ImVec2 position = GetPosition(index, point, cvars::visuals.esp_player_out_of_fov_additional_info[1]);

	ImColor color_text = cvars::visuals.esp_player_weapon_text_color;

	FadeoutDormant(index, color_text, g_Player[index]->m_flHistory);

	g_pRenderer->AddText(position, color_text, FontFlags_CenterX | FontFlags_Shadow, weaponname.c_str());
}

extern std::string GetDistanceString(int index);

void CDrawPlayersOutFOV::DrawDistance(int index, const Vector2D& point)
{
	std::string distance = GetDistanceString(index);

	ImVec2 position = GetPosition(index, point, cvars::visuals.esp_player_out_of_fov_additional_info[2]);

	ImColor color_text = cvars::visuals.esp_player_weapon_text_color;

	FadeoutDormant(index, color_text, g_Player[index]->m_flHistory);

	g_pRenderer->AddText(position, color_text, FontFlags_CenterX | FontFlags_Shadow, distance.c_str());
}