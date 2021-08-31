#include "framework.h"

void CBaseLocal::AllocateMemory()
{
	if (!pBaseInfo.get())
	{
		pBaseInfo = std::make_unique<CBaseEntInfo>();
		pBaseInfo->m_ClassId = EClassEntity_BaseLocal;
		pBaseInfo->m_bIsLocal = true;
		pBaseInfo->m_bIsPlayer = true;
		pBaseInfo->m_bIsDead = true;
		pBaseInfo->m_iFOV = DEFAULT_FOV;
	}
}

void CBaseLocal::FreeMemory()
{
	DELETE_UNIQUE_PTR(pBaseInfo);
}

void CBaseLocal::Update(float frametime, usercmd_s* cmd)
{
	auto IsConnected = [](cl_entity_s* pGameEntity)
	{
		if (!pGameEntity)
			return false;

		if (!pGameEntity->model)
			return false;

		if (!pGameEntity->player)
			return false;

		if (!pGameEntity->index)
			return false;

		return true;
	};

	cl_entity_s* pGameEntity = g_Engine.GetLocalPlayer();

	if (IsConnected(pGameEntity))
	{
		pBaseInfo->m_bIsConnected = true;
		pBaseInfo->m_bIsInPVS = false;
		pBaseInfo->m_iEntIndex = pGameEntity->index;
		pBaseInfo->m_iMessageNum = pGameEntity->curstate.messagenum;
		pBaseInfo->m_iTeamNum = Game::GetTeamNum(g_PlayerExtraInfo.m_pszTeamName(pBaseInfo->m_iEntIndex));
		pBaseInfo->m_iObserverState = pGameEntity->curstate.iuser1;
		pBaseInfo->m_iObserverIndex = pGameEntity->curstate.iuser2;
		pBaseInfo->m_iMoney = 0;

		strcpy_s(pBaseInfo->m_szModelName, pGameEntity->model->name);
		strcpy_s(pBaseInfo->m_szPrintName, g_ClientCvarsMap["name"]->string);

		if (pBaseInfo->m_bIsDead = IsDead(pGameEntity))
			return;

		if (g_PlayerExtraInfo.m_nHealth(pBaseInfo->m_iEntIndex) != -1)
			pBaseInfo->m_iHealth = g_PlayerExtraInfo.m_nHealth(pBaseInfo->m_iEntIndex);

		if (g_PlayerExtraInfo.m_nMoney(pBaseInfo->m_iEntIndex) != -1)
			pBaseInfo->m_iMoney = g_PlayerExtraInfo.m_nMoney(pBaseInfo->m_iEntIndex);

		if (pBaseInfo->m_iHealth <= 0)
			pBaseInfo->m_iHealth = Game::GetRespawnHealth();

		pBaseInfo->m_bIsInPVS = true;
		pBaseInfo->m_flLastTimeInPVS = static_cast<float>(client_state->time);
		pBaseInfo->m_bIsDucked = pmove->flags & FL_DUCKING;
		pBaseInfo->m_bIsOnGround = client_state->onground != -1 || pmove->flags & FL_ONGROUND;
		pBaseInfo->m_bIsOnLadder = pmove->movetype == MOVETYPE_FLY || Game::PM_Ladder();
		pBaseInfo->m_bIsInWater = pmove->waterlevel > 1;
		pBaseInfo->m_bHasC4 = g_PlayerExtraInfo.m_bHasC4(pBaseInfo->m_iEntIndex);
		pBaseInfo->m_bHasDefusalKits = g_PlayerExtraInfo.m_bHasDefusalKits(pBaseInfo->m_iEntIndex);
		pBaseInfo->m_iMoney = g_PlayerExtraInfo.m_nMoney(pBaseInfo->m_iEntIndex);
		pBaseInfo->m_iSequence = Cstrike_SequenceInfo[pGameEntity->curstate.sequence];
		pBaseInfo->m_iSequenceFrame = static_cast<int>(pGameEntity->curstate.frame);
		pBaseInfo->m_flFrameTime = frametime;
		pBaseInfo->m_flForwardMove = cmd->forwardmove;
		pBaseInfo->m_flSideMove = cmd->sidemove;
		pBaseInfo->m_flVelocity = client_state->simvel.ToVec2D().Length();
		pBaseInfo->m_flFallVelocity = -client_state->simvel.z;
		pBaseInfo->m_flEdgeDistance = GetEdgeDistance();
		pBaseInfo->m_QAngles = cmd->viewangles;
		pBaseInfo->m_QAngles.Normalize();
		pBaseInfo->m_vecPrevOrigin = pBaseInfo->m_vecOrigin;
		pBaseInfo->m_vecOrigin = client_state->simorg;
		pBaseInfo->m_vecEyePos = client_state->simorg + client_state->viewheight;
		pBaseInfo->m_vecVelocity = client_state->simvel;

		Game::GetBoundBox(pBaseInfo->m_vecBoundBoxMins, pBaseInfo->m_vecBoundBoxMaxs, client_state->usehull);
		Game::FixPlayersPhysents();

		pmtrace_t tr = Game::GetGroundTrace(client_state->usehull);

		pBaseInfo->m_flGroundAngle = RAD2DEG(acos(tr.plane.normal.z));
		pBaseInfo->m_flHeightGround = client_state->simorg.z - tr.endpos.z;

		tr = Game::GetGroundTrace(HULL_DUCKED);

		pBaseInfo->m_flHeightInDuck = client_state->simorg.z - tr.endpos.z;

		tr = Game::GetGroundTrace(HULL_POINT);

		pBaseInfo->m_flHeightPlane = client_state->simorg.z - tr.endpos.z + pBaseInfo->m_vecBoundBoxMins.z;
		pBaseInfo->m_flHeight = pBaseInfo->m_flHeightGround;

		if (pBaseInfo->m_flGroundAngle >= 0.1f && (pBaseInfo->m_flHeightPlane - pBaseInfo->m_flHeightGround) <= 20.f)
			pBaseInfo->m_flHeight = pBaseInfo->m_flHeightPlane;

		Game::GetWeaponModelName(pGameEntity->curstate.weaponmodel, pBaseInfo->m_szWeaponModelName);

		pBaseInfo->m_bHasShield = strstr(pBaseInfo->m_szWeaponModelName, "/shield/") ? true : false;
	}
}

