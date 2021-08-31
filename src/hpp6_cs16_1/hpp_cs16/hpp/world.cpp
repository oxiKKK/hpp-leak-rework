#include "main.h"

CBaseLocalPlayer g_Local;
CWorld g_World;

const char* sTerroristModels[] =
{
	"models/player/leet/leet.mdl",
	"models/player/guerilla/guerilla.mdl",
	"models/player/arctic/arctic.mdl",
	"models/player/terror/terror.mdl",
};

const char* sCounterTerroristModels[] =
{
	"models/player/vip/vip.mdl",
	"models/player/gign/gign.mdl",
	"models/player/urban/urban.mdl",
	"models/player/gsg9/gsg9.mdl",
	"models/player/sas/sas.mdl",
};

static auto GetTeamFromModel(const char *model)
{
	for (auto i = 0; i < IM_ARRAYSIZE(sTerroristModels); i++)
	{
		if (strstr(sTerroristModels[i], model))
			return TERRORIST;
	}

	for (auto i = 0; i < IM_ARRAYSIZE(sCounterTerroristModels); i++)
	{
		if (strstr(sCounterTerroristModels[i], model))
			return CT;
	}

	return UNASSIGNED;
}

static __forceinline void TraceEdge(float& distance, const int x, const int y)
{
	Vector vecStart = client_state->simorg;
	Vector vecEnd = vecStart;
	vecEnd.z = -4096.0F;

	auto* tr = g_Engine.PM_TraceLine(vecStart, vecEnd, PM_STUDIO_IGNORE, client_state->usehull, -1);

	Vector vecDist = (vecEnd - vecStart) * tr->fraction;
	vecStart.z -= 0.1F;
	vecStart.z -= -vecDist.z;

	vecEnd = vecStart;
	vecEnd.y += x * distance;
	vecEnd.x += y * distance;

	tr = g_Engine.PM_TraceLine(vecEnd, vecStart, PM_STUDIO_IGNORE, client_state->usehull, -1);

	if (!tr->startsolid)
		distance = Vector(tr->endpos - vecStart).Length2D();
}

