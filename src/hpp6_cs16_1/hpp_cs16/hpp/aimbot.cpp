#include "main.h"

CAimbot g_Aimbot;

CTarget::CTarget() :
	m_iScore(0),
	m_iHitbox(-1),
	m_iPoint(-1),
	m_vecCorrectedOrigin(),
	m_pPlayer(nullptr)
{
}

void CAimbot::Run(usercmd_s* cmd)
{
	if (!g_Weapon.IsGun())
		return;

	if (cvar.legitbot_active)
	{
		auto flashed_percentage = (g_Miscellaneous.m_flFlashed / 255.f) * 100.f;

		if (flashed_percentage > cvar.legitbot_flashed_limit)
			return;

		_Trigger(cmd);
		_LegitBot(cmd);
	}
	else if (cvar.ragebot_active)
	{
		_RageBot(cmd);
	}
}

int GetScoreOfHitbox(int a_nHitbox)
{
	switch (a_nHitbox) {
	case HITBOX_HEAD:
		return 8;
		break;
	case HITBOX_NECK:
	case HITBOX_LOWER_CHEST:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		return 6;
	case HITBOX_STOMACH:
		return 4;
		break;
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_HAND:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_WRIST:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_HAND:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_WRIST:
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_THIGH:
		return 2;
		break;
	}

	return 1;
}

