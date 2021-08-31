#include "main.h"

CUtils g_Utils;

bool CUtils::IsBoxIntersectingRay(const Vector& vecBoundBoxMin, const Vector& vecBoundingBoxMax, const Vector& vecOrigin, const Vector& vecDelta)
{
	// FIXME: Surely there's a faster way
	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;

	for (int i = 0; i < 3; ++i)
	{
		// Parallel case...
		if (fabs(vecDelta[i]) < 1e-8)
		{
			// Check that origin is in the box
			// if not, then it doesn't intersect..
			if ((vecOrigin[i] < vecBoundBoxMin[i]) || (vecOrigin[i] > vecBoundingBoxMax[i]))
				return false;

			continue;
		}

		// non-parallel case
		// Find the t's corresponding to the entry and exit of
		// the ray along x, y, and z. The find the furthest entry
		// point, and the closest exit point. Once that is done,
		// we know we don't collide if the closest exit point
		// is behind the starting location. We also don't collide if
		// the closest exit point is in front of the furthest entry point

		float invDelta = 1.f / vecDelta[i];
		float t1 = (vecBoundBoxMin[i] - vecOrigin[i]) * invDelta;
		float t2 = (vecBoundingBoxMax[i] - vecOrigin[i]) * invDelta;

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}

		if (t1 > tmin)
			tmin = t1;

		if (t2 < tmax)
			tmax = t2;

		if (tmin > tmax)
			return false;

		if (tmax < 0)
			return false;
	}

	return true;
}

bool CUtils::IsTeleported(const Vector& v1, const Vector& v2)
{
	for (int i = 0; i < 3; ++i)
	{
		if (fabs(v1[i] - v2[i]) > 128)
			return true;
	}

	return false;
}

void CUtils::GetHitboxes(cl_entity_s* pGameEntity)
{
	if (pGameEntity && pGameEntity->model && pGameEntity->index > 0 && pGameEntity->index < MAX_ENTITIES)
	{
		auto iResolvedIndex = -1;

		auto pEntity = &g_Entity[pGameEntity->index];

		if (pEntity)
		{
			if (pEntity->m_bIsPlayer)
			{
				iResolvedIndex = pEntity->m_iIndex;
			}
			else if (pEntity->m_sPrintName.find("/player", 0) != std::string::npos)
			{
				for (auto j = 0; j < MAX_CLIENTS; ++j)
				{
					auto pPlayer = &g_Player[j];

					if (!pPlayer)
						continue;

					if (pPlayer->m_iIndex == g_Local.m_iIndex)
						continue;

					if (pPlayer->m_vecOrigin.Distance(pEntity->m_vecOrigin) < 16)
						iResolvedIndex = j;
				}
			}
		}

		if (iResolvedIndex > 0 && iResolvedIndex <= MAX_CLIENTS)
		{
			auto pPlayer = &g_Player[iResolvedIndex - 1];

			if (!pPlayer)
				return;

			static cvar_s* cl_minmodels = g_Engine.pfnGetCvarPointer("cl_minmodels");

			if (!cl_minmodels)
				cl_minmodels = g_Engine.pfnGetCvarPointer("cl_minmodels");

			model_t* pModel = (cl_minmodels && cl_minmodels->value > 0)
				? pModel = pGameEntity->model : g_Studio.SetupPlayerModel(pGameEntity->curstate.number - 1);

			if (!pModel)
				return;

			auto pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(pModel);

			if (!pStudioHeader || !pStudioHeader->numbodyparts)
				return;

			auto pBoneMatrix = (TransformMatrix*)g_Studio.StudioGetBoneTransform();
			auto pHitbox = (mstudiobbox_t*)((byte*)pStudioHeader + pStudioHeader->hitboxindex);

			if (!pHitbox)
				return;

			Vector vecBoundBoxMin, vecBoundBoxMax, vecTransform, vecMultiPoint;

			for (auto nHitbox = 0; nHitbox < pStudioHeader->numhitboxes; ++nHitbox)
			{
				g_Utils.VectorTransform(pHitbox[nHitbox].bbmin, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecBoundBoxMin);
				g_Utils.VectorTransform(pHitbox[nHitbox].bbmax, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecBoundBoxMax);

				pPlayer->m_vecHitboxMin[nHitbox] = vecBoundBoxMin;
				pPlayer->m_vecHitboxMax[nHitbox] = vecBoundBoxMax;
				pPlayer->m_vecHitbox[nHitbox] = ((vecBoundBoxMax + vecBoundBoxMin) * 0.5f);
				pPlayer->m_vecHitboxPoints[nHitbox][0] = vecBoundBoxMax;
				pPlayer->m_vecHitboxPoints[nHitbox][1] = vecBoundBoxMin;

				vecMultiPoint = (pHitbox[nHitbox].bbmin / pHitbox[nHitbox].bbmax);

				g_Utils.VectorTransform(pHitbox[nHitbox].bbmax, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecTransform, 0, vecMultiPoint.x);
				pPlayer->m_vecHitboxPoints[nHitbox][2] = vecTransform;

				g_Utils.VectorTransform(pHitbox[nHitbox].bbmax, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecTransform, 1, vecMultiPoint.y);
				pPlayer->m_vecHitboxPoints[nHitbox][3] = vecTransform;

				g_Utils.VectorTransform(pHitbox[nHitbox].bbmax, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecTransform, 2, vecMultiPoint.z);
				pPlayer->m_vecHitboxPoints[nHitbox][4] = vecTransform;

				vecMultiPoint = (pHitbox[nHitbox].bbmax / pHitbox[nHitbox].bbmin);

				g_Utils.VectorTransform(pHitbox[nHitbox].bbmin, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecTransform, 0, vecMultiPoint.x);
				pPlayer->m_vecHitboxPoints[nHitbox][5] = vecTransform;

				g_Utils.VectorTransform(pHitbox[nHitbox].bbmin, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecTransform, 1, vecMultiPoint.y);
				pPlayer->m_vecHitboxPoints[nHitbox][6] = vecTransform;

				g_Utils.VectorTransform(pHitbox[nHitbox].bbmin, (*pBoneMatrix)[pHitbox[nHitbox].bone], vecTransform, 2, vecMultiPoint.z);
				pPlayer->m_vecHitboxPoints[nHitbox][7] = vecTransform;
			}
		}
	}
}

