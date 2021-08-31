#include "main.h"
#include "patternlist.h"

extern CSkipFrames g_SkipFrames;

enum MouseWheelDirections
{
	kWheelDirectionUnknown,
	kWheelDirectionUp,
	kWheelDirectionDown
};

static void UpdateFramesOnGround(unsigned& nFramesOnGround)
{
	const bool onground = pmove->onground != -1;

	if (onground)
	{
		nFramesOnGround++;
		nFramesOnGround = min(nFramesOnGround, 50);
	}
	else
	{
		nFramesOnGround = 0;
	}
}

static auto GetScrollDirectory(const char* pcszCmd)
{
	if (strcmp(pcszCmd, "jump") == 0)
	{
		if (strstr(keybindings[K_MWHEELUP], "+jump"))
			return kWheelDirectionUp;

		if (strstr(keybindings[K_MWHEELDOWN], "+jump"))
			return kWheelDirectionDown;

		if (!strstr(keybindings[K_MWHEELUP], "+duck"))
		{
			g_Engine.pfnClientCmd("bind mwheelup +jump");
			return kWheelDirectionUp;
		}

		if (strstr(keybindings[K_MWHEELDOWN], "+duck"))
		{
			g_Engine.pfnClientCmd("bind mwheelup +jump");
			return kWheelDirectionUp;
		}

		g_Engine.pfnClientCmd("bind mwheeldown +jump");
		return kWheelDirectionDown;
	}
	else if (strcmp(pcszCmd, "duck") == 0)
	{
		if (strstr(keybindings[K_MWHEELDOWN], "+duck"))
			return kWheelDirectionDown;

		if (strstr(keybindings[K_MWHEELUP], "+duck"))
			return kWheelDirectionUp;

		if (!strstr(keybindings[K_MWHEELDOWN], "+jump"))
		{
			g_Engine.pfnClientCmd("bind mwheeldown +duck");
			return kWheelDirectionDown;
		}

		if (strstr(keybindings[K_MWHEELUP], "+jump"))
		{
			g_Engine.pfnClientCmd("bind mwheeldown +duck");
			return kWheelDirectionDown;
		}

		g_Engine.pfnClientCmd("bind mwheelup +duck");
		return kWheelDirectionUp;
	}

	return kWheelDirectionUnknown;
}

static void ScrollEmulation(const char* pcszCmd)
{
	switch (GetScrollDirectory(pcszCmd))
	{
	case kWheelDirectionUp:
		g_Engine.Key_Event(K_MWHEELUP, 1);
		g_Engine.Key_Event(K_MWHEELUP, 0);
		break;
	case kWheelDirectionDown:
		g_Engine.Key_Event(K_MWHEELDOWN, 1);
		g_Engine.Key_Event(K_MWHEELDOWN, 0);
	}
}

static float GetPredictedFallVelocity()
{
	return pmove->flFallVelocity + pmove->movevars->gravity * g_Globals.m_flFrameTime;
}

class CBunnyHopCounter
{
public:
	CBunnyHopCounter()
	{
		m_nBhopCount = 0;

		RtlSecureZeroMemory(&m_nBhopFOG, sizeof(m_nBhopFOG));
		RtlSecureZeroMemory(&m_nBhopIAR, sizeof(m_nBhopIAR));
	}

	unsigned GetBunnyHopsCount()
	{
		return m_nBhopCount;
	}

	unsigned GetBunnyHopsFOG(unsigned nFrames)
	{
		nFrames = max(nFrames, 1);
		nFrames = min(nFrames, m_MAX_FOG);
		return m_nBhopFOG[nFrames - 1];
	}

	unsigned GetBunnyHopsIAR(unsigned nFrames)
	{
		nFrames = max(nFrames, 1);
		nFrames = min(nFrames, m_MAX_FOG);
		return m_nBhopIAR[nFrames - 1];
	}

	void Update()
	{
		static unsigned nFramesOnGround;
		UpdateFramesOnGround(nFramesOnGround);

		// Check the ground
		if (nFramesOnGround == 0)
			return;

		// Check the button
		const usercmd_s* pCmd = cmd::get();
		if (~pCmd->buttons & IN_JUMP)
			return;

		// Check bunnyhop frames
		if (nFramesOnGround > m_BHOP_FRAMES)
			return;

		// Check bunnyhop max session jumps
		if (++m_nBhopCount > m_BHOP_SESSION)
		{
			m_nBhopCount = 0;
			RtlSecureZeroMemory(&m_nBhopFOG, sizeof(m_nBhopFOG));
		}

		switch (nFramesOnGround)
		{
		case 1:
			m_nBhopIAR[0]++;
			m_nBhopIAR[1] = 0;
			m_nBhopIAR[2] = 0;
			m_nBhopFOG[0]++;
			break;
		case 2:
			m_nBhopIAR[0] = 0;
			m_nBhopIAR[1]++;
			m_nBhopIAR[2] = 0;
			m_nBhopFOG[1]++;
			break;
		case 3:
		case 4:
		case 5:
			m_nBhopIAR[0] = 0;
			m_nBhopIAR[1] = 0;
			m_nBhopIAR[2]++;
			m_nBhopFOG[2]++;
		}
	}

