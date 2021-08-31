#include "framework.h"

std::unique_ptr<CLegitBot> g_pLegitBot;

CLegitBot::CLegitBot()
{
	// run code one time when you connected to server
	m_flMinAngleDemoChecker = 1.f;
}

CLegitBot::~CLegitBot()
{
	// run code one time when you disconnected from server
}

void CLegitBot::Run(usercmd_s* cmd)
{
	m_iAimPlayer = -1;
	m_iAimHitbox = -1;
	m_flCurrentFOV = 0.f;

	QAngle QAngles(cmd->viewangles), QNewAngles;

	QAngles.Normalize();

	if (g_Weapon.IsGun())
	{
		Aimbot(cmd);
		StandaloneRecoilControl(cmd);
		Trigger(cmd);
		DesyncHelper(cmd);
	}

	if (cvars::legitbot.aim_demochecker_bypass && client_static->demorecording)
	{
		QAngle QNewAngles(cmd->viewangles);

		if (m_flMinAngleDemoChecker == 1.f)
		{
			cmd->buttons &= ~IN_ATTACK;
			QNewAngles = QAngles;
		}
		
		if (~cmd->buttons & IN_ATTACK)
		{
			static bool bJitter = false;

			QNewAngles.x += bJitter ? 0.000002 : -0.000002;

			Game::MakeAngle(QNewAngles, cmd);

			g_Engine.SetViewAngles(QNewAngles);

			bJitter = !bJitter;
		}
	}
	else
		m_flMinAngleDemoChecker = 1.f;

	QNewAngles = cmd->viewangles;

	QNewAngles.Normalize();

	if (cvars::legitbot.aim_demochecker_bypass && client_static->demorecording)
	{
		QAngle QDeltaAngles = QAngles.Delta360(QNewAngles);

		float flAngle = DEFAULT_FOV * QDeltaAngles.x / g_Local->m_iFOV;

		if (QDeltaAngles.x > 0.000001)
		{
			if (~cmd->buttons & IN_ATTACK)
			{
				if (m_flMinAngleDemoChecker > flAngle)
					m_flMinAngleDemoChecker = flAngle;
			}
		}
	}
}

void CLegitBot::DesyncHelper(usercmd_s* cmd)
{
	if (!cvars::legitbot.desync_helper)
		return;

	if (g_pGlobals->m_flGaitMovement)
		return;

	if (!g_Local->m_bIsOnGround)
		return;

	if (cmd->buttons & IN_ATTACK && g_Weapon.CanAttack())
		return;

	static bool bJitter = false;

	QAngle QTempAngles(0, g_pGlobals->m_flGaitYaw, 0), QTempAngles2(0, g_Local->m_flGaitYaw, 0), QDeltaAngles;

	QTempAngles.Normalize();
	QTempAngles2.Normalize();

	QDeltaAngles = QTempAngles.Delta360(QTempAngles2);

	if (QDeltaAngles.y < 45.f)
		cmd->sidemove = (g_Engine.pfnRandomFloat(0.f, 1.f) > 0.5f ? pmove->maxspeed : -pmove->maxspeed);

	bJitter = !bJitter;
}

