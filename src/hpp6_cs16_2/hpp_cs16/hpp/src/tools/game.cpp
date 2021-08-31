#include "framework.h"

void Game::ClampSpeed(const float &maxspeed, usercmd_s* cmd)
{
	if (maxspeed <= 0.f)
	{
		cmd->sidemove = 0;
		cmd->forwardmove = 0;
	}
	else if (g_Local->m_flVelocity > maxspeed)
	{
		QAngle QAngles(cmd->viewangles);

		Vector vecForward, vecRight, vecWishVelocity;

		QAngles.AngleVectors(&vecForward, &vecRight, NULL);
		
		vecWishVelocity.x = -g_Local->m_vecVelocity.x;
		vecWishVelocity.y = -g_Local->m_vecVelocity.y;

		cmd->forwardmove = (vecWishVelocity.y - (vecRight.y / vecRight.x) * vecWishVelocity.x) / (vecForward.y - (vecRight.y / vecRight.x) * vecForward.x);
		cmd->sidemove = (vecWishVelocity.x - vecForward.x * cmd->forwardmove) / vecRight.x;
	}
}

Vector Game::PredictPlayer(const int& nPlayerID)
{
	assert(nPlayerID >= 1 && nPlayerID <= MAX_CLIENTS);

	return g_Player[nPlayerID]->m_vecVelocity * g_Local->m_flFrameTime;
}

void Game::FixPlayersPhysents()
{
	assert(pmove->numphysent >= 0 && pmove->numphysent < MAX_PHYSENTS);

	for (int i = 0; i < pmove->numphysent; i++)
	{
		if (pmove->physents[i].player)
		{
			cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(pmove->physents[i].info);

			if (!pGameEntity)
				continue;

			if (!pGameEntity->player)
				continue;

			Game::GetBoundBox(pmove->physents[i].mins, pmove->physents[i].maxs, pGameEntity->curstate.usehull);
		}
	}
}

void Game::EstimateGait()
{
	cl_entity_s *pGameEntity = g_Engine.GetLocalPlayer();

	if (!pGameEntity)
		return;

	float dt = std::clamp(g_Local->m_flFrameTime, 0.f, 1.f);

	if (dt == 0.f)
	{
		g_pGlobals->m_flGaitMovement = 0;
		return;
	}

	QAngle QAngles(pmove->angles);

	//g_Engine.Con_NPrintf(19, "QAngles.y %f", QAngles.y);

	Vector est_velocity = pmove->origin - g_pGlobals->m_vecPreviousGaitOrigin;

	g_pGlobals->m_vecPreviousGaitOrigin = pmove->origin;

	g_pGlobals->m_flGaitMovement = est_velocity.Length();

	if (dt <= 0.f || g_pGlobals->m_flGaitMovement / dt < 5.f)
	{
		g_pGlobals->m_flGaitMovement = 0;

		est_velocity.x = 0;
		est_velocity.y = 0;
	}

	if (pGameEntity->curstate.sequence >= 101)
	{
		g_pGlobals->m_flGaitYaw = g_pGlobals->m_flGaitYaw + (QAngles.y - g_pGlobals->m_flGaitYaw);
	}
	else {
		if (est_velocity.x == 0.f && est_velocity.y == 0.f)
		{
			g_pGlobals->m_flYawDiff = QAngles.y - g_pGlobals->m_flGaitYaw;
			g_pGlobals->m_flYaw = fmod(g_pGlobals->m_flYawDiff, 360.f);

			g_pGlobals->m_flYawDiff = g_pGlobals->m_flYawDiff - int64(g_pGlobals->m_flYawDiff / 360) * 360;

			if (g_pGlobals->m_flYawDiff > 180)
				g_pGlobals->m_flYawDiff -= 360;
			else if (g_pGlobals->m_flYawDiff < -180)
				g_pGlobals->m_flYawDiff += 360;

			if (g_pGlobals->m_flYaw < -180)
				g_pGlobals->m_flYaw += 360;
			else if (g_pGlobals->m_flYaw > 180)
				g_pGlobals->m_flYaw -= 360;

			if (g_pGlobals->m_flYaw > -5.0 && g_pGlobals->m_flYaw < 5.0)
				g_pGlobals->m_flYawModifier = 0.05f;

			if (g_pGlobals->m_flYaw < -90.0 || g_pGlobals->m_flYaw > 90.0)
				g_pGlobals->m_flYawModifier = 3.5f;

			if (dt < 0.25f)
				g_pGlobals->m_flYawDiff *= dt * g_pGlobals->m_flYawModifier;
			else
				g_pGlobals->m_flYawDiff *= dt;

			if (abs(int64(g_pGlobals->m_flYawDiff)) < 0.1f)
				g_pGlobals->m_flYawDiff = 0;

			g_pGlobals->m_flGaitYaw += g_pGlobals->m_flYawDiff;
			g_pGlobals->m_flGaitYaw -= int64(g_pGlobals->m_flGaitYaw / 360) * 360;
			g_pGlobals->m_flGaitMovement = 0;
		/*	g_Engine.Con_NPrintf(16, "flYawDiff %f", g_pGlobals->m_flYawDiff);
			g_Engine.Con_NPrintf(17, "m_flYawModifier %f", g_pGlobals->m_flYawModifier);
			g_Engine.Con_NPrintf(18, "flYaw %f", g_pGlobals->m_flYaw);*/
		}
		else
		{
			g_pGlobals->m_flGaitYaw = RAD2DEG(atan2(est_velocity.y, est_velocity.x));
			
			if (g_pGlobals->m_flGaitYaw > 180)
				g_pGlobals->m_flGaitYaw = 180;

			if (g_pGlobals->m_flGaitYaw < -180)
				g_pGlobals->m_flGaitYaw = -180;
		}

		{ // CalculateYawBlend
			float maxyaw = 255.0f;

			// calc side to side turning
			float flYaw = fmod(QAngles.y - g_pGlobals->m_flGaitYaw, 360.f);

			if (flYaw < -180)
				flYaw += 360;
			else if (flYaw > 180)
				flYaw -= 360;

			if (g_pGlobals->m_flGaitMovement != 0.f)
			{
				if (flYaw > 120.f)
				{
					flYaw -= 180;
					g_pGlobals->m_flGaitYaw -= 180;
					g_pGlobals->m_flGaitMovement = -g_pGlobals->m_flGaitMovement;
				}
				else if (flYaw < -120.f)
				{
					flYaw += 180;
					g_pGlobals->m_flGaitYaw += 180;
					g_pGlobals->m_flGaitMovement = -g_pGlobals->m_flGaitMovement;
				}
			}

			flYaw = (flYaw / 90.f) * 128.f + 127.f;
			flYaw = std::clamp(flYaw, 0.f, maxyaw);

			if (!g_pMiscellaneous->m_iChokedCommands)
			{
				g_pGlobals->m_flBlendYaw = maxyaw - flYaw;
				//g_Engine.Con_NPrintf(14, "m_flBlendYaw %f", g_pGlobals->m_flBlendYaw);
			}

			g_pGlobals->m_flYaw = g_pGlobals->m_flBlendYaw;
		}
	}

	//g_Engine.Con_NPrintf(15, "m_flGaityaw %f", g_pGlobals->m_flGaitYaw);
}

