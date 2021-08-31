#include "framework.h"

void CBasePlayer::AllocateMemory()
{
	for (auto& pPlayer : pBaseInfo)
	{
		if (pPlayer.get())
			continue;

		pPlayer = std::make_unique<CBaseEntInfo>();
		pPlayer->m_ClassId = EClassEntity_BasePlayer;
		pPlayer->m_bIsPlayer = true;
		pPlayer->m_bIsDead = true;
		pPlayer->m_iFOV = DEFAULT_FOV;

		Game::GetBoundBox(pPlayer->m_vecBoundBoxMins, pPlayer->m_vecBoundBoxMaxs, HULL_REGULAR);
	}
}

void CBasePlayer::FreeMemory()
{
	for (auto& pPlayer : pBaseInfo)
		DELETE_UNIQUE_PTR(pPlayer);
}

void CBasePlayer::Update()
{
	for (int i = 1; i <= client_state->maxclients; i++)
	{
		auto* pPlayer = pBaseInfo[i - 1].get();

		if (g_Local->m_iEntIndex == i)
		{
			*pPlayer = *g_Local.GetBaseInfo();
			continue;
		}

		auto IsConnected = [](int index)
		{
			const char* name = g_PlayerInfoList.m_pszName(index);

			return name && name[0] && strnlen_s(name, MAX_PLAYER_NAME_LENGTH);
		};

		if (IsConnected(i))
		{
			pPlayer->m_bIsConnected = true;

			strcpy_s(pPlayer->m_szPrintName, g_PlayerInfoList.m_pszName(i));

			cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

			if (!Game::IsValidEntity(pGameEntity))
				continue;

			UpdatePlayer(pPlayer, pGameEntity);
		}
		else
		{
			if (pPlayer->m_bIsConnected)
				ClearPlayer(pPlayer);
		}
	}
}

void CBasePlayer::Clear()
{
	for (auto& pPlayer : pBaseInfo)
		ClearPlayer(pPlayer.get());
}

CBaseEntInfo* CBasePlayer::GetBaseInfo(int index)
{
	return pBaseInfo[index - 1].get();
}

CBaseEntInfo* CBasePlayer::operator[](int index)
{
	assert(index > 0 && index <= MAX_CLIENTS);

	return GetBaseInfo(index);
}

bool CBasePlayer::IsPlayerDead(cl_entity_s* pGameEntity)
{
	static const char* sequences[] = { "death1", "death2", "death3", "head", "gutshot", "left", "back", "right", "forward", "crouch_die" };

	if (!g_PlayerExtraInfo.m_bDead(pGameEntity->index))
		return false;

	return (pGameEntity->curstate.sequence == Game::LookupSequence(pGameEntity->model, sequences, IM_ARRAYSIZE(sequences)) ||
		(pGameEntity->curstate.sequence > 100 && pGameEntity->curstate.sequence < 111));
}

