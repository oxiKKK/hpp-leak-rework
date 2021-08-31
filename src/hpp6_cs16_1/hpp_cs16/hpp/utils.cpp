#include "main.h"

CUtils g_Utils;
CSkipFrames g_SkipFrames;

void CUtils::ReplaceCall(uintptr_t callPtr, uintptr_t destPtr)
{
	DWORD oldProt;
	VirtualProtect(LPVOID(callPtr + 1), sizeof(intptr_t), PAGE_EXECUTE_READWRITE, &oldProt);

	*(intptr_t*)(callPtr + 1) = destPtr - (callPtr + 5);

	VirtualProtect(LPVOID(callPtr + 1), sizeof(intptr_t), oldProt, &oldProt);
}

int CUtils::ExceptionFilter(const char* func, const int code, const _EXCEPTION_POINTERS* ep)
{
	if (ep && func)
	{
		TraceLog("[hpp] %s: exeption code 0x%X, data address: 0x%X, instruction address: 0x%X\n", func, ep->ExceptionRecord->ExceptionCode, ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionAddress);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

bool CUtils::IsBoxIntersectingRay(const Vector& vecBoundBoxMin, const Vector& vecBoundBoxMax, const Vector& vecOrigin, const Vector& vecDelta)
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
			if ((vecOrigin[i] < vecBoundBoxMin[i]) || (vecOrigin[i] > vecBoundBoxMax[i]))
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
		float t2 = (vecBoundBoxMax[i] - vecOrigin[i]) * invDelta;

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

void CUtils::VectorAngles(const float* forward, float* angles)
{
	float tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / IM_PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / IM_PI);
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

qboolean CL_FindInterpolationUpdates(cl_entity_t *ent, float targettime, position_history_t **ph0, position_history_t **ph1)
{
	qboolean	extrapolate = true;
	int	i, i0, i1, imod;
	float	at;

	i0 = (ent->current_position) & HISTORY_MASK;
	i1 = (ent->current_position - 1) & HISTORY_MASK;
	imod = ent->current_position;

	for (i = 1; i < HISTORY_MAX - 1; i++)
	{
		at = ent->ph[(imod - i) & HISTORY_MASK].animtime;

		if (at == 0.0)
			break;

		if (at < targettime)
		{
			i0 = ((imod - i) + 1) & HISTORY_MASK;
			i1 = (imod - i) & HISTORY_MASK;
			extrapolate = false;
			break;
		}
	}

	if (ph0 != NULL) *ph0 = &ent->ph[i0];
	if (ph1 != NULL) *ph1 = &ent->ph[i1];

	return extrapolate;
}

void CUtils::LagCompensation(int index, short lerp_msec, Vector &origin)
{
	static cvar_s *cl_lc = nullptr;
	static cvar_s *cl_updaterate = nullptr;
	static cvar_s *sv_maxunlag = nullptr;
	static cvar_s *sv_unlagpush = nullptr;
	static cvar_s* ex_interp = nullptr;

	if (cl_lc == nullptr)
		cl_lc = g_Engine.pfnGetCvarPointer("cl_lc");

	if (cl_updaterate == nullptr)
		cl_updaterate = g_Engine.pfnGetCvarPointer("cl_updaterate");

	if (sv_maxunlag == nullptr)
		sv_maxunlag = g_Engine.pfnGetCvarPointer("sv_maxunlag");

	if (sv_unlagpush == nullptr)
		sv_unlagpush = g_Engine.pfnGetCvarPointer("sv_unlagpush");

	if (ex_interp == nullptr)
		ex_interp = g_Engine.pfnGetCvarPointer("ex_interp");

	if (!cl_lc || !cl_updaterate || !sv_maxunlag || !sv_unlagpush || !ex_interp)
		return;

	// Player not wanting lag compensation
	if (cl_lc->value == 0.0f)
		return;

	const auto pGameEntity = g_Engine.GetEntityByIndex(index);

	if (!pGameEntity)
		return;

	auto fakelatency = g_Miscellaneous.m_bFakeLatencyActive ? cvar.fakelatency_value / 1000.0 : 0.0;

	// Get true latency
	auto latency = g_Status.latency / 1000.0 + fakelatency;

	if (latency > 1.5)
		latency = 1.5;

	auto update_interval = 0.1;

	if (cl_updaterate->value > 10.0f)
		update_interval = 1.0 / (double)cl_updaterate->value;

	// Fixup delay based on message interval (cl_updaterate, default 20 so 50 msec)
	latency -= update_interval;

	// Further fixup due to client side delay because packets arrive 1/2 through the frame loop, on average
	latency -= (g_Globals.m_flFrameTime) * 0.5f;

	// Absolute bounds on lag compensation
	auto correct = min(LAG_COMPENSATION_DATA_TIME, latency);

	// See if server is applying a lower cap
	if (sv_maxunlag->value != 0.0f)
	{
		// Make sure it's not negative
		if (sv_maxunlag->value < 0.0f)
			g_Engine.Cvar_SetValue("sv_maxunlag", 0.0f);

		// Apply server cap
		correct = min(correct, sv_maxunlag->value);
	}

	// Get true timestamp
	const auto realtime = client_state->time;

	// Figure out timestamp for which we are looking for data
	auto targettime = realtime - correct;

	// Remove lag based on player interpolation, as well
	auto interptime = lerp_msec / 1000.0;

	if (lerp_msec == -1)
		interptime = ex_interp->value;

	if (interptime > 0.1)
		interptime = 0.1;

	if (update_interval > interptime)
		interptime = update_interval;

	targettime -= interptime;

	// Server can apply a fudge, probably not needed, defaults to 0.0f
	targettime += sv_unlagpush->value;

	// Cap target to present time, of course
	targettime = min(realtime, targettime);

	position_history_t *newer, *older;

	CL_FindInterpolationUpdates(pGameEntity, targettime, &newer, &older);

	if (!newer || !older)
		return;

	float frac = 0.0;

	auto t0 = newer->animtime;
	auto t1 = older->animtime;

	auto timeDiff = t0 - t1;

	if (timeDiff == 0.0)
		frac = 0.0;
	else
	{
		frac = (targettime - t1) / timeDiff;

		if (frac <= 1.0)
		{
			if (frac < 0.0)
				frac = 0.0;
		}
		else
			frac = 1.0;
	}

	auto delta = newer->origin - older->origin;

	if (delta.LengthSqr() > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR)
		return;

	origin = older->origin + delta * frac;
}

void CUtils::CreateRandomString(char *pszDest, int nLength)
{
	static const char c_szAlphaNum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < nLength; ++i)
		pszDest[i] = c_szAlphaNum[rand() % (sizeof(c_szAlphaNum) - 1)];

	pszDest[nLength] = '\0';
}

void CUtils::UpdateSequences()
{
	if (client_static->netchan.incoming_sequence > g_Local.m_iLastIncomingSequence)
	{
		CIncomingSequence sequence;
		sequence.incoming_sequence = client_static->netchan.incoming_sequence;
		sequence.time = client_state->time;
		sequences.push_front(sequence);

		g_Local.m_iLastIncomingSequence = client_static->netchan.incoming_sequence;
	}

	if (sequences.size() > 2048)
		sequences.pop_back();
}

void CUtils::SetFakeLatency(const double latency)
{
	for (auto& seq : sequences)
	{
		const auto time_difference = client_state->time - seq.time;

		if (time_difference >= latency)
		{
			client_static->netchan.incoming_sequence = seq.incoming_sequence;
			break;
		}
	}
}

void CUtils::FixMoveStart(usercmd_s* cmd)
{
	NormalizeAngles(cmd->viewangles);

	_vecPreviousAngles = cmd->viewangles;
}

void CUtils::FixMoveEnd(usercmd_s* cmd)
{
	auto speed = Vector(cmd->forwardmove, cmd->sidemove, 0.f).Length2D();

	if (speed == 0.f)
		return;

	auto yaw = cmd->viewangles.y - _vecPreviousAngles.y;
	auto move_yaw_rad = atan2(cmd->sidemove, cmd->forwardmove) + DEG2RAD(yaw);

	cmd->forwardmove = cos(move_yaw_rad) * speed;
	cmd->sidemove = sin(move_yaw_rad) * speed;

	auto old_backwards = (_vecPreviousAngles.x > 90.f || _vecPreviousAngles.x < -90.f);
	auto new_backwards = (cmd->viewangles.x > 90.f || cmd->viewangles.x < -90.f);

	if (old_backwards != new_backwards)
		cmd->forwardmove = -cmd->forwardmove;
}

void CUtils::NormalizeAngles(float* flAngles)
{
	for (auto i = 0; i < 3; ++i)
	{
		auto revolutions = flAngles[i] / 360;

		if (flAngles[i] > 180 || flAngles[i] < -180)
		{
			if (revolutions < 0)
				revolutions = -revolutions;

			revolutions = round(revolutions);

			if (flAngles[i] < 0)
				flAngles[i] = (flAngles[i] + 360 * revolutions);
			else
				flAngles[i] = (flAngles[i] - 360 * revolutions);
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

void CUtils::SendCommand(const bool a_bStatus)
{
	if (a_bStatus)
		client_static->nextcmdtime = -1.f;
	else
		client_static->nextcmdtime = FLT_MAX;
}

float CUtils::fInterp(const float s1, const float s2, const float s3, const float f1, const float f3)
{
	if (s2 == s1)
		return f1;

	if (s2 == s3)
		return f3;

	if (s3 == s1)
		return f1;

	return f1 + ((s2 - s1) / (s3 - s1)) * (f3 - f1);
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

bool CUtils::FileExists(const char* _FileName)
{
	return _access(_FileName, 0) != -1;
}

bool CUtils::EnablePageWrite(const DWORD dwAddress, const SIZE_T dwSize)
{
	return VirtualProtect((PVOID)(dwAddress), dwSize, PAGE_EXECUTE_READWRITE, &_dwOldPageProtection) != FALSE;
}

bool CUtils::RestorePageProtection(const DWORD dwAddress, const SIZE_T dwSize)
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

void TraceLog(const char *fmt, ...)
{
	va_list argptr;
	static char string[4096];

	RtlSecureZeroMemory(string, sizeof(string));

	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	FILE *fp = fopen(g_Globals.m_sDebugFile.c_str(), "a+");

	if (!fp)
		return;

	fprintf(fp, "%s", string);
	fclose(fp);

	if (g_pConsole && cvar.debug_console)
		g_pConsole->DPrintf(string);

	if (cvar.debug_visuals)
		g_ScreenLog.Log(ImColor(IM_COL32_WHITE), string);
}

int CUtils::LookupSequence(model_s* a_pModel, const char** a_pcszLabel, const int a_nSize)
{
	const auto* const pstudiohdr = (studiohdr_t*)(g_Studio.Mod_Extradata(a_pModel));

	if (pstudiohdr == nullptr)
		return ACT_INVALID;

	const auto* const pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	if (pseqdesc == nullptr)
		return ACT_INVALID;

	for (int i = 0; i < pstudiohdr->numseq; ++i)
	{
		for (int j = 0; j < a_nSize; ++j)
		{
			if (!_stricmp(pseqdesc[i].label, a_pcszLabel[j]))
				return i;
		}
	}

	return ACT_INVALID;
}

void CUtils::SpoofPhysent(const int a_iIndex, const bool a_bDisableTraces)
{
	physent_t* physent = nullptr;

	for (auto i = 0; i < pmove->numphysent; ++i)
	{
		if (pmove->physents[i].info == a_iIndex)
		{
			physent = &pmove->physents[i];
			break;
		}
	}

	if (!physent)
	{
		if (pmove->numphysent + 1 < MAX_PHYSENTS)
		{
			physent = &pmove->physents[pmove->numphysent];
			++pmove->numphysent;
		}
	}

	if (physent)
	{
		physent->info = a_iIndex;
		physent->classnumber = 0;

		if (a_iIndex >= 1 && a_iIndex <= MAX_CLIENTS && a_iIndex != g_Local.m_iIndex)
		{
			physent->player = TRUE;

			if (a_bDisableTraces)
			{
				physent->solid = SOLID_NOT;
				physent->angles.Clear();
				physent->origin.Clear();
				physent->mins.Clear();
				physent->maxs.Clear();
			}
			else
			{
				const auto pPlayer = g_World.GetPlayer(a_iIndex);

				if (pPlayer)
				{
					physent->solid = SOLID_BBOX;
					physent->angles = pPlayer->m_vecAngles;
					physent->origin = pPlayer->m_vecOrigin;
					physent->maxs = pPlayer->m_vecBoundBoxMaxs;
					physent->mins = pPlayer->m_vecBoundBoxMins;
				}
			}
		}
		else if (a_iIndex > MAX_CLIENTS && a_iIndex < MAX_ENTITIES)
		{
			physent->player = FALSE;

			if (a_bDisableTraces)
			{
				physent->solid = SOLID_NOT;
				physent->mins.Clear();
				physent->maxs.Clear();
			}
			else
			{
				const auto pEntity = g_World.GetEntity(a_iIndex);

				if (pEntity)
				{
					physent->solid = SOLID_BBOX;
					physent->angles = pEntity->m_vecAngles;
					physent->origin = pEntity->m_vecOrigin;
					physent->maxs = pEntity->m_vecBoundBoxMaxs;
					physent->mins = pEntity->m_vecBoundBoxMins;
				}
			}
		}
	}
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

/*int MyFireBullets(Vector a_vecSrc, Vector a_vecEnd)
{
	int print = 0;
	//g_Engine.Con_NPrintf(print++, "");

	int iPenetrationPower;

	float flPenetrationDistance;
	float flDistance = g_Weapon.GetDistance();

	int iBulletType = g_Weapon.GetBulletType();

	switch (iBulletType)
	{
	case BULLET_PLAYER_9MM:
		iPenetrationPower = 21;
		flPenetrationDistance = 800;
		break;
	case BULLET_PLAYER_BUCKSHOT:
		iPenetrationPower = 8;
		flPenetrationDistance = 500;
		break;
	case BULLET_PLAYER_45ACP:
		iPenetrationPower = 15;
		flPenetrationDistance = 500;
		break;
	case BULLET_PLAYER_338MAG:
		iPenetrationPower = 45;
		flPenetrationDistance = 8000;
		break;
	case BULLET_PLAYER_762MM:
		iPenetrationPower = 39;
		flPenetrationDistance = 5000;
		break;
	case BULLET_PLAYER_556MM:
		iPenetrationPower = 35;
		flPenetrationDistance = 4000;
		break;
	case BULLET_PLAYER_50AE:
		iPenetrationPower = 30;
		flPenetrationDistance = 1000;
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

//	g_Engine.Con_NPrintf(print++, "iPenetrationPower: %i", iPenetrationPower);
//	g_Engine.Con_NPrintf(print++, "flPenetrationDistance: %f", flPenetrationDistance);

	int v37;

	Vector vStartPos, vEndPos;
	pmtrace_s trOriginal, tr;

	int idx = g_World.GetLocalPlayer()->m_iIndex;

	Vector vecSrc = a_vecSrc;
	Vector vecEnd = a_vecEnd;

	Vector vecDir = vecEnd - vecSrc;

	vecDir /= vecDir.Length();
	vecEnd = vecSrc + (vecDir * flDistance);

	g_Engine.pEventAPI->EV_SetUpPlayerPrediction(0, 1);
	//	g_Engine.pEventAPI->EV_PushPMStates();
	g_Engine.pEventAPI->EV_SetSolidPlayers(idx - 1);
	g_Engine.pEventAPI->EV_SetTraceHull(2);

	int iPenetration_0 = 2;//ak47

	g_Engine.Con_NPrintf(print++, "iPenetration: %i", iPenetration_0);
	g_Engine.Con_NPrintf(print++, "flDistance: %f", flDistance);

	float flDamageModifier = 0.5f;
	int iCurrentDamage = g_Weapon.GetDamage();
	float flRangeModifier = g_Weapon.GetWallPierce();

LABEL_10:
	while (iPenetration_0)
	{
		g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, 0, -1, &tr);

		float flCurrentDistance = flDistance * tr.fraction;

		iCurrentDamage *= pow(flRangeModifier, flCurrentDistance / 500);

		if (flCurrentDistance == 0.0f)
			break;

		g_Engine.Con_NPrintf(print++, "iPenetration: %i | flCurrentDistance: %f", iPenetration_0, flCurrentDistance);
		g_Engine.Con_NPrintf(print++, "iPenetration: %i | iCurrentDamage: %i", iPenetration_0, iCurrentDamage);

		int iPenetration_0a = iPenetration_0 - 1;

		if (flCurrentDistance <= flPenetrationDistance)
			iPenetration_0 = iPenetration_0a;

		const auto cTextureType = GetTextureType(&tr, vecSrc, vecEnd);

		g_Engine.Con_NPrintf(print++, "iPenetration: %i | cTextureType: %i", iPenetration_0, cTextureType);

		switch (cTextureType)
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
		default:
			break;
		}

		g_Engine.Con_NPrintf(print++, "iPenetration: %i | iPenetrationPower: %i", iPenetration_0, iPenetrationPower);

		iCurrentDamage *= flDamageModifier;

		if (!iPenetration_0)
			break;

		flDistance = (flDistance - flCurrentDistance) * 0.5;
		vecSrc = (iPenetrationPower * vecDir) + tr.endpos;
		vecEnd = vecSrc + (vecDir * flDistance);

		if (iPenetrationPower > 1)
		{
			v37 = 1;

			while (1)
			{
				int v39 = v37;

				vStartPos = (v39 * vecDir) + tr.endpos;
				vEndPos = vStartPos + vecDir;

				g_Engine.pEventAPI->EV_SetTraceHull(2);
				g_Engine.pEventAPI->EV_PlayerTrace(vStartPos, vEndPos, 0, -1, &trOriginal);

				if (trOriginal.startsolid)
				{
					if (trOriginal.inopen)
						break;
				}

				if (++v37 == iPenetrationPower)
					goto LABEL_10;

				tr.endpos = trOriginal.endpos;
			}
		}

		g_Engine.pEventAPI->EV_PlayerTrace(&vEndPos.x, &vStartPos.x, 0, -1, &trOriginal);
	}

	if (iPenetration_0 == 0)
		return iCurrentDamage;

	return 0;
}*/

int CUtils::SimulateFireBullet(Vector vecStart, Vector vecEnd, float flDistance, int iOrigPenetration, int iBulletType, int iDamage, float flRangeModifier)
{
	//return MyFireBullets(vecStart,vecEnd);
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

void CUtils::GetHitboxes(const cl_entity_s* a_pGameEntity)
{
	if (!a_pGameEntity || !pCStudioModelRenderer)
		return;

	if (a_pGameEntity->index < 1 || a_pGameEntity->index > MAX_ENTITIES)
		return;

	const auto pEntity = g_World.GetEntity(a_pGameEntity->index);

	auto iResolved = -1;

	if (pEntity->m_bIsPlayer)
	{
		iResolved = pEntity->m_iIndex;
	}
	else if (strstr(pEntity->m_szModelName, "/player"))
	{
		for (auto j = 1; j <= MAX_CLIENTS; ++j)
		{
			if (j == g_Local.m_iIndex)
				continue;

			const auto pPlayer = g_World.GetPlayer(j);

			if (!pPlayer)
				continue;

			if (pPlayer->m_vecOrigin.Distance(pEntity->m_vecOrigin) < 16.f)
				iResolved = pPlayer->m_iIndex;
		}
	}

	if (iResolved >= 1 && iResolved <= MAX_CLIENTS && iResolved != g_Local.m_iIndex)
	{
		const auto pStudioHeader = pCStudioModelRenderer->m_pStudioHeader;

		if (!pStudioHeader || !pStudioHeader->numbodyparts)
			return;

		if (pCStudioModelRenderer->m_pPlayerInfo == NULL) //detect not player model https://github.com/FWGS/xash3d/blob/ab7c3848f85571a3f86817a2ced3fb87371ea560/engine/client/gl_studio.c#L3480
		{
			if (pCStudioModelRenderer->m_pCurrentEntity && pCStudioModelRenderer->m_pCurrentEntity->curstate.weaponmodel) //zombie and etc fix
				return;
		}

		if (pStudioHeader->numhitboxes > HITBOX_MAX)
		{
			TraceLog("> %s: numhitboxes %i > %i\n", __FUNCTION__, pStudioHeader->numhitboxes, HITBOX_MAX);
			g_Engine.Cvar_SetValue("cl_minmodels", 1.f);
			return;
		}

		const auto pBoneMatrix = pCStudioModelRenderer->m_pbonetransform;
		const auto pHitbox = (mstudiobbox_t*)((byte*)(pStudioHeader)+pStudioHeader->hitboxindex);

		if (pHitbox == nullptr)
			return;

		const auto pPlayer = g_World.GetPlayer(iResolved);

		Vector vecBoundBoxMins, vecBoundBoxMaxs, vecTransform, vecMultiPoint;

		for (auto i = 0; i < pStudioHeader->numhitboxes; i++)
		{
			VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vecBoundBoxMins);
			VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vecBoundBoxMaxs);

			pPlayer->m_vecHitboxMin[i] = vecBoundBoxMins;
			pPlayer->m_vecHitboxMax[i] = vecBoundBoxMaxs;
			pPlayer->m_vecHitbox[i] = (vecBoundBoxMaxs + vecBoundBoxMins) * 0.5f;
			pPlayer->m_vecHitboxPoints[i][0] = vecBoundBoxMaxs;
			pPlayer->m_vecHitboxPoints[i][1] = vecBoundBoxMins;

			vecMultiPoint = (pHitbox[i].bbmin / pHitbox[i].bbmax);

			VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vecTransform, 0, vecMultiPoint.x);
			pPlayer->m_vecHitboxPoints[i][2] = vecTransform;

			VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vecTransform, 1, vecMultiPoint.y);
			pPlayer->m_vecHitboxPoints[i][3] = vecTransform;

			VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vecTransform, 2, vecMultiPoint.z);
			pPlayer->m_vecHitboxPoints[i][4] = vecTransform;

			vecMultiPoint = (pHitbox[i].bbmax / pHitbox[i].bbmin);

			VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vecTransform, 0, vecMultiPoint.x);
			pPlayer->m_vecHitboxPoints[i][5] = vecTransform;

			VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vecTransform, 1, vecMultiPoint.y);
			pPlayer->m_vecHitboxPoints[i][6] = vecTransform;

			VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vecTransform, 2, vecMultiPoint.z);
			pPlayer->m_vecHitboxPoints[i][7] = vecTransform;
		}

		if (pStudioHeader->numhitboxes < HITBOX_MAX)
		{
			for (auto j = pStudioHeader->numhitboxes; j < HITBOX_MAX; j++)
			{
				pPlayer->m_vecHitbox[j] = pPlayer->m_vecHitbox[0];
				pPlayer->m_vecHitboxMin[j] = pPlayer->m_vecHitboxMin[0];
				pPlayer->m_vecHitboxMax[j] = pPlayer->m_vecHitboxMax[0];

				for (auto x = 0; x < IM_ARRAYSIZE(pPlayer->m_vecHitboxPoints[j]); x++)
					pPlayer->m_vecHitboxPoints[j][x] = pPlayer->m_vecHitboxPoints[0][x];
			}
		}
	}
}