	static const unsigned m_MAX_FOG = 3;
	static const unsigned m_BHOP_FRAMES = 5;
	static const unsigned m_BHOP_SESSION = 100;

private:
	unsigned m_nBhopCount;
	unsigned m_nBhopFOG[m_MAX_FOG];
	unsigned m_nBhopIAR[m_MAX_FOG];
};

class CScrollInfo
{
public:
	CScrollInfo()
	{
		m_nScrollCount = 0;
		m_bJumpPressed = false;
		m_bJumpReleased = false;
	}

	void JumpPressed()
	{
		m_bJumpPressed = true;
	}

	void JumpReleased()
	{
		if (m_bJumpPressed)
			m_bJumpReleased = true;
	}

	unsigned GetScrollCount()
	{
		return m_nScrollCount;
	}

	void Update()
	{
		static float flLastScrollTime;
		const float flClientTime = g_Engine.GetClientTime();

		if (m_bJumpReleased)
		{
			m_nScrollCount++;
			flLastScrollTime = flClientTime + 0.15f;
		}

		if (flLastScrollTime - flClientTime > 1.f)
			flLastScrollTime = 0.f;

		if (flLastScrollTime <= flClientTime + 0.075f)
			m_nScrollCount = 0;

		if (m_nScrollCount > MAX_SCROLL_COUNT)
			m_nScrollCount = 0;

		m_bJumpPressed = false;
		m_bJumpReleased = false;
	}

private:
	unsigned m_nScrollCount;

	bool m_bJumpPressed;
	bool m_bJumpReleased;
};

class CBunnyHop
{
public:
	CBunnyHop()
	{
		m_nFramesOnGround = 0;
		m_nCurrentFOG = 0;
		m_bJumpState = false;

		m_flHeightStart = 0.f;
		m_nExtraJumps = 0;

		RtlSecureZeroMemory(&m_iPatternIndex, sizeof(m_iPatternIndex));
	}

	void JumpPressed()
	{
		m_ScrollInfo.JumpPressed();
	}

	void JumpReleased()
	{
		m_ScrollInfo.JumpReleased();
	}

	float GetMaxScaledSpeed()
	{
		return pmove->maxspeed * BUNNYJUMP_MAX_SPEED_FACTOR;
	}

	bool JumpPermission()
	{
		if (cvar.kreedz.bunnyhop_correct_type == EfficiencyAdjustment)
		{
			if (pmove->fuser2 > 0.f)
			{
				if (m_nFramesOnGround >= m_nCurrentFOG)
					return true;
			}
			else
			{
				// Current player speed
				const float spd = pmove->velocity.Length2D();
				// Speed at which bunny jumping is limited
				const float maxscaledspeed = GetMaxScaledSpeed();

				if (spd < maxscaledspeed)
					return true;
			}
		}
		else
		{
			if (m_nFramesOnGround >= m_nCurrentFOG)
				return true;
		}

		return false;
	}

	bool IsCrouchStuck()
	{
		// Check the ground
		if (m_nFramesOnGround == 0)
			return false;

		// Get height to the top
		Vector vecEnd = pmove->origin;
		vecEnd.z = 4096.f;
		const auto* pTrace = g_Engine.PM_TraceLine(pmove->origin, vecEnd, PM_NORMAL, client_state->usehull, -1);
		return pTrace->endpos.z - pmove->origin.z < 5.f;
	}