bool Game::TraceShield(const Vector& vecSrc, const Vector& vecAdjustedOrigin, const QAngle& QInAngles, const int& iPlayer, const float& flCustomDistance)
{
	assert(iPlayer >= 1 && iPlayer <= MAX_CLIENTS);

	if (!g_Player[iPlayer]->m_bHasShield)
		return false;

	float flDistance = g_Weapon->m_flDistance;

	if (flCustomDistance != -1.f)
		flDistance = flCustomDistance;

	QAngle QAngles(QInAngles.x, QInAngles.y, QInAngles.z);

	Vector vecSpreadDir, vecForward, vecRight, vecUp, vecRandom;

	QAngles[0] += client_state->punchangle[0] * 2.f;
	QAngles[1] += client_state->punchangle[1] * 2.f;

	QAngles.Normalize();

	QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

	g_pNoSpread->GetSpreadXY(g_Weapon->m_iRandomSeed, 1, vecRandom);

	vecSpreadDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

	vecSpreadDir.Normalize();

	matrix3x4_t matHitboxShield = g_Player[iPlayer]->m_matHitbox[HITBOX_SHIELD];

	matHitboxShield.SetOrigin(vecAdjustedOrigin + matHitboxShield.GetOrigin() - g_Player[iPlayer]->m_vecOrigin);

	float fraction = -1.f; int hitside = 0; bool startsolid = false;

	if (Math::IntersectRayWithOBB(vecSrc, vecSpreadDir * flDistance, matHitboxShield, g_Player[iPlayer]->m_vecOBBMin[HITBOX_SHIELD], g_Player[iPlayer]->m_vecOBBMax[HITBOX_SHIELD], fraction, hitside, startsolid))
	{
		for (int hitbox = 0; hitbox < HITBOX_MAX; hitbox++)
		{
			if (hitbox == HITBOX_SHIELD)
				continue;

			matrix3x4_t matHitbox = g_Player[iPlayer]->m_matHitbox[hitbox];

			matHitbox.SetOrigin(vecAdjustedOrigin + matHitbox.GetOrigin() - g_Player[iPlayer]->m_vecOrigin);

			float fraction2 = -1.f; int hitside2 = 0; bool startsolid2 = false;

			if (Math::IntersectRayWithOBB(vecSrc, vecSpreadDir * flDistance, matHitbox, g_Player[iPlayer]->m_vecOBBMin[hitbox], g_Player[iPlayer]->m_vecOBBMax[hitbox], fraction2, hitside2, startsolid2))
			{
				if (fraction2 <= fraction)
					return false;
			}
		}

		return true;
	}

	return false;
}

bool Game::FindSpanningContexts(cl_entity_t* ent, float targettime, position_history_t** newer, position_history_t** older)
{
	assert(newer);
	assert(older);

	bool extrapolate = true;

	int imod = ent->current_position;
	int i0 = (imod - 0) & HISTORY_MASK;	// curpos (lerp end)
	int i1 = (imod - 1) & HISTORY_MASK;	// oldpos (lerp start)

	for (int i = 1; i < HISTORY_MAX - 1; i++)
	{
		position_history_t c1 = ent->ph[((imod - i) + 0) & HISTORY_MASK];
		position_history_t c2 = ent->ph[((imod - i) + 1) & HISTORY_MASK];

		if (c1.animtime == 0.f)
			break;

		if (c2.animtime >= targettime && c1.animtime <= targettime)
		{
			i0 = ((imod - i) + 1) & HISTORY_MASK;
			i1 = ((imod - i) + 0) & HISTORY_MASK;
			extrapolate = false;
			break;
		}
	}

	if (newer) *newer = &ent->ph[i0];
	if (older) *older = &ent->ph[i1];

	return extrapolate;
}