typedef struct
{
	const char	*name;
	int		keynum;
} keyname_t;

keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},

	{"KP_HOME",			K_KP_HOME },
	{"KP_UPARROW",		K_KP_UPARROW },
	{"KP_PGUP",			K_KP_PGUP },
	{"KP_LEFTARROW",	K_KP_LEFTARROW },
	{"KP_5",			K_KP_5 },
	{"KP_RIGHTARROW",	K_KP_RIGHTARROW },
	{"KP_END",			K_KP_END },
	{"KP_DOWNARROW",	K_KP_DOWNARROW },
	{"KP_PGDN",			K_KP_PGDN },
	{"KP_ENTER",		K_KP_ENTER },
	{"KP_INS",			K_KP_INS },
	{"KP_DEL",			K_KP_DEL },
	{"KP_SLASH",		K_KP_SLASH },
	{"KP_MINUS",		K_KP_MINUS },
	{"KP_PLUS",			K_KP_PLUS },
	{"CAPSLOCK",		K_CAPSLOCK },

	{"MWHEELUP", K_MWHEELUP },
	{"MWHEELDOWN", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{NULL,0}
};

const char* CUtils::KeyToString(int keynum)
{
	keyname_t	*kn;
	static	char	tinystr[2];

	if (keynum == -1)
		return "<KEY NOT FOUND>";
	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	for (kn = keynames; kn->name; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}

void CUtils::ConvertToASCII(int* keynum)
{
	if (keynum != nullptr)
	{
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) *keynum = K_SPACE;
		else if (GetAsyncKeyState(VK_RETURN) & 0x8000) *keynum = K_ENTER;
		else if (GetAsyncKeyState(VK_TAB) & 0x8000) *keynum = K_TAB;
		else if (GetAsyncKeyState(VK_UP) & 0x8000) *keynum = K_UPARROW;
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000) *keynum = K_DOWNARROW;
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000) *keynum = K_LEFTARROW;
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) *keynum = K_RIGHTARROW;
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

