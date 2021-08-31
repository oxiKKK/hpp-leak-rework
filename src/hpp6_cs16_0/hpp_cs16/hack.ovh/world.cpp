#include "main.h"

CWorld g_World;
CBaseLocalPlayer g_Local;
std::array<CBasePlayer, MAX_CLIENTS> g_Player;
std::array<CBaseEntity, MAX_ENTITIES> g_Entity;
CInfoMapParameters g_MapInfo;
std::deque<my_sound_t> g_Sounds;
std::deque<CSoundResolver> g_SoundResolver;

bool CPlayerInfo::_IsLocalAlive(const CBaseLocalPlayer* pPlayer, const usercmd_s* cmd, const cl_entity_s* pGameEntity)
{
	return (pPlayer->m_iHealth > 0 || !cmd->viewangles.IsZero()) && pPlayer->m_iTeam != UNASSIGNED &&
		pmove->view_ofs.z != PM_DEAD_VIEWHEIGHT && !pGameEntity->curstate.iuser1;
}

bool CPlayerInfo::_IsPlayerDead(const cl_entity_s* pGameEntity)
{
	static const char* pcszSequenceNames[] =
	{
		"death1", "death2", "death3", "head", "gutshot",
		"left", "back", "right", "forward", "crouch_die"
	};

	return g_PlayerExtraInfo[pGameEntity->index].dead && pGameEntity->curstate.sequence ==
		g_Utils.LookupSequence(pGameEntity->model, &pcszSequenceNames[0], IM_ARRAYSIZE(pcszSequenceNames)) ||
		(pGameEntity->curstate.sequence > 100 && pGameEntity->curstate.sequence < 111);
}

void CPlayerInfo::UpdateLocalPlayer(CBaseLocalPlayer* pPlayer, float frametime, const usercmd_s* cmd)
{
	if (pPlayer == nullptr)
		return;

	pPlayer->m_bIsDead = true;

	const auto* const pGameEntity = g_Engine.GetLocalPlayer();

	if (pGameEntity == nullptr)
		return;

	if (pGameEntity->model == nullptr)
		return;

	if (!pGameEntity->player)
		return;

	if (!pGameEntity->index)
		return;

	pPlayer->m_iIndex = pGameEntity->index;
	pPlayer->m_flFrametime = frametime;
	pPlayer->m_vecAngles = cmd->viewangles;
	pPlayer->m_iMessageNum = pGameEntity->curstate.messagenum;

	if (pPlayer->m_iTeam == UNASSIGNED && g_PlayerExtraInfo[pPlayer->m_iIndex].teamnumber)
		pPlayer->m_iTeam = gsl::narrow_cast<int>(g_PlayerExtraInfo[pPlayer->m_iIndex].teamnumber);

	if ((pPlayer->m_bIsDead = !_IsLocalAlive(pPlayer, cmd, pGameEntity)) == true)
		return;

	if (pPlayer->m_iFOV)
		pPlayer->m_bIsScoped = pPlayer->m_iFOV != DEFAULT_FOV;

	pPlayer->m_flVelocity = pmove->velocity.Length2D();
	pPlayer->m_vecOrigin = pmove->origin;
	pPlayer->m_vecEye = pPlayer->m_vecOrigin + pmove->view_ofs;

	if (pPlayer->m_vecBoundBoxMins.IsZero())
		pPlayer->m_vecBoundBoxMins = Vector(-16, -16, pmove->flags & FL_DUCKING ? -18.f : -36.f);

	if (pPlayer->m_vecBoundBoxMaxs.IsZero())
		pPlayer->m_vecBoundBoxMaxs = Vector(16, 16, pmove->flags & FL_DUCKING ? 18.f : 36.f);

	Vector vecEnd = Vector(pPlayer->m_vecOrigin.x, pPlayer->m_vecOrigin.y, -4096);
	pmtrace_s* pTrace = nullptr;

	if ((pTrace = g_Engine.PM_TraceLine(pPlayer->m_vecOrigin, vecEnd, PM_NORMAL, pmove->flags & FL_DUCKING ? 1 : 0, -1)) != nullptr)
	{
		pPlayer->m_flHeightGround = pPlayer->m_vecOrigin.z - pTrace->endpos.z;
		pPlayer->m_flGroundAngle = DEG2RAD(acosf(pTrace->plane.normal.z));

		if ((pTrace = g_Engine.PM_TraceLine(pPlayer->m_vecOrigin, vecEnd, PM_NORMAL, 2, -1)) != nullptr)
		{
			pPlayer->m_flHeightPlane = pPlayer->m_vecOrigin.z - pTrace->endpos.z - (pmove->flags & FL_DUCKING ? 18.f : 36.f);
			pPlayer->m_flHeight = pPlayer->m_flHeightGround;

			if (pPlayer->m_flGroundAngle >= 0.1f && (pPlayer->m_flHeightPlane - pPlayer->m_flHeightGround) <= 20.f)
				pPlayer->m_flHeight = pPlayer->m_flHeightPlane;
		}
	}

	const auto* const pPlayerInfo = g_Studio.PlayerInfo(pPlayer->m_iIndex - 1);

	if (pPlayerInfo)
	{
		pPlayer->m_flLowerBodyYaw = pPlayerInfo->gaityaw;
		pPlayer->m_iOutPing = pPlayerInfo->ping;
	}
}