float CUtils::Armor(float flDamage, int nArmorValue)
{
	if (nArmorValue > 0)
	{
		float flNew = flDamage * ARMOR_RATIO;
		float flArmor = (flDamage - flNew) * ARMOR_BONUS;

		if (flArmor > nArmorValue)
		{
			flArmor *= (1.f / ARMOR_BONUS);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}

	return flDamage;
}

void CUtils::UTIL_TextureHit(Vector vecSrc, Vector vecEnd, Vector vecDir, int ignore, pmtrace_t& Trace)
{
	if (vecSrc == vecEnd)
	{
		RtlSecureZeroMemory(&Trace, sizeof(pmtrace_t));
		Trace.endpos = vecEnd;
		Trace.fraction = 1.f;
		return;
	}

	g_Engine.pEventAPI->EV_SetTraceHull(2);
	g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_GLASS_IGNORE, ignore, &Trace);

	if (!Trace.fraction && Trace.startsolid && !Trace.allsolid)
	{
		Vector vecTmp = vecSrc;

		while (!Trace.allsolid && !Trace.fraction)
		{
			vecTmp = vecTmp + vecDir;

			g_Engine.pEventAPI->EV_SetTraceHull(2);
			g_Engine.pEventAPI->EV_PlayerTrace(vecTmp, vecEnd, PM_WORLD_ONLY, ignore, &Trace);
		}

		if (!Trace.allsolid && Trace.fraction != 1.f)
		{
			vecTmp = vecEnd - vecSrc;
			float flLength1 = vecTmp.Length();

			vecTmp = Trace.endpos - vecSrc;
			float flLength2 = vecTmp.Length();

			Trace.fraction = flLength1 / flLength2;
			Trace.startsolid = 1;
		}
	}

	if (Trace.allsolid)
		Trace.fraction = 1.f;
}