bool Game::BacktrackPlayer(cl_entity_s* pGameEntity, int lerp_msec, Vector& origin)
{
	static cvar_t* sv_unlag = g_ClientCvarsMap["sv_unlag"];
	static cvar_t* cl_lw = g_ClientCvarsMap["cl_lw"];
	static cvar_t* cl_lc = g_ClientCvarsMap["cl_lc"];
	static cvar_t* cl_updaterate = g_ClientCvarsMap["cl_updaterate"];
	static cvar_t* sv_maxunlag = g_ClientCvarsMap["sv_maxunlag"];
	static cvar_t* ex_interp = g_ClientCvarsMap["ex_interp"];
	static cvar_t* sv_unlagpush = g_ClientCvarsMap["sv_unlagpush"];

	// Player not wanting lag compensation
	if (!sv_unlag->value || !cl_lw->value || !cl_lc->value)
		return false;

	// Get true latency
	const double fakelatency = g_pMiscellaneous->m_bFakeLatencyActive ? cvars::misc.fakelatency_amount / 1000.0 : 0.0;

	double latency = client_state->frames[client_state->parsecountmod].latency + fakelatency;

	if (latency > 1.5)
		latency = 1.5;
	
	double update_interval = 0.1;

	if (cl_updaterate->value > 10.f)
		update_interval = 1.0 / double(cl_updaterate->value);

	// Fixup delay based on message interval (cl_updaterate, default 20 so 50 msec)
	//latency -= update_interval;

	// Further fixup due to client side delay because packets arrive 1/2 through the frame loop, on average
	//latency -= (g_Local->m_flFrameTime/* * 0.5*/);

	// Absolute bounds on lag compensation
	double correct = min(LAG_COMPENSATION_DATA_TIME, latency);

	// See if server is applying a lower cap
	if (sv_maxunlag->value)
	{
		// Make sure it's not negative
		if (sv_maxunlag->value < 0.f)
			sv_maxunlag->value = 0.f;

		// Apply server cap
		correct = min(correct, sv_maxunlag->value);
	}

	// Get true timestamp
	const double realtime = client_state->time;

	// Figure out timestamp for which we are looking for data
	double targettime = realtime - correct;

	// Remove lag based on player interpolation, as well
	double interptime = (lerp_msec == -1) ? ex_interp->value : (lerp_msec / 1000.0);

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

	FindSpanningContexts(pGameEntity, float(targettime), &newer, &older);

	if (!newer || !older)
		return false;

	float frac = 0.f;

	if (newer->animtime != older->animtime)
	{
		frac = float(targettime - older->animtime) / (newer->animtime - older->animtime);
		frac = std::clamp(frac, 0.f, 1.f);
	}

	Vector delta = newer->origin - older->origin;

	if (delta.LengthSqr() > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR)
		return false;

	origin = older->origin + delta * frac;

	return true;
}

bool Game::GetBacktrackOrigin(cl_entity_s* pGameEntity, Vector& origin)
{
	origin = pGameEntity->origin;

	int best_lerp_msec = -1;

	if (cvars::legitbot.active && cvars::legitbot.position_adjustment && g_Weapon.IsGun())
	{
		Vector vecForward;

		g_Local->m_QAngles.AngleVectors(&vecForward, NULL, NULL);

		vecForward.Normalize();

		float best_fov = FLT_MAX;

		for (int lerp_msec = 0; lerp_msec <= 100; lerp_msec++)
		{
			Vector vecTempOrigin;

			if (Game::BacktrackPlayer(pGameEntity, lerp_msec, vecTempOrigin))
			{
				const float current_fov = vecForward.AngleBetween(vecTempOrigin - g_Local->m_vecEyePos);

				if (current_fov < best_fov)
				{
					best_lerp_msec = lerp_msec;
					best_fov = current_fov;
				}
			}
		}
	}
	else if (cvars::misc.kb_enabled && cvars::misc.kb_position_adjustment && g_Weapon.IsKnife())
	{
		float best_distance = FLT_MAX;

		for (int lerp_msec = 0; lerp_msec <= 100; lerp_msec++)
		{
			Vector vecTempOrigin;

			if (Game::BacktrackPlayer(pGameEntity, lerp_msec, vecTempOrigin))
			{
				const float current_distance = vecTempOrigin.Distance(g_Local->m_vecOrigin);

				if (current_distance < best_distance)
				{
					best_lerp_msec = lerp_msec;
					best_distance = current_distance;
				}
			}
		}
	}
	else if (cvars::misc.fakelatency && cvars::misc.fakelatency_amount)
	{
		if (Game::BacktrackPlayer(pGameEntity, best_lerp_msec, origin))
			return true;
	}

	if (best_lerp_msec != -1)
	{
		const float interp_amount = g_pMiscellaneous->GetInterpAmount(best_lerp_msec);
		const int lerp_msec = static_cast<int>(interp_amount * 1000.0);

		if (Game::BacktrackPlayer(pGameEntity, lerp_msec, origin))
			return true;
	}

	return false;
}