static float GetEdgeDistance()
{
	float flDistance = 250.0F;
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

static void CopyString(char* pszDestination, const char* pcszSource, size_t length)
{
	RtlSecureZeroMemory(pszDestination, length);
	RtlCopyMemory(pszDestination, pcszSource, length);
}

CBasePlayer* CWorld::GetPlayer(const int index)
{
	if (index == 0)
		return m_Player;

	return index < g_Local.m_iIndex ? &m_Player[index - 1] : &m_Player[index - 2];
}

CBaseEntity* CWorld::GetEntity(const int index)
{
	if (index == 0)
		return m_Entity;

	return &m_Entity[index - 1];
}

static bool IsAlive(const cl_entity_s* pGameEntity)
{
	if (pmove->view_ofs.z == PM_DEAD_VIEWHEIGHT)
		return false;

	if (pGameEntity->curstate.iuser1)
		return false;

	return true;
}

static bool CheckLadder()
{
	Vector vecOffset;

	for (auto i = 0; i < pmove->nummoveent; i++)
	{
		const auto pe = &pmove->moveents[i];

		if (pe->model && pmove->PM_GetModelType(pe->model) == mod_brush && pe->skin == CONTENTS_LADDER)
		{
			const auto hull = static_cast<hull_t*>(pmove->PM_HullForBsp(pe, vecOffset));
			const auto num = hull->firstclipnode;

			// Offset the test point appropriately for this hull.
			vecOffset = pmove->origin - vecOffset;

			// Test the player's hull for intersection with this model
			if (pmove->PM_HullPointContents(hull, num, vecOffset) == CONTENTS_EMPTY)
				continue;

			return true;
		}
	}

	return false;
}

void CWorld::UpdateLocalPlayer()
{
	const auto pGameEntity = g_Engine.GetLocalPlayer();

	g_Local.m_bIsDead = true;
	g_Local.m_iIndex = pGameEntity->index;
	g_Local.m_QAngles.Init(cmd::get()->viewangles);
	g_Local.m_QAngles.Normalize();
	g_Local.m_iMessageNum = pGameEntity->curstate.messagenum;

	CopyString(g_Local.m_szModelName, pGameEntity->model->name, sizeof(g_Local.m_szModelName));

	if (g_Local.m_iTeam == UNASSIGNED)
	{
		if(g_PlayerExtraInfo)
			g_Local.m_iTeam = g_PlayerExtraInfo[g_Local.m_iIndex].teamnumber;
		else
			g_Local.m_iTeam = GetTeamFromModel(g_Local.m_szModelName);
	}

	if (g_Local.m_bIsDead = !IsAlive(pGameEntity))
		return;

	g_Local.m_iHealth = g_Local.m_iHealth <= 0 ? 100 : g_Local.m_iHealth;
	g_Local.m_vecEye = pmove->origin + pmove->view_ofs;
	g_Local.m_flVelocity = pmove->velocity.Length2D();
	g_Local.m_bIsDucked = pmove->flags & FL_DUCKING;
	g_Local.m_vecBoundBoxMaxs = Vector(16.0F, 16.0F, g_Local.m_bIsDucked ? 18.0F : 36.0F);
	g_Local.m_vecBoundBoxMins = -g_Local.m_vecBoundBoxMaxs;
	g_Local.m_bIsScoped = g_Local.m_iFOV ? (g_Local.m_iFOV != DEFAULT_FOV) : false;
	g_Local.m_bIsOnGround = pmove->onground != -1;

	if (g_Local.m_bIsOnLadder = !g_Local.m_bIsOnGround)
		g_Local.m_bIsOnLadder = CheckLadder();

	Vector vecEnd(pmove->origin.x, pmove->origin.y, pmove->origin.z - 4096.0F);

	auto pTrace = g_Engine.PM_TraceLine(pmove->origin, vecEnd, PM_NORMAL, client_state->usehull, -1);

	g_Local.m_flHeightGround = pmove->origin[2] - pTrace->endpos[2];
	g_Local.m_flGroundAngle = RAD2DEG(acosf(pTrace->plane.normal[2]));

	static const auto flDuckedPlayerOffset = 13.959999F;

	if (g_Local.m_flHeightGround >= flDuckedPlayerOffset)
	{
		for (auto i = 1; i <= MAX_CLIENTS; i++)
		{
			if (i == g_Local.m_iIndex)
				continue;

			const auto pPlayer = g_World.GetPlayer(i);

			if (pPlayer->m_bIsDead)
				continue;

			if (!pPlayer->m_bIsInPVS)
				continue;

			if (!pPlayer->m_bIsDucked)
				continue;

			if (fabs(pmove->origin[0] - pPlayer->m_vecOrigin[0]) < 32.0F &&
				fabs(pmove->origin[1] - pPlayer->m_vecOrigin[1]) < 32.0F)
			{
				g_Local.m_flHeightGround += pPlayer->m_flHeight;
				g_Local.m_flHeightGround -= flDuckedPlayerOffset;
			}
		}
	}

	pTrace = g_Engine.PM_TraceLine(pmove->origin, vecEnd, PM_NORMAL, HULL_PLAYER_DUCKED, -1);

	g_Local.m_flHeightDucked = pmove->origin[2] - pTrace->endpos[2];

	pTrace = g_Engine.PM_TraceLine(pmove->origin, vecEnd, PM_NORMAL, HULL_POINT, -1);

	g_Local.m_flHeightPlane = pmove->origin[2] - pTrace->endpos[2] - g_Local.m_vecBoundBoxMaxs[2];
	g_Local.m_flHeight = g_Local.m_flHeightGround;

	if (g_Local.m_flGroundAngle >= 0.1F && (g_Local.m_flHeightPlane - g_Local.m_flHeightGround) <= 20.0F)
		g_Local.m_flHeight = g_Local.m_flHeightPlane;

	g_Local.m_flEdgeDist = GetEdgeDistance();
}

static bool IsPlayerDead(const CBasePlayer *pPlayer, const cl_entity_s* pGameEntity)
{
	static const char* pcszSequences[] =
	{
		"death1",
		"death2",
		"death3",
		"head",
		"gutshot",
		"left",
		"back",
		"right",
		"forward",
		"crouch_die"
	};

	if (g_PlayerExtraInfo && !g_PlayerExtraInfo[pGameEntity->index].dead)
		return false;

	if (!(pPlayer->m_iScoreAttribFlags & PLAYER_DEAD))
		return false;

	return (pGameEntity->curstate.sequence == g_Utils.LookupSequence(pGameEntity->model, pcszSequences, IM_ARRAYSIZE(pcszSequences)) ||
		(pGameEntity->curstate.sequence > 100 && pGameEntity->curstate.sequence < 111));
}

void CWorld::UpdatePlayers()
{
	for (auto i = 1; i <= MAX_CLIENTS; i++)
	{
		const auto pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!pGameEntity)
			continue;

		if (pGameEntity->index < 1 || pGameEntity->index > MAX_CLIENTS)
			continue;

		if (pGameEntity->index == g_Local.m_iIndex)
			continue;

		const auto pPlayer = GetPlayer(i);

		pPlayer->m_bIsConnected = false;
		pPlayer->m_bIsInPVS = false;
		pPlayer->m_iIndex = pGameEntity->index;

		hud_player_info_t hud_player_info;
		g_Engine.pfnGetPlayerInfo(pGameEntity->index, &hud_player_info);

		if (hud_player_info.name)
		{
			CopyString(pPlayer->m_szPrintName, hud_player_info.name, sizeof(pPlayer->m_szPrintName));
			pPlayer->m_bIsConnected = true;
		}

		if (!pGameEntity->model)
			continue;

		if (!pGameEntity->player)
			continue;

		if (g_PlayerExtraInfo)
		{
			if (pPlayer->m_iTeam == UNASSIGNED)
				pPlayer->m_iTeam = g_PlayerExtraInfo[pPlayer->m_iIndex].teamnumber;

			pPlayer->m_bHasC4 = g_PlayerExtraInfo[pPlayer->m_iIndex].has_c4;
		}
		else
			pPlayer->m_bHasC4 = (pPlayer->m_iScoreAttribFlags & PLAYER_HAS_C4);

		if (pGameEntity->curstate.messagenum < g_Local.m_iMessageNum)
			continue;

		if ((pPlayer->m_bIsDead = IsPlayerDead(pPlayer, pGameEntity)))
		{
			pPlayer->m_flLastTimeInPVS = 0.f;
			continue;
		}

		pPlayer->m_bIsInPVS = true;
		pPlayer->m_flLastTimeInPVS = (float)client_state->time;

		CopyString(pPlayer->m_szModelName, pGameEntity->model->name, sizeof(pPlayer->m_szModelName));

		if (pPlayer->m_iTeam == UNASSIGNED)
			pPlayer->m_iTeam = GetTeamFromModel(pPlayer->m_szModelName);

		pPlayer->m_vecPreviousOrigin = pPlayer->m_vecOrigin;
		pPlayer->m_vecOrigin = pGameEntity->origin;
		pPlayer->m_flDistance = pPlayer->m_vecOrigin.Distance(pmove->origin);
		pPlayer->m_vecAngles = pGameEntity->angles;
		pPlayer->m_vecAngles[0] *= -3.0F; // stupid quake bug?
		pPlayer->m_vecVelocity = pGameEntity->curstate.origin - pGameEntity->prevstate.origin;
		pPlayer->m_flFrametime = pGameEntity->curstate.animtime - pGameEntity->prevstate.animtime;
		pPlayer->m_vecBoundBoxMaxs = pGameEntity->curstate.maxs;
		pPlayer->m_vecBoundBoxMins = pGameEntity->curstate.mins;

		if (!pPlayer->m_vecBoundBoxMins[0] || !pPlayer->m_vecBoundBoxMins[1] || !pPlayer->m_vecBoundBoxMins[2])	// Bypass nulling
			pPlayer->m_vecBoundBoxMins = -Vector(16, 16, pGameEntity->curstate.usehull == 1 ? 18.0F : 36.0F);	// default bbox player

		if (!pPlayer->m_vecBoundBoxMaxs[0] || !pPlayer->m_vecBoundBoxMaxs[1] || !pPlayer->m_vecBoundBoxMaxs[2])	// Bypass nulling
			pPlayer->m_vecBoundBoxMaxs = Vector(16, 16, pGameEntity->curstate.usehull == 1 ? 18.0F : 36.0F);	// default bbox player

		pPlayer->m_bIsDucked = (pPlayer->m_vecBoundBoxMaxs[2] - pPlayer->m_vecBoundBoxMins[2]) < 54.0F;
		pPlayer->m_flVelocity = (pPlayer->m_flFrametime > 0.0F) ? (pPlayer->m_vecVelocity.Length2D() * (1.0F / pPlayer->m_flFrametime)) : 0.0F;
		pPlayer->m_vecEye = pPlayer->m_vecOrigin + Vector(0, 0, pPlayer->m_bIsDucked ? (float)(PM_VEC_DUCK_VIEW) : (float)(PM_VEC_VIEW));
		pPlayer->m_flHistory = (float)client_state->time;

		if (pGameEntity->curstate.weaponmodel >= 0 && pGameEntity->curstate.weaponmodel < MAX_MODELS)
		{
			auto pModel = client_state->model_precache[pGameEntity->curstate.weaponmodel];

			if (pModel && pModel->name && strlen(pModel->name))
				CopyString(pPlayer->m_szWeaponModelName, pModel->name, sizeof(pPlayer->m_szWeaponModelName));
		}

		if (pPlayer->m_flFrametime != 0.f && pPlayer->m_flVelocity != 0.f)
		{
			Vector vecVelocityRotated;
			vecVelocityRotated.VectorRotate(pPlayer->m_vecVelocity, QAngle(pPlayer->m_vecAngles));

			pPlayer->m_flForwardMove = vecVelocityRotated[0] * (1.0F / pPlayer->m_flFrametime);
			pPlayer->m_flSideMove = -1.0F * vecVelocityRotated[1] * (1.0F / pPlayer->m_flFrametime);

			pPlayer->m_flForwardMove = ImClamp(pPlayer->m_flForwardMove, -250.0F, 250.0F);
			pPlayer->m_flSideMove = ImClamp(pPlayer->m_flSideMove, -250.0F, 250.0F);
		}

		if (pPlayer->m_iHealth <= 0)
		{
			pPlayer->m_iHealth = 100;
			pPlayer->m_iArmorType = ARMOR_NONE;
		}

		Vector vecEnd(pPlayer->m_vecOrigin.x, pPlayer->m_vecOrigin.y, pPlayer->m_vecOrigin.z - 4096.0F);

		auto pTrace = g_Engine.PM_TraceLine(pPlayer->m_vecOrigin, vecEnd, PM_TRACELINE_ANYVISIBLE, pPlayer->m_bIsDucked, -1);

		pPlayer->m_flHeight = -Vector((vecEnd - pPlayer->m_vecOrigin) * pTrace->fraction)[2];
		pPlayer->m_flGroundAngle = DEG2RAD(acosf(pTrace->plane.normal[2]));
	}

	static auto prev_time = client_state->time;

	if (cvar.shared_esp && abs(prev_time - client_state->time) > 0.1)
	{
		std::string ServerAddress = g_Engine.pNetAPI->AdrToString(&g_Status.remote_address);

		if (ServerAddress.find("loopback") == std::string::npos)
		{
			std::stringstream ssmsg;
			ssmsg << "shared>" << ServerAddress << ">";

			auto nPlayers = 0;

			for (auto i = 1; i <= MAX_CLIENTS; i++)
			{
				if (i == g_Local.m_iIndex)
					continue;

				const auto pPlayer = g_World.GetPlayer(i);

				if (pPlayer->m_bIsDead)
					continue;

				if (!pPlayer->m_bIsInPVS)
					continue;

				const auto pGameEntity = g_Engine.GetEntityByIndex(pPlayer->m_iIndex);

				if (!pGameEntity)
					continue;

				ssmsg << pPlayer->m_iIndex << ";";
				ssmsg << pGameEntity->curstate.origin.x << ";";
				ssmsg << pGameEntity->curstate.origin.y << ";";
				ssmsg << pGameEntity->curstate.origin.z << "^";

				nPlayers++;
			}

			if (nPlayers)
			{
				websocket->send(ssmsg.str());
				prev_time = client_state->time;
			}
		}
	}
}