	void UpdateInformation()
	{
		// Update bunnyhop information one time when player landing on the ground
		if (m_nFramesOnGround != 1)
			return;

		const auto randomlong = g_Engine.pfnRandomLong(0, 100);

		switch (cvar.kreedz.bunnyhop_correct_type)
		{
		case EfficiencyAdjustment:
			if (pmove->velocity.Length2D() < GetMaxScaledSpeed())
			{
				m_nCurrentFOG = randomlong <= cvar.kreedz.bunnyhop_correct_value ? 1 : 2;
			}
			else if (pmove->fuser2 > 0.f)
			{
				m_nCurrentFOG = randomlong > cvar.kreedz.bunnyhop_correct_value ? 1 : 2;
			}
			else
			{
				m_nCurrentFOG = randomlong > cvar.kreedz.bunnyhop_correct_value ? 1 : CBunnyHopCounter::m_BHOP_FRAMES;
			}
			break;
		case PercentPerfectJumps:
			m_nCurrentFOG = randomlong <= cvar.kreedz.bunnyhop_correct_value ? 1 : 2;
			break;
		case PercentFramesOnGround:
			if (randomlong < cvar.kreedz.bunnyhop_frames_on_ground[0])
			{
				m_nCurrentFOG = 1;
			}
			else if (randomlong < cvar.kreedz.bunnyhop_frames_on_ground[0] +
				cvar.kreedz.bunnyhop_frames_on_ground[1])
			{
				m_nCurrentFOG = 2;
			}
			else if (randomlong < cvar.kreedz.bunnyhop_frames_on_ground[0] +
				cvar.kreedz.bunnyhop_frames_on_ground[1] + cvar.kreedz.bunnyhop_frames_on_ground[2])
			{
				m_nCurrentFOG = 3;
			}
			else
			{
				m_nCurrentFOG = m_DEFAULT_FOG;
			}
		}

		if (cvar.kreedz.bunnyhop_ground_equal)
		{
			for (unsigned i = 0; i < m_Counter.m_MAX_FOG; i++)
			{
				const unsigned nFrames = i + 1;
				const unsigned nBhopIAR = m_Counter.GetBunnyHopsIAR(nFrames);

				if (nBhopIAR < static_cast<unsigned>(cvar.kreedz.bunnyhop_ground_equal))
					continue;

				if (m_nCurrentFOG != nFrames)
					continue;

				switch (m_nCurrentFOG)
				{
				case 1: m_nCurrentFOG = 2; break;
				case 2:
				case 3: m_nCurrentFOG = 1;
				}
			}
		}

		// Get random height ground on which will start scrolling
		m_flHeightStart = g_Engine.pfnRandomFloat(6.f, 10.f);
		// Get random jumps amount for execute when player jumped off the ground
		m_nExtraJumps = static_cast<unsigned>(g_Engine.pfnRandomLong(1, 2));

		do
		{
			// Get new scroll pattern index
			m_iPatternIndex[INCOMING] = static_cast<unsigned>(g_Engine.pfnRandomLong(0, MAX_PATTERN_INDEX - 1));
		} while (m_iPatternIndex[INCOMING] == m_iPatternIndex[CURRENT]);
	}

	bool StandUpPermission()
	{
		// Check the ground
		if (m_nFramesOnGround > 0)
			return false;

		// Avoid fallrunning on surface
		if (g_Local.m_flGroundAngle > 22.5f && g_Local.m_flGroundAngle <= 45.f)
			return false;

		// If at standup you will have damage, skip it
		const float flFinalFallVelocity = g_Utils.FinalFallVelocity(HULL_PLAYER_DUCKED);
		if (flFinalFallVelocity > PM_PLAYER_MAX_SAFE_FALL_SPEED)
			return false;

		const float flHeightGround = g_Local.m_flHeightDucked + PM_VEC_DUCK_HULL_MIN;

		if (pmove->flFallVelocity <= 0.f || flHeightGround > cvar.kreedz.bunnyhop_standup_distance)
		{
			if (!pmove->velocity.IsZero2D())
			{
				// Cumpute predicted origin (without .z position)
				Vector vecPredOrigin = pmove->origin + pmove->velocity * g_Globals.m_flFrameTime;
				vecPredOrigin.z = pmove->origin.z;

				// Check the barrier
				const auto* pTrace = g_Engine.PM_TraceLine(pmove->origin, vecPredOrigin, PM_NORMAL, HULL_PLAYER_REGULAR, -1);
				if (pTrace->endpos != vecPredOrigin)
				{
					// Check possible to standup jump
					pTrace = g_Engine.PM_TraceLine(pmove->origin, vecPredOrigin, PM_NORMAL, HULL_PLAYER_DUCKED, -1);
					if (pTrace->endpos == vecPredOrigin)
						return true;
				}
				else
				{
					// Fix fast bunny jumping without standup
					Vector vecEnd = vecPredOrigin;
					vecEnd.z = -4096.f;
					pTrace = g_Engine.PM_TraceLine(vecPredOrigin, vecEnd, PM_NORMAL, HULL_PLAYER_REGULAR, -1);
					const float flPredHeightGround = vecPredOrigin.z - pTrace->endpos.z;

					// Check difference of height ground
					if (flPredHeightGround == g_Local.m_flHeightGround)
						return false;

					if (flPredHeightGround < cvar.kreedz.bunnyhop_standup_distance)
						return true;
				}
			}

			return false;
		}

		return true;
	}