void CBasePlayer::UpdatePlayer(CBaseEntInfo* pPlayer, cl_entity_s* pGameEntity)
{
	pPlayer->m_bIsInPVS = false;
	pPlayer->m_iEntIndex = pGameEntity->index;
	pPlayer->m_iMessageNum = pGameEntity->curstate.messagenum;
	pPlayer->m_iTeamNum = Game::GetTeamNum(g_PlayerExtraInfo.m_pszTeamName(pPlayer->m_iEntIndex));
	pPlayer->m_iSequence = SEQUENCE_IDLE;
	pPlayer->m_iSequenceFrame = 0;
	pPlayer->m_iMoney = -1;

	if (g_PlayerExtraInfo.m_nHealth(pGameEntity->index) != -1)
		pPlayer->m_iHealth = g_PlayerExtraInfo.m_nHealth(pGameEntity->index);

	if (g_PlayerExtraInfo.m_nMoney(pGameEntity->index) != -1)
		pPlayer->m_iMoney = g_PlayerExtraInfo.m_nMoney(pGameEntity->index);

	strcpy_s(pPlayer->m_szModelName, pGameEntity->model->name);

	Game::GetBoundBox(pPlayer->m_vecBoundBoxMins, pPlayer->m_vecBoundBoxMaxs, HULL_REGULAR);

	if (pPlayer->m_iMessageNum < g_Local->m_iMessageNum)
		return;

	pPlayer->m_bIsInPVS = true;
	pPlayer->m_flHistory = static_cast<float>(client_state->time);
	pPlayer->m_flLastTimeInPVS = static_cast<float>(client_state->time);	

	if ((pPlayer->m_bIsDead = IsPlayerDead(pGameEntity)))
		return;

	pPlayer->m_bSoundUpdated = false;

	if (pPlayer->m_iHealth <= 0)
	{
		pPlayer->m_iHealth = Game::GetRespawnHealth();
		pPlayer->m_iArmorType = ARMOR_NONE;
	}

	pPlayer->m_flFrameTime = pGameEntity->curstate.animtime - pGameEntity->prevstate.animtime;

	if (pPlayer->m_flFrameTime)
		pPlayer->m_vecVelocity = (pGameEntity->curstate.origin - pGameEntity->prevstate.origin) / pPlayer->m_flFrameTime;

	Game::GetBoundBox(pPlayer->m_vecBoundBoxMins, pPlayer->m_vecBoundBoxMaxs, pGameEntity->curstate.usehull);

	pPlayer->m_bIsDucked = pGameEntity->curstate.usehull == HULL_DUCKED;
	pPlayer->m_bHasC4 = g_PlayerExtraInfo.m_bHasC4(pGameEntity->index);
	pPlayer->m_bHasDefusalKits = g_PlayerExtraInfo.m_bHasDefusalKits(pGameEntity->index);
	pPlayer->m_iSequence = Cstrike_SequenceInfo[pGameEntity->curstate.sequence];
	pPlayer->m_iSequenceFrame = static_cast<int>(pGameEntity->curstate.frame);
	pPlayer->m_vecPrevOrigin = pPlayer->m_vecOrigin;
	pPlayer->m_vecOrigin = pGameEntity->origin;
	pPlayer->m_vecEyePos = pGameEntity->origin + Vector(0, 0, pPlayer->m_bIsDucked ? PM_VEC_DUCK_VIEW : PM_VEC_VIEW);
	pPlayer->m_QAngles = pGameEntity->angles;
	pPlayer->m_QAngles.x *= -3.f;
	pPlayer->m_QAngles.Normalize();
	pPlayer->m_flVelocity = pPlayer->m_vecVelocity.ToVec2D().Length();
	pPlayer->m_flFallVelocity = -pPlayer->m_vecVelocity.z;

	Math::ComputeMove(pPlayer->m_vecVelocity, pPlayer->m_QAngles, &pPlayer->m_flForwardMove, &pPlayer->m_flSideMove);

	Vector vecEnd(pGameEntity->origin.x, pGameEntity->origin.y, -4096.f);

	pmtrace_t* tr = g_Engine.PM_TraceLine(pGameEntity->origin, vecEnd, PM_TRACELINE_ANYVISIBLE, pGameEntity->curstate.usehull, -1);

	pPlayer->m_flHeightGround = (pGameEntity->origin.z - vecEnd.z) * tr->fraction;
	pPlayer->m_flHeight = pPlayer->m_flHeightGround;
	pPlayer->m_flGroundAngle = RAD2DEG(acos(tr->plane.normal.z));
	pPlayer->m_flDistance = g_Local->m_vecOrigin.Distance(pPlayer->m_vecOrigin);
	pPlayer->m_bIsOnGround = pPlayer->m_flHeight < 1.f;
	
	Game::GetWeaponModelName(pGameEntity->curstate.weaponmodel, pPlayer->m_szWeaponModelName);

	pPlayer->m_bHasShield = strstr(pPlayer->m_szWeaponModelName, "/shield/") ? true : false;
}

