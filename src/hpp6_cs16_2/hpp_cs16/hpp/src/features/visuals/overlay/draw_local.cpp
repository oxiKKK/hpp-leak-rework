#include "framework.h"

constexpr auto TOGGLE_STATUS_TIME = 4.0;

ToggleStatusData g_ToggleStatus;

CDrawLocal::CDrawLocal()
	: m_pDefaultCrosshair(std::make_unique<CDefaultCrosshair>())
{
}

static ImVec2 GetPunchPosition()
{
	ImVec2 punchangle_position;

	punchangle_position = GImGui->IO.DisplaySize * 0.5f;

	Vector vecForward, vecPunchangle;

	g_Local->m_QAngles.AngleVectors(&vecForward, NULL, NULL);

	vecPunchangle = g_Local->m_vecEyePos + vecForward * g_Weapon->m_flDistance;

	Vector2D vecScreenPos;

	if (Game::WorldToScreen(vecPunchangle, vecScreenPos))
	{
		punchangle_position.x = GImGui->IO.DisplaySize.x - vecScreenPos.x;
		punchangle_position.y = GImGui->IO.DisplaySize.y - vecScreenPos.y;
	}

	return punchangle_position;
}

void CDrawLocal::Overlay()
{
	if (g_pIGameUI->IsGameUIActive())
		return;

	if (!g_Local->m_bIsDead)
	{
		if (cvars::visuals.esp_other_local_spread_circle)
			SpreadCircle();

		if (cvars::visuals.esp_other_local_aim_fov)
			AimFOV();

		if (cvars::visuals.esp_other_local_recoil_point)
			RecoilPoint();

		if (cvars::visuals.esp_other_local_spread_point)
			SpreadPoint();

		if (cvars::visuals.esp_other_local_toggle_status)
			ToggleStatus();
	}

	if (cvars::visuals.esp_other_local_sniper_crosshair)
		SniperCrosshair();
}

void CDrawLocal::SniperCrosshair()
{
	if (g_Weapon.IsSniper() && !g_Local->m_bIsScoped)
		m_pDefaultCrosshair->Draw();
}

void CDrawLocal::AimFOV()
{
	float flCurrentFOV = g_pLegitBot.get() ? g_pLegitBot->m_flCurrentFOV : g_pRageBot.get() ? cvars::ragebot.raim_fov : 0.f;

	if (flCurrentFOV && flCurrentFOV < g_Local->m_iRealFOV)
	{
		const auto punchangle_position = GetPunchPosition();
		const auto radius = tan(DEG2RAD(flCurrentFOV * 0.5f)) / tan(DEG2RAD(g_Local->m_iRealFOV * 0.5f)) * GImGui->IO.DisplaySize.x;

		for (float angle = 0.f, step = 1.5f; angle < 360.f; angle += step)
		{
			const float current_angle = angle * IM_PI / 180.f;
			const float next_angle = (angle + step) * IM_PI / 180.f;

			ImVec2 a = punchangle_position;
			ImVec2 b = punchangle_position + ImVec2(cos(current_angle), sin(current_angle)) * radius;
			ImVec2 c = punchangle_position + ImVec2(cos(next_angle), sin(next_angle)) * radius;

			ImVec2 uv_white = ImGui::GetFontTexUvWhitePixel();

			GImGui->CurrentWindow->DrawList->PrimReserve(3, 3);
			GImGui->CurrentWindow->DrawList->PrimVtx(a, uv_white, ImColor(cvars::visuals.esp_other_local_aim_fov_color2));
			GImGui->CurrentWindow->DrawList->PrimVtx(b, uv_white, ImColor(cvars::visuals.esp_other_local_aim_fov_color));
			GImGui->CurrentWindow->DrawList->PrimVtx(c, uv_white, ImColor(cvars::visuals.esp_other_local_aim_fov_color));
		}

		ImColor color = cvars::visuals.esp_other_local_aim_fov_color;

		g_pRenderer->AddCircle(punchangle_position, radius, color, 100, 2);
	}
}

void CDrawLocal::RecoilPoint()
{
	if (g_Local->m_bIsDead)
		return;

	if (!g_Weapon.IsGun())
		return;

	if (client_state->punchangle.IsZero2D())
		return;

	const auto punchangle_position = GetPunchPosition();

	g_pRenderer->AddRectFilled(punchangle_position - 2, punchangle_position + 2, cvars::visuals.esp_other_local_recoil_point_color);
}