	void HUD_Frame()
	{
		static unsigned nPatternFrames;

		UpdateFramesOnGround(m_nFramesOnGround);
		UpdateInformation();

		if (!cvar.kreedz.bunnyhop_scroll_emulation)
			return;

		// Clear if player is stuck
		if (IsCrouchStuck())
		{
			m_bJumpState = false;
			m_nExtraJumps = 0;
			nPatternFrames = 0;
			return;
		}

		if (g_Local.m_bIsOnLadder)
			m_bJumpState = false;

		m_ScrollInfo.Update();

		float flHeightInUnits = g_Local.m_flHeightGround + pmove->velocity.z * g_Globals.m_flFrameTime;
		float flHeightInFrames = fabs(flHeightInUnits / pmove->flFallVelocity / g_Globals.m_flFrameTime);

		// Clear if player has no vertical speed
		if (pmove->velocity.z == 0.f)
		{
			flHeightInUnits = 0.f;
			flHeightInFrames = 0.f;
		}

		const auto nScrollCount = m_ScrollInfo.GetScrollCount();

		if ((cvar.kreedz.bunnyhop_scroll_helper || state.bunnyhop_active) && pmove->flFallVelocity > 0.f)
			m_bJumpState = nScrollCount > 0 && (flHeightInUnits < 6.f || flHeightInFrames < 3.5f);

		if (m_bJumpState)
		{
			if (!m_nFramesOnGround || !JumpPermission())
				return;

			ScrollEmulation("jump");
			m_bJumpState = false;
		}

		if (nPatternFrames > 0)
		{
			nPatternFrames--;
			return;
		}

		// Check active key
		if (!state.bunnyhop_active)
			return;

		// Check swimming in water
		if (pmove->waterlevel > 1)
			return;

		// Check the slide surface
		if (g_Local.m_flGroundAngle > 45.f)
			return;

		// Set new pattern index when player falling
		if (m_iPatternIndex[CURRENT] != m_iPatternIndex[INCOMING] && pmove->flFallVelocity > 0.f)
			m_iPatternIndex[CURRENT] = m_iPatternIndex[INCOMING];

		if (m_nFramesOnGround > 0 || g_Local.m_bIsOnLadder)
		{
			ScrollEmulation("jump");
			nPatternFrames = PatternsList[m_iPatternIndex[CURRENT]][nScrollCount];
		}
		else if (m_nExtraJumps > 0 && pmove->flFallVelocity < 0.f)
		{
			ScrollEmulation("jump");
			nPatternFrames = PatternsList[m_iPatternIndex[CURRENT]][nScrollCount];
			m_nExtraJumps--;
		}
		else if (pmove->flFallVelocity > 0.f && flHeightInFrames < m_flHeightStart)
		{
			ScrollEmulation("jump");
			nPatternFrames = PatternsList[m_iPatternIndex[CURRENT]][nScrollCount];
			m_nExtraJumps = 0;
		}
	}

	void CL_CreateMove()
	{
		static bool waterstate;

		// Check swimming in water
		if (state.bunnyhop_active && pmove->waterlevel > 1)
			waterstate = true;

		usercmd_s* pCmd = cmd::get();

		// If water state is true, then hold jump
		if (state.bunnyhop_active && waterstate)
		{
			pCmd->buttons |= IN_JUMP;
			return;
		}
		else
		{
			// Reset state after key release
			waterstate = false;
		}

		if (cvar.kreedz.bunnyhop_scroll_emulation && m_bJumpState)
		{
			// Block jump for normal scrolling emulation
			pCmd->buttons &= ~IN_JUMP;
		}

		if (!cvar.kreedz.bunnyhop_scroll_emulation && state.bunnyhop_active)
		{
			if (m_nFramesOnGround > 0 && JumpPermission() && !cmd::oldbutton_is(IN_JUMP))
			{
				pCmd->buttons |= IN_JUMP;
			}
			else
			{
				pCmd->buttons &= ~IN_JUMP;

				if (g_Local.m_bIsOnLadder)
					pCmd->buttons |= IN_JUMP;
			}
		}

		if (state.bunnyhop_active && cvar.kreedz.bunnyhop_standup_distance > 0.f)
		{
			if (StandUpPermission())
				pCmd->buttons |= IN_DUCK;
		}

		if (cvar.kreedz.bunnyhop_break_jump_animation)
		{
			// Break jump animation by using duck
			if (m_nFramesOnGround > 0 && !g_Local.m_bIsDucked && pCmd->buttons & IN_JUMP)
				pCmd->buttons |= IN_DUCK;

			if (cvar.kreedz.bunnyhop_notouch_ground_illusion)
			{
				static int nFramesInDuck;

				if (m_nFramesOnGround == 0)
				{
					if (nFramesInDuck)
					{
						nFramesInDuck--;
						pCmd->buttons |= IN_DUCK;
					}
				}
				else
				{
					// Minimum frames for change to duck state
					nFramesInDuck = 2;
				}
			}
		}

		m_Counter.Update();
	}

private:
	static const unsigned m_DEFAULT_FOG = 1;
	enum PatternDefinitions { CURRENT, INCOMING, MAX };
	enum BunnyHopCorrectTypes
	{
		EfficiencyAdjustment,
		PercentPerfectJumps,
		PercentFramesOnGround
	};

	unsigned m_nFramesOnGround;
	unsigned m_nCurrentFOG;
	unsigned m_iPatternIndex[MAX];

	float m_flHeightStart;
	unsigned m_nExtraJumps;

	bool m_bJumpState;

	CBunnyHopCounter m_Counter;
	CScrollInfo m_ScrollInfo;
};

class CGroundStrafeCounter
{
public:
	CGroundStrafeCounter()
	{
		m_nGstrafeCount = 0;

		RtlSecureZeroMemory(&m_nGstrafeFOG, sizeof(m_nGstrafeFOG));
		RtlSecureZeroMemory(&m_nGstrafeIAR, sizeof(m_nGstrafeIAR));
	}

	unsigned GetGroundStrafesCount()
	{
		return m_nGstrafeCount;
	}