void CAimbot::_RageBot(usercmd_s* cmd)
{
	if (!cvar.ragebot_aim_enabled)
		return;

	if (!g_Weapon.CanAttack())
		return;

	if (cvar.ragebot_tapping_mode && client_state->punchangle.Length2D() > 0.f)
		return;

	if (!cvar.ragebot_aim_auto_fire && !(cmd->buttons & IN_ATTACK))
		return;

	std::deque<int> hitboxes;

	if (cvar.ragebot_aim_hitbox == 0)
		hitboxes.push_back(HITBOX_HEAD);
	else if (cvar.ragebot_aim_hitbox == 1)
	{
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
		hitboxes.push_back(HITBOX_NECK);
	}
	else if (cvar.ragebot_aim_hitbox == 2)
		hitboxes.push_back(HITBOX_STOMACH);
	else if (cvar.ragebot_aim_hitbox == 3) {
		hitboxes.push_back(HITBOX_HEAD);
		hitboxes.push_back(HITBOX_NECK);
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
		hitboxes.push_back(HITBOX_STOMACH);
		hitboxes.push_back(HITBOX_LEFT_UPPER_ARM);
		hitboxes.push_back(HITBOX_LEFT_HAND);
		hitboxes.push_back(HITBOX_LEFT_FOREARM);
		hitboxes.push_back(HITBOX_LEFT_WRIST);
		hitboxes.push_back(HITBOX_RIGHT_UPPER_ARM);
		hitboxes.push_back(HITBOX_RIGHT_HAND);
		hitboxes.push_back(HITBOX_RIGHT_FOREARM);
		hitboxes.push_back(HITBOX_RIGHT_WRIST);
		hitboxes.push_back(HITBOX_LEFT_FOOT);
		hitboxes.push_back(HITBOX_LEFT_CALF);
		hitboxes.push_back(HITBOX_LEFT_THIGH);
		hitboxes.push_back(HITBOX_RIGHT_FOOT);
		hitboxes.push_back(HITBOX_RIGHT_CALF);
		hitboxes.push_back(HITBOX_RIGHT_THIGH);
	}
	else if (cvar.ragebot_aim_hitbox == 4) {
		hitboxes.push_back(HITBOX_HEAD);
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
		hitboxes.push_back(HITBOX_NECK);
		hitboxes.push_back(HITBOX_STOMACH);
	}

	if (hitboxes.empty())
		return;

	std::deque<CTarget> targets;

	auto best_target = CTarget();
	bool target_found = false;

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, true);

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (i == g_Local.m_iIndex)
			continue;

		const auto pPlayer = g_World.GetPlayer(i);

		if (!pPlayer)
			continue;

		if (!cvar.ragebot_aim_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS && !g_Miscellaneous.m_bFakeLatencyActive)
			continue;

		if (cvar.player_list && pPlayer->m_iPlayerListType == 1)
			continue;

		if (cvar.ragebot_delay_shot)
		{
			auto delta = pPlayer->m_vecOrigin - pPlayer->m_vecPreviousOrigin;

			if (delta.LengthSqr() > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR)
				continue;
		}

		Vector vecCorrectedOrigin(pPlayer->m_vecOrigin);

		if (g_Miscellaneous.m_bFakeLatencyActive)
			g_Utils.LagCompensation(pPlayer->m_iIndex, -1, vecCorrectedOrigin);

		auto fov = g_Local.m_vecForward.AngleBetween(vecCorrectedOrigin - g_Local.m_vecEye);

		if (fov > cvar.ragebot_aim_fov)
			continue;

		{
			Vector vecBackupOrigin(pPlayer->m_vecOrigin);

			pPlayer->m_vecOrigin = vecCorrectedOrigin;

			g_Utils.SpoofPhysent(pPlayer->m_iIndex, false);

			pPlayer->m_vecOrigin = vecBackupOrigin;
		}

		auto new_target = CTarget();
		new_target.m_pPlayer = pPlayer;
		new_target.m_vecCorrectedOrigin = vecCorrectedOrigin;

		targets.push_back(new_target);
	}

	for (auto&& target : targets)
	{
		for (auto&& hitbox : hitboxes)
		{
			Vector vecAimOrigin = target.m_vecCorrectedOrigin + target.m_pPlayer->m_vecHitbox[hitbox] - target.m_pPlayer->m_vecOrigin;

			pmtrace_s tr;
			g_Engine.pEventAPI->EV_SetTraceHull(2);

			if (cvar.ragebot_resolver)
			{
				g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecAimOrigin, PM_WORLD_ONLY, -1, &tr);

				if (tr.fraction == 1.f && !g_Engine.pEventAPI->EV_IndexFromTrace(&tr))
				{
					target.m_iHitbox = hitbox;
					target.m_iScore += GetScoreOfHitbox(hitbox);
					target_found = true;
					break;
				}
			}
			else
			{
				g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecAimOrigin, PM_NORMAL, -1, &tr);

				if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) == target.m_pPlayer->m_iIndex)
				{
					target.m_iHitbox = hitbox;
					target.m_iScore += GetScoreOfHitbox(hitbox);
					target_found = true;
					break;
				}
			}
		}
	}

	if (!target_found)
	{
		if (cvar.ragebot_aim_multipoint)
		{
			for (auto&& target : targets)
			{
				for (auto&& hitbox : hitboxes)
				{
					if ((cvar.ragebot_aim_multipoint == 1 && hitbox == hitboxes[0]) || cvar.ragebot_aim_multipoint > 1)
					{
						Vector vecHitbox = target.m_vecCorrectedOrigin + target.m_pPlayer->m_vecHitbox[hitbox] - target.m_pPlayer->m_vecOrigin;

						for (auto point = 0; point < IM_ARRAYSIZE(target.m_pPlayer->m_vecHitboxPoints[hitbox]); ++point)
						{
							Vector vecHitbox = target.m_vecCorrectedOrigin + target.m_pPlayer->m_vecHitbox[hitbox] - target.m_pPlayer->m_vecOrigin;
							Vector vecAimOrigin = vecHitbox + ((target.m_pPlayer->m_vecHitboxPoints[hitbox][point] - target.m_pPlayer->m_vecHitbox[hitbox]) * (cvar.ragebot_aim_multipoint_scale / 100.f));

							pmtrace_s tr;
							g_Engine.pEventAPI->EV_SetTraceHull(2);

							if (cvar.ragebot_resolver)
							{
								g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecAimOrigin, PM_WORLD_ONLY, -1, &tr);

								if (tr.fraction == 1.f && !g_Engine.pEventAPI->EV_IndexFromTrace(&tr))
								{
									target.m_iHitbox = hitbox;
									target.m_iPoint = point;
									target.m_iScore += GetScoreOfHitbox(hitbox);
									target_found = true;
									goto next_check;
								}
							}
							else
							{
								g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecAimOrigin, PM_NORMAL, -1, &tr);

								if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) == target.m_pPlayer->m_iIndex)
								{
									target.m_iHitbox = hitbox;
									target.m_iPoint = point;
									target.m_iScore += GetScoreOfHitbox(hitbox);
									target_found = true;
									goto next_check;
								}
							}
						}
					}
				}
			}
		}

	next_check:

		if (!target_found && cvar.ragebot_aim_auto_penetration)
		{
			for (auto&& target : targets)
			{
				for (auto&& hitbox : hitboxes)
				{
					Vector vecAimOrigin = target.m_vecCorrectedOrigin + target.m_pPlayer->m_vecHitbox[hitbox] - target.m_pPlayer->m_vecOrigin;

					auto damage = g_Utils.SimulateFireBullet(g_Local.m_vecEye, vecAimOrigin, g_Weapon.GetDistance(), g_Weapon.GetPenetration(), g_Weapon.GetBulletType(), g_Weapon.GetDamage(), g_Weapon.GetWallPierce());

					if (damage > 0)
					{
						target.m_iHitbox = hitbox;
						target.m_iScore += GetScoreOfHitbox(hitbox);
						target_found = true;
						break;
					}
				}
			}

			if (!target_found && cvar.ragebot_aim_multipoint == 3)
			{
				for (auto&& target : targets)
				{
					for (auto&& hitbox : hitboxes)
					{
						Vector vecHitbox = target.m_vecCorrectedOrigin + target.m_pPlayer->m_vecHitbox[hitbox] - target.m_pPlayer->m_vecOrigin;

						for (auto point = 0; point < IM_ARRAYSIZE(target.m_pPlayer->m_vecHitboxPoints[hitbox]); ++point)
						{
							Vector vecHitbox = target.m_vecCorrectedOrigin + target.m_pPlayer->m_vecHitbox[hitbox] - target.m_pPlayer->m_vecOrigin;
							Vector vecAimOrigin = vecHitbox + ((target.m_pPlayer->m_vecHitboxPoints[hitbox][point] - target.m_pPlayer->m_vecHitbox[hitbox]) * (cvar.ragebot_aim_multipoint_scale / 100.f));

							auto damage = g_Utils.SimulateFireBullet(g_Local.m_vecEye, vecAimOrigin, g_Weapon.GetDistance(), g_Weapon.GetPenetration(), g_Weapon.GetBulletType(), g_Weapon.GetDamage(), g_Weapon.GetWallPierce());

							if (damage > 0)
							{
								target.m_iHitbox = hitbox;
								target.m_iPoint = point;
								target.m_iScore += GetScoreOfHitbox(hitbox);
								target_found = true;
								goto sorting_targets;
							}
						}
					}
				}
			}
		}
	}

