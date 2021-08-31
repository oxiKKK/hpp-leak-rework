#include "main.h"

CAimbot g_Aimbot;

CTarget::CTarget() :
	m_bValid(false),
	m_iScore(0),
	m_iHitbox(-1),
	m_iPoint(-1),
	m_pPlayer(nullptr)
{
}

void CAimbot::Run(usercmd_s* cmd)
{
	if (!g_Weapon.IsCurrentWeaponGun())
		return;

	RageBot(cmd);
	LegitBot(cmd);
}

void CAimbot::SmoothAimAngles(QAngle MyViewAngles, QAngle AimAngles, QAngle &OutAngles, float Smoothing)
{
	if (Smoothing < 1)
	{
		OutAngles = AimAngles;
		return;
	}

	OutAngles = AimAngles - MyViewAngles;

	OutAngles.Normalize();

	OutAngles /= Smoothing;

	OutAngles = MyViewAngles + OutAngles;

	OutAngles.Normalize();
}

int GetScoreOfHitbox(uint hitbox)
{
	switch (hitbox) {
	case 11:
		return 24;
		break;
	case 10:
	case 9:
	case 8:
	case 7:
	case 0:
		return 16;
		break;
	case 20:
	case 19:
	case 18:
	case 17:
	case 16:
	case 15:
	case 14:
	case 13:
	case 12:
	case 6:
	case 5:
	case 4:
	case 3:
	case 2:
	case 1:
		return 8;
		break;
	}

	return 1;
}

