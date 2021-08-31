#include "main.h"

CMisc g_Misc;

void CMisc::ThirdPerson(struct ref_params_s *pparams)
{
	if (cvar.thirdperson && !g_Local.m_bIsDead && cvar.visuals)
		pparams->vieworg += pparams->forward * cvar.thirdperson * -1;
}

bool CMisc::Edge(float &flAngle)
{
	Vector vecSrc = g_Local.m_vecEye;
	float flClosestDistance = FLT_MAX;
	float flRadius = cvar.antiaim_edge_distance + 0.1f;
	float flStep = M_PI * 2.0 / 8;

	pmtrace_t Trace;

	for (float a = 0; a < (M_PI * 2.0); a += flStep)
	{
		Vector vecLocation(flRadius * cos(a) + vecSrc.x, flRadius * sin(a) + vecSrc.y, vecSrc.z);

		g_Engine.pEventAPI->EV_SetTraceHull(2);
		g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecLocation, PM_GLASS_IGNORE, -1, &Trace);

		float flDistance = vecSrc.Distance(Trace.endpos);

		if (flDistance < flClosestDistance)
		{
			flClosestDistance = flDistance;
			flAngle = RAD2DEG(a);
		}
	}

	return flClosestDistance < cvar.antiaim_edge_distance;
}

void CMisc::AntiAim(struct usercmd_s *cmd)
{
	if (!cvar.ragebot_active)
		return;

	if (!cvar.antiaim_enabled)
		return;

	if (g_Weapon.IsCurrentWeaponNonAttack())
		return;

	if (!cvar.antiaim_on_knife && g_Weapon.IsCurrentWeaponKnife())
		return;

	if (pmove->movetype == MOVETYPE_FLY && g_Local.m_flVelocity > 0)
		return;

	if (cmd->buttons & IN_USE)
		return;

	if (cmd->buttons & IN_ATTACK && g_Weapon.CanAttack())
		return;

	int iTarget = -1;
	float flBestFOV = FLT_MAX;

	for (auto i = 1; i <= g_Engine.GetMaxClients(); ++i)
	{
		if (i == g_Local.m_iIndex)
			continue;

		auto *pPlayer = &g_Player[i - 1];

		if (!pPlayer)
			continue;

		if (!cvar.ragebot_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS)
			continue;

		Vector vecFOV = pPlayer->m_vecOrigin - g_Local.m_vecEye;

		float flFOV = g_Local.m_vecForward.AngleBetween(vecFOV);

		if (flFOV < flBestFOV || iTarget == -1)
		{
			flBestFOV = flFOV;
			iTarget = i;
		}
	}

	QAngle QAngles(cmd->viewangles);

	if (iTarget > 0 && iTarget <= g_Engine.GetMaxClients())
	{
		auto *pPlayer = &g_Player[iTarget - 1];

		if (pPlayer)
			g_Utils.VectorAngles(pPlayer->m_vecOrigin - g_Local.m_vecEye, QAngles);
	}

	g_Utils.FixMoveStart(cmd);

	cmd->viewangles = QAngles;

	if (!m_bFakeLagActive)
	{
		static int iChoke = cvar.antiaim_choke_limit;

		if (iChoke > 0)
		{
			g_Globals.send_command = false;
			iChoke--;
		}
		else
			iChoke = cvar.antiaim_choke_limit;
	}

	static bool bSide = false;

	if (g_Globals.send_command)
		bSide = !bSide;

	if (!g_Globals.send_command)//Yaw
	{
		if (g_Local.m_flVelocity > 0)
		{
			if (cvar.antiaim_yaw_while_running == 1)
				cmd->viewangles.y += 180; 
			else if (cvar.antiaim_yaw_while_running == 2)
			{
				if (bSide)
					cmd->viewangles.y += 180;
			}
			else if (cvar.antiaim_yaw_while_running == 3)
			{
				if (bSide)
					cmd->viewangles.y += 140;
				else
					cmd->viewangles.y -= 140;
			}
			else if (cvar.antiaim_yaw_while_running == 4)
			{
				cmd->viewangles.y += fmod(g_Engine.GetClientTime() * cvar.antiaim_yaw_spin * 360, 360);
			}
			else if (cvar.antiaim_yaw_while_running == 5)
			{
				if (bSide)
					cmd->viewangles.y += 90;
				else
					cmd->viewangles.y -= 90;
			}
			else  if (cvar.antiaim_yaw_while_running == 6)
				cmd->viewangles.y += g_Engine.pfnRandomFloat(-180, 180);
			else  if (cvar.antiaim_yaw_while_running == 7)
				cmd->viewangles.y += cvar.antiaim_yaw_static;
		}
		else {
			if (cvar.antiaim_yaw == 1)
				cmd->viewangles.y += 180;
			else if (cvar.antiaim_yaw == 2)
			{
				float flAngle = fmod(g_Engine.GetClientTime() * 90, 90);

				if (bSide)
					cmd->viewangles.y += 180 - 45 + flAngle;
				else
					cmd->viewangles.y += 180 + 45 - flAngle;
			}
			else if (cvar.antiaim_yaw == 3)
			{
				if (bSide)
					cmd->viewangles.y += 180;
			}
			else if (cvar.antiaim_yaw == 4)
			{
				if (bSide)
					cmd->viewangles.y += 140;
				else
					cmd->viewangles.y -= 140;
			}
			else if (cvar.antiaim_yaw == 5)
			{
				cmd->viewangles.y += fmod(g_Engine.GetClientTime() * cvar.antiaim_yaw_spin * 360, 360);
			}
			else if (cvar.antiaim_yaw == 6)
			{
				if (bSide)
					cmd->viewangles.y += 90;
				else
					cmd->viewangles.y -= 90;
			}
			else  if (cvar.antiaim_yaw == 7)
				cmd->viewangles.y += g_Engine.pfnRandomFloat(-180, 180);
			else  if (cvar.antiaim_yaw == 8)
				cmd->viewangles.y += cvar.antiaim_yaw_static;
		}
		//Edge
		if (cvar.antiaim_edge)
		{
			if ((cvar.antiaim_edge_triggers == 0 && g_Local.m_flVelocity <= 0) || (cvar.antiaim_edge_triggers == 1 && g_Local.m_flHeight < 1) || cvar.antiaim_edge_triggers == 2)
			{
				float angle = cmd->viewangles.y;

				if (Edge(angle))
					cmd->viewangles.y = angle + cvar.antiaim_edge_offset;
			}
		}

		m_vRealAngles.y = cmd->viewangles.y;
	}
	else { //Fake yaw if choking
		if (cvar.antiaim_fake_yaw == 1)
			cmd->viewangles.y += 180;
		else if (cvar.antiaim_fake_yaw == 2)
		{
			if (!bSide)
				cmd->viewangles.y += 90;
			else
				cmd->viewangles.y -= 90;
		}
		else if (cvar.antiaim_fake_yaw == 3)
			cmd->viewangles.y = g_Local.m_vecAngles[1];
		else  if (cvar.antiaim_fake_yaw == 4)
			cmd->viewangles.y += g_Engine.pfnRandomFloat(-180, 180);
		else  if (cvar.antiaim_fake_yaw == 5)
			cmd->viewangles.y += cvar.antiaim_fake_yaw_static;
	}

	if (cvar.antiaim_pitch == 1)
		cmd->viewangles.x = 180;
	else if (cvar.antiaim_pitch == 2)
		cmd->viewangles.x = 89;
	else if (cvar.antiaim_pitch == 3)
		cmd->viewangles.x = -89;
	else if (cvar.antiaim_pitch == 4)
	{
		if (bSide)
			cmd->viewangles.x = -89;
		else
			cmd->viewangles.x = 89;
	}
	else if (cvar.antiaim_pitch == 5)
		cmd->viewangles.x = 179.99999f;
	else if (cvar.antiaim_pitch == 6)
		cmd->viewangles.x = g_Engine.pfnRandomFloat(-180, 180);

	cmd->viewangles.z = cvar.antiaim_roll;

	m_vRealAngles.x = cmd->viewangles.x;
	m_vRealAngles.z = cmd->viewangles.z;

	g_Utils.FixMoveEnd(cmd);
}