	unsigned GetGroundStrafesFOG(unsigned nFrames)
	{
		nFrames = max(nFrames, 1);
		nFrames = min(nFrames, m_MAX_FOG);
		return m_nGstrafeFOG[nFrames - 1];
	}

	unsigned GetGroundStrafesIAR(unsigned nFrames)
	{
		nFrames = max(nFrames, 1);
		nFrames = min(nFrames, m_MAX_FOG);
		return m_nGstrafeIAR[nFrames - 1];
	}

	void Update()
	{
		static unsigned nFramesOnGround;
		UpdateFramesOnGround(nFramesOnGround);

		// Check the ground
		if (nFramesOnGround == 0)
			return;

		// Check the button
		const usercmd_s* pCmd = cmd::get();
		if (~pCmd->buttons & IN_DUCK)
			return;

		// Check the oldbutton
		if (cmd::oldbutton_is(IN_DUCK))
			return;

		// Check groundstrafe frames
		if (nFramesOnGround > m_GSTRAFE_FRAMES)
			return;

		// Check groundstrafe max session ducks
		if (++m_nGstrafeCount > m_GSTRAFE_SESSION)
		{
			m_nGstrafeCount = 0;
			RtlSecureZeroMemory(&m_nGstrafeFOG, sizeof(m_nGstrafeFOG));
		}

		switch (nFramesOnGround)
		{
		case 1:
			m_nGstrafeIAR[0]++;
			m_nGstrafeIAR[1] = 0;
			m_nGstrafeIAR[2] = 0;
			m_nGstrafeFOG[0]++;
			break;
		case 2:
			m_nGstrafeIAR[0] = 0;
			m_nGstrafeIAR[1]++;
			m_nGstrafeIAR[2] = 0;
			m_nGstrafeFOG[1]++;
			break;
		case 3:
		case 4:
		case 5:
			m_nGstrafeIAR[0] = 0;
			m_nGstrafeIAR[1] = 0;
			m_nGstrafeIAR[2]++;
			m_nGstrafeFOG[2]++;
		}
	}

	static const unsigned m_MAX_FOG = 3;
	static const unsigned m_GSTRAFE_FRAMES = 5;
	static const unsigned m_GSTRAFE_SESSION = 100;

private:
	unsigned m_nGstrafeCount;
	unsigned m_nGstrafeFOG[m_MAX_FOG];
	unsigned m_nGstrafeIAR[m_MAX_FOG];
};

class CGroundStrafe
{
public:
	CGroundStrafe()
	{
		m_nFramesOnGround = 0;
		m_nCurrentFOG = 0;
		m_nExtraDucks = 0;
	}

	bool DuckPermission()
	{
		unsigned nNeedFrames = m_nCurrentFOG;

		if (m_nFramesOnGround >= nNeedFrames)
			return true;

		return false;
	}

	void UpdateInformation()
	{
		// Update groundstrafe information one time when player landing on the ground
		if (m_nFramesOnGround != 1)
			return;

		const auto randomlong = g_Engine.pfnRandomLong(0, 100);

		switch (cvar.kreedz.gstrafe_correct_type)
		{
		case EfficiencyAdjustment:
			m_nCurrentFOG = randomlong < cvar.kreedz.gstrafe_correct_value ? 1 : 2;
			break;
		case PercentFramesOnGround:
			if (randomlong < cvar.kreedz.gstrafe_frames_on_ground[0])
			{
				m_nCurrentFOG = 1;
			}
			else if (randomlong < cvar.kreedz.gstrafe_frames_on_ground[0] +
				cvar.kreedz.gstrafe_frames_on_ground[1])
			{
				m_nCurrentFOG = 2;
			}
			else if (randomlong < cvar.kreedz.gstrafe_frames_on_ground[0] +
				cvar.kreedz.gstrafe_frames_on_ground[1] + cvar.kreedz.gstrafe_frames_on_ground[2])
			{
				m_nCurrentFOG = 3;
			}
			else
			{
				m_nCurrentFOG = m_DEFAULT_FOG;
			}
		}

		if (cvar.kreedz.gstrafe_ground_equal)
		{
			for (unsigned i = 0; i < m_Counter.m_MAX_FOG; i++)
			{
				const unsigned nFrames = i + 1;
				const unsigned nGstrafeIAR = m_Counter.GetGroundStrafesIAR(nFrames);

				if (nGstrafeIAR < static_cast<unsigned>(cvar.kreedz.gstrafe_ground_equal))
					continue;

				if (m_nCurrentFOG != nFrames)
					continue;

				switch (m_nCurrentFOG)
				{
				case 1: m_nCurrentFOG = 2; break;
				case 2:
				case 3: m_nCurrentFOG = 1;
				}
			}
		}

		// Get random ducks amount for execute when player ducked off the ground
		m_nExtraDucks = static_cast<unsigned>(g_Engine.pfnRandomLong(1, 2));
	}