const char* CUtils::GetHitboxNameByIndex(const int hitbox)
{
	switch (hitbox)
	{
	case HITBOX_HEAD:
		return "head";
	case HITBOX_LOWER_CHEST:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
	case HITBOX_NECK:
		return "chest";
	case HITBOX_STOMACH:
		return "stomach";
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_THIGH:
		return "left leg";
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_THIGH:
		return "right leg";
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_HAND:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_WRIST:
		return "left arm";
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_HAND:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_WRIST:
		return "right arm";
	case HITBOX_SHIELD:
		return "shield";
	}

	return "unknown";
}

const char* CUtils::GetWeaponNameByIndex(int nWeaponIndex)
{
	switch (nWeaponIndex)
	{
	case WEAPON_P228:
		return "p228";
	case WEAPON_SCOUT:
		return "scout";
	case WEAPON_XM1014:
		return "xm1014";
	case WEAPON_MAC10:
		return "mac10";
	case WEAPON_AUG:
		return "aug";
	case WEAPON_ELITE:
		return "elite";
	case WEAPON_FIVESEVEN:
		return "fiveseven";
	case WEAPON_UMP45:
		return "ump45";
	case WEAPON_SG550:
		return "sg550";
	case WEAPON_GALIL:
		return "galil";
	case WEAPON_FAMAS:
		return "famas";
	case WEAPON_USP:
		return "usp";
	case WEAPON_GLOCK18:
		return "glock18";
	case WEAPON_AWP:
		return "awp";
	case WEAPON_MP5N:
		return "mp5n";
	case WEAPON_M249:
		return "m249";
	case WEAPON_M3:
		return "m3";
	case WEAPON_M4A1:
		return "m4a1";
	case WEAPON_TMP:
		return "tmp";
	case WEAPON_G3SG1:
		return "g3sg1";
	case WEAPON_DEAGLE:
		return "deagle";
	case WEAPON_SG552:
		return "sg552";
	case WEAPON_AK47:
		return "ak47";
	case WEAPON_P90:
		return "p90";
	case WEAPON_KNIFE:
		return "knife";
	case WEAPON_HEGRENADE:
		return "hegrenade";
	case WEAPON_SMOKEGRENADE:
		return "smokegrenade";
	case WEAPON_C4:
		return "c4";
	case WEAPON_FLASHBANG:
		return "flashbang";
	}

	return "unknown";
}