void CMisc::FakeLag(struct usercmd_s *cmd)
{
	m_bFakeLagActive = false;

	if (!cvar.ragebot_active)
		return;

	if (!cvar.fakelag_enabled)
		return;

	if (cmd->buttons & IN_ATTACK && g_Weapon.CanAttack() && !cvar.fakelag_while_shooting)
		return;

	if (cvar.fakelag_triggers == 0 && g_Local.m_flHeight > 1)
		return;
	else if (cvar.fakelag_triggers == 1 && g_Local.m_flHeight < 1)
		return;

	m_bFakeLagActive = true;

	static int iChoked = 0;

	if (cvar.fakelag_type == 0 && cvar.fakelag_variance > 0)//Dynamic
	{
		static int iNotChoked = 0;

		if (iChoked < cvar.fakelag_choke_limit)
		{
			g_Globals.send_command = false;
			iChoked++;
			iNotChoked = 0;
		}
		else {
			float tmp = (cvar.fakelag_choke_limit / 100) * cvar.fakelag_variance;

			iNotChoked++;

			if (iNotChoked > tmp)
				iChoked = 0;
		}
	}
	else if (cvar.fakelag_type == 1)//Maximum
	{
		iChoked++;

		if (iChoked > 0)
			g_Globals.send_command = false;

		if (iChoked > cvar.fakelag_choke_limit)
			iChoked = -1;//1 tick valid
	}
	else if (cvar.fakelag_type == 2)//Flucture
	{
		static bool bJitter = false;

		if (bJitter)
			g_Globals.send_command = false;

		bJitter = !bJitter;
	}
	else if (cvar.fakelag_type == 3)//Break lag compensation
	{
		Vector vecVelocity(pmove->velocity[0], pmove->velocity[1], 0);

		float len = vecVelocity.Length() * g_Local.m_flFrametime;

		if (len < 64.0f && vecVelocity.Length() > 0.05f && len)
		{
			int iNeedChoke = 64.0f / len;

			if (iNeedChoke > cvar.fakelag_choke_limit)
				iNeedChoke = cvar.fakelag_choke_limit;

			if (iChoked < iNeedChoke)
			{
				g_Globals.send_command = false;
				iChoked++;
			}
			else 
				iChoked = 0;
		}
		else 
			m_bFakeLagActive = false;
	}
}


void CMisc::AddFakeLatency(float flLatency)
{
	m_bFakeLatencyActive = false;

	auto *pCvar = g_Engine.pfnGetCvarPointer("fakelag");

	if (!pCvar)
		return;

	if (flLatency > 0)
	{
		*g_Globals.allow_cheats = true;

		if(client_static->netchan.remote_address.type == NA_LOOPBACK)
			flLatency /= 2;

		pCvar->value = flLatency;

		m_bFakeLatencyActive = true;

		
		//Other old backtrack
		/*int incoming_sequence = client_static->netchan.incoming_sequence;

		if (incoming_sequence - backtrack > 0 && g_Status.connection_time > 1) //Fix bugs
		{
			m_bBackTracking = true;
			client_static->netchan.incoming_sequence = incoming_sequence + backtrack;//backtracking
		}*/
	}
	else
		pCvar->value = 0;
}