	void HUD_Frame()
	{
		static unsigned nPatternFrames;

		UpdateFramesOnGround(m_nFramesOnGround);
		UpdateInformation();

		if (!cvar.kreedz.gstrafe_scroll_emulation)
			return;

		if (nPatternFrames > 0)
		{
			nPatternFrames--;
			return;
		}

		// Check active keys
		if (!state.gstrafe_active && !state.gstrafe_standup_active)
			return;

		// Check the ladder
		if (g_Local.m_bIsOnLadder)
			return;

		// Check swimming in water
		if (pmove->waterlevel > 1)
			return;

		// Check slide surface
		if (g_Local.m_flGroundAngle > 45.f)
			return;

		if (m_nFramesOnGround > 0)
		{
			if (DuckPermission())
			{
				ScrollEmulation("duck");
				nPatternFrames = 1;
			}
		}
		else if (m_nExtraDucks > 0)
		{
			ScrollEmulation("duck");
			nPatternFrames = 1;
			m_nExtraDucks--;
		}
	}

	void CL_CreateMove()
	{
		usercmd_s* pCmd = cmd::get();

		if (g_Local.m_bIsOnLadder)
			return;

		if (state.gstrafe_active || state.gstrafe_standup_active)
		{
			if (m_nFramesOnGround > 0)
			{
				// Change player speed to adjust slowdown
				if (g_Local.m_flGroundAngle <= cvar.kreedz.gstrafe_slowdown_angle)
					g_Utils.AdjustSpeed(cvar.kreedz.gstrafe_slowdown_scale);
			}

			if (!cvar.kreedz.gstrafe_scroll_emulation)
			{
				if (m_nFramesOnGround > 0 && DuckPermission() && !g_Local.m_bIsDucked && !cmd::oldbutton_is(IN_DUCK))
				{
					pCmd->buttons |= IN_DUCK;
				}
				else
				{
					pCmd->buttons &= ~IN_DUCK;
				}
			}

			// Use the jump animation
			if (cvar.kreedz.gstrafe_jump_animation)
			{
				unsigned nNeedFrames = m_nCurrentFOG + 1;

				if (state.gstrafe_standup_active)
					nNeedFrames++;

				if (m_nFramesOnGround == nNeedFrames)
					pCmd->buttons |= IN_JUMP;
			}
		}

		if (state.gstrafe_standup_active)
		{
			const float flFallVelocityStart = g_Local.m_flGroundAngle > 0.f ? 0.f : 80.f;
			const float flFallVelocityEnd = g_Local.m_flGroundAngle > 0.f ? FLT_MAX : 192.f;

			if (pmove->flFallVelocity > flFallVelocityStart && pmove->flFallVelocity < flFallVelocityEnd)
				pCmd->buttons |= IN_DUCK;
		}

		m_Counter.Update();
	}

private:
	static const unsigned m_DEFAULT_FOG = 1;
	enum GroundStrafeCorrectTypes
	{
		EfficiencyAdjustment,
		PercentFramesOnGround
	};

	unsigned m_nFramesOnGround;
	unsigned m_nCurrentFOG;
	unsigned m_nExtraDucks;

	CGroundStrafeCounter m_Counter;
};

class CJumpBug
{
public:
	float GetFallDamage()
	{
		const float flFinalFallVelocity = g_Utils.FinalFallVelocity(HULL_PLAYER_REGULAR);
		const float flFallDamage = (flFinalFallVelocity - PM_PLAYER_MAX_SAFE_FALL_SPEED) * PM_DAMAGE_FOR_FALL_SPEED;
		return max(flFallDamage, 0.f);
	}

	bool AutomaticByDamage()
	{
		bool bAutomatic = false;
		const float flFallDamage = GetFallDamage();

		if (cvar.kreedz.jumpbug_auto_damage_min)
			bAutomatic = cvar.kreedz.jumpbug_auto_damage_min <= flFallDamage;

		if (cvar.kreedz.jumpbug_auto_damage_fatal && g_Local.m_iHealth <= flFallDamage)
			bAutomatic = true;

		if (bAutomatic && !(cvar.kreedz.jumpbug_slowdown_velocity && g_Local.m_flGroundAngle > 0.f))
		{
			const float flFrameVelocityZ = GetPredictedFallVelocity() * g_Globals.m_flFrameTime;

			if (g_Local.m_flHeight - flFrameVelocityZ * 20.f > 0.f)
				bAutomatic = false;
		}

		return bAutomatic;
	}

	void SlowdownVelocity(usercmd_s* pCmd)
	{
		const float velocity_length = pmove->velocity.Length();
		const Vector vecForward = Vector(
			pmove->velocity.x / velocity_length,
			pmove->velocity.y / velocity_length,
			0.f);

		Vector vecAngles;
		g_Utils.VectorAngles(vecForward, vecAngles);

		float diff = vecAngles.y - pCmd->viewangles.y;
		while (diff < -180.f) diff += 360.f;
		while (diff > 180.f) diff -= 360.f;

		const float frametime = g_Globals.m_flFrameTime;
		const float fmove = (velocity_length + velocity_length * frametime) / (frametime * 100.f);

		pCmd->forwardmove = -fmove;
		pCmd->sidemove = 0.f;

		const float fixed_yaw = vecAngles.y - pCmd->viewangles.y;
		g_Utils.RotateInvisible(-fixed_yaw, 0.f, pCmd);
	}

