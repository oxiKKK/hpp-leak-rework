#include "framework.h"

std::unique_ptr<CKnifeBot> g_pKnifeBot;

CKnifeBot::CKnifeBot()
{
	// run code one time when you connected to server
}

CKnifeBot::~CKnifeBot()
{
	// run code one time when you disconnected from server
}

void CKnifeBot::Run(usercmd_s* cmd)
{
	if (!cvars::misc.kb_enabled)
		return;

	if (!g_Weapon.IsKnife())
		return;

	if (!g_Weapon.CanAttack())
		return;

	float flDistance = (cvars::misc.kb_attack_type == 0) ? cvars::misc.kb_swing_distance : cvars::misc.kb_stab_distance;

	if (flDistance <= 16.f)
		return;

	Vector vecSrc(g_Local->m_vecEyePos), vecSpreadDir;

	{
		QAngle QAngles(cmd->viewangles);

		QAngles.Normalize();

		QAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize(); 
	}

	std::deque<int> hitboxes;

	if (cvars::misc.kb_aim_hitbox[0])
		hitboxes.push_back(HITBOX_HEAD);

	if (cvars::misc.kb_aim_hitbox[1])
		hitboxes.push_back(HITBOX_NECK);

	if (cvars::misc.kb_aim_hitbox[2])
	{
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
	}

	if (cvars::misc.kb_aim_hitbox[3])
		hitboxes.push_back(HITBOX_STOMACH);

	if (cvars::misc.kb_aim_hitbox[4])
	{
		hitboxes.push_back(HITBOX_LEFT_UPPER_ARM);
		hitboxes.push_back(HITBOX_LEFT_HAND);
		hitboxes.push_back(HITBOX_LEFT_FOREARM);
		hitboxes.push_back(HITBOX_LEFT_WRIST);
		hitboxes.push_back(HITBOX_RIGHT_UPPER_ARM);
		hitboxes.push_back(HITBOX_RIGHT_HAND);
		hitboxes.push_back(HITBOX_RIGHT_FOREARM);
		hitboxes.push_back(HITBOX_RIGHT_WRIST);
	}

	if (cvars::misc.kb_aim_hitbox[5])
	{
		hitboxes.push_back(HITBOX_LEFT_FOOT);
		hitboxes.push_back(HITBOX_LEFT_CALF);
		hitboxes.push_back(HITBOX_LEFT_THIGH);
		hitboxes.push_back(HITBOX_RIGHT_FOOT);
		hitboxes.push_back(HITBOX_RIGHT_CALF);
		hitboxes.push_back(HITBOX_RIGHT_THIGH);
	}

	for (int i = 1; i <= client_state->maxclients; i++)
	{
		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!Game::IsValidEntity(pGameEntity))
			continue;

		if (g_Player[i]->m_bIsLocal)
			continue;

		if (!g_Player[i]->m_bIsConnected)
			continue;

		if (g_Player[i]->m_bIsDead)
			continue;

		if (!g_Player[i]->m_bIsInPVS)
			continue;

		if (!cvars::misc.kb_friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			continue;

		if (cvars::misc.kb_conditions[0] && g_Player[i]->m_iTeamNum != TEAM_CT && g_Player[i]->m_iTeamNum != TEAM_TERRORIST)
			continue;

		if (cvars::misc.kb_conditions[1])
		{
			Vector vecMyForward(vecSpreadDir), vecPlayerForward;

			g_Player[i]->m_QAngles.AngleVectors(&vecPlayerForward, NULL, NULL);

			vecMyForward.z = 0;
			vecPlayerForward.z = 0;

			vecMyForward.Normalize();
			vecPlayerForward.Normalize();

			float flDot = vecPlayerForward.Dot2D(vecMyForward);

			if (flDot > 0.80f)
				continue;
		}

		Vector vecTempAdjustedOrigin(g_Player[i]->m_vecOrigin);

		if (cvars::misc.kb_position_adjustment)
		{
			auto best_lerp_msec = -1;
			auto best_distance = FLT_MAX;

			for (int lerp_msec = 0; lerp_msec <= 100; lerp_msec++)
			{
				Vector vecTempOrigin;

				if (Game::BacktrackPlayer(pGameEntity, lerp_msec, vecTempOrigin))
				{
					const auto distance = vecTempOrigin.Distance(g_Local->m_vecOrigin);

					if (distance < best_distance)
					{
						best_lerp_msec = lerp_msec;
						best_distance = distance;
					}
				}
			}

			if (best_lerp_msec != -1)
			{
				const auto interp_amount = g_pMiscellaneous->GetInterpAmount(best_lerp_msec);
				const auto lerp_msec = (int)(interp_amount * 1000.0);

				if (Game::BacktrackPlayer(pGameEntity, lerp_msec, vecTempAdjustedOrigin))
				{
					g_pMiscellaneous->m_bPositionAdjustmentActive = true;
					g_pMiscellaneous->m_flPositionAdjustmentInterpAmount = interp_amount;
				}
			}
			else if (g_pMiscellaneous.get() && g_pMiscellaneous->m_bFakeLatencyActive)
				Game::BacktrackPlayer(pGameEntity, -1, vecTempAdjustedOrigin);
		}
		else if (g_pMiscellaneous.get() && g_pMiscellaneous->m_bFakeLatencyActive)
			Game::BacktrackPlayer(pGameEntity, -1, vecTempAdjustedOrigin);

		vecTempAdjustedOrigin += Game::PredictPlayer(i);

		float flFOV = vecSpreadDir.AngleBetween(vecTempAdjustedOrigin - vecSrc);

		if (flFOV > cvars::misc.kb_fov)
			continue;

		bool bAttack = false;

		CorrectPhysentSolid(i);

		Physent::SetOrigin(i, vecTempAdjustedOrigin);

		if (cvars::misc.kb_aim_type && !hitboxes.empty())
		{
			QAngle QBestAimAngles;

			float flBestFraction = 1.f;

			for (auto hitbox : hitboxes)
			{
				for (auto point = 0; point < HITBOX_POINTS_MAX; point++)
				{
					QAngle QAimAngles;

					Vector vecForward, vecAimOrigin, vecHitbox(vecTempAdjustedOrigin + g_Player[i]->m_vecHitbox[hitbox] - g_Player[i]->m_vecOrigin);

					vecAimOrigin = vecHitbox + ((g_Player[i]->m_vecHitboxPoints[hitbox][point] - g_Player[i]->m_vecHitbox[hitbox]) * (cvars::misc.kb_aim_hitbox_scale / 100.f));

					Math::VectorAngles(vecAimOrigin - vecSrc, QAimAngles);

					QAimAngles.AngleVectors(&vecForward, NULL, NULL);

					pmtrace_s pmTrace;
					g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
					g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + vecForward * flDistance, PM_NORMAL, -1, &pmTrace);

					if (g_Engine.pEventAPI->EV_IndexFromTrace(&pmTrace) == g_Player[i]->m_iEntIndex && pmTrace.fraction < flBestFraction)
					{
						QBestAimAngles = QAimAngles;
						flBestFraction = pmTrace.fraction;
					}
				}
			}

			//g_Engine.Con_NPrintf(7, "flBestFraction: %f", flBestFraction);

			if (flBestFraction < 1.f)
			{
				Game::MakeAngle(QBestAimAngles, cmd);
				
				if (cvars::misc.kb_aim_type == 1)
					g_Engine.SetViewAngles(QBestAimAngles);
				else if (cvars::misc.kb_aim_type == 3 && g_pMiscellaneous->m_iChokedCommands < g_pGlobals->m_nNewCommands)
					Game::SendCommand(false);

				bAttack = true;
			}
		}
		else
		{
			pmtrace_t pmTrace;
			g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
			g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + vecSpreadDir * flDistance, PM_NORMAL, -1, &pmTrace);

			if (g_Engine.pEventAPI->EV_IndexFromTrace(&pmTrace) == g_Player[i]->m_iEntIndex)
				bAttack = true;
		}

		if (bAttack)
		{
			bool bBlock = false;

			if (cvars::misc.kb_conditions[2] && Game::TraceShield(vecSrc, vecTempAdjustedOrigin, cmd->viewangles, i, flDistance))
				bBlock = true;

			if (!bBlock)
			{
				cmd->buttons |= (cvars::misc.kb_attack_type == 0) ? IN_ATTACK : IN_ATTACK2;
				return;
			}
		}
	}
}

void CKnifeBot::CorrectPhysentSolid(const int& nPlayerID)
{
	assert(nPlayerID >= 1 && nPlayerID <= MAX_CLIENTS);

	for (int i = 1; i <= client_state->maxclients; i++)
	{
		if (g_Player[i]->m_bIsLocal)
			continue;

		if (!g_Player[i]->m_bIsConnected)
			continue;

		if (g_Player[i]->m_bIsDead)
			continue;

		if (!g_Player[i]->m_bIsInPVS)
			continue;

		if (i == nPlayerID)
		{
			Physent::SetSolid(i, SOLID_BBOX);
			continue;
		}

		if (!cvars::misc.kb_friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			Physent::SetSolid(i, SOLID_BBOX);
		else
			Physent::SetSolid(i, SOLID_NOT);
	}
}