void Game::SetFakeLatency(const double &latency)
{
	for (auto sequence : g_Sequences)
	{
		const auto time_difference = client_state->time - sequence.time;

		if (time_difference >= latency)
		{
			client_static->netchan.incoming_sequence = sequence.seq;
			break;
		}
	}
}

void Game::MakeAngle(float* angles, usercmd_s* cmd)
{
	QAngle QAngles(angles), QPreviousAngles(cmd->viewangles);

	QAngles.Normalize();

	QPreviousAngles.Normalize();

	float flSpeed = Vector2D(cmd->forwardmove, cmd->sidemove).Length();

	if (flSpeed)
	{
		float flYaw = QAngles.y - QPreviousAngles.y;
		float flMoveYawRadian = atan2(cmd->sidemove, cmd->forwardmove) + DEG2RAD(flYaw);

		cmd->forwardmove = cos(flMoveYawRadian) * flSpeed;
		cmd->sidemove = sin(flMoveYawRadian) * flSpeed;

		bool bBackwardsOld = (QPreviousAngles.x > 90.f || QPreviousAngles.x < -90.f);
		bool bBackwardsNew = (QAngles.x > 90.f || QAngles.x < -90.f);

		if (bBackwardsOld != bBackwardsNew)
			cmd->forwardmove = -cmd->forwardmove;
	}

	cmd->viewangles = QAngles;
}

void Game::TextureHit(Vector vecSrc, Vector vecEnd, Vector vecDir, int nSkipPhysent, pmtrace_t& pmTrace)
{
	if (vecSrc == vecEnd)
	{
		RtlSecureZeroMemory(&pmTrace, sizeof(pmtrace_t));
		pmTrace.endpos = vecEnd;
		pmTrace.fraction = 1.f;
		return;
	}

	g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
	g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_GLASS_IGNORE, nSkipPhysent, &pmTrace);

	if (!pmTrace.fraction && pmTrace.startsolid && !pmTrace.allsolid)
	{
		Vector vecTemp = vecSrc;

		while (!pmTrace.allsolid && !pmTrace.fraction)
		{
			vecTemp += vecDir;

			g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
			g_Engine.pEventAPI->EV_PlayerTrace(vecTemp, vecEnd, PM_WORLD_ONLY, nSkipPhysent, &pmTrace);
		}

		if (!pmTrace.allsolid && pmTrace.fraction != 1.f)
		{
			float flLengthOne, flLengthTwo;

			vecTemp = vecEnd - vecSrc;
			flLengthOne = vecTemp.Length();

			vecTemp = pmTrace.endpos - vecSrc;
			flLengthTwo = vecTemp.Length();

			pmTrace.fraction = flLengthOne / flLengthTwo;
			pmTrace.startsolid = TRUE;
		}
	}

	if (pmTrace.allsolid)
		pmTrace.fraction = 1.f;
}

#pragma warning(disable: 4244)

int Game::SimulateFireBullet(const Vector& a_vecSrc, const Vector& a_vecEnd, const float& a_flDistance, const int& a_iPenetration, const int& a_iBulletType, const int& a_iDamage, const float& a_flRangeModifier)
{
	float flPenetrationDistance, flDamageModifier(0.5), flCurrentDistance, flDistance(a_flDistance), flLength;

	int iPenetrationPower, iPenetration(a_iPenetration + 1), iCurrentDamage(a_iDamage);
	
	switch (a_iBulletType)
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

	Vector vecSrc(a_vecSrc), vecEnd(a_vecEnd), vecDir;

	vecDir = vecEnd - vecSrc;

	flLength = vecDir.Length();

	vecDir /= flLength;
	vecEnd = vecDir * flDistance + vecSrc;

	pmtrace_t pmTrace;

	while (iPenetration)
	{
		TextureHit(vecSrc, vecEnd, vecDir, -1, pmTrace);

		switch (GetTextureType(&pmTrace, vecSrc, vecEnd))
		{
		case CHAR_TEX_METAL:
			iPenetrationPower *= 0.15;
			flDamageModifier = 0.2;
			break;
		case CHAR_TEX_CONCRETE:
			iPenetrationPower *= 0.25;
			break;
		case CHAR_TEX_GRATE:
			iPenetrationPower *= 0.5;
			flDamageModifier = 0.4;
			break;
		case CHAR_TEX_VENT:
			iPenetrationPower *= 0.5;
			flDamageModifier = 0.45;
			break;
		case CHAR_TEX_TILE:
			iPenetrationPower *= 0.65;
			flDamageModifier = 0.3;
			break;
		case CHAR_TEX_COMPUTER:
			iPenetrationPower *= 0.4;
			flDamageModifier = 0.45;
			break;
		case CHAR_TEX_WOOD:
			flDamageModifier = 0.6;
			break;
		default:
			break;
		}

		if (pmTrace.fraction != 1.f)
		{
			iPenetration--;

			Vector vecTemp = pmTrace.endpos - a_vecSrc;

			if (vecTemp.Length() >= flLength)
			{
				float flLengthOne, flLengthTwo;

				vecTemp = a_vecEnd - vecSrc;
				flLengthOne = vecTemp.Length();

				vecTemp = vecEnd - vecSrc;
				flLengthTwo = vecTemp.Length();

				pmTrace.fraction = flLengthOne / flLengthTwo;

				flCurrentDistance = flDistance * pmTrace.fraction;
				iCurrentDamage *= pow(a_flRangeModifier, flCurrentDistance / 500);

				return iCurrentDamage;
			}

			flCurrentDistance = flDistance * pmTrace.fraction;
			iCurrentDamage *= pow(a_flRangeModifier, flCurrentDistance / 500);

			if (flCurrentDistance > flPenetrationDistance)
				iPenetration = 0;

			float flDistanceModifier;

			if (!iPenetration)
			{
				iPenetrationPower = 42;
				flDamageModifier = 0.75;
				flDistanceModifier = 0.75;
			}
			else
				flDistanceModifier = 0.5;

			vecSrc = pmTrace.endpos + (vecDir * iPenetrationPower);
			flDistance = (flDistance - flCurrentDistance) * flDistanceModifier;
			vecEnd = vecSrc + (vecDir * flDistance);

			iCurrentDamage *= flDamageModifier;
		}
		else
			iPenetration = 0;
	}

	return 0;
}