void CUtils::AdjustSpeed(const double speed)
{
	static double previous_speed = 1.0;

	if (previous_speed != speed)
	{
		EnablePageWrite((DWORD)g_Offsets.m_dwpSpeed, sizeof(double));

		*(double*)g_Offsets.m_dwpSpeed = (speed * 1000.0);

		RestorePageProtection((DWORD)g_Offsets.m_dwpSpeed, sizeof(double));

		previous_speed = speed;
	}
}

float CUtils::FinalFallVelocity(const int usehull)
{
	float fallvel = pmove->flFallVelocity;

	if (fallvel <= 0.f)
		return fallvel;

	float heightground = usehull == HULL_PLAYER_DUCKED ?  g_Local.m_flHeightDucked : g_Local.m_flHeightGround;

	if (heightground == 0.f)
		return fallvel;

	const float gravity = pmove->movevars->gravity;

	if (gravity == 0.f)
		return fallvel;

	float finalfallvel = 0.f;

	while (heightground > 0.f)
	{
		finalfallvel = fallvel;
		fallvel += gravity * g_Globals.m_flFrameTime;
		heightground -= fallvel * g_Globals.m_flFrameTime;
	}

	return finalfallvel;
}

bool CUtils::IsLineBlockedBySmoke(const Vector &from, const Vector &to)
{
	const float smokeRadiusSq = smokeRadius * smokeRadius;

	// distance along line of sight covered by smoke
	float totalSmokedLength = 0.0f;

	// compute unit vector and length of line of sight segment
	Vector sightDir = to - from;
	float sightLength = sightDir.Normalize();

	for (int i = 1; i < MAX_ENTITIES; ++i)
	{
		CBaseEntity* pEntity = g_World.GetEntity(i);

		if (!pEntity->m_bIsInPVS)
			continue;

		if (pEntity->m_bIsPlayer)
			continue;

		if (!pEntity->m_bIsSmoke)
			continue;

		if (pEntity->m_vecDetonatedOrigin.IsZero())
			continue;

		Vector smokeOrigin = pEntity->m_vecDetonatedOrigin;

		Vector toGrenade = smokeOrigin - from;

		float alongDist = toGrenade.Dot(sightDir);

		// compute closest point to grenade along line of sight ray
		Vector close;

		// constrain closest point to line segment
		if (alongDist < 0.0f)
			close = from;

		else if (alongDist >= sightLength)
			close = to;
		else
			close = from + sightDir * alongDist;

		// if closest point is within smoke radius, the line overlaps the smoke cloud
		Vector toClose = close - smokeOrigin;
		float lengthSq = toClose.LengthSqr();

		if (lengthSq < smokeRadiusSq)
		{
			// some portion of the ray intersects the cloud
			float fromSq = toGrenade.LengthSqr();
			float toSq = (smokeOrigin - to).LengthSqr();

			if (fromSq < smokeRadiusSq)
			{
				if (toSq < smokeRadiusSq)
				{
					// both 'from' and 'to' lie within the cloud
					// entire length is smoked
					totalSmokedLength += (to - from).Length();
				}
				else
				{
					// 'from' is inside the cloud, 'to' is outside
					// compute half of total smoked length as if ray crosses entire cloud chord
					float halfSmokedLength = sqrt(smokeRadiusSq - lengthSq);

					if (alongDist > 0.0f)
					{
						// ray goes thru 'close'
						totalSmokedLength += halfSmokedLength + (close - from).Length();
					}
					else
					{
						// ray starts after 'close'
						totalSmokedLength += halfSmokedLength - (close - from).Length();
					}
				}
			}
			else if (toSq < smokeRadiusSq)
			{
				// 'from' is outside the cloud, 'to' is inside
				// compute half of total smoked length as if ray crosses entire cloud chord
				float halfSmokedLength = sqrt(smokeRadiusSq - lengthSq);

				Vector v = to - smokeOrigin;
				if (v.Dot(sightDir) > 0.0f)
				{
					// ray goes thru 'close'
					totalSmokedLength += halfSmokedLength + (close - *to).Length();
				}
				else
				{
					// ray ends before 'close'
					totalSmokedLength += halfSmokedLength - (close - *to).Length();
				}
			}
			else
			{
				// 'from' and 'to' lie outside of the cloud - the line of sight completely crosses it
				// determine the length of the chord that crosses the cloud

				float smokedLength = 2.0f * sqrt(smokeRadiusSq - lengthSq);
				totalSmokedLength += smokedLength;
			}
		}
	}

	// define how much smoke a bot can see thru
	const float maxSmokedLength = 0.7f * smokeRadius;

	// return true if the total length of smoke-covered line-of-sight is too much
	return (totalSmokedLength > maxSmokedLength);
}

