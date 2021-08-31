#include "main.h"

CKnifebot g_Knifebot;

void CKnifebot::Run(usercmd_s* cmd)
{
	if (!cvar.knifebot_enabled)
		return;

	if (!g_Weapon.IsKnife())
		return;

	if (cvar.knifebot_key_toggle && !state.knifebot_active)
		return;

	if (!g_Weapon.CanAttack())
		return;

	auto distance = (cvar.knifebot_type == 1) ? cvar.knifebot_distance_slash : cvar.knifebot_distance_stab;

	if (distance <= 16.f)
		return;

	auto hull = (g_Weapon.data.m_iFlags & FL_DUCKING) ? HULL_PLAYER_DUCKED : HULL_PLAYER_REGULAR;

	//if (pmove->movetype == MOVETYPE_FLY)
	//	hull = HULL_PLAYER_DUCKED;

	Vector vecSrc = g_Local.m_vecEye;

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, true);

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (i == g_Local.m_iIndex)
			continue;

		const auto pPlayer = g_World.GetPlayer(i);

		if (!pPlayer)
			continue;

		if (!cvar.knifebot_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS && !g_Miscellaneous.m_bFakeLatencyActive)
			continue;

		if (cvar.player_list && pPlayer->m_iPlayerListType == 1)
			continue;

		if (cvar.knifebot_dont_shoot_spectators && pPlayer->m_iTeam != CT && pPlayer->m_iTeam != TERRORIST)
			continue;

		if (cvar.knifebot_dont_shoot_in_back)
		{
			Vector vecMyForward(g_Local.m_vecForward), vecPlayerForward;

			g_Engine.pfnAngleVectors(pPlayer->m_vecAngles, vecPlayerForward, NULL, NULL);

			vecMyForward.z = 0;
			vecPlayerForward.z = 0;

			vecMyForward.Normalize();
			vecPlayerForward.Normalize();

			float flDot = vecPlayerForward.Dot2D(vecMyForward);

			if (flDot > 0.80f)
				continue;
		}

		Vector vecCorrectedOrigin(pPlayer->m_vecOrigin);

		if (g_Miscellaneous.m_bFakeLatencyActive || cvar.knifebot_position_adjustment)
		{
			auto best_lerp = -1;

			if (cvar.knifebot_position_adjustment)
			{
				auto best_dist = FLT_MAX;

				for (short x = 0; x <= 100; x++)
				{
					Vector vecPositionAdjustmentOrigin(vecCorrectedOrigin);

					g_Utils.LagCompensation(pPlayer->m_iIndex, x, vecPositionAdjustmentOrigin);

					auto dist = pmove->origin.Distance(vecPositionAdjustmentOrigin);

					if (dist < best_dist)
					{
						best_lerp = x;
						best_dist = dist;
						vecCorrectedOrigin = vecPositionAdjustmentOrigin;
					}
				}

				g_Miscellaneous.m_bPositionAdjustmentActive = true;
				g_Miscellaneous.m_sPositionAdjustmentLerp = best_lerp;
			}
			else
				g_Utils.LagCompensation(pPlayer->m_iIndex, best_lerp, vecCorrectedOrigin);
		}
		
		Vector vecPrediction = pPlayer->m_vecVelocity * g_Globals.m_flFrameTime;

		vecCorrectedOrigin += vecPrediction;

		auto fov = g_Local.m_vecForward.AngleBetween(vecCorrectedOrigin - vecSrc);

		if (fov > cvar.knifebot_fov)
			continue;

		{
			Vector vecBackupOrigin(pPlayer->m_vecOrigin);

			pPlayer->m_vecOrigin = vecCorrectedOrigin;

			g_Utils.SpoofPhysent(pPlayer->m_iIndex, false);

			pPlayer->m_vecOrigin = vecBackupOrigin;
		}

		int iLogHitbox = -1;
		bool bAttack = false;

		if (cvar.knifebot_aim_perfect_silent || cvar.knifebot_aim_silent)
		{
			auto best_fraction = 1.f;
			auto best_point = Vector();
			auto best_hitbox = -1;
			auto best_aim_angles = QAngle();

			for (auto hitbox = 0; hitbox < HITBOX_MAX; hitbox++)
			{
				if (hitbox == HITBOX_SHIELD)
					continue;

				if (cvar.knifebot_aim_hitbox == 0 && hitbox != HITBOX_HEAD)
					continue;

				for (auto point = 0; point < HITBOX_POINTS_MAX; point++)
				{
					Vector vecHitbox = vecCorrectedOrigin + pPlayer->m_vecHitbox[hitbox] - pPlayer->m_vecOrigin;
					Vector vecPoint = vecHitbox + ((pPlayer->m_vecHitboxPoints[hitbox][point] - pPlayer->m_vecHitbox[hitbox]) * (cvar.knifebot_hitbox_scale / 100.f));

					Vector vecForward;
					QAngle QAimAngles;

					g_Utils.VectorAngles(vecPoint - vecSrc, QAimAngles);

					QAimAngles.AngleVectors(&vecForward, NULL, NULL);

					//Vector vecAim = ((vecPoint - vecSrc) / Vector(vecPoint - vecSrc).Length());
					Vector vecEnd = vecSrc + vecForward * distance;

					pmtrace_s tr;
					g_Engine.pEventAPI->EV_SetTraceHull(hull);
					g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

					if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) == pPlayer->m_iIndex && tr.fraction < best_fraction)
					{
						best_fraction = tr.fraction;
						best_point = vecPoint;
						best_hitbox = hitbox;
						best_aim_angles = QAimAngles;
					}
				}
			}

			if (best_hitbox != -1)
			{
				g_Utils.MakeAngle(false, best_aim_angles, cmd);

				if (cvar.knifebot_aim_perfect_silent)
					g_Globals.m_bSendCommand = false;

				bAttack = true;

				iLogHitbox = best_hitbox;
			}
		}
		else 
		{
			Vector vecEnd = vecSrc + g_Local.m_vecForward * distance;

			pmtrace_s tr;
			g_Engine.pEventAPI->EV_SetTraceHull(hull);
			g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

			if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) == pPlayer->m_iIndex)
				bAttack = true;
		}

		if (bAttack)
		{
			cmd->buttons |= (cvar.knifebot_type == 0) ? IN_ATTACK2 : IN_ATTACK;

			if (cvar.screen_log)
			{
				std::string sLogMode;

				if (vecPrediction.Length() > 0.f)
					sLogMode.append(sLogMode.length() ? " + prediction" : "prediction");

				if (g_Miscellaneous.m_bFakeLatencyActive)
					sLogMode.append(sLogMode.length() ? " + backtrack" : "backtrack");

				if (g_Miscellaneous.m_bPositionAdjustmentActive)
				{
					sLogMode.append(sLogMode.length() ? " + lagcompensation " : "lagcompensation ");
					sLogMode.append(std::to_string(g_Miscellaneous.m_sPositionAdjustmentLerp));
					sLogMode.append(" tick");
				}

				if (!sLogMode.length())
					sLogMode.append("none");

				g_ScreenLog.Log(ImColor(IM_COL32_WHITE), "Shoot %s in %s from %s (mode: %s)", pPlayer->m_szPrintName, g_Utils.GetHitboxNameByIndex(iLogHitbox), g_Utils.GetWeaponNameByIndex(g_Weapon.data.m_iWeaponID), sLogMode.c_str());
			}

			goto kb_restore_players;
		}
	}

kb_restore_players:

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, false);
}