#pragma warning(default: 4244)

void Game::SendCommand(const bool &status)
{
	client_static->nextcmdtime = status ? -1.f : FLT_MAX;
}

pcmd_t Game::CommandByName(const char* name)
{
	auto* cmdlist = g_pEngine->pfnGetCmdList();

	while (cmdlist)
	{
		if (!strcmp(cmdlist->name, name))
			return cmdlist;

		cmdlist = cmdlist->next;
	}

	return NULL;
}

int Game::GetTeamNum(std::string teamname)
{
	if (!teamname.compare("SPECTATOR"))
		return TEAM_SPECTATOR;

	if (!teamname.compare("TERRORIST"))
		return TEAM_TERRORIST;

	if (!teamname.compare("CT"))
		return TEAM_CT;

	return TEAM_UNASSIGNED;
}

pmtrace_t Game::GetGroundTrace(int usehull)
{
	Vector vecEnd(client_state->simorg.x, client_state->simorg.y, -4096.f);

	pmtrace_t tr;
	g_Engine.pEventAPI->EV_SetTraceHull(usehull);
	g_Engine.pEventAPI->EV_PlayerTrace(client_state->simorg, vecEnd, PM_NORMAL, -1, &tr);

	return tr;
}

void Game::GetBoundBox(Vector& mins, Vector& maxs, int usehull)
{
	mins.Init(-16.f, -16.f, usehull == HULL_DUCKED ? -18.f : -36.f);
	maxs.Init(16.f, 16.f, usehull == HULL_DUCKED ? 32.f : 36.f);
}

void Game::GetWeaponModelName(int weaponmodel, char* out)
{
	if (weaponmodel >= 0 && weaponmodel < MAX_MODELS)
	{
		const auto* model = client_state->model_precache[weaponmodel];

		if (model && model->name && strnlen_s(model->name, MAX_MODEL_NAME))
			strcpy_s(out, MAX_MODEL_NAME, model->name);
	}
}

int Game::LookupSequence(model_s* model, const char** label, int size)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)(g_Studio.Mod_Extradata(model));

	if (!pstudiohdr)
		return ACT_INVALID;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (!_stricmp(pseqdesc[i].label, label[j]))
				return i;
		}
	}

	return ACT_INVALID;
}

bool Game::PM_Ladder()
{
	Vector offset;

	for (int i = 0; i < pmove->nummoveent; i++)
	{
		const auto pe = &pmove->moveents[i];

		if (pe->model && pmove->PM_GetModelType(pe->model) == mod_brush && pe->skin == CONTENTS_LADDER)
		{
			const auto hull = (hull_t*)(pmove->PM_HullForBsp(pe, offset));
			const auto num = hull->firstclipnode;

			offset = client_state->simorg - offset;

			if (pmove->PM_HullPointContents(hull, num, offset) == CONTENTS_EMPTY)
				continue;

			return true;
		}
	}

	return false;
}