void CWorld::UpdateEntities()
{
	for (auto i = 1; i <= MAX_ENTITIES; i++)
	{
		const auto pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!pGameEntity)
			continue;

		if (!pGameEntity->model)
			continue;

		if (!pGameEntity->index)
			continue;

		const auto pEntity = GetEntity(i);

		pEntity->m_iIndex = pGameEntity->index;
		pEntity->m_bIsInPVS = false;

		if (pGameEntity->curstate.messagenum < g_Local.m_iMessageNum)
			continue;

		pEntity->m_bIsInPVS = true;
		pEntity->m_bIsPlayer = pGameEntity->player && pEntity->m_iIndex >= 1 && pEntity->m_iIndex <= MAX_CLIENTS;
		pEntity->m_vecBoundBoxMins = pGameEntity->curstate.mins;
		pEntity->m_vecBoundBoxMaxs = pGameEntity->curstate.maxs;
		pEntity->m_vecAngles = pGameEntity->angles;
		pEntity->m_vecOrigin = pGameEntity->origin;
		pEntity->m_vecVelocity = pGameEntity->curstate.origin - pGameEntity->prevstate.origin;

		CopyString(pEntity->m_szModelName, pGameEntity->model->name, sizeof(pEntity->m_szModelName));

		if (strstr(pEntity->m_szModelName, "smokegrenade"))
			pEntity->m_bIsSmoke = true;

		if (!pEntity->m_bIsPlayer)
		{
			if (strstr(pEntity->m_szModelName, "/p_")) // Bypass player->curstate.weaponmodel nulling po idei??
			{
				for (auto j = 1; j <= MAX_CLIENTS; j++)
				{
					if (j == g_Local.m_iIndex)
						continue;

					const auto pPlayer = g_World.GetPlayer(j);

					if (pPlayer->m_vecOrigin.Distance(pEntity->m_vecOrigin) < 16.0F)
						CopyString(pPlayer->m_szWeaponModelName, pGameEntity->model->name, sizeof(pPlayer->m_szWeaponModelName));
				}
			}

			if (strstr(pEntity->m_szModelName, "player.mdl")/*need check invisible?*/)
				g_Utils.SpoofPhysent(pEntity->m_iIndex, true);
		}
	}
}