	void CL_CreateMove()
	{
		static unsigned iState, nJumpCount;

		usercmd_s* pCmd = cmd::get();

		if (nJumpCount > 0)
		{
			nJumpCount--;

			if (nJumpCount % 2 == 0)
				pCmd->buttons |= IN_JUMP;
			else
				pCmd->buttons &= ~IN_JUMP;
		}

		if (!state.jumpbug_active && !AutomaticByDamage())
			return;

		// Check slide surface
		if (g_Local.m_flGroundAngle > 45.f)
			return;

		if (pmove->flFallVelocity <= 0.f)
			return;

		if (cvar.kreedz.jumpbug_slowdown_velocity && g_Local.m_flGroundAngle > 0.f)
			SlowdownVelocity(pCmd);

		pCmd->buttons |= IN_DUCK;
		pCmd->buttons &= ~IN_JUMP;

		const float flFrameVelocityZ = GetPredictedFallVelocity() * g_Globals.m_flFrameTime;

		switch (iState)
		{
		case 0:
			// Note: multiplication by 2.f need for avoid slowmotion in scale speed
			if (g_Local.m_flHeight >= 18.f && g_Local.m_flHeight - flFrameVelocityZ * 2.f <= 20.f)
			{
				const double offset = g_Local.m_flGroundAngle == 45.f ? 20.0 : 19.0;
				const double scale = fabs((g_Local.m_flHeight - offset) / flFrameVelocityZ);
				g_Utils.AdjustSpeed(scale);
				iState = 1;
			}
			break;
		case 1:
			pCmd->buttons &= ~IN_DUCK;
			pCmd->buttons |= IN_JUMP;
			nJumpCount = 5; // corrects the display of statistics jumpbug by execute N jumps
			iState = 0;
			break;
		}
	}
};

class CEdgeBug
{
public:
	bool Automatic()
	{
		const bool bTakeDamage = pmove->flFallVelocity > PM_PLAYER_MAX_SAFE_FALL_SPEED;
		return cvar.kreedz.edgebug_auto_efficiency > 0 && bTakeDamage;
	}

	void CL_CreateMove()
	{
		static unsigned iState;

		if (!state.edgebug_active && !Automatic())
			return;

		if (g_Local.m_flGroundAngle > 0.f)
			return;

		if (pmove->flFallVelocity < 0.f)
			return;

		if (g_Local.m_flVelocity == 0.f)
			return;

		if (g_Local.m_flEdgeDist > 65.f)
			return;

		const float flFrameVelocityZ = GetPredictedFallVelocity() * g_Globals.m_flFrameTime;
		const float flPredHeightGround = g_Local.m_flHeightGround - flFrameVelocityZ * 3.f;

		if (flPredHeightGround > 0.f)
			return;

		const float flEfficiency = state.edgebug_active ? cvar.kreedz.edgebug_efficiency : cvar.kreedz.edgebug_auto_efficiency;
		const float flFallVelocity = pmove->flFallVelocity == 0.f ? 1.f : pmove->flFallVelocity;
		const float flHeightGround = g_Local.m_flHeightGround == 0.f ? 1.f : g_Local.m_flHeightGround;
		const float flEdgeOffset = (g_Local.m_flEdgeDist / g_Local.m_flVelocity) /
			(g_Local.m_flEdgeDist / flFallVelocity) * (g_Local.m_flEdgeDist / flHeightGround);

		auto globaltime = *g_Globals.m_pGlobalTime;

		switch (iState)
		{
		case 0:
			globaltime -= flEdgeOffset * g_Globals.m_flFrameTime / 10.f;
			iState = 1;
			break;
		case 1:
			globaltime += (flEfficiency - flEdgeOffset) * g_Globals.m_flFrameTime / 10.f;
			iState = 0;
			break;
		}

		*g_Globals.m_pGlobalTime = globaltime;
	}
};

class CStrafeInvisible
{
public:
	CStrafeInvisible()
	{
		m_StrafeDirection = 0;
	}

	void GetStrafeDirection(usercmd_s* pCmd)
	{
		if (cvar.kreedz.strafe_invisible_direction != StrafeDirection_Automatic)
		{
			m_StrafeDirection = cvar.kreedz.strafe_invisible_direction;
			return;
		}
	}