void Game::GetHitboxes(cl_entity_s* pGameEntity)
{
	auto ent_index = -1;

	if (g_Entity[pGameEntity->index]->m_bIsPlayer)
	{
		ent_index = g_Entity[pGameEntity->index]->m_iEntIndex;
	}
	else if (strstr(g_Entity[pGameEntity->index]->m_szModelName, "/player"))
	{
		for (int i = 1; i <= client_state->maxclients; i++)
		{
			if (g_Player[i]->m_vecOrigin.Distance(g_Entity[pGameEntity->index]->m_vecOrigin) <= sqrt(2.f) * 16.f)
				ent_index = g_Player[i]->m_iEntIndex;
		}
	}

	if (ent_index >= 1 && ent_index <= MAX_CLIENTS && !g_Player[ent_index]->m_bIsDead)
	{
		studiohdr_t* pstudiohdr = pCStudioModelRenderer->m_pStudioHeader;

		if (!pstudiohdr || !pstudiohdr->numbodyparts)
			return;

		if (!pCStudioModelRenderer->m_pPlayerInfo && pCStudioModelRenderer->m_pCurrentEntity && pCStudioModelRenderer->m_pCurrentEntity->curstate.weaponmodel) // Skip player weapon
			return;

		if (pstudiohdr->numhitboxes > HITBOX_MAX)
		{
			if(pCStudioModelRenderer->m_pRenderModel)
				Utils::TraceLog(V("%s: player %s using bogus model %s. (%s)\n"), V(__FUNCTION__), g_Player[ent_index]->m_szPrintName, pCStudioModelRenderer->m_pRenderModel->name, g_Engine.pNetAPI->AdrToString(&client_static->netchan.remote_address));
			
			if (g_Local->m_iEntIndex == ent_index)
				g_Local->m_bReplaceModel = true;
			else
				g_Player[ent_index]->m_bReplaceModel = true;

			return;
		}

		const auto* pBoneMatrix = pCStudioModelRenderer->m_pbonetransform;
		const auto* pHitbox = (mstudiobbox_t*)((byte*)(pstudiohdr) + pstudiohdr->hitboxindex);

		Vector vecBoundBoxMins, vecBoundBoxMaxs, vecMultiPoint, vecTransform;

		for (int i = 0; i < pstudiohdr->numhitboxes; i++)
		{
			const auto matrix = matrix3x4_t((*pBoneMatrix)[pHitbox[i].bone]);

			g_Player[ent_index]->m_matHitbox[i] = matrix;
			g_Player[ent_index]->m_vecOBBMin[i] = pHitbox[i].bbmin;
			g_Player[ent_index]->m_vecOBBMax[i] = pHitbox[i].bbmax;

			Math::VectorTransform(pHitbox[i].bbmin, matrix, vecBoundBoxMins);
			Math::VectorTransform(pHitbox[i].bbmax, matrix, vecBoundBoxMaxs);

			g_Player[ent_index]->m_vecHitboxMin[i] = vecBoundBoxMins;
			g_Player[ent_index]->m_vecHitboxMax[i] = vecBoundBoxMaxs;
			g_Player[ent_index]->m_vecHitbox[i] = (vecBoundBoxMaxs + vecBoundBoxMins) * 0.5f;

			g_Player[ent_index]->m_vecHitboxPoints[i][0] = vecBoundBoxMaxs;
			g_Player[ent_index]->m_vecHitboxPoints[i][1] = vecBoundBoxMins;

			vecMultiPoint = pHitbox[i].bbmin / pHitbox[i].bbmax;

			Math::VectorTransform(pHitbox[i].bbmax, matrix, vecTransform, 0, vecMultiPoint.x);
			g_Player[ent_index]->m_vecHitboxPoints[i][2] = vecTransform;

			Math::VectorTransform(pHitbox[i].bbmax, matrix, vecTransform, 1, vecMultiPoint.y);
			g_Player[ent_index]->m_vecHitboxPoints[i][3] = vecTransform;

			Math::VectorTransform(pHitbox[i].bbmax, matrix, vecTransform, 2, vecMultiPoint.z);
			g_Player[ent_index]->m_vecHitboxPoints[i][4] = vecTransform;

			vecMultiPoint = pHitbox[i].bbmax / pHitbox[i].bbmin;

			Math::VectorTransform(pHitbox[i].bbmin, matrix, vecTransform, 0, vecMultiPoint.x);
			g_Player[ent_index]->m_vecHitboxPoints[i][5] = vecTransform;

			Math::VectorTransform(pHitbox[i].bbmin, matrix, vecTransform, 1, vecMultiPoint.y);
			g_Player[ent_index]->m_vecHitboxPoints[i][6] = vecTransform;

			Math::VectorTransform(pHitbox[i].bbmin, matrix, vecTransform, 2, vecMultiPoint.z);
			g_Player[ent_index]->m_vecHitboxPoints[i][7] = vecTransform;
		}

		if (pstudiohdr->numhitboxes < HITBOX_MAX)
		{
			for (auto i = pstudiohdr->numhitboxes; i < HITBOX_MAX; i++)
			{
				g_Player[ent_index]->m_matHitbox[i].Clear();
				g_Player[ent_index]->m_vecOBBMin[i].Clear();
				g_Player[ent_index]->m_vecOBBMax[i].Clear();

				g_Player[ent_index]->m_vecHitbox[i].Clear();
				g_Player[ent_index]->m_vecHitboxMin[i].Clear();
				g_Player[ent_index]->m_vecHitboxMax[i].Clear();

				for (int j = 0; j < HITBOX_POINTS_MAX; j++)
					g_Player[ent_index]->m_vecHitboxPoints[i][j].Clear();
			}
		}
	}
}

static void FindPoint(Vector2D& point, float degrees)
{
	ImVec2 center_screen = GImGui->IO.DisplaySize * 0.5f;

	const float d = sqrt(pow((point.x - center_screen.x), 2) + pow((point.y - center_screen.y), 2));
	const float r = degrees / d;

	point.x = r * point.x + (1.f - r) * center_screen.x;
	point.y = r * point.y + (1.f - r) * center_screen.y;
}

bool Game::WorldToScreen(Vector origin, Vector2D& screen)
{
	const bool behind = g_Engine.pTriAPI->WorldToScreen(origin, screen);

	screen.x = screen.x * GImGui->IO.DisplaySize.x * 0.5f + GImGui->IO.DisplaySize.x * 0.5f + 0.5f;
	screen.y = -screen.y * GImGui->IO.DisplaySize.y * 0.5f + GImGui->IO.DisplaySize.y * 0.5f + 0.5f;

	if (behind || screen.x < 0.f || screen.y < 0.f || screen.x > GImGui->IO.DisplaySize.x || screen.y > GImGui->IO.DisplaySize.y)
	{
		FindPoint(screen, GImGui->IO.DisplaySize.y * 0.5f);

		if (behind)
		{
			screen.x = GImGui->IO.DisplaySize.x - screen.x;
			screen.y = GImGui->IO.DisplaySize.y - screen.y;
		}

		return false;
	}

	return true;
}