static void CheckCvars()
{
	static cvar_s* cl_lw = nullptr;
	static cvar_s* cl_lc = nullptr;
	static cvar_s* cl_shadows = nullptr;
	static cvar_s* gl_ztrick_old = nullptr;

	if (cl_lw == nullptr) 
		cl_lw = g_Engine.pfnGetCvarPointer("cl_lw");

	if (cl_lc == nullptr) 
		cl_lc = g_Engine.pfnGetCvarPointer("cl_lc");

	if (cl_shadows == nullptr) 
		cl_shadows = g_Engine.pfnGetCvarPointer("cl_shadows");

	if (gl_ztrick_old == nullptr) 
		gl_ztrick_old = g_Engine.pfnGetCvarPointer("gl_ztrick_old");

	if (cl_lw && cl_lw->value != 1.f)
	{
		TraceLog("[hpp] Bad cvar value cl_lw: %.0f => 1\n", cl_lw->value);
		g_Engine.Cvar_SetValue("cl_lw", 1.f);
	}

	if (cl_lc && cl_lc->value != 1.f)
	{
		TraceLog("[hpp] Bad cvar value cl_lc: %.0f => 1\n", cl_lc->value);
		g_Engine.Cvar_SetValue("cl_lc", 1.f);
	}

	if (cl_shadows && cl_shadows->value != 0.f)
	{
		TraceLog("[hpp] Bad cvar value cl_shadows: %.0f => 0\n", cl_shadows->value);
		g_Engine.Cvar_SetValue("cl_shadows", 0.f);
	}

	if (gl_ztrick_old && gl_ztrick_old->value != 0.f)
	{
		TraceLog("[hpp] Bad cvar value gl_ztrick_old: %.0f => 0\n", gl_ztrick_old->value);
		g_Engine.Cvar_SetValue("gl_ztrick_old", 0.f);
	}
}