int CUtils::FireBullets(Vector vecStart, Vector vecEnd, float flDistance, int iOrigPenetration, int iBulletType, int iDamage, float flRangeModifier)
{
	Vector vecDir, vecTmp;
	Vector vecSrc = vecStart;
	Vector vecDest = vecEnd;

	int iPenetration = iOrigPenetration + 1;
	int iPenetrationPower;
	int iCurrentDamage = iDamage;

	float flLength;
	float flPenetrationDistance;
	float flCurrentDistance;

	switch (iBulletType)
	{
	case BULLET_PLAYER_9MM:
		iPenetrationPower = 21;
		flPenetrationDistance = 800;
		break;

	case BULLET_PLAYER_45ACP:
		iPenetrationPower = 15;
		flPenetrationDistance = 500;
		break;

	case BULLET_PLAYER_50AE:
		iPenetrationPower = 30;
		flPenetrationDistance = 1000;
		break;

	case BULLET_PLAYER_762MM:
		iPenetrationPower = 39;
		flPenetrationDistance = 5000;
		break;

	case BULLET_PLAYER_556MM:
		iPenetrationPower = 35;
		flPenetrationDistance = 4000;
		break;

	case BULLET_PLAYER_338MAG:
		iPenetrationPower = 45;
		flPenetrationDistance = 8000;
		break;

	case BULLET_PLAYER_57MM:
		iPenetrationPower = 30;
		flPenetrationDistance = 2000;
		break;

	case BULLET_PLAYER_357SIG:
		iPenetrationPower = 25;
		flPenetrationDistance = 800;
		break;

	default:
		iPenetrationPower = 0;
		flPenetrationDistance = 0;
		break;
	}

	vecDir = vecDest - vecSrc;
	flLength = vecDir.Length();
	vecDir /= flLength;
	vecDest = vecDir * flDistance + vecSrc;

	float flDamageModifier = 0.5f;

	pmtrace_t tr;

	while (iPenetration)
	{
		UTIL_TextureHit(vecSrc, vecDest, vecDir, -1, tr);

		switch (GetTextureType(&tr, vecSrc, vecDest))
		{
		case CHAR_TEX_CONCRETE:
			iPenetrationPower *= 0.25;
			break;

		case CHAR_TEX_GRATE:
			iPenetrationPower *= 0.5;
			flDamageModifier = 0.4f;
			break;

		case CHAR_TEX_METAL:
			iPenetrationPower *= 0.15;
			flDamageModifier = 0.2f;
			break;

		case CHAR_TEX_COMPUTER:
			iPenetrationPower *= 0.4;
			flDamageModifier = 0.45f;
			break;

		case CHAR_TEX_TILE:
			iPenetrationPower *= 0.65;
			flDamageModifier = 0.3f;
			break;

		case CHAR_TEX_VENT:
			iPenetrationPower *= 0.5;
			flDamageModifier = 0.45f;
			break;

		case CHAR_TEX_WOOD:
			flDamageModifier = 0.6f;
			break;
		}

		if (tr.fraction != 1.f)
		{
			--iPenetration;

			vecTmp = tr.endpos - vecStart;
			float flTmpLen = vecTmp.Length();

			if (flTmpLen >= flLength)
			{
				vecTmp = vecEnd - vecSrc;
				float flLength1 = vecTmp.Length();

				vecTmp = vecDest - vecSrc;
				float flLength2 = vecTmp.Length();

				tr.fraction = flLength1 / flLength2;

				flCurrentDistance = flDistance * tr.fraction;
				iCurrentDamage *= pow(flRangeModifier, flCurrentDistance / 500);

				return iCurrentDamage;
			}

			flCurrentDistance = flDistance * tr.fraction;
			iCurrentDamage *= pow(flRangeModifier, flCurrentDistance / 500);

			if (flCurrentDistance > flPenetrationDistance)
				iPenetration = 0;

			if (iPenetration)
			{
				vecSrc = iPenetrationPower * vecDir + tr.endpos;
				flDistance = (flDistance - flCurrentDistance) * 0.5f;
				vecDest = vecDir * flDistance + vecSrc;

				flCurrentDistance = (float)iCurrentDamage;
				iCurrentDamage = (int)(flCurrentDistance * flDamageModifier);
			}
			else
			{
				vecSrc = 42 * vecDir + tr.endpos;
				flDistance = (flDistance - flCurrentDistance) * 0.75f;
				vecDest = vecDir * flDistance + vecSrc;

				iCurrentDamage *= 0.75;
			}
		}
		else
			iPenetration = 0;
	}

	return 0;
}