std::string Game::GetWeaponName(int index)
{
	for (auto& weapon : weapon_id_name)
		if (weapon.id == index)
			return weapon.name;

	return "unknown";
}

int Game::GetWeaponIndex(std::string name)
{
	for (auto& weapon : weapon_id_name)
		if (strstr(name.c_str(), weapon.name))
			return weapon.id;

	return WEAPON_NONE;
}

std::string Game::KeynumToString(int keynum)
{
	keyname_t* kn;
	static char tinystr[2];

	if (keynum > 32 && keynum < 127)
	{
		tinystr[0] = toupper(keynum);
		tinystr[1] = 0;
		return tinystr;
	}

	for (kn = keynames_ascii; kn->name; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return std::string();
}

int Game::GetRespawnHealth()
{
	if (strstr(client_state->levelname, "1hp")) return 1;
	else if (strstr(client_state->levelname, "35hp")) return 35;
	else return 100;
}

bool Game::ObservedPlayer(int index)
{
	if (!g_Local->m_bIsDead)
		return false;

	if (g_Local->m_iObserverState != OBS_IN_EYE)
		return false;

	if (g_Local->m_iObserverIndex != index)
		return false;

	return true;
}

bool Game::IsTakenScreenshot()
{
	return g_pGlobals->m_bSnapshot || g_pGlobals->m_bScreenshot;
}

bool Game::IsConnected()
{
	return g_pGlobals->m_bIsInGame && g_Local->m_bIsConnected && g_pGlobals->m_flGameTime;
}

void Game::GetColorHealthBased(int health, float* out)
{
	auto fhealth = static_cast<float>(health);
	fhealth = std::clamp(fhealth, 0.f, 100.f);

	out[0] = fhealth <= 50.f ? 1.00f : static_cast<float>(Math::Interp(50.0, fhealth, 100.0, 1.00, 0.00));
	out[1] = fhealth >= 50.f ? 1.00f : static_cast<float>(Math::Interp(0.0, fhealth, 50.0, 0.00, 1.00));
	out[2] = 0.20f;
	out[3] = 1.00f;
}

void Game::CountGameTime()
{
	static double start_point_time;

	if (g_pGlobals->m_bIsInGame && g_Local->m_bIsConnected)
	{
		if (!start_point_time)
			start_point_time = client_state->time;

		g_pGlobals->m_flGameTime = static_cast<float>(client_state->time - start_point_time);
	}
	else
	{
		if (start_point_time)
			start_point_time = 0.0;
	}
};

void Game::CountRoundTime()
{
	g_pGlobals->m_flRoundTime -= g_pGlobals->m_flFrameTime;

	if (g_pGlobals->m_flRoundTime < 0.f)
		g_pGlobals->m_flRoundTime = 0.f;
};

void Game::SetRenderFOV(int iFOV)
{
	int iRenderFOV = g_Local->m_iFOV;

	if (iFOV != iRenderFOV)
		iRenderFOV = iFOV;

	g_ClientUserMsgsMap["SetFOV"]("SetFOV", sizeof(BYTE), &iRenderFOV);
}

bool Game::IsValidEntity(cl_entity_s* pGameEntity)
{
	if (!pGameEntity)
		return false;

	if (!pGameEntity->model)
		return false;

	if (!strnlen_s(pGameEntity->model->name, MAX_MODEL_NAME))
		return false;

	if (!pGameEntity->index)
		return false;

	if (!pGameEntity->curstate.number)
		return false;

	return true;
}

int Game::PM_GetPhysEntInfo(int ent)
{
	if (ent >= 0 && ent <= pmove->numphysent)
		return pmove->physents[ent].info;

	return -1;
}

void Game::V_GetChaseOrigin(const Vector& origin, float distance, Vector& returnvec)
{
	Vector forward, right, up;

	g_Local->m_QAngles.AngleVectors(&forward, &right, &up);

	forward.Negate();

	Vector start = origin;
	Vector end = start + forward * distance;

	int ignoreent = -1;
	cl_entity_t* pGameEntity = nullptr;

	pmtrace_t* tr;
	int maxLoops = 8;

	while (maxLoops > 0)
	{
		tr = g_Engine.PM_TraceLine(start, end, PM_TRACELINE_PHYSENTSONLY, HULL_POINT, ignoreent);

		if (tr->ent <= 0)
			break;

		pGameEntity = g_Engine.GetEntityByIndex(PM_GetPhysEntInfo(tr->ent));

		if (!pGameEntity) 
			break;

		if (pGameEntity->curstate.solid == SOLID_BSP && !pGameEntity->player)
			break;

		if (tr->fraction < 1.0f)
		{
			break;
		}
		else
		{
			ignoreent = tr->ent;
			start = tr->endpos;
		}

		maxLoops--;
	}

	returnvec.x = tr->endpos.x + 8.f * tr->plane.normal.x;
	returnvec.y = tr->endpos.y + 8.f * tr->plane.normal.y;
	returnvec.z = tr->endpos.z + 8.f * tr->plane.normal.z;
}

int Game::GetHitgroup(int hitboxnum)
{
	switch (hitboxnum)
	{
	case HITBOX_HEAD:
		return HITGROUP_HEAD;
	case HITBOX_CHEST:
	case HITBOX_LOWER_CHEST:
	case HITBOX_UPPER_CHEST:
	case HITBOX_NECK:
		return HITGROUP_CHEST;
	case HITBOX_STOMACH:
		return HITGROUP_STOMACH;
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_THIGH:
		return HITGROUP_LEFTLEG;
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_THIGH:
		return HITGROUP_RIGHTLEG;
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_HAND:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_WRIST:
		return HITGROUP_LEFTARM;
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_HAND:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_WRIST:
		return HITGROUP_RIGHTARM;
	default:
		return HITGROUP_GENERIC;
	}
}

float Game::TraceAttack(const float& a_flDamage, const int& a_nHitGroup)
{
	float flDamage = a_flDamage;

	if (flDamage)
	{
		switch (a_nHitGroup)
		{
		case HITGROUP_HEAD:
			return flDamage *= 4.f;
		case HITGROUP_CHEST:
			return flDamage *= 1.f;
		case HITGROUP_STOMACH:
			return flDamage *= 0.8f;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return flDamage *= 1.f;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			return flDamage *= 0.75f;
		}
	}

	return flDamage;
}

float Game::ApplyArmor(const float& a_flDamage, const int& a_nHitGroup, const int& a_iPlayer)
{
	float flNewDamage = a_flDamage;

	if (g_Player[a_iPlayer]->m_iArmorType)
	{
		bool bApply = false;

		if (g_Player[a_iPlayer]->m_iArmorType == ARMOR_VESTHELM && a_nHitGroup == HITGROUP_HEAD)
			bApply = true;
		else if (g_Player[a_iPlayer]->m_iArmorType == ARMOR_KEVLAR && a_nHitGroup != HITGROUP_HEAD)
			bApply = true;

		if (bApply)
		{
			float flRatio = 1.f;

			switch (g_Weapon->m_iWeaponID)
			{
			case WEAPON_AUG:
			case WEAPON_M4A1:		flRatio *= 1.4;  break;
			case WEAPON_AWP:		flRatio *= 1.95; break;
			case WEAPON_G3SG1:		flRatio *= 1.65; break;
			case WEAPON_SG550:		flRatio *= 1.45; break;
			case WEAPON_M249:		flRatio *= 1.5;  break;
			case WEAPON_ELITE:		flRatio *= 1.05; break;
			case WEAPON_DEAGLE:		flRatio *= 1.5;  break;
			case WEAPON_GLOCK18:	flRatio *= 1.05; break;
			case WEAPON_FIVESEVEN:
			case WEAPON_P90:
				flRatio *= 1.5;
				break;
			case WEAPON_MAC10:
				flRatio *= 0.95;
				break;
			case WEAPON_P228:
				flRatio *= 1.25;
				break;
			case WEAPON_SCOUT:
			case WEAPON_KNIFE:
				flRatio *= 1.7;
				break;
			case WEAPON_FAMAS:
			case WEAPON_SG552:
				flRatio *= 1.4;
				break;
			case WEAPON_GALIL:
			case WEAPON_AK47:
				flRatio *= 1.55;
				break;
			}

			flNewDamage = (a_flDamage * ARMOR_BONUS) * flRatio;
		}
	}

	return flNewDamage;
}

int Game::TakeDamage(const Vector& a_vecSrc, const Vector& a_vecEnd, const int& a_nHitGroup, const int& a_iPlayer)
{
	float flLengthOne, flLengthTwo, flFraction, flCurrentDistance, flDamage, flRangeModifier(g_Weapon.GetWallPierce());

	int iDamage = g_Weapon.GetDamage();

	Vector vecSrc(a_vecSrc), vecEnd(a_vecEnd), vecDir, vecTemp;

	vecDir = vecEnd - vecSrc;

	vecDir /= vecDir.Length();

	vecEnd = vecDir * g_Weapon->m_flDistance + vecSrc;

	vecTemp = a_vecEnd - vecSrc;

	flLengthOne = vecTemp.Length();

	vecTemp = vecEnd - vecSrc;

	flLengthTwo = vecTemp.Length();

	flFraction = flLengthOne / flLengthTwo;

	flCurrentDistance = g_Weapon->m_flDistance * flFraction;

	iDamage *= pow(flRangeModifier, flCurrentDistance / 500);

	flDamage = TraceAttack(iDamage, a_nHitGroup);

	flDamage = ApplyArmor(flDamage, a_nHitGroup, a_iPlayer);

	return (int)flDamage;
}

int Game::TakeSimulatedDamage(const Vector& a_vecSrc, const Vector& a_vecEnd, const int &a_nHitGroup, const int &a_iPlayer)
{
	Physent::SetMaxs(a_iPlayer, Vector());
	Physent::SetMins(a_iPlayer, Vector());

	float flDamage = (float)SimulateFireBullet(a_vecSrc, a_vecEnd, g_Weapon->m_flDistance, g_Weapon->m_iPenetration, g_Weapon->m_iBulletType, g_Weapon.GetDamage(), g_Weapon.GetWallPierce());

	Physent::SetMaxs(a_iPlayer, g_Player[a_iPlayer]->m_vecBoundBoxMaxs);
	Physent::SetMins(a_iPlayer, g_Player[a_iPlayer]->m_vecBoundBoxMins);

	flDamage = TraceAttack(flDamage, a_nHitGroup);

	flDamage = ApplyArmor(flDamage, a_nHitGroup, a_iPlayer);

	return (int)flDamage;
}