void CAimbot::RageBot(usercmd_s* cmd)
{
	if (!cvar.ragebot_active)
		return;

	if (!cvar.ragebot_aim_enabled)
		return;

	g_Globals.backtrack = cvar.ragebot_backtrack;

	if (!g_Weapon.CanAttack())
		return;

	std::deque<int> aHitboxes;

	if (cvar.ragebot_hitbox == 0)
		aHitboxes.push_back(11);
	else if (cvar.ragebot_hitbox == 1)
		aHitboxes.push_back(10);
	else if (cvar.ragebot_hitbox == 2)
		aHitboxes.push_back(7);
	else if (cvar.ragebot_hitbox == 3)
		aHitboxes.push_back(0);
	else if (cvar.ragebot_hitbox == 4) {
		aHitboxes.push_back(11);
		aHitboxes.push_back(10);
		aHitboxes.push_back(9);
		aHitboxes.push_back(8);
		aHitboxes.push_back(7);
		aHitboxes.push_back(0);
		aHitboxes.push_back(1);
		aHitboxes.push_back(2);
		aHitboxes.push_back(3);
		aHitboxes.push_back(4);
		aHitboxes.push_back(5);
		aHitboxes.push_back(6);
		aHitboxes.push_back(12);
		aHitboxes.push_back(13);
		aHitboxes.push_back(14);
		aHitboxes.push_back(15);
		aHitboxes.push_back(16);
		aHitboxes.push_back(17);
		aHitboxes.push_back(18);
		aHitboxes.push_back(19);
		aHitboxes.push_back(20);
	}
	else if (cvar.ragebot_hitbox == 5) {
		aHitboxes.push_back(11);
		aHitboxes.push_back(10);
		aHitboxes.push_back(9);
		aHitboxes.push_back(8);
		aHitboxes.push_back(7);
		aHitboxes.push_back(12);
		aHitboxes.push_back(17);
		aHitboxes.push_back(0);
	}

	if (aHitboxes.empty())
		return;

	CTarget aTargets[MAX_CLIENTS];
	CTarget *pBestTarget = nullptr;

	for (auto y = 1; y <= MAX_CLIENTS; ++y)
		g_Utils.SpoofPhysent(y, true);

	for (auto j = 0; j < MAX_CLIENTS; ++j)
	{
		auto pPlayer = &g_Player[j];

		if (!pPlayer)
			continue;

		auto pTarget = &aTargets[j];

		if (!pTarget)
			continue;

		pTarget->m_pPlayer = pPlayer;

		if (pTarget->m_pPlayer->m_iIndex == g_Local.m_iIndex)
			continue;

		if (!cvar.ragebot_friendly_fire && pTarget->m_pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pTarget->m_pPlayer->m_bIsDead)
			continue;

		if (!pTarget->m_pPlayer->m_bIsInPVS)
			continue;

		g_Utils.SpoofPhysent(pTarget->m_pPlayer->m_iIndex, false);

		pTarget->m_bValid = true;

		for (auto&& hitbox : aHitboxes)
		{
			Vector vecAimOrigin = pTarget->m_pPlayer->m_vecHitbox[hitbox];

			pmtrace_t Trace;
			g_Engine.pEventAPI->EV_SetTraceHull(2);
			g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecAimOrigin, PM_NORMAL, -1, &Trace);
	
			if (g_Engine.pEventAPI->EV_IndexFromTrace(&Trace) == pTarget->m_pPlayer->m_iIndex)
			{
				pTarget->m_iHitbox = hitbox;
				pTarget->m_iScore += GetScoreOfHitbox(hitbox);
				pBestTarget = pTarget;
				break;
			}
		}
	}

	for (auto f = 1; f <= MAX_CLIENTS; ++f)
		g_Utils.SpoofPhysent(f, false);

	for (auto y = 1; y <= MAX_CLIENTS; ++y)
		g_Utils.SpoofPhysent(y, true);

	if (!pBestTarget && cvar.ragebot_multipoint_type > 0)
	{
		for (auto n = 0; n < MAX_CLIENTS; ++n)
		{
			auto pTarget = &aTargets[n];

			if (!pTarget)
				continue;

			if (!pTarget->m_bValid)
				continue;

			if (!pTarget->m_pPlayer)
				continue;

			for (auto&& hitbox : aHitboxes)
			{
				if ((cvar.ragebot_multipoint_type == 1 && hitbox == aHitboxes[0]) || cvar.ragebot_multipoint_type > 1)
				{
					Vector vecHitbox = pTarget->m_pPlayer->m_vecHitbox[hitbox];

					for (auto point = 0; point < 8; ++point)
					{
						Vector vecAimOrigin = pTarget->m_pPlayer->m_vecHitboxPoints[hitbox][point];

						if (cvar.ragebot_multipoint_scale > 0)
							vecAimOrigin = vecHitbox + ((vecAimOrigin - vecHitbox) * (cvar.ragebot_multipoint_scale / 100));

						pmtrace_t Trace;
						g_Engine.pEventAPI->EV_SetTraceHull(2);
						g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecAimOrigin, PM_NORMAL, -1, &Trace);

						if (g_Engine.pEventAPI->EV_IndexFromTrace(&Trace) == pTarget->m_pPlayer->m_iIndex)
						{
							pTarget->m_iHitbox = hitbox;
							pTarget->m_iPoint = point;
							pTarget->m_iScore += GetScoreOfHitbox(hitbox) / 4;
							pBestTarget = pTarget;
							goto fullbreak_link;
						}
					}
				}
				continue;

			fullbreak_link:
				break;
			}
		}
	}

	for (auto f = 1; f <= MAX_CLIENTS; ++f)
		g_Utils.SpoofPhysent(f, false);

	for (auto y = 1; y <= MAX_CLIENTS; ++y)
		g_Utils.SpoofPhysent(y, true);

	if (!pBestTarget && cvar.ragebot_automatic_penetration)
	{
		for (auto n = 0; n < MAX_CLIENTS; ++n)
		{
			auto pTarget = &aTargets[n];

			if (!pTarget)
				continue;

			if (!pTarget->m_bValid)
				continue;

			if (!pTarget->m_pPlayer)
				continue;

			for (auto&& hitbox : aHitboxes)
			{
				Vector vecAimOrigin = pTarget->m_pPlayer->m_vecHitbox[hitbox];

				if (g_Utils.FireBullets(g_Local.m_vecEye, vecAimOrigin, g_Weapon.CurrentDistance(), g_Weapon.CurrentPenetration(), g_Weapon.CurrentBulletType(), g_Weapon.CurrentDamage(), g_Weapon.CurrentWallPierce()) > 0)
				{
					pTarget->m_iHitbox = hitbox;
					pTarget->m_iPoint = -1;
					pTarget->m_iScore += GetScoreOfHitbox(hitbox) / 2;
					pBestTarget = pTarget;
					break;
				}
			}
		}

		if (!pBestTarget && cvar.ragebot_multipoint_type == 3)
		{
			for (auto n = 0; n < MAX_CLIENTS; ++n)
			{
				auto pTarget = &aTargets[n];

				if (!pTarget)
					continue;

				if (!pTarget->m_bValid)
					continue;

				if (!pTarget->m_pPlayer)
					continue;

				for (auto&& hitbox : aHitboxes)
				{
					Vector vecHitbox = pTarget->m_pPlayer->m_vecHitbox[hitbox];

					for (auto point = 0; point < 8; ++point)
					{
						Vector vecAimOrigin = pTarget->m_pPlayer->m_vecHitboxPoints[hitbox][point];

						if (cvar.ragebot_multipoint_scale > 0)
							vecAimOrigin = vecHitbox + ((vecAimOrigin - vecHitbox) * (cvar.ragebot_multipoint_scale / 100));

						if (g_Utils.FireBullets(g_Local.m_vecEye, vecAimOrigin, g_Weapon.CurrentDistance(), g_Weapon.CurrentPenetration(), g_Weapon.CurrentBulletType(), g_Weapon.CurrentDamage(), g_Weapon.CurrentWallPierce()) > 0)
						{
							pTarget->m_iHitbox = hitbox;
							pTarget->m_iPoint = point;
							pTarget->m_iScore += GetScoreOfHitbox(hitbox) / 8;
							pBestTarget = pTarget;
							goto fullbreak_link2;
						}
					}
					continue;

				fullbreak_link2:
					break;
				}
			}
		}
	}

	for (auto f = 1; f <= MAX_CLIENTS; ++f)
		g_Utils.SpoofPhysent(f, false);

	float flBestFOV = FLT_MAX;
	CTarget *pBestTargetFOV = nullptr;

	for (auto n = 0; n < MAX_CLIENTS; ++n)
	{
		auto pTarget = &aTargets[n];

		if (!pTarget)
			continue;

		if (!pTarget->m_bValid)
			continue;

		if (!pTarget->m_pPlayer)
			continue;

		if (pTarget->m_iHitbox < 0 || pTarget->m_iHitbox >= 21)
			continue;

		Vector vecAimOrigin = pTarget->m_pPlayer->m_vecHitbox[pTarget->m_iHitbox];

		Vector vecDifference = vecAimOrigin - g_Local.m_vecEye;

		float flFOV = g_Local.m_vecForward.AngleBetween(vecDifference);

		if (flFOV < flBestFOV)
		{
			flBestFOV = flFOV;
			pBestTargetFOV = pTarget;
		}
	}

	if (pBestTargetFOV)
		pBestTargetFOV->m_iScore += 8;

	int iBestScore = 0;

	for (auto h = 0; h < MAX_CLIENTS; ++h)
	{
		auto pTarget = &aTargets[h];

		if (!pTarget)
			continue;

		if (!pTarget->m_pPlayer)
			continue;

		if (!pTarget->m_bValid)
			continue;

		if (pTarget->m_iHitbox < 0 || pTarget->m_iHitbox >= 21)
			continue;

		if (pTarget->m_iScore > iBestScore)
		{
			pBestTarget = pTarget;
			iBestScore = pTarget->m_iScore;
		}
	}

	if (pBestTarget)
	{
		if (cvar.ragebot_automatic_scope && g_Weapon.IsCurrentWeaponSniper() && g_Local.m_iFOV == DEFAULT_FOV) 
		{
			cmd->buttons |= IN_ATTACK2;
			return;
		}

		QAngle QAimAngles = QAngle();
		Vector vecAimOrigin = Vector();

		if (pBestTarget->m_iPoint > -1 && pBestTarget->m_iPoint < 8)
			vecAimOrigin = pBestTarget->m_pPlayer->m_vecHitboxPoints[pBestTarget->m_iHitbox][pBestTarget->m_iPoint];
		else
			vecAimOrigin = pBestTarget->m_pPlayer->m_vecHitbox[pBestTarget->m_iHitbox];

		g_Utils.VectorAngles(vecAimOrigin - g_Local.m_vecEye, QAimAngles);

		if (cvar.ragebot_remove_recoil)
		{
			QAimAngles[0] -= g_Weapon.m_vecPunchangle[0] * 2;
			QAimAngles[1] -= g_Weapon.m_vecPunchangle[1] * 2;
		}

		cmd->buttons |= IN_ATTACK;

		g_Utils.MakeAngle(false, QAimAngles, cmd);

		if(!cvar.ragebot_silent_aim && !cvar.ragebot_perfect_silent_aim)
			g_Engine.SetViewAngles(QAimAngles);

		if (cvar.ragebot_perfect_silent_aim)
			g_Globals.send_command = false;
	}
}