#define DotProduct(a, b) (a.x * b.x + a.y * b.y + a.z * b.z)

void CUtils::RotateInvisible(const float fixed_yaw, const float fixed_pitch, usercmd_s* cmd)
{
	Vector vViewReal = cmd->viewangles;

	Vector vecViewForward, vecViewRight, vecViewUp;
	g_Engine.pfnAngleVectors(Vector(0, vViewReal.y, 0), vecViewForward, vecViewRight, vecViewUp);

	vViewReal.y += fixed_yaw;

	Vector vecAimForward, vecAimRight, vecAimUp;
	g_Engine.pfnAngleVectors(Vector(0, vViewReal.y, 0), vecAimForward, vecAimRight, vecAimUp);

	const float flForward =
		DotProduct(cmd->forwardmove * vecViewForward.vecNormalize(), vecAimForward) +
		DotProduct(cmd->sidemove * vecViewRight.vecNormalize(), vecAimForward) +
		DotProduct(cmd->upmove * vecViewUp.vecNormalize(), vecAimForward);

	const float flRight =
		DotProduct(cmd->forwardmove * vecViewForward.vecNormalize(), vecAimRight) +
		DotProduct(cmd->sidemove * vecViewRight.vecNormalize(), vecAimRight) +
		DotProduct(cmd->upmove * vecViewUp.vecNormalize(), vecAimRight);

	const float flUp =
		DotProduct(cmd->forwardmove * vecViewForward.vecNormalize(), vecAimUp) +
		DotProduct(cmd->sidemove * vecViewRight.vecNormalize(), vecAimUp) +
		DotProduct(cmd->upmove * vecViewUp.vecNormalize(), vecAimUp);

	cmd->forwardmove = fixed_pitch > 81.0F ? -flForward : flForward;
	cmd->sidemove = flRight;
	cmd->upmove = flUp;
}