void CPlayerInfo::UpdatePlayers(std::array<CBasePlayer, MAX_CLIENTS>& Player)
{
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		auto pPlayer = &Player.at(i - 1);

		if (pPlayer == nullptr)
			return;

		if ((pPlayer->m_iIndex = i) == g_Local.m_iIndex)
			continue;

		pPlayer->m_bIsInPVS = false;

		const auto pGameEntity = g_Engine.GetEntityByIndex(pPlayer->m_iIndex);

		if (pGameEntity == nullptr)
			continue;

		if (pGameEntity->model == nullptr)
			continue;

		if (!pGameEntity->player)
			continue;

		if (pGameEntity->index < 1)
			continue;

		if (pGameEntity->index > MAX_CLIENTS)
			continue;

		if (pGameEntity->curstate.messagenum < g_Local.m_iMessageNum)
			continue;

		pPlayer->m_bIsInPVS = true;

		if ((pPlayer->m_bIsDead = _IsPlayerDead(pGameEntity)) == true)
			continue;

		pPlayer->m_vecPreviousOrigin = pPlayer->m_vecOrigin;
		pPlayer->m_vecOrigin = g_Globals.interpolate ? pGameEntity->origin : pGameEntity->curstate.origin;
		pPlayer->m_flDistance = pPlayer->m_vecOrigin.Distance(g_Local.m_vecOrigin);
		pPlayer->m_vecAngles = g_Globals.interpolate ? pGameEntity->angles : pGameEntity->curstate.angles;
		pPlayer->m_vecAngles.x *= -3.f; //stupid quake bug?
		pPlayer->m_vecVelocity = pGameEntity->curstate.origin - pGameEntity->prevstate.origin;
		pPlayer->m_flFrametime = pGameEntity->curstate.animtime - pGameEntity->prevstate.animtime;
		pPlayer->m_vecBoundBoxMaxs = pGameEntity->curstate.maxs;
		pPlayer->m_vecBoundBoxMins = pGameEntity->curstate.mins;
		pPlayer->m_dwHistory = GetTickCount();

		if (!pPlayer->m_vecBoundBoxMins.x || !pPlayer->m_vecBoundBoxMins.y || !pPlayer->m_vecBoundBoxMins.z)	//Bypass nulling
			pPlayer->m_vecBoundBoxMins = Vector(-16, -16, pGameEntity->curstate.usehull == 1 ? -18.f : -36.f);	//default bbox player

		if (!pPlayer->m_vecBoundBoxMaxs.x || !pPlayer->m_vecBoundBoxMaxs.y || !pPlayer->m_vecBoundBoxMaxs.z)	//Bypass nulling
			pPlayer->m_vecBoundBoxMaxs = Vector(16, 16, pGameEntity->curstate.usehull == 1 ? 18.f : 36.f);		//default bbox player

		pPlayer->m_bIsDucked = (pPlayer->m_vecBoundBoxMaxs.z - pPlayer->m_vecBoundBoxMins.z) < 54.f;
		pPlayer->m_flVelocity = (pPlayer->m_flFrametime > 0.f) ? (pPlayer->m_vecVelocity.Length2D() * (1.f / pPlayer->m_flFrametime)) : 0.f;
		pPlayer->m_vecEye = pPlayer->m_vecOrigin + Vector(0, 0, pPlayer->m_bIsDucked ? PM_VEC_DUCK_VIEW : PM_VEC_VIEW);

		const auto* const pPlayerInfo = g_Studio.PlayerInfo(pGameEntity->index - 1);

		if (pPlayerInfo)
		{
			pPlayer->m_flLowerBodyYaw = pPlayerInfo->gaityaw;
			pPlayer->m_sPrintName = &pPlayerInfo->name[0];
		}

		Vector vecEnd = Vector(pPlayer->m_vecOrigin.x, pPlayer->m_vecOrigin.y, -4096);
		pmtrace_s* pTrace = nullptr;

		if ((pTrace = g_Engine.PM_TraceLine(pPlayer->m_vecOrigin, vecEnd, PM_TRACELINE_ANYVISIBLE, pPlayer->m_bIsDucked, -1)) != nullptr)
		{
			pPlayer->m_flHeight = -Vector((vecEnd - pPlayer->m_vecOrigin) * pTrace->fraction).z;
			pPlayer->m_flGroundAngle = DEG2RAD(acosf(pTrace->plane.normal.z));
		}

		if (pGameEntity->curstate.weaponmodel >= 0 && pGameEntity->curstate.weaponmodel < MAX_MODELS)
		{
			const auto* const pModel = client_state->model_precache[pGameEntity->curstate.weaponmodel];

			if (pModel && strlen(&pModel->name[0]) > 0)
				pPlayer->m_sPlayerModel = &pModel->name[0];
		}

		g_Utils.ComputeMove(pPlayer);

		if (pPlayer->m_iHealth < 1)
		{
			pPlayer->m_iHealth = 100;
			pPlayer->m_iArmorType = ARMOR_NONE;
		}
	}
}