sorting_targets:

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, false);

	if (!target_found)
		return;

	{ //Add points to best target by fov
		CTarget* best_target_by_fov = nullptr;

		auto best_fov = FLT_MAX;

		for (auto&& target : targets)
		{
			auto fov = g_Local.m_vecForward.AngleBetween(target.m_vecCorrectedOrigin - g_Local.m_vecEye);

			if (fov < best_fov)
			{
				best_fov = fov;
				best_target_by_fov = &target;
			}
		}

		if (best_target_by_fov)
			best_target_by_fov->m_iScore += 2;
	}

	if (cvar.player_list) //Add points by priority
	{
		for (auto&& target : targets)
		{
			if (target.m_pPlayer->m_iPlayerListType == 2)
				target.m_iScore += 10;
		}
	}

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, false);

	if (!target_found)
		return;

	for (auto&& target : targets)
	{
		if (target.m_iScore > best_target.m_iScore)
			best_target = target;
	}

	if (best_target.m_pPlayer)
	{
		if (cvar.ragebot_aim_auto_scope && g_Weapon.IsSniper() && g_Local.m_iFOV == DEFAULT_FOV)
		{
			cmd->buttons |= IN_ATTACK2;
			return;
		}

		const auto player = best_target.m_pPlayer;
		const auto hitbox = best_target.m_iHitbox;
		const auto hitbox_point = best_target.m_iPoint;
		const auto corrected_origin = best_target.m_vecCorrectedOrigin;

		Vector vecAimOrigin;

		if (hitbox_point >= 0 && hitbox_point < HITBOX_POINTS_MAX)
			vecAimOrigin = player->m_vecHitboxPoints[hitbox][hitbox_point];
		else
			vecAimOrigin = player->m_vecHitbox[hitbox];

		vecAimOrigin = corrected_origin + vecAimOrigin - player->m_vecOrigin;

		Vector vecPrediction = player->m_vecVelocity * g_Globals.m_flFrameTime;

		vecAimOrigin += vecPrediction;

		QAngle QAimAngles;

		g_Utils.VectorAngles(vecAimOrigin - g_Local.m_vecEye, QAimAngles);

		if (cvar.ragebot_remove_recoil)
		{
			QAimAngles[0] -= client_state->punchangle[0] * 2.f;
			QAimAngles[1] -= client_state->punchangle[1] * 2.f;
		}

		cmd->buttons |= IN_ATTACK;

		g_Utils.MakeAngle(false, QAimAngles, cmd);

		if (!cvar.ragebot_aim_silent && !cvar.ragebot_aim_perfect_silent)
			g_Engine.SetViewAngles(QAimAngles);

		if (cvar.ragebot_aim_perfect_silent)
			g_Globals.m_bSendCommand = false;

		if (cvar.screen_log)
		{
			std::string sLogMode;

			if (vecPrediction.Length() > 0.f)
				sLogMode.append(sLogMode.length() ? " + prediction" : "prediction");

			if (g_Miscellaneous.m_bFakeLatencyActive)
				sLogMode.append(sLogMode.length() ? " + backtrack" : "backtrack");

			if (!g_Globals.m_bSendCommand)
				sLogMode.append(sLogMode.length() ? " + choke" : "choke");

			if (!sLogMode.length())
				sLogMode.append("none");

			g_ScreenLog.Log(ImColor(IM_COL32_WHITE), "Shoot %s in %s from %s (mode: %s)", player->m_szPrintName, g_Utils.GetHitboxNameByIndex(hitbox), g_Utils.GetWeaponNameByIndex(g_Weapon.data.m_iWeaponID), sLogMode.c_str());
		}
	}
}