void CMisc::AutoReload(struct usercmd_s *cmd)
{
	if (cvar.automatic_reload && cmd->buttons & IN_ATTACK && g_Weapon.m_iClip < 1 && g_Weapon.IsCurrentWeaponGun())
	{
		cmd->buttons &= ~IN_ATTACK;
		cmd->buttons |= IN_RELOAD;
	}
}

void CMisc::AutoPistol(struct usercmd_s *cmd)
{
	if (cvar.automatic_pistol && cmd->buttons & IN_ATTACK && g_Weapon.IsCurrentWeaponPistol() && !g_Weapon.m_iInReload)
	{
		static bool bFire = false;

		if (g_Weapon.CanAttack() && bFire)
		{
			cmd->buttons |= IN_ATTACK;
			bFire = false;
		}
		else if (!bFire)
		{
			cmd->buttons &= ~IN_ATTACK;
			bFire = true;
		}
	}
}

void CrossProduct(const Vector v1, const Vector v2, Vector cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

int PM_ClipVelocity(Vector in, Vector normal, Vector out, float overbounce)
{
	float change;
	double angle;
	double backoff;
	int i, blocked;

	angle = normal[2];

	// Assume unblocked.
	blocked = 0x00;

	// If the plane that is blocking us has a positive z component, then assume it's a floor.
	if (angle > 0)
	{
		blocked |= 0x01;
	}

	// If the plane has no Z, it is vertical (wall/step)
	if (!angle)
	{
		blocked |= 0x02;
	}

	// Determine how far along plane to slide based on incoming direction.
	// Scale by overbounce factor.
	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = in[i] - normal[i] * backoff;
		out[i] = change;

		// If out velocity is too small, zero it out.
		if (out[i] > -0.1 && out[i] <  0.1)
		{
			out[i] = 0;
		}
	}

	// Return blocking flags.
	return blocked;
}

void PM_CheckParameters(playermove_t *playermove)
{
	float spd;
	double maxspeed;
	Vector v_angle;

	spd = sqrt(double(playermove->cmd.sidemove * playermove->cmd.sidemove + playermove->cmd.forwardmove * playermove->cmd.forwardmove + playermove->cmd.upmove * playermove->cmd.upmove));

	maxspeed = playermove->clientmaxspeed;

	if (maxspeed != 0.0f)
		playermove->maxspeed = min(maxspeed, double(playermove->maxspeed));

	if (spd != 0.0f && spd > double(playermove->maxspeed))
	{
		double fRatio = playermove->maxspeed / spd;

		playermove->cmd.forwardmove *= fRatio;
		playermove->cmd.sidemove *= fRatio;
		playermove->cmd.upmove *= fRatio;
	}

	if ((playermove->flags & (FL_FROZEN | FL_ONTRAIN)) || playermove->dead)
	{
		playermove->cmd.forwardmove = 0;
		playermove->cmd.sidemove = 0;
		playermove->cmd.upmove = 0;
	}

	playermove->punchangle.Clear();

	playermove->angles = playermove->cmd.viewangles;

	// Adjust client view angles to match values used on server.
	if (playermove->angles[1] > 180.0f)
		playermove->angles[1] -= 360.0f;
}


void PM_CategorizePosition(playermove_t *playermove)
{
	Vector point;
	pmtrace_t tr;

	// if the player hull point one unit down is solid, the player
	// is on ground

	// see if standing on something solid

	// Doing this before we move may introduce a potential latency in water detection, but
	// doing it after can get us stuck on the bottom in water if the amount we move up
	// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
	// this several times per frame, so we really need to avoid sticking to the bottom of
	// water on each call, and the converse case will correct itself if called twice.
	//PM_CheckWater();

	point[0] = playermove->origin[0];
	point[1] = playermove->origin[1];
	point[2] = playermove->origin[2] - 2;

	// Shooting up really fast.  Definitely not on ground.
	if (playermove->velocity[2] > 180)
	{
		playermove->onground = -1;
		return;
	}

	// Try and move down.
	g_Engine.pEventAPI->EV_PlayerTrace(playermove->origin, point, PM_NORMAL, -1, &tr);

	// If we hit a steep plane, we are not on ground
	if (tr.plane.normal[2] < 0.7f)
	{
		// too steep
		playermove->onground = -1;
	}
	else
	{
		// Otherwise, point to index of ent under us.
		playermove->onground = tr.ent;
	}

	// If we are on something...
	if (playermove->onground != -1)
	{
		// Then we are not in water jump sequence
		playermove->waterjumptime = 0;

		// If we could make the move, drop us down that 1 pixel
		if (playermove->waterlevel < 2 && !tr.startsolid && !tr.allsolid)
			VectorCopy(tr.endpos, playermove->origin);
	}
}

void PM_UnDuck(playermove_t *playermove)
{
	pmtrace_t trace;
	Vector newOrigin;

	VectorCopy(playermove->origin, newOrigin);

	if (playermove->onground != -1)
	{
		Vector offset;
		offset = playermove->player_mins[1] - playermove->player_mins[0];
		VectorAdd(newOrigin, offset, newOrigin);
	}

	g_Engine.pEventAPI->EV_PlayerTrace(newOrigin, newOrigin, PM_NORMAL, -1, &trace);
	if (!trace.startsolid)
	{
		playermove->usehull = 0;

		// Oh, no, changing hulls stuck us into something, try unsticking downward first.
		g_Engine.pEventAPI->EV_PlayerTrace(newOrigin, newOrigin, PM_NORMAL, -1, &trace);

		if (trace.startsolid)
		{
			// See if we are stuck?  If so, stay ducked with the duck hull until we have a clear spot
			// Con_Printf("unstick got stuck\n");
			playermove->usehull = 1;
			return;
		}

		playermove->flags &= ~FL_DUCKING;
		playermove->bInDuck = FALSE;
		playermove->view_ofs[2] = PM_VEC_VIEW;
		playermove->flDuckTime = 0;

		playermove->flTimeStepSound -= 100;

		if (playermove->flTimeStepSound < 0)
		{
			playermove->flTimeStepSound = 0;
		}

		VectorCopy(newOrigin, playermove->origin);

		// Recatagorize position since ducking can change origin
		PM_CategorizePosition(playermove);
	}
}