void CPlayerInfo::ClearLocalPlayer(CBaseLocalPlayer* pPlayer)
{
	if (pPlayer == nullptr)
		return;

	pPlayer->m_iIndex = 0;
	pPlayer->m_iTeam = UNASSIGNED;
	pPlayer->m_iHealth = 0;
	pPlayer->m_iArmor = ARMOR_NONE;
	pPlayer->m_iFOV = DEFAULT_FOV;
	pPlayer->m_iOutPing = 0;
	pPlayer->m_iMessageNum = 0;

	pPlayer->m_bIsDead = true;
	pPlayer->m_bIsScoped = false;

	pPlayer->m_flFrametime = 0.f;
	pPlayer->m_flVelocity = 0.f;
	pPlayer->m_flHeightGround = 0.f;
	pPlayer->m_flGroundAngle = 0.f;
	pPlayer->m_flHeight = 0.f;
	pPlayer->m_flHeightPlane = 0.f;
	pPlayer->m_flLowerBodyYaw = 0.f;

	pPlayer->m_vecAngles.Clear();
	pPlayer->m_vecOrigin.Clear();
	pPlayer->m_vecEye.Clear();
	pPlayer->m_vecForward.Clear();
	pPlayer->m_vecUp.Clear();
	pPlayer->m_vecRight.Clear();
	pPlayer->m_vecBoundBoxMins.Clear();
	pPlayer->m_vecBoundBoxMaxs.Clear();
}