void CAimbot::_LegitBot(usercmd_s* cmd)
{
	_bAimbotActive = false;
	m_flLegitBotFOV = 0.f;

	if (_bTriggerActive)
		return;

	if (!cvar.legitbot[g_Weapon.data.m_iWeaponID].aim)
		return;

	if (g_Weapon.data.m_iInReload)
		return;

	std::deque<int> hitboxes;

	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_head)
		hitboxes.push_back(HITBOX_HEAD);
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_chest)
	{
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
		hitboxes.push_back(HITBOX_NECK);
	}
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_stomach)
		hitboxes.push_back(HITBOX_STOMACH);
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_legs) //legs
	{
		hitboxes.push_back(HITBOX_LEFT_FOOT);
		hitboxes.push_back(HITBOX_LEFT_CALF);
		hitboxes.push_back(HITBOX_LEFT_THIGH);
		hitboxes.push_back(HITBOX_RIGHT_FOOT);
		hitboxes.push_back(HITBOX_RIGHT_CALF);
		hitboxes.push_back(HITBOX_RIGHT_THIGH);
	}
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_arms)
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

	if (hitboxes.empty())
		return;

	auto best_fov = cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_fov;
	float flSmooth = (cmd->buttons & IN_ATTACK) ? cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_smooth_in_attack : cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_smooth_auto;

	if ((cmd->buttons & IN_ATTACK) && g_Weapon.data.m_iShotsFired >= cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_start)
	{
		if ((client_state->punchangle[0] != 0.f && cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_pitch) || (client_state->punchangle[1] != 0.f && cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_yaw))
		{
			best_fov = cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_fov;
			flSmooth = cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_smooth;
		}
	}

	if (best_fov <= 0.f)
		return;

	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_reaction_time && abs(client_state->time - m_dbLastTimeKilled) <= (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_reaction_time / 1000.f))
		return;

	m_flLegitBotFOV = best_fov;

	QAngle QAngles(cmd->viewangles);
	QAngles[0] += client_state->punchangle[0] * 2.f;
	QAngles[1] += client_state->punchangle[1] * 2.f;
	QAngles.Normalize();

	Vector vecRecoil, vecSrc(g_Local.m_vecEye);
	QAngles.AngleVectors(&vecRecoil, NULL, NULL);

	auto best_target = CTarget();

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, true);

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (i == g_Local.m_iIndex)
			continue;

		const auto pPlayer = g_World.GetPlayer(i);

		if (!pPlayer)
			continue;

		if (!cvar.legitbot_friendlyfire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS && !g_Miscellaneous.m_bFakeLatencyActive)
			continue;

		if (cvar.player_list && pPlayer->m_iPlayerListType == 1)
			continue;

		Vector vecCorrectedOrigin(pPlayer->m_vecOrigin);

		if (g_Miscellaneous.m_bFakeLatencyActive || cvar.legitbot_position_adjustment)
		{
			auto best_lerp = -1;

			if (cvar.legitbot_position_adjustment)
			{
				auto best_fov = FLT_MAX;

				for (short x = 0; x <= 100; x++)
				{
					Vector vecPositionAdjustmentOrigin(vecCorrectedOrigin);

					g_Utils.LagCompensation(pPlayer->m_iIndex, x, vecPositionAdjustmentOrigin);

					auto fov = vecRecoil.AngleBetween(vecPositionAdjustmentOrigin - vecSrc);

					if (fov < best_fov)
					{
						best_lerp = x;
						best_fov = fov;
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

		{
			Vector vecBackupOrigin(pPlayer->m_vecOrigin);

			pPlayer->m_vecOrigin = vecCorrectedOrigin;

			g_Utils.SpoofPhysent(pPlayer->m_iIndex, false);

			pPlayer->m_vecOrigin = vecBackupOrigin;
		}

		for (auto&& hitbox : hitboxes)
		{
			Vector vecHitbox = vecCorrectedOrigin + pPlayer->m_vecHitbox[hitbox] - pPlayer->m_vecOrigin;

			auto fov = vecRecoil.AngleBetween(vecHitbox - vecSrc);

			if (fov < best_fov)
			{
				bool found = false;

				pmtrace_s tr;
				g_Engine.pEventAPI->EV_SetTraceHull(2);
				g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecHitbox, PM_NORMAL, -1, &tr);

				if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) == pPlayer->m_iIndex)
				{
					found = true;
				}
				else if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_penetration)
				{
					auto damage = g_Utils.SimulateFireBullet(vecSrc, vecHitbox, g_Weapon.GetDistance(), g_Weapon.GetPenetration(), g_Weapon.GetBulletType(), g_Weapon.GetDamage(), g_Weapon.GetWallPierce());

					if (damage > 0)
						found = true;
				}

				if (found && cvar.legitbot_smoke_check && g_Utils.IsLineBlockedBySmoke(vecSrc, vecHitbox))
					continue;

				if (found)
				{
					best_fov = fov;

					best_target.m_pPlayer = pPlayer;
					best_target.m_iHitbox = hitbox;
					best_target.m_vecCorrectedOrigin = vecCorrectedOrigin;
				}
			}
		}
	}

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, false);

	{
		const auto current_target_index = best_target.m_pPlayer ? best_target.m_pPlayer->m_iIndex : -1;

		static auto previous_time = client_state->time;
		static auto previous_index = current_target_index;

		if (previous_index != current_target_index)
		{
			previous_time = client_state->time;
			previous_index = current_target_index;
		}

		const auto time_difference = abs(client_state->time - previous_time) * 1000.0;

		if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_maximum_lock_on_time > 0.f && time_difference > cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_maximum_lock_on_time)
			return;

		if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_delay_before_aiming > 0.f && time_difference <= cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_delay_before_aiming)
			return;

		if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_delay_before_firing > 0.f && time_difference <= cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_delay_before_firing)
			cmd->buttons &= ~IN_ATTACK;
	}

	if (best_target.m_pPlayer)
	{
		if (cvar.legitbot_automatic_scope && g_Weapon.IsSniper() && g_Local.m_iFOV == DEFAULT_FOV && (cmd->buttons & IN_ATTACK))
		{
			cmd->buttons &= ~IN_ATTACK;
			cmd->buttons |= IN_ATTACK2;
			return;
		}

		const auto player = best_target.m_pPlayer;
		const auto hitbox = best_target.m_iHitbox;
		const auto corrected_origin = best_target.m_vecCorrectedOrigin;

		QAngle QAngles(cmd->viewangles), QAimAngles, QSmoothAngles;

		Vector vecCorrectedOrigin = corrected_origin;

		Vector vecPrediction = player->m_vecVelocity * g_Globals.m_flFrameTime;

		vecCorrectedOrigin += vecPrediction;

		Vector vecAimOrigin = vecCorrectedOrigin + player->m_vecHitbox[hitbox] - player->m_vecOrigin;

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

		Vector vecForward = vecAimOrigin - g_Local.m_vecEye;

		g_Utils.VectorAngles(vecAimOrigin - g_Local.m_vecEye, QAimAngles);

		QAngle QNewAngles(QAimAngles);

		if (g_Weapon.data.m_iShotsFired >= cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_start)
		{
			QNewAngles[0] -= client_state->punchangle[0] * (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_pitch / 50.f);
			QNewAngles[1] -= client_state->punchangle[1] * (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_recoil_yaw / 50.f);
		}

		QNewAngles.Normalize();

		if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_smooth_scale_fov > 0.f && client_state->punchangle.IsZero2D())
		{
			auto fov = vecRecoil.AngleBetween(vecForward);

			if (fov > 0.f)
				flSmooth = flSmooth - ((fov * (flSmooth / cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_fov) * cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_smooth_scale_fov) / 100.f);
		}

		if (cvar.legitbot_dependence_fps && g_Globals.m_flFrameTime > 0.f)
		{
			flSmooth *= 1.f / g_Globals.m_flFrameTime;
			flSmooth /= 100.f;
		}

		_SmoothAimAngles(QAngles, QNewAngles, QSmoothAngles, flSmooth);

		if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_angle > 0.f)
		{
			bool psilent_triggered = false;

			if ((cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_triggers == 0 && g_Local.m_flHeight <= 0.f && g_Local.m_flVelocity <= 0.f) ||
				(cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_triggers == 1 && g_Local.m_flHeight <= 0.f) ||
				(cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_triggers == 2 && g_Local.m_flHeight > 0.f) ||
				(cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_triggers == 3))
				psilent_triggered = true;

			if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_tapping_mode && client_state->punchangle.Length2D() > 0.f)
				psilent_triggered = false;

			if (psilent_triggered && g_Weapon.CanAttack() && (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_type == 1 || (cmd->buttons & IN_ATTACK)))
			{
				QAngle QAnglePerfectSilent(QAimAngles), QNewAngles;

				QAnglePerfectSilent[0] -= client_state->punchangle[0] * 2.f;
				QAnglePerfectSilent[1] -= client_state->punchangle[1] * 2.f;

				QAnglePerfectSilent.Normalize();

				g_NoSpread.GetSpreadOffset(g_Weapon.data.m_iRandomSeed, 1, QAnglePerfectSilent, QNewAngles);

				QAngle QDifference = QNewAngles - QAngles;

				QDifference.Normalize();

				//auto max_angle = QDifference.x + QDifference.y + QDifference.z;

				Vector vecDifference = Vector(QDifference);

				if (vecDifference.Length() < cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_angle)
				{
				//if (max_angle <= cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_psilent_angle)
					_bAimbotActive = true;

					cmd->buttons |= IN_ATTACK;

					g_Utils.MakeAngle(false, QNewAngles, cmd);

					g_Globals.m_bSendCommand = false;

					sLogMode.append(sLogMode != "none" ? " + choke" : "choke");

					if (cvar.screen_log)
					{
						g_ScreenLog.Log(ImColor(IM_COL32_WHITE), "Shoot %s in %s from %s (mode: %s)", player->m_szPrintName, g_Utils.GetHitboxNameByIndex(hitbox), g_Utils.GetWeaponNameByIndex(g_Weapon.data.m_iWeaponID), sLogMode.c_str());
					}
					
					return;
				}
			}
		}

		if (flSmooth <= 0.f)
			return;

		_bAimbotActive = true;

		if (cvar.demochecker_bypass)
		{
			const auto prev_frameid = (client_state->validsequence - 1) % IM_ARRAYSIZE(client_state->commands);

			QAngle QPreviousAngles(client_state->commands[prev_frameid].cmd.viewangles), QPreviousAngles2, QPreviousAngles3(cmd->viewangles);

			g_Engine.GetViewAngles(QPreviousAngles2);

			QPreviousAngles.Normalize();
			QPreviousAngles2.Normalize();
			QPreviousAngles3.Normalize();

			QAngle QDelta = QPreviousAngles.Delta360(QSmoothAngles);
			QAngle QDelta2 = QPreviousAngles2.Delta360(QSmoothAngles);
			QAngle QDelta3 = QPreviousAngles3.Delta360(QSmoothAngles);

			if (QDelta.x > 0.000001 && 0.007 > QDelta.x)
				QSmoothAngles.x = QPreviousAngles.x;

			if (QDelta.y > 0.0 && QDelta.y < 0.000013)
				QSmoothAngles.y = QPreviousAngles.y;

			if (QDelta2.x > 0.000001 && 0.007 > QDelta2.x)
				QSmoothAngles.x = QPreviousAngles2.x;

			if (QDelta2.y > 0.0 && QDelta2.y < 0.000013)
				QSmoothAngles.y = QPreviousAngles2.y;

			if (QDelta3.x > 0.000001 && 0.007 > QDelta3.x)
				QSmoothAngles.x = QPreviousAngles3.x;

			if (QDelta3.y > 0.0 && QDelta3.y < 0.000013)
				QSmoothAngles.y = QPreviousAngles3.y;
		}

		g_Utils.MakeAngle(false, QSmoothAngles, cmd);

		g_Engine.SetViewAngles(QSmoothAngles);

		if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_accuracy && (cmd->buttons & IN_ATTACK) && g_Weapon.CanAttack())
		{
			QSmoothAngles[0] += client_state->punchangle[0] * 2.f;
			QSmoothAngles[1] += client_state->punchangle[1] * 2.f;

			QSmoothAngles.Normalize();

			if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_accuracy == 1) //bbox + recoil + spread
			{
				Vector vecForward, vecRight, vecUp, vecRandom, vecSpreadDir;

				QSmoothAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

				g_NoSpread.GetSpreadXY(g_Weapon.data.m_iRandomSeed, 1, vecRandom);

				vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

				vecSpreadDir.Normalize();

				Vector vecMins = vecCorrectedOrigin + player->m_vecBoundBoxMins;
				Vector vecMaxs = vecCorrectedOrigin + player->m_vecBoundBoxMaxs;

				if (!g_Utils.IsBoxIntersectingRay(vecMins, vecMaxs, g_Local.m_vecEye, vecSpreadDir))
					cmd->buttons &= ~IN_ATTACK;
			}
			else if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_accuracy == 2) //recoil
			{
				Vector vecSpreadDir;

				QSmoothAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

				vecSpreadDir.Normalize();

				cmd->buttons &= ~IN_ATTACK;

				for (auto&& hitbox : hitboxes)
				{
					Vector vecPoints[HITBOX_POINTS_MAX];

					for (auto point = 0; point < HITBOX_POINTS_MAX; point++)
						vecPoints[point] = vecCorrectedOrigin + player->m_vecHitboxPoints[hitbox][point] - player->m_vecOrigin;

					if (g_Utils.IsBoxIntersectingRay(vecPoints[0], vecPoints[1], g_Local.m_vecEye, vecSpreadDir) ||
						g_Utils.IsBoxIntersectingRay(vecPoints[6], vecPoints[3], g_Local.m_vecEye, vecSpreadDir) ||
						g_Utils.IsBoxIntersectingRay(vecPoints[7], vecPoints[4], g_Local.m_vecEye, vecSpreadDir) ||
						g_Utils.IsBoxIntersectingRay(vecPoints[2], vecPoints[5], g_Local.m_vecEye, vecSpreadDir))
					{
						cmd->buttons |= IN_ATTACK;
						break;
					}
				}
			}
			else if (cvar.legitbot[g_Weapon.data.m_iWeaponID].aim_accuracy == 3) //recoil + spread
			{
				Vector vecForward, vecRight, vecUp, vecRandom, vecSpreadDir;

				QSmoothAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

				g_NoSpread.GetSpreadXY(g_Weapon.data.m_iRandomSeed, 1, vecRandom);

				vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

				vecSpreadDir.Normalize();

				cmd->buttons &= ~IN_ATTACK;

				for (auto&& hitbox : hitboxes)
				{
					Vector vecPoints[HITBOX_POINTS_MAX];

					for (auto point = 0; point < HITBOX_POINTS_MAX; point++)
						vecPoints[point] = vecCorrectedOrigin + player->m_vecHitboxPoints[hitbox][point] - player->m_vecOrigin;

					if (g_Utils.IsBoxIntersectingRay(vecPoints[0], vecPoints[1], g_Local.m_vecEye, vecSpreadDir) ||
						g_Utils.IsBoxIntersectingRay(vecPoints[6], vecPoints[3], g_Local.m_vecEye, vecSpreadDir) ||
						g_Utils.IsBoxIntersectingRay(vecPoints[7], vecPoints[4], g_Local.m_vecEye, vecSpreadDir) ||
						g_Utils.IsBoxIntersectingRay(vecPoints[2], vecPoints[5], g_Local.m_vecEye, vecSpreadDir))
					{
						cmd->buttons |= IN_ATTACK;
						break;
					}
				}
			}
		}

		if (cvar.screen_log && (cmd->buttons & IN_ATTACK) && g_Weapon.CanAttack())
		{
			g_ScreenLog.Log(ImColor(IM_COL32_WHITE), "Shoot %s in %s from %s (mode: %s)", player->m_szPrintName, g_Utils.GetHitboxNameByIndex(hitbox), g_Utils.GetWeaponNameByIndex(g_Weapon.data.m_iWeaponID), sLogMode.c_str());
		}
	}
}