void CUtils::NormalizeAngles(float* flAngles)
{
	for (auto i = 0; i < 3; ++i)
	{
		float flRevolutions = flAngles[i] / 360;

		if (flAngles[i] > 180 || flAngles[i] < -180)
		{
			if (flRevolutions < 0)
				flRevolutions = -flRevolutions;

			flRevolutions = round(flRevolutions);

			if (flAngles[i] < 0)
				flAngles[i] = (flAngles[i] + 360 * flRevolutions);
			else
				flAngles[i] = (flAngles[i] - 360 * flRevolutions);
		}
	}
}

void CUtils::MakeAngle(bool a_bAddAngles, float* a_flAngles, usercmd_s* cmd)
{
	FixMoveStart(cmd);

	if (a_bAddAngles)
	{
		cmd->viewangles[0] += a_flAngles[0];
		cmd->viewangles[1] += a_flAngles[1];
		cmd->viewangles[2] += a_flAngles[2];
	}
	else
	{
		cmd->viewangles[0] = a_flAngles[0];
		cmd->viewangles[1] = a_flAngles[1];
		cmd->viewangles[2] = a_flAngles[2];
	}

	FixMoveEnd(cmd);
}

void CUtils::VectorAngles(const float* forward, float* angles)
{
	float tmp, yaw, pitch;

	if (!forward[1] && !forward[0])
	{
		yaw = 0;
		pitch = (forward[2] > 0) ? 270.f : 90.f;
	}
	else
	{
		yaw = (float)(atan2(forward[1], forward[0]) * 180 / M_PI);

		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (float)(atan2(-forward[2], tmp) * 180 / M_PI);

		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;

	while (angles[0] < -89)
	{
		angles[0] += 180;
		angles[1] += 180;
	}

	while (angles[0] > 89)
	{
		angles[0] -= 180;
		angles[1] += 180;
	}

	while (angles[1] < -180)
		angles[1] += 360;

	while (angles[1] > 180)
		angles[1] -= 360;
}

void CUtils::FixMoveStart(usercmd_s* cmd)
{
	NormalizeAngles(cmd->viewangles);

	_vecPreviousAngles = cmd->viewangles;
}

void CUtils::FixMoveEnd(usercmd_s* cmd)
{
	float flSpeed = Vector(cmd->forwardmove, cmd->sidemove, 0.f).Length2D();

	if (flSpeed == 0.f)
		return;

	float flYaw = cmd->viewangles[1] - _vecPreviousAngles[1];
	float flMoveYawRad = atan2(cmd->sidemove, cmd->forwardmove) + DEG2RAD(flYaw);

	cmd->forwardmove = cos(flMoveYawRad) * flSpeed;
	cmd->sidemove = sin(flMoveYawRad) * flSpeed;

	bool bOldBackwards = (_vecPreviousAngles[0] > 90.f || _vecPreviousAngles[0] < -90.f);
	bool bNewBackwards = (cmd->viewangles[0] > 90.f || cmd->viewangles[0] < -90.f);

	if (bOldBackwards != bNewBackwards)
		cmd->forwardmove = -cmd->forwardmove;
}

void CUtils::VectorTransform(Vector in1, float in2[3][4], float* out, int xyz, float multi)
{
	in1[xyz] *= multi;

	out[0] = in1.Dot(in2[0]) + in2[0][3];
	out[1] = in1.Dot(in2[1]) + in2[1][3];
	out[2] = in1.Dot(in2[2]) + in2[2][3];
}

void CUtils::VectorTransform(Vector in1, float in2[3][4], float* out)
{
	out[0] = in1.Dot(in2[0]) + in2[0][3];
	out[1] = in1.Dot(in2[1]) + in2[1][3];
	out[2] = in1.Dot(in2[2]) + in2[2][3];
}

int PhysFilter(physent_t* pe)
{
	return pe->classnumber != 696;
}

int CUtils::DevilPlayerTrace(Vector vecStart, Vector vecEnd)
{
	pmtrace_t* tr = pmove->PM_TraceLineEx(vecStart, vecEnd, PM_NORMAL, 2, PhysFilter);
	std::unique_ptr<pmtrace_t> pTrace(tr);

	int detect = g_Engine.pEventAPI->EV_IndexFromTrace(pTrace.get());
	pTrace.release();

	return detect;
}

physent_t CUtils::NewPhysent(int a_iIndex, int a_nPhysent, bool a_bDisableTraces)
{
	auto physent = physent_t();

	if (a_nPhysent >= 0 && a_nPhysent < MAX_PHYSENTS)
		physent = pmove->physents[a_nPhysent];

	physent.info = a_iIndex;
	physent.classnumber = 0;

	if (a_iIndex > 0 && a_iIndex <= MAX_CLIENTS)
	{
		physent.player = TRUE;

		if (a_bDisableTraces)
		{
			physent.solid = SOLID_NOT;
			physent.angles = Vector();
			physent.origin = Vector();
			physent.mins = Vector();
			physent.maxs = Vector();
		}

		auto pPlayer = &g_Player[a_iIndex - 1];

		if (pPlayer && pPlayer->m_bIsInPVS && !pPlayer->m_bIsDead && !a_bDisableTraces)
		{
			physent.solid = SOLID_BBOX;
			physent.maxs = pPlayer->m_vecBoundBoxMaxs;
			physent.mins = pPlayer->m_vecBoundBoxMins;
			physent.angles = pPlayer->m_vecAngles;
			physent.origin = pPlayer->m_vecOrigin;
			physent.classnumber = 696;
		}
	}
	else if (a_iIndex > MAX_CLIENTS && a_iIndex < MAX_ENTITIES)
	{
		physent.player = FALSE;

		if (a_bDisableTraces)
		{
			physent.solid = SOLID_NOT;
			physent.mins = Vector();
			physent.maxs = Vector();
		}

		auto pEntity = &g_Entity[a_iIndex];

		if (pEntity)
		{
			physent.angles = pEntity->m_vecAngles;
			physent.origin = pEntity->m_vecOrigin;
		}
	}

	return physent;
}

void CUtils::SpoofPhysent(int a_iIndex, bool a_bDisableTraces)
{
	for (auto i = 0; i < pmove->numphysent; ++i)
	{
		if (pmove->physents[i].info == a_iIndex)
		{
			pmove->physents[i] = NewPhysent(a_iIndex, i, a_bDisableTraces);
			return;
		}
	}

	if (pmove->numphysent + 1 < MAX_PHYSENTS)
	{
		pmove->physents[pmove->numphysent] = NewPhysent(a_iIndex, -1, a_bDisableTraces);
		++pmove->numphysent;
	}
}

void CUtils::ComputeMove(CBasePlayer* pPlayer)
{
	if (pPlayer && pPlayer->m_flFrametime && pPlayer->m_flVelocity)
	{
		QAngle QAngles(pPlayer->m_vecAngles);
		Vector vecVelocityRotated = Vector();
		vecVelocityRotated.VectorRotate(pPlayer->m_vecVelocity, QAngles);

		pPlayer->m_flForwardMove = vecVelocityRotated[0] * (1.f / pPlayer->m_flFrametime);
		pPlayer->m_flSideMove = -1.f * vecVelocityRotated[1] * (1.f / pPlayer->m_flFrametime);

		pPlayer->m_flForwardMove = ImMin(pPlayer->m_flForwardMove, 250.f);
		pPlayer->m_flForwardMove = ImMax(pPlayer->m_flForwardMove, -250.f);

		pPlayer->m_flSideMove = ImMin(pPlayer->m_flSideMove, 250.f);
		pPlayer->m_flSideMove = ImMax(pPlayer->m_flSideMove, -250.f);
	}
}

void CUtils::SendCommand(bool a_bStatus)
{
	static bool bSendCommand = true;

	if (a_bStatus && !bSendCommand)
	{
		memwrite(g_Offsets.m_dwSendCommandPointer1, (uintptr_t) "\x90\x90\x90\x90\x90", 5);

		bSendCommand = true;
	}
	else if (!a_bStatus && bSendCommand)
	{
		EnablePageWrite(g_Offsets.m_dwSendCommandPointer1, sizeof(uint8_t) + sizeof(intptr_t));
		*(uint8_t*)g_Offsets.m_dwSendCommandPointer1 = 0xE9;
		*(intptr_t*)(g_Offsets.m_dwSendCommandPointer1 + sizeof(uint8_t)) = (intptr_t)g_Offsets.m_dwSendCommandPointer2 - ((intptr_t)g_Offsets.m_dwSendCommandPointer1 + 5);
		RestorePageProtection(g_Offsets.m_dwSendCommandPointer1, sizeof(uint8_t) + sizeof(intptr_t));

		bSendCommand = false;
	}
}

int CUtils::LookupSequence(model_s* a_Model, const char** a_pcszLabel, int a_nSize)
{
	if (a_Model == nullptr)
		return ACT_INVALID;

	const auto pstudiohdr = (studiohdr_t*)g_Studio.Mod_Extradata(a_Model);

	if (pstudiohdr)
	{
		const auto pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

		if (pseqdesc)
		{
			for (int i = 0; i < pstudiohdr->numseq; ++i)
			{
				for (int j = 0; j < a_nSize; ++j)
				{
					if (!_stricmp(pseqdesc[i].label, a_pcszLabel[j]))
						return i;
				}
			}
		}
	}

	return ACT_INVALID;
}

bool CUtils::EnablePageWrite(DWORD dwAddress, SIZE_T dwSize)
{
	return VirtualProtect((PVOID)dwAddress, dwSize, PAGE_EXECUTE_READWRITE, &_dwOldPageProtection) != FALSE;
}

bool CUtils::RestorePageProtection(DWORD dwAddress, SIZE_T dwSize)
{
	bool bReturn = VirtualProtect((PVOID)dwAddress, dwSize, _dwOldPageProtection, &_dwOldPageProtection);

	FlushInstructionCache(GetCurrentProcess(), (PVOID)dwAddress, dwSize);

	return bReturn;
}

void CUtils::memwrite(uintptr_t adr, uintptr_t ptr, SIZE_T size)
{
	EnablePageWrite(adr, size);
	memcpy(LPVOID(adr), LPVOID(ptr), size);
	RestorePageProtection(adr, size);
}

bool CUtils::CalcScreen(Vector a_vecOrigin, float* a_pflVecScreen)
{
	const auto result = g_Engine.pTriAPI->WorldToScreen(a_vecOrigin, a_pflVecScreen);

	if (a_pflVecScreen[0] < 1 && a_pflVecScreen[1] < 1 && a_pflVecScreen[0] > -1 && a_pflVecScreen[1] > -1 && !result)
	{
		a_pflVecScreen[0] = a_pflVecScreen[0] * (g_Screen.iWidth / 2) + (g_Screen.iWidth / 2);
		a_pflVecScreen[1] = -a_pflVecScreen[1] * (g_Screen.iHeight / 2) + (g_Screen.iHeight / 2);

		return true;
	}

	return false;
}

void CUtils::StringReplace(char* buf, const char* search, const char* replace)
{
	if (!strlen(buf) || !strlen(search))
		return;

	char* p = buf;

	size_t l1 = strlen(search);
	size_t l2 = strlen(replace);

	while ((p = strstr(p, search)) != 0)
	{
		RtlMoveMemory(p + l2, p + l1, strlen(p + l1) + 1U);
		RtlCopyMemory(p, replace, l2);

		p += l2;
	}
}

float CUtils::fInterp(float s1, float s2, float s3, float f1, float f3)
{
	if (s2 == s1)
		return f1;

	if (s2 == s3)
		return f3;

	if (s3 == s1)
		return f1;

	return f1 + ((s2 - s1) / (s3 - s1)) * (f3 - f1);
}

qboolean BoundsIntersect(const Vector &mins1, const Vector &maxs1, const Vector &mins2, const Vector &maxs2)
{
	if (mins1[0] > maxs2[0] || mins1[1] > maxs2[1] || mins1[2] > maxs2[2])
		return false;
	if (maxs1[0] < mins2[0] || maxs1[1] < mins2[1] || maxs1[2] < mins2[2])
		return false;
	return true;
}

bool CUtils::FileExists(const char* _FileName)
{
	return _access(_FileName, 0) != -1;
}

void TraceLog(const char *fmt, ...)
{
	va_list argptr;
	static char string[4096];

	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	FILE *fp = fopen(g_Globals.m_sDebugFile.c_str(), "a+");

	if (!fp) 
		return;

	fprintf(fp, "%s", string);
	fclose(fp);
}

const char* CUtils::KeyToString(int keynum)
{
	switch (keynum)
	{
	case K_TAB:			return "TAB";
	case K_ENTER:		return "ENTER";
	case K_ESCAPE:		return "ESCAPE";
	case K_BACKSPACE:	return "BACKSPACE";
	case K_UP:			return "UPARROW";
	case K_DOWN:		return "DOWNARROW";
	case K_LEFT:		return "LEFTARROW";
	case K_RIGHT:		return "RIGHTARROW";
	case K_BL:			return "[";
	case K_BR:			return "]";
	case K_0:			return "0";
	case K_1:			return "1";
	case K_2:			return "2";
	case K_3:			return "3";
	case K_4:			return "4";
	case K_5:			return "5";
	case K_6:			return "6";
	case K_7:			return "7";
	case K_8:			return "8";
	case K_9:			return "9";
	case K_POINT:		return ".";
	case K_COMMA:		return ",";
	case K_A:			return "A";
	case K_B:			return "B";
	case K_C:			return "C";
	case K_D:			return "D";
	case K_E:			return "E";
	case K_F:			return "F";
	case K_G:			return "G";
	case K_H:			return "H";
	case K_I:			return "I";
	case K_J:			return "J";
	case K_K:			return "K";
	case K_L:			return "L";
	case K_M:			return "M";
	case K_N:			return "N";
	case K_O:			return "O";
	case K_P:			return "P";
	case K_Q:			return "Q";
	case K_R:			return "R";
	case K_S:			return "S";
	case K_T:			return "T";
	case K_U:			return "U";
	case K_V:			return "V";
	case K_W:			return "W";
	case K_X:			return "X";
	case K_Y:			return "Y";
	case K_Z:			return "Z";
	case K_F1:			return "F1";
	case K_F2:			return "F2";
	case K_F3:			return "F3";
	case K_F4:			return "F4";
	case K_F5:			return "F5";
	case K_F6:			return "F6";
	case K_F7:			return "F7";
	case K_F8:			return "F8";
	case K_F9:			return "F9";
	case K_F10:			return "F10";
	case K_F11:			return "F11";
	case K_F12:			return "F12";
	case K_INS:			return "INSERT";
	case K_DEL:			return "DELETE";
	case K_PGDN:		return "PGDN";
	case K_PGUP:		return "PGUP";
	case K_HOME:		return "HOME";
	case K_END:			return "END";
	case K_SPACE:		return "SPACE";
	case K_ALT:			return "ALT";
	case K_SHIFT:		return "SHIFT";
	case K_CTRL:		return "CTRL";
	case K_MWHEELDOWN:	return "MWHEELDOWN";
	case K_MWHEELUP:	return "MWHEELUP";
	case K_MOUSE1:		return "MOUSE1";
	case K_MOUSE2:		return "MOUSE2";
	case K_MOUSE3:		return "MOUSE3";
	case K_MOUSE4:		return "MOUSE4";
	case K_MOUSE5:		return "MOUSE5";			
	}

	return "none";
}

void CUtils::ConvertToASCII(int* keynum)
{
	if (keynum != nullptr)
	{
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) *keynum = K_SPACE;
		else if (GetAsyncKeyState(VK_RETURN) & 0x8000) *keynum = K_ENTER;
		else if (GetAsyncKeyState(VK_TAB) & 0x8000) *keynum = K_TAB;
		else if (GetAsyncKeyState(VK_UP) & 0x8000) *keynum = K_UP;
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000) *keynum = K_DOWN;
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000) *keynum = K_LEFT;
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) *keynum = K_RIGHT;
		else if (GetAsyncKeyState(18) & 0x8000) *keynum = K_ALT;
		else if (GetAsyncKeyState(VK_CONTROL) & 0x8000) *keynum = K_CTRL;
		else if (GetAsyncKeyState(VK_SHIFT) & 0x8000) *keynum = K_SHIFT;
		else if (GetAsyncKeyState(VK_F1) & 0x8000) *keynum = K_F1;
		else if (GetAsyncKeyState(VK_F2) & 0x8000) *keynum = K_F2;
		else if (GetAsyncKeyState(VK_F3) & 0x8000) *keynum = K_F3;
		else if (GetAsyncKeyState(VK_F4) & 0x8000) *keynum = K_F4;
		else if (GetAsyncKeyState(VK_F5) & 0x8000) *keynum = K_F5;
		else if (GetAsyncKeyState(VK_F6) & 0x8000) *keynum = K_F6;
		else if (GetAsyncKeyState(VK_F7) & 0x8000) *keynum = K_F7;
		else if (GetAsyncKeyState(VK_F8) & 0x8000) *keynum = K_F8;
		else if (GetAsyncKeyState(VK_F9) & 0x8000) *keynum = K_F9;
		else if (GetAsyncKeyState(VK_F10) & 0x8000) *keynum = K_F10;
		else if (GetAsyncKeyState(VK_F11) & 0x8000) *keynum = K_F11;
		else if (GetAsyncKeyState(VK_F12) & 0x8000) *keynum = K_F12;
		else if (GetAsyncKeyState(34) & 0x8000) *keynum = K_PGDN;
		else if (GetAsyncKeyState(33) & 0x8000) *keynum = K_PGUP;
		else if (GetAsyncKeyState(VK_HOME) & 0x8000) *keynum = K_HOME;
		else if (GetAsyncKeyState(VK_END) & 0x8000) *keynum = K_END;
		else if (GetAsyncKeyState(81) & 0x8000) *keynum = K_Q;
		else if (GetAsyncKeyState(87) & 0x8000) *keynum = K_W;
		else if (GetAsyncKeyState(69) & 0x8000) *keynum = K_E;
		else if (GetAsyncKeyState(82) & 0x8000) *keynum = K_R;
		else if (GetAsyncKeyState(84) & 0x8000) *keynum = K_T;
		else if (GetAsyncKeyState(89) & 0x8000) *keynum = K_Y;
		else if (GetAsyncKeyState(85) & 0x8000) *keynum = K_U;
		else if (GetAsyncKeyState(73) & 0x8000) *keynum = K_I;
		else if (GetAsyncKeyState(79) & 0x8000) *keynum = K_O;
		else if (GetAsyncKeyState(80) & 0x8000) *keynum = K_P;
		else if (GetAsyncKeyState(219) & 0x8000) *keynum = K_BL;
		else if (GetAsyncKeyState(221) & 0x8000) *keynum = K_BR;
		else if (GetAsyncKeyState(65) & 0x8000) *keynum = K_A;
		else if (GetAsyncKeyState(83) & 0x8000) *keynum = K_S;
		else if (GetAsyncKeyState(68) & 0x8000) *keynum = K_D;
		else if (GetAsyncKeyState(70) & 0x8000) *keynum = K_F;
		else if (GetAsyncKeyState(71) & 0x8000) *keynum = K_G;
		else if (GetAsyncKeyState(72) & 0x8000) *keynum = K_H;
		else if (GetAsyncKeyState(74) & 0x8000) *keynum = K_J;
		else if (GetAsyncKeyState(75) & 0x8000) *keynum = K_K;
		else if (GetAsyncKeyState(76) & 0x8000) *keynum = K_L;
		else if (GetAsyncKeyState(90) & 0x8000) *keynum = K_Z;
		else if (GetAsyncKeyState(88) & 0x8000) *keynum = K_X;
		else if (GetAsyncKeyState(67) & 0x8000) *keynum = K_C;
		else if (GetAsyncKeyState(86) & 0x8000) *keynum = K_V;
		else if (GetAsyncKeyState(66) & 0x8000) *keynum = K_B;
		else if (GetAsyncKeyState(78) & 0x8000) *keynum = K_N;
		else if (GetAsyncKeyState(77) & 0x8000) *keynum = K_M;
		else if (GetAsyncKeyState(188) & 0x8000) *keynum = K_COMMA;
		else if (GetAsyncKeyState(190) & 0x8000) *keynum = K_POINT;
		else if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) *keynum = K_MOUSE1;
		else if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) *keynum = K_MOUSE2;
		else if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) *keynum = K_MOUSE3;
		else if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) *keynum = K_MOUSE4;
		else if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) *keynum = K_MOUSE5;
		else if (GetAsyncKeyState(48) & 0x8000) *keynum = K_0;
		else if (GetAsyncKeyState(49) & 0x8000) *keynum = K_1;
		else if (GetAsyncKeyState(50) & 0x8000) *keynum = K_2;
		else if (GetAsyncKeyState(51) & 0x8000) *keynum = K_3;
		else if (GetAsyncKeyState(52) & 0x8000) *keynum = K_4;
		else if (GetAsyncKeyState(53) & 0x8000) *keynum = K_5;
		else if (GetAsyncKeyState(54) & 0x8000) *keynum = K_6;
		else if (GetAsyncKeyState(55) & 0x8000) *keynum = K_7;
		else if (GetAsyncKeyState(56) & 0x8000) *keynum = K_8;
		else if (GetAsyncKeyState(57) & 0x8000) *keynum = K_9;
		else *keynum = 0;
	}
}