void CPlayerInfo::ClearPlayers(std::array<CBasePlayer, MAX_CLIENTS>& Player)
{
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		auto pPlayer = &Player.at(i);

		if (pPlayer == nullptr)
			continue;

		pPlayer->m_iIndex = 0;
		pPlayer->m_iTeam = UNASSIGNED;
		pPlayer->m_iHealth = 0;
		pPlayer->m_iArmorType = ARMOR_NONE;

		pPlayer->m_bIsInPVS = false;
		pPlayer->m_bIsDead = true;
		pPlayer->m_bIsDucked = false;
		pPlayer->m_IsTransparent = false;

		pPlayer->m_flVelocity = 0.f;
		pPlayer->m_flHeightGround = 0.f;
		pPlayer->m_flGroundAngle = 0.f;
		pPlayer->m_flHeight = 0.f;
		pPlayer->m_flHeightPlane = 0.f;
		pPlayer->m_flFrametime = 0.f;
		pPlayer->m_flDistance = 0.f;
		pPlayer->m_flLowerBodyYaw = 0.f;
		pPlayer->m_flForwardMove = 0.f;
		pPlayer->m_flSideMove = 0.f;

		pPlayer->m_vecAngles.Clear();
		pPlayer->m_vecVelocity.Clear();
		pPlayer->m_vecOrigin.Clear();
		pPlayer->m_vecPreviousOrigin.Clear();

		for (int j = 0; j < IM_ARRAYSIZE(pPlayer->m_vecHitbox); ++j)
		{
			pPlayer->m_vecHitbox[j].Clear();
			pPlayer->m_vecHitboxMin[j].Clear();
			pPlayer->m_vecHitboxMax[j].Clear();

			for (int l = 0; l < IM_ARRAYSIZE(pPlayer->m_vecHitboxPoints[j]); ++l)
				pPlayer->m_vecHitboxPoints[j][l].Clear();
		}

		pPlayer->m_vecBoundBoxMins = Vector(-16, -16, -36);		//default bbox player
		pPlayer->m_vecBoundBoxMaxs = Vector(16, 16, 36);		//default bbox player
		pPlayer->m_vecEye.Clear();

		pPlayer->m_dwHistory = 0;

		//pPlayer->m_sPrintName.clear();
		//pPlayer->m_sPlayerModel.clear();
	}
}

void CEntityInfo::UpdateEntities(std::array<CBaseEntity, MAX_ENTITIES>& Entity)
{
	for (int i = 0; i < MAX_ENTITIES; ++i)
	{
		auto pEntity = &Entity.at(i);

		if (pEntity == nullptr)
			continue;

		pEntity->m_bIsInPVS = false;

		const auto pGameEntity = g_Engine.GetEntityByIndex(i);

		if (pGameEntity == nullptr)
			continue;

		if (!pGameEntity->index)
			continue;

		if (pGameEntity->curstate.messagenum < g_Local.m_iMessageNum)
			continue;

		pEntity->m_iIndex = pGameEntity->index;
		pEntity->m_bIsInPVS = true;
		pEntity->m_bIsPlayer = (pGameEntity->player && pEntity->m_iIndex > 0 && pEntity->m_iIndex <= MAX_CLIENTS);
		pEntity->m_vecBoundBoxMins = pGameEntity->curstate.mins;
		pEntity->m_vecBoundBoxMaxs = pGameEntity->curstate.maxs;
		pEntity->m_vecAngles = g_Globals.interpolate ? pGameEntity->angles : pGameEntity->curstate.angles;
		pEntity->m_vecOrigin = g_Globals.interpolate ? pGameEntity->origin : pGameEntity->curstate.origin;
		pEntity->m_vecVelocity = pGameEntity->curstate.origin - pGameEntity->prevstate.origin;

		if (pGameEntity->model && !pEntity->m_bIsPlayer)
		{
			pEntity->m_sPrintName = &pGameEntity->model->name[0];

			if (pEntity->m_sPrintName.find("/p_") != std::string::npos) //Bypass player->curstate.weaponmodel nulling po idei??
			{
				for (int j = 1; j <= MAX_CLIENTS; ++j)
				{
					auto pPlayer = &g_Player.at(j - 1);

					if (pPlayer == nullptr)
						continue;

					if (pPlayer->m_iIndex == g_Local.m_iIndex)
						continue;

					if (pPlayer->m_vecOrigin.Distance(pEntity->m_vecOrigin) < 16.f)
						pPlayer->m_sPlayerModel = pEntity->m_sPrintName;
				}
			}

			if (pEntity->m_sPrintName.find("player.mdl") != std::string::npos /*need check invisible?*/)
				g_Utils.SpoofPhysent(i, true);
		}
	}
}