// Use for ease-in, ease-out style interpolation (accel/decel)
// Used by ducking code.
float PM_SplineFraction(float value, float scale)
{
	double valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void PM_Duck(playermove_t *playermove)
{
	int buttonsChanged = (playermove->oldbuttons ^ playermove->cmd.buttons);	// These buttons have changed this frame
	int nButtonPressed = buttonsChanged & playermove->cmd.buttons;		// The changed ones still down are "pressed"

	int duckchange = buttonsChanged & IN_DUCK ? 1 : 0;
	int duckpressed = nButtonPressed & IN_DUCK ? 1 : 0;

	if (playermove->cmd.buttons & IN_DUCK)
		playermove->oldbuttons |= IN_DUCK;
	else
		playermove->oldbuttons &= ~IN_DUCK;

	if (playermove->dead || !(playermove->cmd.buttons & IN_DUCK) && !playermove->bInDuck && !(playermove->flags & FL_DUCKING))
		return;

	playermove->cmd.forwardmove *= PLAYER_DUCKING_MULTIPLIER;
	playermove->cmd.sidemove *= PLAYER_DUCKING_MULTIPLIER;
	playermove->cmd.upmove *= PLAYER_DUCKING_MULTIPLIER;

	if (playermove->cmd.buttons & IN_DUCK)
	{
		if ((nButtonPressed & IN_DUCK) && !(playermove->flags & FL_DUCKING))
		{
			// Use 1 second so super long jump will work
			playermove->flDuckTime = 1000;
			playermove->bInDuck = TRUE;
		}

		if (playermove->bInDuck)
		{
			// Finish ducking immediately if duck time is over or not on ground
			if (((playermove->flDuckTime / 1000.0) <= (1.0 - TIME_TO_DUCK)) || playermove->onground == -1)
			{
				playermove->usehull = 1;
				playermove->view_ofs[2] = PM_VEC_DUCK_VIEW;
				playermove->flags |= FL_DUCKING;
				playermove->bInDuck = FALSE;

				// HACKHACK - Fudge for collision bug - no time to fix this properly
				if (playermove->onground != -1)
				{
					Vector newOrigin;
					newOrigin = playermove->player_mins[1] - playermove->player_mins[0];
					playermove->origin = playermove->origin - newOrigin;

					// See if we are stuck?
					//PM_FixPlayerCrouchStuck(STUCK_MOVEUP);

					// Recatagorize position since ducking can change origin
					PM_CategorizePosition(playermove);
				}
			}
			else
			{
				double duckFraction = PM_VEC_VIEW;
				double time = (1.0 - playermove->flDuckTime / 1000.0);

				// Calc parametric time
				if (time >= 0.0) {
					duckFraction = PM_SplineFraction(time, (1.0 / TIME_TO_DUCK));
				}

				float fMore = (playermove->player_mins[1][2] - playermove->player_mins[0][2]);

				playermove->view_ofs[2] = ((PM_VEC_DUCK_VIEW - fMore) * duckFraction) + (PM_VEC_VIEW * (1 - duckFraction));
			}
		}
	}
	// Try to unduck
	else
	{
		PM_UnDuck(playermove);
	}
}

void PM_CheckVelocity(playermove_t *playermove)
{
	// bound velocity
	for (int i = 0; i < 3; i++)
	{
		// See if it's bogus.
		if (isnan(playermove->velocity[i]))
		{
			//g_pConsole->DPrintf("[hpp] PM %i  Got a NaN velocity %i\n", playermove->player_index, i);
			playermove->velocity[i] = 0;
		}

		if (isnan(playermove->origin[i]))
		{
			//g_pConsole->DPrintf("[hpp] PM %i  Got a NaN velocity %i\n", playermove->player_index, i);
			playermove->origin[i] = 0;
		}

		// Bound it.
		if (playermove->velocity[i] > playermove->movevars->maxvelocity)
		{
			//g_pConsole->DPrintf("[hpp] PM %i  Got a velocity too high on %i\n", playermove->player_index, i);
			playermove->velocity[i] = playermove->movevars->maxvelocity;
		}
		else if (playermove->velocity[i] < -playermove->movevars->maxvelocity)
		{
			//g_pConsole->DPrintf("[hpp] PM %i  Got a velocity too low on %i\n", playermove->player_index, i);
			playermove->velocity[i] = -playermove->movevars->maxvelocity;
		}
	}
}

void PM_FixupGravityVelocity(playermove_t *playermove)
{
	double ent_gravity;

	if (playermove->waterjumptime)
		return;

	if (playermove->gravity != 0.0)
		ent_gravity = playermove->gravity;
	else
		ent_gravity = 1.0;

	// Get the correct velocity for the end of the dt
	playermove->velocity[2] -= (playermove->movevars->gravity * playermove->frametime * ent_gravity * 0.5);
	PM_CheckVelocity(playermove);
}

void PM_NoClip(playermove_t *playermove)
{
	Vector wishvel;
	float fmove, smove;

	// Copy movement amounts
	fmove = playermove->cmd.forwardmove;
	smove = playermove->cmd.sidemove;

	playermove->forward.Normalize();
	playermove->right.Normalize();

	// Determine x and y parts of velocity
	for (int i = 0; i < 3; i++)
		wishvel[i] = playermove->forward[i] * fmove + playermove->right[i] * smove;

	wishvel[2] += playermove->cmd.upmove;

	playermove->origin = playermove->origin + playermove->frametime * wishvel;

	// Zero out the velocity so that we don't accumulate a huge downward velocity from
	// gravity, etc.
	playermove->velocity.Clear();
}

// Purpose: Corrects bunny jumping (where player initiates a bunny jump before other
// movement logic runs, thus making onground == -1 thus making PM_Friction get skipped and
// running PM_AirMove, which doesn't crop velocity to maxspeed like the ground / other
// movement logic does.
void PM_PreventMegaBunnyJumping(playermove_t *playermove)
{
	// Current player speed
	double spd;
	// If we have to crop, apply this cropping fraction to velocity
	float fraction;
	// Speed at which bunny jumping is limited
	float maxscaledspeed;

	maxscaledspeed = BUNNYJUMP_MAX_SPEED_FACTOR * playermove->maxspeed;

	// Don't divide by zero
	if (maxscaledspeed <= 0.0f)
		return;

	spd = playermove->velocity.Length();

	if (spd <= maxscaledspeed)
		return;

	// Returns the modifier for the velocity
	fraction = (maxscaledspeed / spd) * 0.8;

	// Crop it down!.
	playermove->velocity = playermove->velocity * fraction;
}

void PM_Jump(playermove_t *playermove)
{
	if (playermove->dead)
	{
		// don't jump again until released
		playermove->oldbuttons |= IN_JUMP;
		return;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	if (playermove->waterjumptime != 0.0f)
	{
		playermove->waterjumptime -= playermove->cmd.msec;

		if (playermove->waterjumptime < 0)
		{
			playermove->waterjumptime = 0;
		}

		return;
	}

	// If we are in the water most of the way...
	if (playermove->waterlevel >= 2)
	{
		// swimming, not jumping
		playermove->onground = -1;

		// We move up a certain amount
		if (playermove->watertype == CONTENTS_WATER)
		{
			playermove->velocity[2] = 100;
		}
		else if (playermove->watertype == CONTENTS_SLIME)
		{
			playermove->velocity[2] = 80;
		}
		else // LAVA
			playermove->velocity[2] = 50;
		return;
	}

	// No more effect
	// in air, so no effect
	if (playermove->onground == -1)
	{
		// Flag that we jumped.
		// don't jump again until released
		playermove->oldbuttons |= IN_JUMP;
		return;
	}

	// don't pogo stick
	if (playermove->oldbuttons & IN_JUMP)
		return;

	if (playermove->bInDuck && (playermove->flags & FL_DUCKING))
		return;

	// In the air now.
	playermove->onground = -1;

	PM_PreventMegaBunnyJumping(playermove);

	{
		// NOTE: don't do it in .f (float)
		playermove->velocity[2] = sqrt(2.0 * 800.0f * 45.0f);
	}

	if (playermove->fuser2 > 0.0f)
	{
		// NOTE: don't do it in .f (float)
		double flRatio = (100.0 - playermove->fuser2 * 0.001 * 19.0) * 0.01;
		playermove->velocity[2] *= flRatio;
	}

	playermove->fuser2 = 1315.789429;

	// Decay it for simulation
	PM_FixupGravityVelocity(playermove);

	// Flag that we jumped.
	// don't jump again until released
	playermove->oldbuttons |= IN_JUMP;
}

void PM_CheckWaterJump(playermove_t *playermove)
{

}

void PM_AirAccelerate(playermove_t *playermove, Vector wishdir, float wishspeed, float accel)
{
	float addspeed;
	float wishspd = wishspeed;

	double currentspeed;
	double accelspeed;

	if (playermove->dead || playermove->waterjumptime)
		return;

	// Cap speed
	if (wishspd > 30)
		wishspd = 30;

	// Determine veer amount
	currentspeed = playermove->velocity.Dot(wishdir);

	// See how much to add
	addspeed = wishspd - currentspeed;

	// If not adding any, done.
	if (addspeed <= 0)
		return;

	// Determine acceleration speed after acceleration
	accelspeed = accel * wishspeed * playermove->frametime * playermove->friction;

	// Cap it
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust playermove vel.
	for (int i = 0; i < 3; i++)
		playermove->velocity[i] += accelspeed * wishdir[i];
}

int PM_FlyMove(playermove_t *playermove)
{
	int bumpcount, numbumps;
	Vector dir;
	float d;
	int numplanes;
	Vector planes[MAX_CLIP_PLANES];
	Vector primal_velocity, original_velocity;
	Vector new_velocity;
	int i, j;
	pmtrace_t trace;
	Vector end;
	float time_left, allFraction;
	int blocked;

	numbumps = 4;	// Bump up to four times
	blocked = 0x00;	// Assume not blocked
	numplanes = 0;	// and not sliding along any planes

	VectorCopy(playermove->velocity, original_velocity);		// Store original velocity
	VectorCopy(playermove->velocity, primal_velocity);

	allFraction = 0;
	time_left = playermove->frametime;				// Total time for this movement operation.

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		if (!playermove->velocity[0] && !playermove->velocity[1] && !playermove->velocity[2])
			break;

		// Assume we can move all the way from the current origin to the
		// end point.
		for (i = 0; i < 3; i++)
		{
			double flScale = time_left * playermove->velocity[i];

			end[i] = playermove->origin[i] + flScale;
		}

		// See if we can make it from origin to end point.
		g_Engine.pEventAPI->EV_PlayerTrace(playermove->origin, end, PM_NORMAL, -1, &trace);

		allFraction += trace.fraction;

		// If we started in a solid object, or we were in solid space
		// the whole way, zero out our velocity and return that we
		// are blocked by floor and wall.
		if (trace.allsolid)
		{
			// entity is trapped in another solid
			playermove->velocity.Clear();
			return 4;
		}

		// If we moved some portion of the total distance, then
		// copy the end position into the playermove->origin and
		// zero the plane counter.
		if (trace.fraction > 0.0f)
		{
			// actually covered some distance
			VectorCopy(trace.endpos, playermove->origin);
			VectorCopy(playermove->velocity, original_velocity);

			numplanes = 0;
		}

		// If we covered the entire distance, we are done
		// and can return.
		if (trace.fraction == 1.0f)
		{
			// moved the entire distance
			break;
		}

		// Save entity that blocked us (since fraction was < 1.0)
		// for contact
		// Add it if it's not already in the list
		//PM_AddToTouched(trace, playermove->velocity);

		// If the plane we hit has a high z component in the normal, then
		// it's probably a floor
		if (trace.plane.normal[2] > 0.7f)
		{
			// floor
			blocked |= 0x01;
		}

		// If the plane has a zero z component in the normal, then it's a
		// step or wall
		if (!trace.plane.normal[2])
		{
			// step / wall
			blocked |= 0x02;
		}

		// Reduce amount of playermove->frametime left by total time left * fraction
		// that we covered.
		time_left -= time_left * trace.fraction;

		// Did we run out of planes to clip against?
		if (numplanes >= MAX_CLIP_PLANES)
		{
			// this shouldn't really happen
			// Stop our movement if so.
			playermove->velocity.Clear();
			break;
		}

		// Set up next clipping plane
		VectorCopy(trace.plane.normal, planes[numplanes]);
		numplanes++;

		// modify original_velocity so it parallels all of the clip planes
		// relfect player velocity
		if (numplanes == 1 && playermove->movetype == MOVETYPE_WALK && (playermove->onground == -1 || playermove->friction != 1))
		{
			for (i = 0; i < numplanes; i++)
			{
				if (planes[i][2] > 0.7f)
				{
					// floor or slope
					PM_ClipVelocity(original_velocity, planes[i], new_velocity, 1);
					VectorCopy(new_velocity, original_velocity);
				}
				else
					PM_ClipVelocity(original_velocity, planes[i], new_velocity, 1.0 + playermove->movevars->bounce * (1.0 - playermove->friction));
			}

			VectorCopy(new_velocity, playermove->velocity);
			VectorCopy(new_velocity, original_velocity);
		}
		else
		{
			for (i = 0; i < numplanes; i++)
			{
				PM_ClipVelocity(original_velocity, planes[i], playermove->velocity, 1);

				for (j = 0; j < numplanes; j++)
				{
					if (j != i && playermove->velocity.Dot(planes[j]) < 0)
					{
						break;
					}
				}

				if (j == numplanes)
					break;
			}

			if (i == numplanes)
			{
				if (numplanes != 2)
				{
					playermove->velocity.Clear();
					break;
				}

				CrossProduct(planes[0], planes[1], dir);
				d = dir.Dot(playermove->velocity);
				playermove->velocity = dir * d;
			}

			if (playermove->velocity.Dot(primal_velocity) <= 0)
			{
				playermove->velocity.Clear();
				break;
			}
		}
	}

	if (allFraction == 0.0f)
		playermove->velocity.Clear();

	return blocked;
}

void PM_AirMove(playermove_t *playermove)
{
	int i;
	Vector wishvel;
	float fmove, smove;
	Vector wishdir;
	float wishspeed;

	// Copy movement amounts
	fmove = playermove->cmd.forwardmove;
	smove = playermove->cmd.sidemove;

	// Zero out z components of movement vectors
	playermove->forward[2] = 0;
	playermove->right[2] = 0;

	// Renormalize
	playermove->forward.Normalize();
	playermove->right.Normalize();

	// Determine x and y parts of velocity
	for (i = 0; i < 2; i++)
	{
		wishvel[i] = playermove->forward[i] * fmove + playermove->right[i] * smove;
	}

	// Zero out z part of velocity
	wishvel[2] = 0;

	// Determine maginitude of speed of move
	VectorCopy(wishvel, wishdir);
	wishspeed = wishdir.Normalize();

	// Clamp to server defined max speed
	if (wishspeed > playermove->maxspeed)
	{
		wishvel = wishvel * playermove->maxspeed / wishspeed;
		wishspeed = playermove->maxspeed;
	}

	PM_AirAccelerate(playermove, wishdir, wishspeed, playermove->movevars->airaccelerate);

	// Add in any base velocity to the current velocity.
	VectorAdd(playermove->velocity, playermove->basevelocity, playermove->velocity);

	PM_FlyMove(playermove);
}

void PM_CheckFalling(playermove_t *playermove)
{
	if (playermove->onground != -1)
		playermove->flFallVelocity = 0;
}

void PM_AddCorrectGravity(playermove_t *playermove)
{
	double ent_gravity;

	if (playermove->waterjumptime)
		return;

	if (playermove->gravity != 0.0f)
		ent_gravity = playermove->gravity;
	else
		ent_gravity = 1.0f;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.
	playermove->velocity[2] -= (ent_gravity * playermove->movevars->gravity * 0.5f * playermove->frametime);
	playermove->velocity[2] += playermove->basevelocity[2] * playermove->frametime;

	playermove->basevelocity[2] = 0;

	PM_CheckVelocity(playermove);
}

void PM_Friction(playermove_t *playermove)
{
	float *vel;
	float speed;
	double newspeed, control, friction, drop;
	Vector newvel;

	// If we are in water jump cycle, don't apply friction
	if (playermove->waterjumptime)
		return;

	// Get velocity
	vel = playermove->velocity;

	// Calculate speed
	speed = sqrt(double(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]));

	// If too slow, return
	if (speed < 0.1f)
		return;

	drop = 0;

	// apply ground friction
	// On an entity that is the ground
	if (playermove->onground != -1)
	{
		Vector start, stop;
		pmtrace_t trace;

		start[0] = stop[0] = playermove->origin[0] + vel[0] / speed * 16;
		start[1] = stop[1] = playermove->origin[1] + vel[1] / speed * 16;
		start[2] = playermove->origin[2] + playermove->player_mins[playermove->usehull][2];
		stop[2] = start[2] - 34;

		g_Engine.pEventAPI->EV_PlayerTrace(start, stop, PM_NORMAL, -1, &trace);

		if (trace.fraction == 1.0f)
			friction = playermove->movevars->friction * playermove->movevars->edgefriction;
		else
			friction = playermove->movevars->friction;

		// Grab friction value.
		//friction = pmove->movevars->friction;

		// player friction?
		friction *= playermove->friction;

		// Bleed off some speed, but if we have less than the bleed
		// threshhold, bleed the theshold amount.
		control = (speed < playermove->movevars->stopspeed) ? playermove->movevars->stopspeed : speed;

		// Add the amount to t'he drop amount.
		drop += friction * (control * playermove->frametime);
	}

	// apply water friction
	//if (pmove->waterlevel)
	//{
	//	drop += speed * pmove->movevars->waterfriction * waterlevel * pmove->frametime;
	//}

	// scale the velocity
	newspeed = speed - drop;

	if (newspeed < 0)
		newspeed = 0;
	// Determine proportion of old speed we are using.
	newspeed /= speed;

	// Adjust velocity according to proportion.
	newvel[0] = vel[0] * newspeed;
	newvel[1] = vel[1] * float(newspeed);
	newvel[2] = vel[2] * float(newspeed);

	VectorCopy(newvel, playermove->velocity);
}