CBaseEntInfo* CBaseLocal::GetBaseInfo()
{
	return pBaseInfo.get();
}

CBaseEntInfo* CBaseLocal::operator->()
{
	return GetBaseInfo();
}

bool CBaseLocal::IsDead(cl_entity_s* pGameEntity)
{
	if (!client_state->weapons)
		return true;

	if (client_state->viewheight == PM_DEAD_VIEWHEIGHT)
		return true;

	if (pBaseInfo->m_iObserverState)
		return true;

	if (!pBaseInfo->m_iFOV)
		return true;

	return false;
}

static void TraceEdge(float& dist, int x, int y)
{
	Vector vecSrc, vecEnd, vecDelta;

	vecSrc = client_state->simorg;
	vecEnd = vecSrc;
	vecEnd.z = -4096.f;

	pmtrace_t tr;

	g_Engine.pEventAPI->EV_SetTraceHull(client_state->usehull);
	g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

	vecDelta = (vecEnd - vecSrc) * tr.fraction;
	vecSrc.z -= 0.1f;
	vecSrc.z -= -vecDelta.z;

	vecEnd = vecSrc;
	vecEnd.y += x * dist;
	vecEnd.x += y * dist;

	g_Engine.pEventAPI->EV_SetTraceHull(client_state->usehull);
	g_Engine.pEventAPI->EV_PlayerTrace(vecEnd, vecSrc, PM_NORMAL, -1, &tr);

	if (!tr.startsolid)
	{
		Vector vecDst = tr.endpos - vecSrc;
		dist = vecDst.ToVec2D().Length();
	}
};

float CBaseLocal::GetEdgeDistance()
{
	float flDistance = 250.f;

	TraceEdge(flDistance, -1, 0);
	TraceEdge(flDistance, 1, 0);
	TraceEdge(flDistance, 0, 1);
	TraceEdge(flDistance, 0, -1);
	TraceEdge(flDistance, -1, -1);
	TraceEdge(flDistance, 1, 1);
	TraceEdge(flDistance, 1, -1);
	TraceEdge(flDistance, -1, 1);

	return flDistance;
}

CBaseLocal g_Local;