void CEntityInfo::ClearEntities(std::array<CBaseEntity, MAX_ENTITIES>& Entity)
{
	for (int i = 0; i < MAX_ENTITIES; ++i)
	{
		auto pEntity = &Entity.at(i);

		if (pEntity == nullptr)
			continue;

		pEntity->m_iIndex = 0;

		pEntity->m_bIsInPVS = false;
		pEntity->m_bIsPlayer = false;

		pEntity->m_vecOrigin.Clear();
		pEntity->m_vecAngles.Clear();
		pEntity->m_vecVelocity.Clear();
		pEntity->m_vecBoundBoxMins.Clear();
		pEntity->m_vecBoundBoxMaxs.Clear();

		//pEntity->m_sPrintName.clear();
	}
}

void CWorld::Update(float frametime, const usercmd_s* cmd)
{
	enum
	{
		cl_himodels, cl_minmodels, cl_min_ct, cl_min_t, cl_lw,
		cl_lc, cl_shadows, gl_ztrick_old, ex_interp, MAX_CVARS,
	};

	static cvar_s* cvar[MAX_CVARS];

	if ((cvar[cl_himodels] = g_Engine.pfnGetCvarPointer("cl_himodels")) != nullptr)
		cvar[cl_himodels]->value = 0.f;

	if ((cvar[cl_minmodels] = g_Engine.pfnGetCvarPointer("cl_minmodels")) != nullptr)
		cvar[cl_minmodels]->value = 1.f;

	if ((cvar[cl_min_ct] = g_Engine.pfnGetCvarPointer("cl_min_ct")) != nullptr)
		cvar[cl_min_ct]->value = 2.f;

	if ((cvar[cl_min_t] = g_Engine.pfnGetCvarPointer("cl_min_t")) != nullptr)
		cvar[cl_min_t]->value = 5.f;

	if ((cvar[cl_lw] = g_Engine.pfnGetCvarPointer("cl_lw")) != nullptr)
		cvar[cl_lw]->value = 1.f;

	if ((cvar[cl_lc] = g_Engine.pfnGetCvarPointer("cl_lc")) != nullptr)
		cvar[cl_lc]->value = 1.f;

	if ((cvar[cl_shadows] = g_Engine.pfnGetCvarPointer("cl_shadows")) != nullptr)
		cvar[cl_shadows]->value = 0.f;

	if ((cvar[gl_ztrick_old] = g_Engine.pfnGetCvarPointer("gl_ztrick_old")) != nullptr)
		cvar[gl_ztrick_old]->value = 0.f;

	if (cvar[ex_interp] == nullptr)
		cvar[ex_interp] = g_Engine.pfnGetCvarPointer("ex_interp");

	g_Globals.interpolate = cvar[cl_lc] && cvar[cl_lw] && cvar[ex_interp] &&
		cvar[cl_lc]->value > 0.f && cvar[cl_lw]->value > 0.f && cvar[ex_interp]->value > 0.f;

	UpdateLocalPlayer(&g_Local, frametime, cmd);
	UpdatePlayers(g_Player);
	UpdateEntities(g_Entity);
}

void CWorld::Clear() noexcept
{
	RtlSecureZeroMemory(&g_Local, sizeof(CBaseLocalPlayer));

	for (int i = 0; i < MAX_CLIENTS; ++i)
		RtlSecureZeroMemory(&g_Player.at(i), sizeof(CBasePlayer));

	for (int i = 0; i < MAX_ENTITIES; ++i)
		RtlSecureZeroMemory(&g_Entity.at(i), sizeof(CBaseEntity));

	RtlSecureZeroMemory(&g_MapInfo, sizeof(CInfoMapParameters));
}