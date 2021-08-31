#include "main.h"

CKnifebot g_KnifeBot;

void CKnifebot::Run(struct usercmd_s *cmd)
{
	if (!cvar.knifebot)
		return;

	if (!g_Weapon.IsCurrentWeaponKnife())
		return;

	g_Globals.backtrack = cvar.knifebot_backtrack;

	if (!g_Weapon.CanAttack())
		return;

	for (auto index = 1; index <= g_Engine.GetMaxClients(); ++index)
		g_Utils.SpoofPhysent(index, true);

	for (auto index = 1; index <= g_Engine.GetMaxClients(); ++index)
	{
		if (index == g_Local.m_iIndex)
			continue;

		auto *pPlayer = &g_Player[index - 1];

		if (!pPlayer)
			continue;

		if (!cvar.knifebot_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS)
			continue;

		g_Utils.SpoofPhysent(index, false);

		if (Attack(pPlayer, cmd))
			break;
	}

	for (auto index = 1; index <= g_Engine.GetMaxClients(); ++index)
		g_Utils.SpoofPhysent(index);
}

bool CKnifebot::Attack(CBasePlayer *pPlayer, struct usercmd_s *cmd)
{
	if (!pPlayer)
		return false;

	std::deque <Vector> vecDot;

	float flDistance = cvar.knifebot_distance_stab;

	if (cvar.knifebot_type == 1)
		flDistance = cvar.knifebot_distance_slash;

	CalculateDot(pPlayer, vecDot);

	Vector vecSrc = g_Local.m_vecEye;
	Vector vecAim = ((vecDot[4] - vecSrc) / Vector(vecDot[4] - vecSrc).Length());
	Vector vecEnd = vecSrc + vecAim * flDistance;

	pmtrace_t Trace;
	g_Engine.pEventAPI->EV_SetTraceHull(2);
	g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &Trace);

	if (g_Engine.pEventAPI->EV_IndexFromTrace(&Trace) == pPlayer->m_iIndex)
	{
		QAngle QAimAngle;

		g_Utils.VectorAngles(vecAim, QAimAngle);

		g_Utils.MakeAngle(false, QAimAngle, cmd);

		if (cvar.knifebot_type == 1)
			cmd->buttons |= IN_ATTACK;
		else
			cmd->buttons |= IN_ATTACK2;

		return true;
	}

	return false;
}

void CKnifebot::CalculateDot(CBasePlayer *pPlayer, std::deque <Vector> &dDot)
{
	if (!pPlayer)
		return;

	Vector vecDot[5];

	Vector vecForward, vecRight;

	g_Engine.pfnAngleVectors(pPlayer->m_vecAngles, vecForward, vecRight, 0);

	vecDot[0] = pPlayer->m_vecOrigin + vecForward * 16 + vecRight * 16;
	vecDot[1] = pPlayer->m_vecOrigin + vecForward * 16 - vecRight * 16;
	vecDot[2] = pPlayer->m_vecOrigin - vecForward * 16 + vecRight * 16;
	vecDot[3] = pPlayer->m_vecOrigin - vecForward * 16 - vecRight * 16;
	vecDot[4] = pPlayer->m_vecOrigin;

	float flCritical = pPlayer->m_vecBoundBoxMaxs.z;

	for (auto i = 0; i < 5; i++)
	{
		if (vecDot[i].z - flCritical > g_Local.m_vecEye.z)
			vecDot[i].z = vecDot[i].z - flCritical;
		else if (vecDot[i].z + flCritical < g_Local.m_vecEye.z)
			vecDot[i].z = vecDot[i].z + flCritical;
		else
			vecDot[i].z = g_Local.m_vecEye.z;

		dDot.push_back(vecDot[i]);
	}
}