void PM_Accelerate(playermove_t *playermove, Vector wishdir, double wishspeed, float accel)
{
	// Dead player's don't accelerate
	if (playermove->dead)
		return;

	// If waterjumping, don't accelerate
	if (playermove->waterjumptime)
		return;

	// See if we are changing direction a bit
	double currentspeed = playermove->velocity.Dot(wishdir);

	// Reduce wishspeed by the amount of veer.
	float addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if (addspeed <= 0)
		return;

	// Determine amount of accleration.
	double accelspeed = accel * playermove->frametime * wishspeed * playermove->friction;

	// Cap at addspeed
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust velocity.
	for (int i = 0; i < 3; i++)
		playermove->velocity[i] += accelspeed * wishdir[i];
}

// Only used by players. Moves along the ground when player is a MOVETYPE_WALK.
void PM_WalkMove(playermove_t *playermove)
{
	int clip;
	int oldonground;

	Vector wishvel;
	double spd;
	float fmove, smove;
	Vector wishdir;
	double wishspeed;

	//vec3_t start;	// TODO: unused
	Vector dest;
	Vector original, originalvel;
	Vector down, downvel;
	float downdist, updist;

	pmtrace_t trace;

	if (playermove->fuser2 > 0.0)
	{
		double flRatio = (100 - playermove->fuser2 * 0.001 * 19) * 0.01;

		playermove->velocity[0] *= flRatio;
		playermove->velocity[1] *= flRatio;
	}

	// Copy movement amounts
	fmove = playermove->cmd.forwardmove;
	smove = playermove->cmd.sidemove;

	// Zero out z components of movement vectors
	playermove->forward[2] = 0;
	playermove->right[2] = 0;

	// Normalize remainder of vectors.
	playermove->forward.Normalize();
	playermove->right.Normalize();

	// Determine x and y parts of velocity
	for (int i = 0; i < 2; i++)
		wishvel[i] = playermove->forward[i] * fmove + playermove->right[i] * smove;

	// Zero out z part of velocity
	wishvel[2] = 0;

	// Determine maginitude of speed of move
	VectorCopy(wishvel, wishdir);
	wishspeed = wishdir.Normalize();

	// Clamp to server defined max speed
	if (wishspeed > playermove->maxspeed)
	{
		wishvel = wishvel * playermove->maxspeed / wishspeed;
		wishspeed = playermove->maxspeed;
	}

	// Set playermove velocity
	playermove->velocity[2] = 0;
	PM_Accelerate(playermove, wishdir, wishspeed, playermove->movevars->accelerate);
	playermove->velocity[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd(playermove->velocity, playermove->basevelocity, playermove->velocity);

	spd = playermove->velocity.Length();

	if (spd < 1.0)
	{
		playermove->velocity.Clear();
		return;
	}

	// If we are not moving, do nothing
	//if (!playermove->velocity[0] && !playermove->velocity[1] && !playermove->velocity[2])
	//	return;

	oldonground = playermove->onground;

	// first try just moving to the destination
	dest[0] = playermove->origin[0] + playermove->velocity[0] * playermove->frametime;
	dest[1] = playermove->origin[1] + playermove->velocity[1] * playermove->frametime;
	dest[2] = playermove->origin[2];

	// first try moving directly to the next spot
	// VectorCopy(dest, start);

	g_Engine.pEventAPI->EV_PlayerTrace(playermove->origin, dest, PM_NORMAL, -1, &trace);

	// If we made it all the way, then copy trace end
	// as new player position.
	if (trace.fraction == 1.0f)
	{
		VectorCopy(trace.endpos, playermove->origin);
		return;
	}

	// Don't walk up stairs if not on ground.
	if (oldonground == -1 && playermove->waterlevel == 0)
	{
		return;
	}

	// If we are jumping out of water, don't do anything more.
	if (playermove->waterjumptime)
		return;
	
	// Try sliding forward both on ground and up 16 pixels
	// take the move that goes farthest

	// Save out original pos &
	VectorCopy(playermove->origin, original);

	// velocity.
	VectorCopy(playermove->velocity, originalvel);

	// Slide move
	clip = PM_FlyMove(playermove);

	// Copy the results out
	VectorCopy(playermove->origin, down);
	VectorCopy(playermove->velocity, downvel);

	// Reset original values.
	VectorCopy(original, playermove->origin);
	VectorCopy(originalvel, playermove->velocity);

	// Start out up one stair height
	VectorCopy(playermove->origin, dest);

	dest[2] += playermove->movevars->stepsize;

	g_Engine.pEventAPI->EV_PlayerTrace(playermove->origin, dest, PM_NORMAL, -1, &trace);

	// If we started okay and made it part of the way at least,
	// copy the results to the movement start position and then
	// run another move try.
	if (!trace.startsolid && !trace.allsolid)
		VectorCopy(trace.endpos, playermove->origin);

	// slide move the rest of the way.
	clip = PM_FlyMove(playermove);

	// Now try going back down from the end point
	//  press down the stepheight
	VectorCopy(playermove->origin, dest);
	dest[2] -= playermove->movevars->stepsize;

	g_Engine.pEventAPI->EV_PlayerTrace(playermove->origin, dest, PM_NORMAL, -1, &trace);

	// If we are not on the ground any more then
	// use the original movement attempt
	if (trace.plane.normal[2] < 0.7f)
		goto usedown;

	// If the trace ended up in empty space, copy the end
	// over to the origin.
	if (!trace.startsolid && !trace.allsolid)
		VectorCopy(trace.endpos, playermove->origin);

	// Copy this origion to up.
	VectorCopy(playermove->origin, playermove->up);

	// decide which one went farther
	downdist = (down[0] - original[0]) * (down[0] - original[0]) + (down[1] - original[1]) * (down[1] - original[1]);
	updist = (playermove->up[0] - original[0]) * (playermove->up[0] - original[0]) + (playermove->up[1] - original[1]) * (playermove->up[1] - original[1]);

	if (downdist > updist)
	{
	usedown:
		VectorCopy(down, playermove->origin);
		VectorCopy(downvel, playermove->velocity);
	}
	else
	{
		// copy z value from slide move
		playermove->velocity[2] = downvel[2];
	}
}

void PM_PlayerMove(playermove_t *playermove)
{
	// Adjust speeds etc.
	PM_CheckParameters(playermove);

	// Convert view angles to vectors
	g_Engine.pfnAngleVectors(playermove->angles, playermove->forward, playermove->right, playermove->up);
	
	// Now that we are "unstuck", see where we are (waterlevel and type, pmove->onground).
	PM_CategorizePosition(playermove);

	// Store off the starting water level
	playermove->oldwaterlevel = playermove->waterlevel;

	// If we are not on ground, store off how fast we are moving down
	if (playermove->onground == -1)
		playermove->flFallVelocity = -playermove->velocity[2];

	PM_Duck(playermove);

	// Handle movement
	switch (playermove->movetype)
	{
	default:
	//	g_pConsole->DPrintf("[hpp] Bogus pmove player %i movetype %i\n", playermove->player_index, playermove->movetype);
		break;

	case MOVETYPE_NONE:
		break;

	case MOVETYPE_NOCLIP:
		PM_NoClip(playermove);
		break;

	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		//PM_Physics_Toss(playermove);
		break;

	case MOVETYPE_FLY:
		// Was jump button pressed?
		// If so, set velocity to 270 away from ladder.  This is currently wrong.
		// Also, set MOVE_TYPE to walk, too.
		if (playermove->cmd.buttons & IN_JUMP)
			PM_Jump(playermove);
		else
			playermove->oldbuttons &= ~IN_JUMP;

		// Perform the move accounting for any base velocity.
		VectorAdd(playermove->velocity, playermove->basevelocity, playermove->velocity);
		PM_FlyMove(playermove);
		playermove->velocity = playermove->velocity - playermove->basevelocity;
		break;

	case MOVETYPE_WALK:
		PM_AddCorrectGravity(playermove);

		// Not underwater
		if(playermove->waterlevel < 2) 
		{
			// Was jump button pressed?
			if (playermove->cmd.buttons & IN_JUMP)
				PM_Jump(playermove);
			else
				playermove->oldbuttons &= ~IN_JUMP;

			// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor,
			// we don't slow when standing still, relative to the conveyor.
			if (playermove->onground != -1)
			{
				playermove->velocity[2] = 0;
				PM_Friction(playermove);
			}

			// Make sure velocity is valid.
			PM_CheckVelocity(playermove);

			// Are we on ground now
			if (playermove->onground != -1)
				PM_WalkMove(playermove);
			else
				PM_AirMove(playermove);// Take into account movement when in air.

			// Set final flags.
			PM_CategorizePosition(playermove);

			// Now pull the base velocity back out.
			// Base velocity is set if you are on a moving object, like
			// a conveyor (or maybe another monster?)
			playermove->velocity = playermove->velocity - playermove->basevelocity;

			// Make sure velocity is valid.
			PM_CheckVelocity(playermove);

			// Add any remaining gravitational component.
			PM_FixupGravityVelocity(playermove);

			// If we are on ground, no downward velocity.
			if (playermove->onground != -1)
				playermove->velocity[2] = 0;

			// See if we landed on the ground with enough force to play a landing sound.
			PM_CheckFalling(playermove);
			break; 
		}
	}
}

void CMisc::SimulatePlayerMove(playermove_t *playermove)
{
	// motor!
	PM_PlayerMove(playermove);

	if (playermove->onground != -1)
		playermove->flags |= FL_ONGROUND;
	else
		playermove->flags &= ~FL_ONGROUND;
}