void CAimbot::_Trigger(usercmd_s* cmd)
{
	_bTriggerActive = false;

	if (!cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger)
		return;

	if (!state.trigger_active && cvar.legitbot_trigger_key)
		return;

	if (!g_Weapon.CanAttack())
		return;

	if (cvar.legitbot_trigger_only_scoped && g_Weapon.IsSniper() && g_Local.m_iFOV == DEFAULT_FOV)
		return;

	std::deque<int> hitboxes;

	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_head)
		hitboxes.push_back(HITBOX_HEAD);
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_chest)
	{
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
		hitboxes.push_back(HITBOX_NECK);
	}
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_stomach)
		hitboxes.push_back(HITBOX_STOMACH);
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_legs) //legs
	{
		hitboxes.push_back(HITBOX_LEFT_FOOT);
		hitboxes.push_back(HITBOX_LEFT_CALF);
		hitboxes.push_back(HITBOX_LEFT_THIGH);
		hitboxes.push_back(HITBOX_RIGHT_FOOT);
		hitboxes.push_back(HITBOX_RIGHT_CALF);
		hitboxes.push_back(HITBOX_RIGHT_THIGH);
	}
	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_arms)
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

	if (hitboxes.empty())
		return;

	QAngle QAngles(cmd->viewangles);

	Vector vecSpreadDir;

	if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_accuracy == 1) //Recoil
	{
		QAngles[0] += client_state->punchangle[0] * 2.f;
		QAngles[1] += client_state->punchangle[1] * 2.f;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize();
	}
	else if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_accuracy == 2) //Recoil + Spread
	{
		Vector vecForward, vecRight, vecUp, vecRandom;

		QAngles[0] += client_state->punchangle[0] * 2.f;
		QAngles[1] += client_state->punchangle[1] * 2.f;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		g_NoSpread.GetSpreadXY(g_Weapon.data.m_iRandomSeed, 1, vecRandom);

		vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

		vecSpreadDir.Normalize();
	}
	else {
		QAngles.Normalize();

		QAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize();
	}

	Vector vecSrc = g_Local.m_vecEye;

	if (cvar.legitbot_smoke_check && g_Utils.IsLineBlockedBySmoke(vecSrc, vecSrc + vecSpreadDir * g_Weapon.GetDistance()))
		return;

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, true);

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (i == g_Local.m_iIndex)
			continue;

		const auto pPlayer = g_World.GetPlayer(i);

		if (!pPlayer)
			continue;

		if (!cvar.legitbot_friendlyfire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS && !g_Miscellaneous.m_bFakeLatencyActive)
			continue;

		if (cvar.player_list && pPlayer->m_iPlayerListType == 1)
			continue;

		Vector vecCorrectedOrigin(pPlayer->m_vecOrigin);

		if (g_Miscellaneous.m_bFakeLatencyActive || cvar.legitbot_position_adjustment)
		{
			auto best_lerp = -1;

			if (cvar.legitbot_position_adjustment)
			{
				auto best_fov = FLT_MAX;

				for (short x = 0; x <= 100; x++)
				{
					Vector vecPositionAdjustmentOrigin(vecCorrectedOrigin);

					g_Utils.LagCompensation(pPlayer->m_iIndex, x, vecPositionAdjustmentOrigin);

					auto fov = vecSpreadDir.AngleBetween(vecPositionAdjustmentOrigin - vecSrc);

					if (fov < best_fov)
					{
						best_lerp = x;
						best_fov = fov;
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

		{
			Vector vecBackupOrigin(pPlayer->m_vecOrigin);

			pPlayer->m_vecOrigin = vecCorrectedOrigin;

			g_Utils.SpoofPhysent(pPlayer->m_iIndex, false);

			pPlayer->m_vecOrigin = vecBackupOrigin;
		}

		for (auto &&hitbox : hitboxes)
		{
			Vector vecHitbox = vecCorrectedOrigin + pPlayer->m_vecHitbox[hitbox] - pPlayer->m_vecOrigin;

			Vector vecPoints[HITBOX_POINTS_MAX];

			for (auto point = 0; point < HITBOX_POINTS_MAX; point++)
				vecPoints[point] = vecHitbox + ((pPlayer->m_vecHitboxPoints[hitbox][point] - pPlayer->m_vecHitbox[hitbox]) * (cvar.legitbot_trigger_hitbox_scale / 100.f));

			if (g_Utils.IsBoxIntersectingRay(vecPoints[0], vecPoints[1], vecSrc, vecSpreadDir) ||
				g_Utils.IsBoxIntersectingRay(vecPoints[6], vecPoints[3], vecSrc, vecSpreadDir) ||
				g_Utils.IsBoxIntersectingRay(vecPoints[7], vecPoints[4], vecSrc, vecSpreadDir) ||
				g_Utils.IsBoxIntersectingRay(vecPoints[2], vecPoints[5], vecSrc, vecSpreadDir))
			{
				pmtrace_s tr;
				g_Engine.pEventAPI->EV_SetTraceHull(2);
				g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + vecSpreadDir * g_Weapon.GetDistance(), PM_NORMAL, -1, &tr);

				if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) == pPlayer->m_iIndex)
				{
					_bTriggerActive = true;
				}
				else if (cvar.legitbot[g_Weapon.data.m_iWeaponID].trigger_penetration)
				{
					auto damage = g_Utils.SimulateFireBullet(vecSrc, vecHitbox, g_Weapon.GetDistance(), g_Weapon.GetPenetration(), g_Weapon.GetBulletType(), g_Weapon.GetDamage(), g_Weapon.GetWallPierce());

					if (damage > 0)
						_bTriggerActive = true;
				}

				if (_bTriggerActive)
				{
					cmd->buttons |= IN_ATTACK;

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

						g_ScreenLog.Log(ImColor(IM_COL32_WHITE), "Shoot %s in %s from %s (mode: %s)", pPlayer->m_szPrintName, g_Utils.GetHitboxNameByIndex(hitbox), g_Utils.GetWeaponNameByIndex(g_Weapon.data.m_iWeaponID), sLogMode.c_str());
					}

					goto trigger_restore_players;
				}
			}
		}
	}

trigger_restore_players:

	for (auto k = 1; k <= MAX_CLIENTS; ++k)
		g_Utils.SpoofPhysent(k, false);
}

void CAimbot::_SmoothAimAngles(const QAngle a_QAngles, const QAngle a_QAimAngles, QAngle &a_QNewAngles, const float a_flSmoothing)
{
	if (a_flSmoothing <= 1.f || isnan(a_flSmoothing))
	{
		a_QNewAngles = a_QAimAngles;
		return;
	}

	a_QNewAngles = a_QAimAngles - a_QAngles;

	a_QNewAngles.Normalize();

	a_QNewAngles /= a_flSmoothing;

	a_QNewAngles = a_QAngles + a_QNewAngles;

	a_QNewAngles.Normalize();
}