void CDrawLocal::SpreadPoint()
{
	if (g_Local->m_bIsDead)
		return;

	if (!g_Weapon.IsGun())
		return;

	if (!g_Weapon->m_flSpread)
		return;

	QAngle QSpreadAngles = g_Local->m_QAngles;

	g_pNoSpread->GetSpreadOffset(g_Weapon->m_iRandomSeed, 1, QSpreadAngles, QSpreadAngles, NOSPREAD_PITCH_YAW);

	Vector vecForward, vecSpread;

	QSpreadAngles.AngleVectors(&vecForward, NULL, NULL);

	vecSpread = g_Local->m_vecEyePos + vecForward * g_Weapon->m_flDistance;

	Vector2D vecScreenPos;

	if (Game::WorldToScreen(vecSpread, vecScreenPos))
	{
		ImVec2 spread_position = GImGui->IO.DisplaySize - ImVec2(vecScreenPos.x, vecScreenPos.y);

		g_pRenderer->AddRectFilled(spread_position - 2, spread_position + 2, cvars::visuals.esp_other_local_spread_point_color);
	}
}

void CDrawLocal::SpreadCircle()
{
	enum { previous, current, calculated, max_count };

	static double radius[max_count], change_timestamp;

	const auto current_radius = g_Weapon->m_flSpread > 0.015 ? 1000.0 * g_Weapon->m_flSpread / g_Local->m_iRealFOV * DEFAULT_FOV : 0.0;
	constexpr auto animation_time = 0.2;

	if (radius[current] != current_radius)
	{
		if (change_timestamp + animation_time >= client_state->time)
			radius[previous] = radius[calculated];

		radius[current] = current_radius;
		change_timestamp = client_state->time;
	}

	if (change_timestamp + animation_time >= client_state->time)
	{
		radius[calculated] = static_cast<float>(Math::Interp(change_timestamp, client_state->time,
			change_timestamp + animation_time, radius[previous], radius[current]));
	}
	else
	{
		radius[calculated] = radius[current];
		radius[previous] = radius[current];
	}

	if (radius[calculated] < 5.f)
		return;

	const auto punchangle_position = GetPunchPosition();

	for (float angle = 0.f, step = 1.5f; angle < 360.f; angle += step)
	{
		const float current_angle = angle * IM_PI / 180.f;
		const float next_angle = (angle + step) * IM_PI / 180.f;

		ImVec2 a = punchangle_position;
		ImVec2 b = punchangle_position + ImVec2(cos(current_angle), sin(current_angle)) * static_cast<float>(radius[calculated]);
		ImVec2 c = punchangle_position + ImVec2(cos(next_angle), sin(next_angle)) * static_cast<float>(radius[calculated]);

		ImVec2 uv_white = ImGui::GetFontTexUvWhitePixel();

		GImGui->CurrentWindow->DrawList->PrimReserve(3, 3);
		GImGui->CurrentWindow->DrawList->PrimVtx(a, uv_white, ImColor(cvars::visuals.esp_other_local_spread_circle_color2));
		GImGui->CurrentWindow->DrawList->PrimVtx(b, uv_white, ImColor(cvars::visuals.esp_other_local_spread_circle_color));
		GImGui->CurrentWindow->DrawList->PrimVtx(c, uv_white, ImColor(cvars::visuals.esp_other_local_spread_circle_color));
	}

	ImColor color = cvars::visuals.esp_other_local_spread_circle_color;

	g_pRenderer->AddCircle(punchangle_position, static_cast<float>(radius[calculated]), color, 100, 2);
}

void CDrawLocal::ToggleStatus()
{
	constexpr auto FADEOUT_TIME = 1.0;

	const auto text_size = GImGui->Font->CalcTextSizeA(GImGui->Font->FontSize, FLT_MAX, 0, g_ToggleStatus.name.c_str());

	ImVec2 pos = ImVec2(GImGui->IO.DisplaySize.x * 0.5f, GImGui->IO.DisplaySize.y * 0.25f);

	ImColor color = g_ToggleStatus.status ? cvars::visuals.esp_other_local_toggle_status_color : cvars::visuals.esp_other_local_toggle_status_color2;

	if (g_ToggleStatus.timestamp + (TOGGLE_STATUS_TIME - FADEOUT_TIME) < client_state->time)
	{
		color.value.w = static_cast<float>(Math::Interp(g_ToggleStatus.timestamp + (TOGGLE_STATUS_TIME - FADEOUT_TIME), 
			client_state->time, g_ToggleStatus.timestamp + TOGGLE_STATUS_TIME, 1.00, 0.00));
	}

	ImGui::PushFont(g_pFontList[Verdana_17px]);

	g_pRenderer->AddText(pos, color, FontFlags_CenterX, g_ToggleStatus.status ? "%s: ON" : "%s: OFF", g_ToggleStatus.name.c_str());

	ImGui::PopFont();
}