void CLegitBot::Aimbot(usercmd_s* cmd)
{
	if (cvars::legitbot.aim_key.keynum && !m_bAimState)
		return;

	if (g_Weapon->m_fInReload > 0.f)
		return;

	if (g_Weapon->m_flNextAttack > 0.f)
		return;

	if (!g_Weapon->m_iClip)
		return;

	if (g_Weapon.IsPistol() && g_Weapon->m_iShotsFired)
		return;

	if (!cvars::weapons[g_Weapon->m_iWeaponID].aim_enabled)
		return;

	if (cvars::legitbot.aim_block_attack_after_kill && (client_state->time - g_Local->m_flLastKillTime) * 1000.0 <= cvars::legitbot.aim_block_attack_after_kill)
		cmd->buttons &= ~IN_ATTACK;

	if (cvars::legitbot.target_switch_delay && (client_state->time - g_Local->m_flLastKillTime) * 1000.0 <= cvars::legitbot.target_switch_delay)
		return;

	std::deque<int> hitboxes;

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_hitboxes[0])
		hitboxes.push_back(HITBOX_HEAD);

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_hitboxes[1])
		hitboxes.push_back(HITBOX_NECK);

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_hitboxes[2])
	{
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
	}

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_hitboxes[3])
		hitboxes.push_back(HITBOX_STOMACH);

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_hitboxes[4])
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

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_hitboxes[5])
	{
		hitboxes.push_back(HITBOX_LEFT_FOOT);
		hitboxes.push_back(HITBOX_LEFT_CALF);
		hitboxes.push_back(HITBOX_LEFT_THIGH);
		hitboxes.push_back(HITBOX_RIGHT_FOOT);
		hitboxes.push_back(HITBOX_RIGHT_CALF);
		hitboxes.push_back(HITBOX_RIGHT_THIGH);
	}

	if (hitboxes.empty())
		return;

	bool bAutomaticFire = false;

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_auto_fire && (~cmd->buttons & IN_ATTACK))
	{
		cmd->buttons |= IN_ATTACK;
		bAutomaticFire = true;
	}

	float flBestFOV = cvars::weapons[g_Weapon->m_iWeaponID].aim_fov;
	float flSmooth = (cmd->buttons & IN_ATTACK) ? cvars::weapons[g_Weapon->m_iWeaponID].aim_smooth_in_attack : cvars::weapons[g_Weapon->m_iWeaponID].aim_smooth_auto;

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_fov && g_Weapon->m_iShotsFired >= cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_start && (cmd->buttons & IN_ATTACK))
	{
		if ((client_state->punchangle[0] != 0.f && cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_pitch) ||
			(client_state->punchangle[1] != 0.f && cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_yaw))
		{
			flBestFOV = cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_fov;
			flSmooth = cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_smooth;
		}
	}

	if (flBestFOV <= 0.f)
		return;

	m_flCurrentFOV = flBestFOV;

	Vector vecSrc(g_Local->m_vecEyePos), vecSpreadDir, vecAdjustedOrigin;

	{
		QAngle QAngles(cmd->viewangles);

		QAngles[0] += client_state->punchangle[0] * 2.f;
		QAngles[1] += client_state->punchangle[1] * 2.f;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize();
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

		if (!cvars::legitbot.friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			continue;

		Vector vecTempAdjustedOrigin(g_Player[i]->m_vecOrigin);

		if (cvars::legitbot.position_adjustment)
		{
			auto best_lerp_msec = -1;
			auto best_fov = FLT_MAX;

			for (int lerp_msec = 0; lerp_msec <= 100; lerp_msec++)
			{
				Vector vecTempOrigin;

				if (Game::BacktrackPlayer(pGameEntity, lerp_msec, vecTempOrigin))
				{
					Vector vecForward = vecTempOrigin - vecSrc;

					const auto fov = vecSpreadDir.AngleBetween(vecForward);

					if (fov < best_fov)
					{
						best_lerp_msec = lerp_msec;
						best_fov = fov;
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

		for (auto hitbox : hitboxes)
		{
			Vector vecHitbox(vecTempAdjustedOrigin + g_Player[i]->m_vecHitbox[hitbox] - g_Player[i]->m_vecOrigin);

			float flFOV = vecSpreadDir.AngleBetween(vecHitbox - vecSrc);

			if (flFOV < flBestFOV)
			{
				bool bFound = false;

				CorrectPhysentSolid(i);

				Physent::SetOrigin(i, vecTempAdjustedOrigin);

				pmtrace_s pmTrace;
				g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
				g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + (vecHitbox - vecSrc) * g_Weapon->m_flDistance, PM_NORMAL, -1, &pmTrace);

				if (g_Engine.pEventAPI->EV_IndexFromTrace(&pmTrace) == g_Player[i]->m_iEntIndex)
					bFound = true;
				else if (cvars::weapons[g_Weapon->m_iWeaponID].aim_auto_penetration)
				{
					int iDamage = Game::TakeSimulatedDamage(vecSrc, vecHitbox, Game::GetHitgroup(hitbox), i);

					if (iDamage >= cvars::weapons[g_Weapon->m_iWeaponID].aim_auto_penetration_min_damage || iDamage > g_Player[i]->m_iHealth)
						bFound = true;
				}

				if (bFound)
				{
					flBestFOV = flFOV;
					m_iAimPlayer = i;
					m_iAimHitbox = hitbox;
					vecAdjustedOrigin = vecTempAdjustedOrigin;
				}
			}
		}
	}

	if (cvars::weapons[g_Weapon->m_iWeaponID].aim_auto_fire && bAutomaticFire && m_iAimPlayer == -1)
		cmd->buttons &= ~IN_ATTACK;
	else if (m_iAimPlayer != -1 && cvars::legitbot.aim_auto_scope && g_Weapon.IsSniper() && (cmd->buttons & IN_ATTACK))
	{
		if (g_Local->m_iFOV == DEFAULT_FOV)
		{
			cmd->buttons &= ~IN_ATTACK;
			cmd->buttons |= IN_ATTACK2;
		}
		else
		{
			if (g_Weapon->m_flNextSecondaryAttack > 0.f)
				cmd->buttons &= ~IN_ATTACK;
		}
	}

	/*g_Engine.Con_NPrintf(2, "m_iAimPlayer: %i", m_iAimPlayer);
	g_Engine.Con_NPrintf(3, "m_iAimHitbox: %i", m_iAimHitbox);*/

	static auto previous_time = client_state->time;

	if (~cmd->buttons & IN_ATTACK)
		previous_time = client_state->time;

	if (m_iAimPlayer != -1)
	{
		assert(m_iAimPlayer >= 0 && m_iAimPlayer <= MAX_CLIENTS);
		assert(m_iAimHitbox > -1 && m_iAimHitbox < HITBOX_MAX);

		const auto time_difference = abs(client_state->time - previous_time) * 1000.0;

		if (cvars::weapons[g_Weapon->m_iWeaponID].aim_delay_before_firing && time_difference <= cvars::weapons[g_Weapon->m_iWeaponID].aim_delay_before_firing)
			cmd->buttons &= ~IN_ATTACK;

		if (cvars::weapons[g_Weapon->m_iWeaponID].aim_maximum_lock_on_time && time_difference > cvars::weapons[g_Weapon->m_iWeaponID].aim_maximum_lock_on_time)
			return;

		if (cvars::weapons[g_Weapon->m_iWeaponID].aim_delay_before_aiming && time_difference <= cvars::weapons[g_Weapon->m_iWeaponID].aim_delay_before_aiming)
			return;

		QAngle QAngles(cmd->viewangles), QAimAngles;

		Vector vecAimForward, vecAimOrigin(g_Player[m_iAimPlayer]->m_vecHitbox[m_iAimHitbox]);

		vecAimOrigin = vecAdjustedOrigin + vecAimOrigin - g_Player[m_iAimPlayer]->m_vecOrigin; // Correct position

		vecAimForward = vecAimOrigin - vecSrc;

		Math::VectorAngles(vecAimForward, QAimAngles);

		if (cvars::weapons[g_Weapon->m_iWeaponID].aim_psilent_angle)
		{
			bool bTriggered = false;

			if ((cvars::weapons[g_Weapon->m_iWeaponID].aim_psilent_triggers[0] && g_Local->m_bIsOnGround && !g_Local->m_flVelocity) ||
				(cvars::weapons[g_Weapon->m_iWeaponID].aim_psilent_triggers[1] && g_Local->m_bIsOnGround) ||
				(cvars::weapons[g_Weapon->m_iWeaponID].aim_psilent_triggers[2] && !g_Local->m_bIsOnGround))
			{
				bTriggered = true;

				if (cvars::weapons[g_Weapon->m_iWeaponID].aim_psilent_tapping_mode && g_Weapon->m_iShotsFired)
					bTriggered = false;

				if (bTriggered && cvars::legitbot.aim_psilent_key.keynum && !m_bAimPerfectSilentState)
					bTriggered = false;
			}

			if (bTriggered && g_Weapon.CanAttack() && (cvars::weapons[g_Weapon->m_iWeaponID].aim_psilent_type == 1 || (cmd->buttons & IN_ATTACK)))
			{
				QAngle QNewAngles(QAimAngles);

				QNewAngles[0] -= client_state->punchangle[0] * 2.f;
				QNewAngles[1] -= client_state->punchangle[1] * 2.f;

				QNewAngles.Normalize();

				g_pNoSpread->GetSpreadOffset(g_Weapon->m_iRandomSeed, 1, QNewAngles, QNewAngles, NOSPREAD_PITCH_YAW_ROLL);

				bool bAttack = !DemoChecker(QAngles, QNewAngles, QNewAngles);
				
				if (bAttack && cvars::legitbot.aim_dont_shoot_in_shield && Game::TraceShield(vecSrc, vecAdjustedOrigin, QNewAngles, m_iAimPlayer))
					bAttack = false;

				if (bAttack)
				{
					QAngle QDifference = QNewAngles - QAngles;

					QDifference.Normalize();

					Vector vecDifference(QDifference);

					if (vecDifference.Length() < cvars::weapons[g_Weapon->m_iWeaponID].aim_psilent_angle)
					{
						cmd->buttons |= IN_ATTACK;
						Game::MakeAngle(QNewAngles, cmd);
						Game::SendCommand(false);
						return;
					}
				}
			}
		}
		if (cvars::weapons[g_Weapon->m_iWeaponID].aim_smooth_scale_fov && cvars::weapons[g_Weapon->m_iWeaponID].aim_fov && client_state->punchangle.IsZero2D() && flSmooth > 1.f)
		{
			float flFOV = vecSpreadDir.AngleBetween(vecAimForward);

			if (flFOV)
				flSmooth = flSmooth - ((flFOV * (flSmooth / cvars::weapons[g_Weapon->m_iWeaponID].aim_fov) * cvars::weapons[g_Weapon->m_iWeaponID].aim_smooth_scale_fov) / 100.f);
		}

		if (cvars::legitbot.aim_smooth_independence_fps && g_Local->m_flFrameTime > 0.f)
		{
			flSmooth *= 1.f / g_Local->m_flFrameTime;
			flSmooth /= 100.f; // Stable FPS value
		}

		if (flSmooth <= 0.f)
			return;

		QAngle QNewAngles(QAimAngles), QSmoothAngles;

		if (g_Weapon->m_iShotsFired >= cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_start)
		{
			QNewAngles[0] -= client_state->punchangle[0] * (cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_pitch / 50.f);
			QNewAngles[1] -= client_state->punchangle[1] * (cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_yaw / 50.f);

			QNewAngles.Normalize();
		}

		SmoothAimAngles(QAngles, QNewAngles, QSmoothAngles, flSmooth);

		DemoChecker(QAngles, QSmoothAngles, QSmoothAngles);

		Game::MakeAngle(QSmoothAngles, cmd);

		g_Engine.SetViewAngles(QSmoothAngles);

		if (cvars::legitbot.aim_dont_shoot_in_shield && (cmd->buttons & IN_ATTACK) && Game::TraceShield(vecSrc, vecAdjustedOrigin, cmd->viewangles, m_iAimPlayer))
			cmd->buttons &= ~IN_ATTACK;
		else if (cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost && (cmd->buttons & IN_ATTACK) && g_Weapon.CanAttack())
		{
			assert(cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost >= 1 && cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost <= 4);

			cmd->buttons &= ~IN_ATTACK;

			QAngle QAngles(cmd->viewangles);

			QAngles[0] += client_state->punchangle[0] * 2.f;
			QAngles[1] += client_state->punchangle[1] * 2.f;

			QAngles.Normalize();

			Vector vecAccuracySpreadDir;

			if (cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 1 || cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 3)
			{
				QAngles.AngleVectors(&vecAccuracySpreadDir, NULL, NULL);
			}
			else if (cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 2 || cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 4)
			{
				Vector vecForward, vecRight, vecUp, vecRandom;

				QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

				g_pNoSpread->GetSpreadXY(g_Weapon->m_iRandomSeed, 1, vecRandom);

				vecAccuracySpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);
			}

			vecAccuracySpreadDir.Normalize();

			if (cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 1 || cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 2)
			{
				Vector vecMins = vecAdjustedOrigin + g_Player[m_iAimPlayer]->m_vecBoundBoxMins;
				Vector vecMaxs = vecAdjustedOrigin + g_Player[m_iAimPlayer]->m_vecBoundBoxMaxs;

				float flFraction = -1.f; int iHitSide = 0; bool bStartSolid = false;

				if (Math::IntersectRayWithBox(vecSrc, vecAccuracySpreadDir * g_Weapon->m_flDistance, vecMins, vecMaxs, flFraction, iHitSide, bStartSolid))
					cmd->buttons |= IN_ATTACK;
			}
			else if (cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 3 || cvars::weapons[g_Weapon->m_iWeaponID].aim_accuracy_boost == 4)
			{
				for (int hitbox = 0; hitbox < HITBOX_MAX; hitbox++)
				{
					if (hitbox == HITBOX_SHIELD)
						continue;

					matrix3x4_t matHitbox = g_Player[m_iAimPlayer]->m_matHitbox[hitbox];

					Vector vecMatOrigin = vecAdjustedOrigin + matHitbox.GetOrigin() - g_Player[m_iAimPlayer]->m_vecOrigin;

					matHitbox.SetOrigin(vecMatOrigin);

					float flFraction = -1.f; int iHitSide = 0; bool bStartSolid = false;

					if (Math::IntersectRayWithOBB(vecSrc, vecAccuracySpreadDir * g_Weapon->m_flDistance, matHitbox, g_Player[m_iAimPlayer]->m_vecOBBMin[hitbox], g_Player[m_iAimPlayer]->m_vecOBBMax[hitbox], flFraction, iHitSide, bStartSolid))
					{
						cmd->buttons |= IN_ATTACK;
						break;
					}
				}
			}
		}
	}
}

void CLegitBot::StandaloneRecoilControl(usercmd_s* cmd)
{
	if (cvars::legitbot.aim_recoil_standalone)
	{
		static QAngle QReturnAngles = QAngle();

		bool bIsRCS = (g_Weapon.IsRifle() || g_Weapon.IsMachineGun() || g_Weapon.IsSubMachineGun()) && (cmd->buttons & IN_ATTACK) && g_Weapon->m_iShotsFired >= cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_start && g_Weapon->m_iClip;
		bool IsReturn = (cvars::legitbot.aim_recoil_standalone == 2) && (((~cmd->buttons & IN_ATTACK) && g_Weapon->m_iShotsFired) || g_Weapon->m_fInReload > 0.f || g_Weapon->m_flNextAttack > 0.f);

		if ((cvars::legitbot.aim_recoil_standalone == 1) && (~cmd->buttons & IN_ATTACK))
			QReturnAngles.Clear();

		QAngle QAngles(cmd->viewangles), QPunchAngles, QAimAngles, QSmoothAngles;

		if (cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_fov)
		{
			QPunchAngles[0] = client_state->punchangle[0] * (cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_pitch / 50.f);
			QPunchAngles[1] = client_state->punchangle[1] * (cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_yaw / 50.f);
		}

		QAimAngles = QAngles + QReturnAngles - QPunchAngles;

		QAimAngles.Normalize();

		SmoothAimAngles(QAngles, QAimAngles, QSmoothAngles, cvars::weapons[g_Weapon->m_iWeaponID].aim_recoil_smooth);

		if ((bIsRCS || IsReturn) && (m_iAimPlayer == -1))
		{
			DemoChecker(QAngles, QSmoothAngles, QSmoothAngles);

			Game::MakeAngle(QSmoothAngles, cmd);

			g_Engine.SetViewAngles(QSmoothAngles);
		}

		QAimAngles += QPunchAngles;

		QReturnAngles = QAimAngles - QSmoothAngles;
	}
}

bool CLegitBot::DemoChecker(const QAngle& a_QPreviousAngles, const QAngle& a_QNewAngles, QAngle& a_QCorrectedAngles)
{
	bool bReturn = false;

	if (cvars::legitbot.aim_demochecker_bypass && client_static->demorecording)
	{
		QAngle QPreviousAngles, QNewAngles, QDeltaAngles;

		QPreviousAngles = a_QPreviousAngles;

		QNewAngles = a_QNewAngles;

		QDeltaAngles = QPreviousAngles.Delta360(QNewAngles);

		if (QDeltaAngles.x > 0.000001)
		{
			if (0.007 > QDeltaAngles.x) // v1
			{
				a_QCorrectedAngles.x = QPreviousAngles.x;

				bReturn = true;
			}

			{ // v2
				float flAngle = DEFAULT_FOV * QDeltaAngles.x / g_Local->m_iFOV;

				if (1.f != m_flMinAngleDemoChecker && m_flMinAngleDemoChecker - flAngle > 0.000001)
				{
					a_QCorrectedAngles.x = QPreviousAngles.x;

					bReturn = true;
				}
			}
		}

		if (QDeltaAngles.y > 0.0 && QDeltaAngles.y < 0.000013) // v1 && v2
		{
			a_QCorrectedAngles.y = QPreviousAngles.y;

			bReturn = true;
		}
	}

	return bReturn;
}

void CLegitBot::Trigger(usercmd_s* cmd)
{
	if (cvars::legitbot.trigger_key.keynum && !m_bTriggerState)
		return;

	if (!g_Weapon.CanAttack())
		return;

	if (!cvars::weapons[g_Weapon->m_iWeaponID].trigger_enabled)
		return;

	if (cvars::legitbot.trigger_only_scoped && g_Weapon.IsSniper() && g_Local->m_iFOV == DEFAULT_FOV)
		return;

	if (cvars::legitbot.target_switch_delay && (client_state->time - g_Local->m_flLastKillTime) * 1000.0 <= cvars::legitbot.target_switch_delay)
		return;

	std::deque<int> hitboxes;

	if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_hitboxes[0])
		hitboxes.push_back(HITBOX_HEAD);

	if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_hitboxes[1])
		hitboxes.push_back(HITBOX_NECK);

	if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_hitboxes[2])
	{
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
	}

	if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_hitboxes[3])
		hitboxes.push_back(HITBOX_STOMACH);

	if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_hitboxes[4])
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

	if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_hitboxes[5])
	{
		hitboxes.push_back(HITBOX_LEFT_FOOT);
		hitboxes.push_back(HITBOX_LEFT_CALF);
		hitboxes.push_back(HITBOX_LEFT_THIGH);
		hitboxes.push_back(HITBOX_RIGHT_FOOT);
		hitboxes.push_back(HITBOX_RIGHT_CALF);
		hitboxes.push_back(HITBOX_RIGHT_THIGH);
	}

	if (hitboxes.empty())
		return;

	Vector vecSpreadDir, vecSrc(g_Local->m_vecEyePos);

	QAngle QAngles(cmd->viewangles);

	if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_accuracy_boost == 1)
	{
		QAngles[0] += client_state->punchangle[0] * 2.f;
		QAngles[1] += client_state->punchangle[1] * 2.f;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize();
	}
	else if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_accuracy_boost == 2)
	{
		Vector vecForward, vecRight, vecUp, vecRandom;

		QAngles[0] += client_state->punchangle[0] * 2.f;
		QAngles[1] += client_state->punchangle[1] * 2.f;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		g_pNoSpread->GetSpreadXY(g_Weapon->m_iRandomSeed, 1, vecRandom);

		vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

		/*auto kek2 = vecSrc + vecSpreadDir;

		if (g_Weapon->m_iWeaponID == WEAPON_ELITE)
		{
			if (g_Weapon->m_iWeaponState & WPNSTATE_ELITE_LEFT)
				vecSrc -= vecRight * 5.f;
			else
				vecSrc += vecRight * 5.f;
		}

		auto perfect = vecSrc + vecSpreadDir;
		

		g_Engine.Con_Printf("perfect: %f %f %f | %f %f %f\n", perfect.x,perfect.y,perfect.z, kek2.x, kek2.y, kek2.z);*/

		vecSpreadDir.Normalize();
	}
	else {
		QAngles.Normalize();

		QAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize();
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

		if (!cvars::legitbot.friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			continue;

		Vector vecTempAdjustedOrigin(g_Player[i]->m_vecOrigin);

		if (cvars::legitbot.position_adjustment)
		{
			auto best_lerp_msec = -1;
			auto best_fov = FLT_MAX;

			for (int lerp_msec = 0; lerp_msec <= 100; lerp_msec++)
			{
				Vector vecTempOrigin;

				if (Game::BacktrackPlayer(pGameEntity, lerp_msec, vecTempOrigin))
				{
					Vector vecForward = vecTempOrigin - vecSrc;

					const auto fov = vecSpreadDir.AngleBetween(vecForward);

					if (fov < best_fov)
					{
						best_lerp_msec = lerp_msec;
						best_fov = fov;
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

		for (auto hitbox : hitboxes)
		{
			Vector vecMatOrigin, vecOBBMin, vecOBBMax;

			matrix3x4_t matHitbox(g_Player[i]->m_matHitbox[hitbox]);

			vecMatOrigin = vecTempAdjustedOrigin + matHitbox.GetOrigin() - g_Player[i]->m_vecOrigin;

			matHitbox.SetOrigin(vecMatOrigin);

			vecOBBMin = g_Player[i]->m_vecOBBMin[hitbox] * (cvars::legitbot.trigger_hitbox_scale / 100.f);
			vecOBBMax = g_Player[i]->m_vecOBBMax[hitbox] * (cvars::legitbot.trigger_hitbox_scale / 100.f);

			float flFraction = -1.f; int iHitSide = 0; bool bStartSolid = false;

			if (Math::IntersectRayWithOBB(vecSrc, vecSpreadDir * g_Weapon->m_flDistance, matHitbox, vecOBBMin, vecOBBMax, flFraction, iHitSide, bStartSolid))
			{
				if (cvars::legitbot.trigger_accurate_traces)
				{
					bool bSkipHitbox = false;

					for (int j = 0; j < HITBOX_MAX; j++)
					{
						if (j == hitbox)
							continue;

						if (j == HITBOX_SHIELD && !g_Player[i]->m_bHasShield)
							continue;

						matHitbox = g_Player[i]->m_matHitbox[j];

						vecMatOrigin = vecTempAdjustedOrigin + matHitbox.GetOrigin() - g_Player[i]->m_vecOrigin;

						matHitbox.SetOrigin(vecMatOrigin);

						float flFraction2 = -1.f; int iHitSide2 = 0; bool bStartSolid2 = false;

						if (Math::IntersectRayWithOBB(vecSrc, vecSpreadDir * g_Weapon->m_flDistance, matHitbox, g_Player[i]->m_vecOBBMin[j], g_Player[i]->m_vecOBBMax[j], flFraction2, iHitSide2, bStartSolid2))
						{
							if (flFraction2 <= flFraction)
							{
								bSkipHitbox = true;
								break;
							}
						}
					}

					if (bSkipHitbox)
						continue;
				}

				bool bAttack = false;

				CorrectPhysentSolid(i);

				Physent::SetOrigin(i, vecTempAdjustedOrigin);

				pmtrace_t pmTrace;
				g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
				g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + vecSpreadDir * g_Weapon->m_flDistance, PM_NORMAL, -1, &pmTrace);

				if (g_Engine.pEventAPI->EV_IndexFromTrace(&pmTrace) == g_Player[i]->m_iEntIndex)
					bAttack = true;
				else if (cvars::weapons[g_Weapon->m_iWeaponID].trigger_auto_penetration)
				{
					int iDamage = Game::TakeSimulatedDamage(vecSrc, g_Player[i]->m_vecHitbox[hitbox], Game::GetHitgroup(hitbox), i);

					if (iDamage >= cvars::weapons[g_Weapon->m_iWeaponID].trigger_auto_penetration_min_damage || iDamage > g_Player[i]->m_iHealth)
						bAttack = true;
				}

				if (bAttack)
				{
					cmd->buttons |= IN_ATTACK;
					return;
				}
			}
		}
	}
}

void CLegitBot::CorrectPhysentSolid(const int& nPlayerID)
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

		if (!cvars::legitbot.friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			Physent::SetSolid(i, SOLID_BBOX);
		else
			Physent::SetSolid(i, SOLID_NOT);
	}
}

void CLegitBot::SmoothAimAngles(const QAngle& QAngles, const QAngle& QAimAngles, QAngle& QNewAngles, const float& flSmoothing)
{
	assert(isfinite(flSmoothing));

	if (flSmoothing <= 1.f)
	{
		QNewAngles = QAimAngles;
		return;
	}

	QNewAngles = QAimAngles - QAngles;

	QNewAngles.Normalize();

	QNewAngles /= flSmoothing;

	//QNewAngles.x *= g_ClientCvarsMap["m_pitch"]->value * g_ClientCvarsMap["sensitivity"]->value;
	//QNewAngles.y *= g_ClientCvarsMap["m_yaw"]->value * g_ClientCvarsMap["sensitivity"]->value;

	QNewAngles = QAngles + QNewAngles;

	QNewAngles.Normalize();
}