void CAimbot::LegitBot(usercmd_s* cmd)
{
	m_bAimbotActive = false;
	m_flLegitBotFOV = 0;

	if (!cvar.legitbot_active)
		return;

	if (m_bTriggerActive)
		return;

	if (!cvar.legitbot[g_Weapon.m_iWeaponID].aim)
		return;

	g_Globals.backtrack = cvar.legitbot_backtrack;

	if (g_Weapon.m_iInReload)
		return;

	std::deque<int> aHitboxes;

	if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_all)
	{
		for (auto i = 0; i < 21; i++)
			aHitboxes.push_back(i);
	}
	else 
	{
		if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_head)
			aHitboxes.push_back(11);

		if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_chest)
		{
			aHitboxes.push_back(7);
			aHitboxes.push_back(8);
			aHitboxes.push_back(9);
			aHitboxes.push_back(10);
			aHitboxes.push_back(11);
			aHitboxes.push_back(12);
			aHitboxes.push_back(17);
		}

		if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_stomach)
			aHitboxes.push_back(0);
	}

	if (aHitboxes.empty())
		return;

	float flBestFOV = cvar.legitbot[g_Weapon.m_iWeaponID].aim_fov;

	if ((g_Weapon.m_vecPunchangle[0] != 0 && cvar.legitbot[g_Weapon.m_iWeaponID].aim_recoil_compensation_pitch > 0) ||
		(g_Weapon.m_vecPunchangle[1] != 0 && cvar.legitbot[g_Weapon.m_iWeaponID].aim_recoil_compensation_yaw > 0))
		flBestFOV = cvar.legitbot[g_Weapon.m_iWeaponID].aim_recoil_compensation_fov;

	if (flBestFOV <= 0)
		return;

	m_flLegitBotFOV = flBestFOV;

	float flSmooth = cvar.legitbot[g_Weapon.m_iWeaponID].aim_smooth_in_attack;

	if (!(cmd->buttons & IN_ATTACK))
		flSmooth = cvar.legitbot[g_Weapon.m_iWeaponID].aim_smooth;

	if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_reaction_time > 0 && GetTickCount() - m_flLastTimeKilled < cvar.legitbot[g_Weapon.m_iWeaponID].aim_reaction_time)
		return;

	QAngle QAngles(cmd->viewangles);

	QAngles[0] += g_Weapon.m_vecPunchangle[0] * 2;
	QAngles[1] += g_Weapon.m_vecPunchangle[1] * 2;

	QAngles.Normalize();

	Vector vecRecoil;

	QAngles.AngleVectors(&vecRecoil, 0, 0);

	int iTarget = -1;
	int iHitbox = -1;

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, true);

	for (int index = 1; index <= g_Engine.GetMaxClients(); ++index)
	{
		if (index == g_Local.m_iIndex)
			continue;

		auto *pPlayer = &g_Player[index - 1];

		if (!pPlayer)
			continue;

		if (!cvar.legitbot_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS)
			continue;

		g_Utils.SpoofPhysent(pPlayer->m_iIndex, false);

		for (auto &&hitbox : aHitboxes)
		{
			Vector vecHitbox = pPlayer->m_vecHitbox[hitbox];

			pmtrace_t Trace;
			g_Engine.pEventAPI->EV_SetTraceHull(2);
			g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecHitbox, PM_NORMAL, -1, &Trace);

			int detect = g_Engine.pEventAPI->EV_IndexFromTrace(&Trace);

			if (detect == index)
			{
				Vector vecForward = vecHitbox - g_Local.m_vecEye;

				float flFOV = vecRecoil.AngleBetween(vecForward);

				if (flFOV < flBestFOV)
				{
					flBestFOV = flFOV;
					iTarget = index;
					iHitbox = hitbox;
				}
			}
		}
	}

	for (auto j = 1; j <= MAX_CLIENTS; ++j)
		g_Utils.SpoofPhysent(j, false);
	
	if (iTarget > 0 && iTarget <= g_Engine.GetMaxClients())
	{
		auto *pPlayer = &g_Player[iTarget - 1];

		if (!pPlayer)
			return;

		m_bAimbotActive = true;

		QAngle QAimAngles, QSmoothAngles;

		Vector vAimOrigin = pPlayer->m_vecHitbox[iHitbox];

		Vector vecForward = vAimOrigin - g_Local.m_vecEye;

		g_Utils.VectorAngles(vecForward, QAimAngles);

		if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_smooth_scale_fov > 0 && g_Weapon.m_vecPunchangle.IsZero2D())
		{
			float flFOV = vecRecoil.AngleBetween(vecForward);

			if (isfinite(flFOV) && flFOV > 0)
				flSmooth = flSmooth - ((flFOV * (flSmooth / cvar.legitbot[g_Weapon.m_iWeaponID].aim_fov) * cvar.legitbot[g_Weapon.m_iWeaponID].aim_smooth_scale_fov) / 100);
		}

		float flRecoilCompensationPitch = cvar.legitbot[g_Weapon.m_iWeaponID].aim_recoil_compensation_pitch / 50.f;
		float flRecoilCompensationYaw = cvar.legitbot[g_Weapon.m_iWeaponID].aim_recoil_compensation_yaw / 50.f;

		QAngle QNewAngles(QAimAngles);

		QNewAngles[0] -= g_Weapon.m_vecPunchangle[0] * flRecoilCompensationPitch;
		QNewAngles[1] -= g_Weapon.m_vecPunchangle[1] * flRecoilCompensationYaw;

		QNewAngles.Normalize();

		SmoothAimAngles(QAngle(cmd->viewangles), QNewAngles, QSmoothAngles, flSmooth);

		bool bPerfectSilentTriggered = false;

		if ((cvar.legitbot[g_Weapon.m_iWeaponID].aim_psilent_triggers == 0 && g_Local.m_flHeight <= 0 && g_Local.m_flVelocity <= 0) ||
			(cvar.legitbot[g_Weapon.m_iWeaponID].aim_psilent_triggers == 1 && g_Local.m_flHeight <= 0) ||
			(cvar.legitbot[g_Weapon.m_iWeaponID].aim_psilent_triggers == 2 && g_Local.m_flHeight > 0) ||
			(cvar.legitbot[g_Weapon.m_iWeaponID].aim_psilent_triggers == 3))
			bPerfectSilentTriggered = true;

		if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_psilent_angle > 0 && g_Weapon.CanAttack() && bPerfectSilentTriggered && (cvar.legitbot[g_Weapon.m_iWeaponID].aim_psilent_type == 1 || cmd->buttons & IN_ATTACK))
		{
			QAngle QAnglePerfectSilent(QAimAngles), QNewAngles;

			QAnglePerfectSilent[0] -= g_Weapon.m_vecPunchangle[0] * 2;
			QAnglePerfectSilent[1] -= g_Weapon.m_vecPunchangle[1] * 2;

			QAnglePerfectSilent.Normalize();

			g_NoSpread.GetSpreadOffset(g_Weapon.m_iRandomSeed, 1, QAnglePerfectSilent, QNewAngles);

			Vector vecDifference = Vector(QSmoothAngles) - Vector(QNewAngles);

			if (vecDifference.Length() < cvar.legitbot[g_Weapon.m_iWeaponID].aim_psilent_angle)
			{
				cmd->buttons |= IN_ATTACK;

				g_Utils.MakeAngle(false, QNewAngles, cmd);

				g_Globals.send_command = false;
				return;
			}
		}

		if (flSmooth <= 0)
			return;

		QSmoothAngles.Normalize();

		g_Utils.MakeAngle(false, QSmoothAngles, cmd);

		g_Engine.SetViewAngles(QSmoothAngles);

		if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_accuracy > 0)
		{
			Vector vecRandom, vecForward, vecRight, vecUp, vecSpreadDir;

			g_NoSpread.GetSpreadXY(g_Weapon.m_iRandomSeed, 1, vecRandom, true);

			QSmoothAngles[0] += g_Weapon.m_vecPunchangle[0] * 2;
			QSmoothAngles[1] += g_Weapon.m_vecPunchangle[1] * 2;

			QSmoothAngles.Normalize();

			QSmoothAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

			if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_accuracy == 1) //bbox + recoil + spread
			{
				vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

				vecSpreadDir.Normalize();

				Vector vecMins = pPlayer->m_vecOrigin + pPlayer->m_vecBoundBoxMins;
				Vector vecMaxs = pPlayer->m_vecOrigin + pPlayer->m_vecBoundBoxMaxs;

				if (!g_Utils.IsBoxIntersectingRay(vecMins, vecMaxs, g_Local.m_vecEye, vecSpreadDir))
					cmd->buttons &= ~IN_ATTACK;
			}
			else if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_accuracy == 2) //recoil
			{
				vecSpreadDir = vecForward;

				vecSpreadDir.Normalize();

				bool bAllowAttack = false;

				for (auto &&hitbox : aHitboxes)
				{
					Vector vecMins = pPlayer->m_vecHitboxMin[hitbox];
					Vector vecMaxs = pPlayer->m_vecHitboxMax[hitbox];

					if (g_Utils.IsBoxIntersectingRay(vecMins, vecMaxs, g_Local.m_vecEye, vecSpreadDir))
					{
						bAllowAttack = true;
						break;
					}
				}

				if (!bAllowAttack)
					cmd->buttons &= ~IN_ATTACK;
			}
			else if (cvar.legitbot[g_Weapon.m_iWeaponID].aim_accuracy == 3) //recoil + spread
			{
				vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

				vecSpreadDir.Normalize();

				bool bAllowAttack = false;

				for (auto &&hitbox : aHitboxes)
				{
					Vector vecMins = pPlayer->m_vecHitboxMin[hitbox];
					Vector vecMaxs = pPlayer->m_vecHitboxMax[hitbox];

					if (g_Utils.IsBoxIntersectingRay(vecMins, vecMaxs, g_Local.m_vecEye, vecSpreadDir))
					{
						bAllowAttack = true;
						break;
					}
				}

				if (!bAllowAttack)
					cmd->buttons &= ~IN_ATTACK;
			}
		}
	}
}