	void CL_CreateMove()
	{
		if (!state.strafe_active)
		{
			g_SkipFrames.End();
			return;
		}

		g_SkipFrames.Start(cvar.kreedz.strafe_invisible_skipframes);

		if (g_Local.m_bIsOnLadder)
			return;

		usercmd_s* pCmd = cmd::get();

		/*if (cvar.kreedz.strafe_invisible_direction != StrafeDirection_Automatic)
		{
			Direction = static_cast<unsigned>(cvar.kreedz.strafe_invisible_direction);
		}
		else
		{
			if (pCmd->buttons & IN_FORWARD) Direction = StrafeDirection_Forward;
			else if (pCmd->buttons & IN_MOVELEFT) Direction = StrafeDirection_LeftSide;
			else if (pCmd->buttons & IN_MOVERIGHT) Direction = StrafeDirection_RightSide;
			else if (pCmd->buttons & IN_BACK) Direction = StrafeDirection_Backward;
		}*/

		const bool onground = pmove->onground != -1;
		if (onground)
			return;

		if (cvar.kreedz.strafe_invisible_fps_helper)
		{
			const double scale = (1.0 / g_Globals.m_flFrameTime) / cvar.kreedz.strafe_invisible_fps_helper;
			g_Utils.AdjustSpeed(scale);
		}

		float angle_direction;

		switch (m_StrafeDirection)
		{
		case StrafeDirection_Forward:	angle_direction = 0.f;		break;
		case StrafeDirection_LeftSide:	angle_direction = -90.f;	break;
		case StrafeDirection_RightSide:	angle_direction = 90.f;		break;
		case StrafeDirection_Backward:	angle_direction = 180.f;
		}

		angle_direction = angle_direction * IM_PI / 180.f;

		if (g_Local.m_flVelocity < 30.f)
		{
			switch (m_StrafeDirection)
			{
			case StrafeDirection_Forward:	pCmd->forwardmove = pmove->maxspeed;	break;
			case StrafeDirection_LeftSide:	pCmd->sidemove = -pmove->maxspeed;		break;
			case StrafeDirection_RightSide:	pCmd->sidemove = pmove->maxspeed;		break;
			case StrafeDirection_Backward:	pCmd->forwardmove = -pmove->maxspeed;
			}
		}
		else
		{
			const float angle_speed = atan2(pmove->velocity.y, pmove->velocity.x);
			float angle_diff = sin(angle_speed - pCmd->viewangles.y * IM_PI / 180.f - angle_direction);
			angle_diff = atan2(angle_diff, sqrt(1.f - angle_diff * angle_diff));

			pCmd->forwardmove = 0.f;
			pCmd->sidemove = angle_diff > 0.f ? 435.f : -435.f;

			float angle;

			angle = pCmd->viewangles.y * IM_PI / 180.f;
			const float osin = sin(angle);
			const float ocos = cos(angle);

			angle = 2.f * pCmd->viewangles.y * IM_PI / 180.f - angle_speed + angle_direction;
			const float nsin = sin(angle);
			const float ncos = cos(angle);

			pCmd->forwardmove = pCmd->sidemove * (osin * ncos - ocos * nsin);
			pCmd->sidemove = pCmd->sidemove * (osin * nsin + ocos * ncos);
		}

		const float fmove = pCmd->forwardmove;
		const float smove = pCmd->sidemove;

		switch (m_StrafeDirection)
		{
		case StrafeDirection_Forward:
			pCmd->forwardmove = fmove;
			pCmd->sidemove = smove;
			break;
		case StrafeDirection_LeftSide:
			pCmd->forwardmove = smove;
			pCmd->sidemove = -fmove;
			break;
		case StrafeDirection_RightSide:
			pCmd->forwardmove = -smove;
			pCmd->sidemove = fmove;
			break;
		case StrafeDirection_Backward:
			pCmd->forwardmove = -fmove;
			pCmd->sidemove = -smove;
		}
	}

private:
	enum StrafeDirectionList
	{
		StrafeDirection_Forward,
		StrafeDirection_LeftSide,
		StrafeDirection_RightSide,
		StrafeDirection_Backward,
		StrafeDirection_Automatic
	};

	typedef unsigned StrafeDirection;
	StrafeDirection m_StrafeDirection;
};

static CBunnyHop g_BunnyHop;
static CGroundStrafe g_GroundStrafe;
static CJumpBug g_JumpBug;
static CEdgeBug g_EdgeBug;
static CStrafeInvisible g_StrafeInvisible;

void CKreedz::HUD_Frame(void)
{
	g_BunnyHop.HUD_Frame();
	g_GroundStrafe.HUD_Frame();
}

void CKreedz::CL_CreateMove(void)
{
	g_StrafeInvisible.CL_CreateMove();
	g_BunnyHop.CL_CreateMove();
	g_GroundStrafe.CL_CreateMove();
	g_JumpBug.CL_CreateMove();
	g_EdgeBug.CL_CreateMove();

	/*static int old_mouse[2];

	int dir;

	if (g_Local.m_iMousePos[0] - old_mouse[0] < 0)
		dir = 0;
	else
		dir = 1;

	g_Engine.Con_NPrintf(1, "%d", dir);

	old_mouse[0] = g_Local.m_iMousePos[0];
	old_mouse[1] = g_Local.m_iMousePos[1];*/
}

void Scroll_JumpPressed(void)
{
	g_BunnyHop.JumpPressed();
}

void Scroll_JumpReleased(void)
{
	g_BunnyHop.JumpReleased();
}