void CUtils::AngleMatrix(const float *angles, float(*matrix)[4])
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = angles[1] * (IM_PI * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[0] * (IM_PI * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[2] * (IM_PI * 2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	// matrix = (YAW * PITCH) * ROLL
	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;
	matrix[0][1] = sr * sp*cy + cr * -sy;
	matrix[1][1] = sr * sp*sy + cr * cy;
	matrix[2][1] = sr * cp;
	matrix[0][2] = (cr*sp*cy + -sr * -sy);
	matrix[1][2] = (cr*sp*sy + -sr * cy);
	matrix[2][2] = cr * cp;
	matrix[0][3] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][3] = 0.0;
}

std::vector<std::string> CUtils::explode(std::string const & s, char delim)
{
	std::vector<std::string> result;
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim); )
	{
		result.push_back(std::move(token));
	}

	return result;
}

CSkipFrames::CSkipFrames() :
	m_frames(0),
	m_norefresh_active(false),
	m_init(false)
{
	RtlSecureZeroMemory(m_cvar, sizeof(m_cvar));
	RtlSecureZeroMemory(m_saved, sizeof(m_saved));
}

void CSkipFrames::InitCvars()
{
	if (!m_init)
	{
		m_cvar[hud_draw] = g_Engine.pfnGetCvarPointer("hud_draw");
		m_cvar[r_drawentities] = g_Engine.pfnGetCvarPointer("r_drawentities");
		m_cvar[net_graph] = g_Engine.pfnGetCvarPointer("net_graph");
		m_cvar[cl_showfps] = g_Engine.pfnGetCvarPointer("cl_showfps");
		m_cvar[r_norefresh] = g_Engine.pfnGetCvarPointer("r_norefresh");

		for (size_t i = 0; i < MAX_COUNT - 1; i++)
			m_saved[i] = m_cvar[i]->value;

		m_init = true;
	}
}

void CSkipFrames::NoRefreshEnable()
{
	for (size_t i = 0; i < MAX_COUNT - 1; i++)
		m_cvar[i]->value = 0.f;

	m_cvar[r_norefresh]->value = 1.f;
	m_norefresh_active = true;
}

void CSkipFrames::NoRefreshDisable()
{
	for (size_t i = 0; i < MAX_COUNT - 1; i++)
		m_cvar[i]->value = m_saved[i];

	m_cvar[r_norefresh]->value = 0.f;
	m_norefresh_active = false;
}

void CSkipFrames::Start(const int value)
{
	InitCvars();

	if (m_frames-- <= 0)
	{
		m_frames = value;
		m_frames += (m_frames % 2 == 0) ? 0 : 1;

		NoRefreshDisable();
	}
	else
	{
		NoRefreshEnable();
	}
}

void CSkipFrames::End()
{
	if (m_norefresh_active)
	{
		m_frames = 0;

		NoRefreshDisable();
	}

	m_init = false;
}