void CAimbot::Trigger(usercmd_s* cmd)
{
	m_bTriggerActive = false;

	if (!cvar.legitbot_active)
		return;

	if (!g_Weapon.IsCurrentWeaponGun())
		return;

	if (!cvar.legitbot[g_Weapon.m_iWeaponID].trigger)
		return;

	if (!m_bTriggerStatus && cvar.legitbot_trigger_key)
		return;

	g_Globals.backtrack = cvar.legitbot_backtrack;

	if (!g_Weapon.CanAttack())
		return;

	if (cvar.legitbot_trigger_only_scoped && g_Weapon.IsCurrentWeaponSniper() && g_Local.m_iFOV == DEFAULT_FOV)
		return;

	std::deque<int> aHitboxes;

	if (cvar.legitbot[g_Weapon.m_iWeaponID].trigger_all)
	{
		for(auto g = 0; g < 21; g++)
			aHitboxes.push_back(g);
	}
	else 
	{
		if (cvar.legitbot[g_Weapon.m_iWeaponID].trigger_head)
			aHitboxes.push_back(11);

		if (cvar.legitbot[g_Weapon.m_iWeaponID].trigger_chest)
		{
			aHitboxes.push_back(7);
			aHitboxes.push_back(8);
			aHitboxes.push_back(9);
			aHitboxes.push_back(10);
			aHitboxes.push_back(11);
			aHitboxes.push_back(12);
			aHitboxes.push_back(17);
		}

		if (cvar.legitbot[g_Weapon.m_iWeaponID].trigger_stomach)
			aHitboxes.push_back(0);
	}

	if (aHitboxes.empty())
		return;

	Vector vecSpreadDir, vecForward, vecRight, vecUp, vecRandom;

	QAngle QAngles(cmd->viewangles);

	if (cvar.legitbot[g_Weapon.m_iWeaponID].trigger_accuracy == 1) //Recoil
	{
		QAngles[0] += g_Weapon.m_vecPunchangle[0] * 2;
		QAngles[1] += g_Weapon.m_vecPunchangle[1] * 2;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		vecSpreadDir = vecForward;

		vecSpreadDir.Normalize();
	}
	else if (cvar.legitbot[g_Weapon.m_iWeaponID].trigger_accuracy == 2) //Recoil + Spread
	{
		QAngles[0] += g_Weapon.m_vecPunchangle[0] * 2;
		QAngles[1] += g_Weapon.m_vecPunchangle[1] * 2;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		g_NoSpread.GetSpreadXY(g_Weapon.m_iRandomSeed, 1, vecRandom, true);

		vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

		vecSpreadDir.Normalize();
	}
	else {
		QAngles.Normalize();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		vecSpreadDir = vecForward;

		vecSpreadDir.Normalize();
	}

	for (auto j = 1; j <= MAX_CLIENTS; ++j) //Disable traces for players
		g_Utils.SpoofPhysent(j, true);

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		auto *pPlayer = &g_Player[i - 1];

		if (!pPlayer)
			continue;

		if (pPlayer->m_iIndex == g_Local.m_iIndex)
			continue;

		if (!cvar.legitbot_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS)
			continue;

		for (auto &&hitbox : aHitboxes)
		{
			auto vecHitbox = pPlayer->m_vecHitbox[hitbox];
			auto vecMin = pPlayer->m_vecHitboxMin[hitbox];
			auto vecMax = pPlayer->m_vecHitboxMax[hitbox];

			if (cvar.legitbot_trigger_hitbox_scale > 0)
			{
				vecMin = vecHitbox + ((vecMin - vecHitbox) * (cvar.legitbot_trigger_hitbox_scale / 100));
				vecMax = vecHitbox + ((vecMax - vecHitbox) * (cvar.legitbot_trigger_hitbox_scale / 100));
			}

			if (g_Utils.IsBoxIntersectingRay(vecMin, vecMax, g_Local.m_vecEye, vecSpreadDir))
			{
				g_Utils.SpoofPhysent(i, false);

				pmtrace_t Trace;
				g_Engine.pEventAPI->EV_SetTraceHull(2);
				g_Engine.pEventAPI->EV_PlayerTrace(g_Local.m_vecEye, vecHitbox, PM_NORMAL, -1, &Trace);

				if (g_Engine.pEventAPI->EV_IndexFromTrace(&Trace) == i)
				{
					m_bTriggerActive = true;
				}
				else if (cvar.legitbot[g_Weapon.m_iWeaponID].trigger_penetration)
				{
					auto iCurrentDamage = g_Utils.FireBullets(g_Local.m_vecEye, vecHitbox, g_Weapon.CurrentDistance(), g_Weapon.CurrentPenetration(),
						g_Weapon.CurrentBulletType(), g_Weapon.CurrentDamage(), g_Weapon.CurrentWallPierce());

					if (iCurrentDamage > 0)
						m_bTriggerActive = true;
				}

				if (m_bTriggerActive)
				{
					cmd->buttons |= IN_ATTACK;
					break;
				}
			}
		}
	}

	for (auto k = 1; k <= MAX_CLIENTS; ++k) //Enable traces for players
		g_Utils.SpoofPhysent(k, false);
}