void CWorld::Update()
{
	CheckCvars();

	UpdateLocalPlayer();
	UpdatePlayers();
	UpdateEntities();
}

void CWorld::Clear()
{
	RtlSecureZeroMemory(&g_Local, sizeof(g_Local));
	RtlSecureZeroMemory(m_Player, sizeof(m_Player));
	RtlSecureZeroMemory(m_Entity, sizeof(m_Entity));

	sequences.clear();
	g_SoundResolver.clear();
	g_Sounds.clear();
	g_ScreenLog.Clear();
}

void CWorld::Reset()
{
	static bool bResetState, bPrevResetState;

	if (!bResetState && client_static->state != ca_active)
		bResetState = true;

	if (bResetState && client_static->state == ca_active)
		bResetState = false;

	if (bResetState != bPrevResetState)
	{
		Clear();

		g_Local.m_bIsDead = true;
		g_Local.m_vecBoundBoxMaxs = Vector(16, 16, 36);
		g_Local.m_vecBoundBoxMins = -g_Local.m_vecBoundBoxMaxs;

		for (auto i = 0; i < IM_ARRAYSIZE(m_Player); i++)
		{
			m_Player[i].m_bIsDead = true;
			m_Player[i].m_vecBoundBoxMaxs = Vector(16, 16, 36);
			m_Player[i].m_vecBoundBoxMins = -m_Player[i].m_vecBoundBoxMaxs;
		}

		bPrevResetState = bResetState;
	}

	if (!g_Local.m_iFOV)
		g_Local.m_iFOV = DEFAULT_FOV;
}