void CBasePlayer::ClearPlayer(CBaseEntInfo* pPlayer)
{
	g_pSound->ClearResolvedPlayer(pPlayer->m_iEntIndex);

	pPlayer->m_ClassId = EClassEntity_BasePlayer;

	pPlayer->m_bIsConnected = false;
	pPlayer->m_bIsLocal = false;
	pPlayer->m_bIsInPVS = false;
	pPlayer->m_bIsDead = true;
	pPlayer->m_bIsDucked = false;
	pPlayer->m_bIsOnGround = false;
	pPlayer->m_bIsOnLadder = false;
	pPlayer->m_bIsInWater = false;
	pPlayer->m_bIsScoped = false;
	pPlayer->m_bHasC4 = false;
	pPlayer->m_bHasDefusalKits = false;
	pPlayer->m_bHasShield = false;
	pPlayer->m_bSoundUpdated = false;
	pPlayer->m_bReplaceModel = false;

	pPlayer->m_iEntIndex = 0;
	pPlayer->m_iMessageNum = 0;
	pPlayer->m_iTeamNum = TEAM_UNASSIGNED;
	pPlayer->m_iHealth = 0;
	pPlayer->m_iArmor = 0;
	pPlayer->m_iArmorType = ARMOR_NONE;
	pPlayer->m_iMoney = 0;
	pPlayer->m_iObserverState = OBS_NONE;
	pPlayer->m_iObserverIndex = 0;
	pPlayer->m_iSequence = SEQUENCE_IDLE;
	pPlayer->m_iSequenceFrame = 0;

	pPlayer->m_flVelocity = 0.f;
	pPlayer->m_flFallVelocity = 0.f;
	pPlayer->m_flGroundAngle = 0.f;
	pPlayer->m_flHeightGround = 0.f;
	pPlayer->m_flHeightInDuck = 0.f;
	pPlayer->m_flHeightPlane = 0.f;
	pPlayer->m_flHeight = 0.f;
	pPlayer->m_flEdgeDistance = 0.f;
	pPlayer->m_flFrameTime = 0.f;
	pPlayer->m_flDistance = 0.f;
	pPlayer->m_flForwardMove = 0.f;
	pPlayer->m_flSideMove = 0.f;
	pPlayer->m_flHistory = 0.f;
	pPlayer->m_flLastTimeInPVS = 0.f;
	pPlayer->m_flLastKillTime = 0.f;
	pPlayer->m_flUpdateIntervalTime = 0.f;
	pPlayer->m_flGaitYaw = 0.f;

	memset(pPlayer->m_szPrintName, 0, MAX_PLAYER_NAME_LENGTH);
	memset(pPlayer->m_szModelName, 0, MAX_MODEL_NAME);
	memset(pPlayer->m_szWeaponModelName, 0, MAX_MODEL_NAME);

	pPlayer->m_QAngles.Clear();

	pPlayer->m_vecPrevOrigin.Clear();
	pPlayer->m_vecOrigin.Clear();
	pPlayer->m_vecEyePos.Clear();
	pPlayer->m_vecVelocity.Clear();
	
	Game::GetBoundBox(pPlayer->m_vecBoundBoxMins, pPlayer->m_vecBoundBoxMaxs, HULL_REGULAR);

	for (int i = 0; i < HITBOX_MAX; i++)
	{
		pPlayer->m_vecHitbox[i].Clear();
		pPlayer->m_vecHitboxMin[i].Clear();
		pPlayer->m_vecHitboxMax[i].Clear();
		pPlayer->m_vecOBBMin[i].Clear();
		pPlayer->m_vecOBBMax[i].Clear();
		pPlayer->m_matHitbox[i].Clear();

		for (auto& points : pPlayer->m_vecHitboxPoints[i])
			points.Clear();
	}
}

CBasePlayer g_Player;