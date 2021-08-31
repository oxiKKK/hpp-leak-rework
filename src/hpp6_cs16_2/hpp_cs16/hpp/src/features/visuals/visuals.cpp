#include "framework.h"

CVisuals::CVisuals()
	: m_pChamsPlayers(std::make_unique<CChamsPlayers>())
	, m_pChamsHitPosition(std::make_unique<CChamsHitPosition>())
	, m_pChamsDesyncAA(std::make_unique<CChamsDesyncAA>())
	, m_pChamsBacktrack(std::make_unique<CChamsBacktrack>())
	, m_pChamsHands(std::make_unique<CChamsHands>())
	, m_pDlight(std::make_unique<CDlight>())
	, m_pElight(std::make_unique<CElight>())
	, m_pDrawPlayers(std::make_unique<CDrawPlayers>())
	, m_pGlowPlayers(std::make_unique<CGlowPlayers>())
	, m_pDrawWorld(std::make_unique<CDrawWorld>())
	, m_pDrawLocal(std::make_unique<CDrawLocal>())
{
}

void CVisuals::CreateEntities()
{
	if (cvars::visuals.colored_models_players_desync_aa)
		m_pChamsDesyncAA->CreateDesyncAA();

	if (cvars::visuals.colored_models_players_hit_position)
		m_pChamsHitPosition->CreateHitPositionPlayers();

	if (cvars::visuals.colored_models_players)
	{
		if (cvars::visuals.streamer_mode)
			m_pChamsPlayers->CreatePlayers();

		if (!cvars::visuals.streamer_mode)
			m_pChamsPlayers->CreateDormantPlayers();
	}

	if (cvars::visuals.colored_models_backtrack)
		m_pChamsBacktrack->CreateBacktrackPlayers();
}

bool CVisuals::Chams(cl_entity_s* pGameEntity)
{
	auto StreamerEdict = [](cl_entity_s* pGameEntity)
	{
		if (pGameEntity->trivial_accept == BACKTRACK_MARKER)
			return true;

		if (pGameEntity->trivial_accept == PLAYER_MARKER)
			return true;

		if (pGameEntity->trivial_accept == LOCAL_DESYNC_MARKER)
			return true;

		if (pGameEntity->trivial_accept == HIT_POSITION_MARKER)
			return true;

		return false;
	};

	if (g_Local->m_iObserverState >= OBS_MAP_FREE)
		return false;

	if (cvars::visuals.streamer_mode && !StreamerEdict(pGameEntity))
		return false;

	if (pGameEntity->trivial_accept == LOCAL_DESYNC_MARKER)
	{
		if (cvars::visuals.colored_models_players_desync_aa)
		{
			if (m_pChamsDesyncAA->Draw(pGameEntity))
				return true;
		}
	}
	else
	{
		if (pGameEntity->trivial_accept == HIT_POSITION_MARKER)
		{
			if (cvars::visuals.colored_models_players_hit_position)
			{
				if (m_pChamsHitPosition->Draw(pGameEntity))
					return true;
			}
		}
		else if (pGameEntity->trivial_accept == BACKTRACK_MARKER)
		{
			if (cvars::visuals.colored_models_backtrack)
			{
				if (m_pChamsBacktrack->Draw(pGameEntity))
					return true;
			}
		}
		else
		{
			if (cvars::visuals.colored_models_players)
			{
				if (m_pChamsPlayers->Draw(pGameEntity))
					return true;
			}

			if (cvars::visuals.colored_models_hands)
			{
				if (m_pChamsHands->Draw(pGameEntity))
					return true;
			}
		}
	}

	return false;
}

bool CVisuals::Glow(cl_entity_s* pGameEntity)
{
	return cvars::visuals.esp_player_glow && m_pGlowPlayers->Draw(pGameEntity);
}

void CVisuals::Light()
{
	if (cvars::visuals.colored_models_dlight)
		m_pDlight->Draw();

	if (cvars::visuals.colored_models_elight)
		m_pElight->Draw();
}

void CVisuals::LineSight()
{
	if (cvars::visuals.esp_player && cvars::visuals.esp_player_line_of_sight)
		m_pDrawPlayers->LineSight();
}

static void cancer(int куда, ImColor цвет)
{
	Vector2D screen;

	const auto screen_width = GImGui->IO.DisplaySize.x;
	const auto screen_height = GImGui->IO.DisplaySize.y;

	screen.x = screen_width;
	screen.y = screen_height;

	const auto screen_center = Vector2D(screen_width * 0.5f, screen_height * 0.5f);
	const auto forward = Vector(screen.x - screen_center.x, screen_center.y - screen.y, 0);

	QAngle angles;

	Math::VectorAngles(forward, angles);

	const auto yaw = DEG2RAD(куда ? 0 : 180);
	const auto radius_x = (screen_center.x - 30) / 100.f * 30.f;
	const auto radius_y = (screen_center.y - 30) / 100.f * 30.f;

	const auto new_point_x = screen_center.x - radius_x * cos(yaw);
	const auto new_point_y = screen_center.y - radius_y * sin(yaw);

	if (куда)
	{
		ImVec2 points[] =
		{
			ImVec2(new_point_x + 10, new_point_y - 10),
			ImVec2(new_point_x - 15, new_point_y),
			ImVec2(new_point_x + 10, new_point_y + 10)
		};

		g_pRenderer->AddTriangleFilled(points[0], points[1], points[2], цвет);
	}
	else
	{
		ImVec2 points[] =
		{
			ImVec2(new_point_x - 10, new_point_y - 10),
			ImVec2(new_point_x + 15, new_point_y),
			ImVec2(new_point_x - 10, new_point_y + 10)
		};

		g_pRenderer->AddTriangleFilled(points[0], points[1], points[2], цвет);
	}	
}

void CVisuals::Overlay()
{
	m_pDrawWorld->Overlay();
	m_pDrawPlayers->Overlay();
	m_pDrawLocal->Overlay();

	// перенеси меня рома
	if (!g_Local->m_bIsDead && cvars::visuals.esp_other_local_aa_side_arrows && cvars::ragebot.active && cvars::ragebot.aa_enabled && cvars::ragebot.aa_stand_desync && !g_pGlobals->m_flGaitMovement)
	{
		if (cvars::ragebot.aa_side)
		{
			cancer(0, ImColor(100, 255, 100));
			cancer(1, ImColor(100, 100, 100));
		}
		else {
			cancer(0, ImColor(100, 100, 100));
			cancer(1, ImColor(100, 255, 100));
		}
	}
}

std::unique_ptr<CVisuals> g_pVisuals;