#include "main.h"

CMiscellaneous g_Miscellaneous;

void CMiscellaneous::HUDCommands(usercmd_s *cmd)
{
	if (cmd->buttons & IN_ATTACK && !(hud_buttons & IN_ATTACK))
	{
		CL_RecordHUDCommand_Hooked("+attack");
		hud_buttons |= IN_ATTACK;
	}
	else if (!(cmd->buttons & IN_ATTACK) && hud_buttons & IN_ATTACK)
	{
		CL_RecordHUDCommand_Hooked("-attack");
		hud_buttons &= ~IN_ATTACK;
	}
	if (cmd->buttons & IN_ATTACK2 && !(hud_buttons & IN_ATTACK2))
	{
		CL_RecordHUDCommand_Hooked("+attack2");
		hud_buttons |= IN_ATTACK2;
	}
	else if (!(cmd->buttons & IN_ATTACK2) && hud_buttons & IN_ATTACK2)
	{
		CL_RecordHUDCommand_Hooked("-attack2");
		hud_buttons &= ~IN_ATTACK2;
	}
}

void CMiscellaneous::NameStealer()
{
	if (!cvar.name_stealer_enabled)
		return;

	static auto prev_time = client_state->time;

	if (abs(client_state->time - prev_time) > cvar.name_stealer_interval)
	{
		prev_time = client_state->time;

		std::deque<std::string> names;

		for (auto i = 1; i <= MAX_CLIENTS; ++i)
		{
			if (i == g_Local.m_iIndex)
				continue;

			const auto pPlayer = g_World.GetPlayer(i);

			if (!pPlayer)
				continue;

			if (!pPlayer->m_bIsConnected)
				continue;

			if (pPlayer->m_iTeam == SPECTATOR || pPlayer->m_iTeam == UNASSIGNED)
				continue;

			if (strlen(pPlayer->m_szPrintName))
				names.push_back(pPlayer->m_szPrintName);
		}

		if (!names.size())
			return;

		auto random_num = g_Engine.pfnRandomLong(0, names.size() - 1);

		for (auto i = 0; i < names.size(); i++)
		{
			if (random_num == i)
			{
				auto name = names[i];

				if (name.find("a") != std::string::npos)//start en2ru
					name = name.replace(name.find("a"), std::string("a").size(), u8"à");
				else if (name.find("A") != std::string::npos)
					name = name.replace(name.find("A"), std::string("A").size(), u8"À");
				else if (name.find("o") != std::string::npos)
					name = name.replace(name.find("o"), std::string("o").size(), u8"î");
				else if (name.find("O") != std::string::npos)
					name = name.replace(name.find("O"), std::string("O").size(), u8"Î");
				else if (name.find("e") != std::string::npos)
					name = name.replace(name.find("e"), std::string("e").size(), u8"å");
				else if (name.find("E") != std::string::npos)
					name = name.replace(name.find("E"), std::string("E").size(), u8"Å");
				else if (name.find("x") != std::string::npos)
					name = name.replace(name.find("x"), std::string("x").size(), u8"õ");
				else if (name.find("X") != std::string::npos)
					name = name.replace(name.find("X"), std::string("X").size(), u8"Õ");
				else if (name.find("c") != std::string::npos)
					name = name.replace(name.find("c"), std::string("c").size(), u8"ñ");
				else if (name.find("C") != std::string::npos)
					name = name.replace(name.find("C"), std::string("C").size(), u8"Ñ");
				else if (name.find("T") != std::string::npos)
					name = name.replace(name.find("T"), std::string("T").size(), u8"Ò");
				else if (name.find("p") != std::string::npos)
					name = name.replace(name.find("p"), std::string("p").size(), u8"ð");
				else if (name.find("P") != std::string::npos)
					name = name.replace(name.find("P"), std::string("P").size(), u8"Ð");
				else if (name.find("H") != std::string::npos)
					name = name.replace(name.find("H"), std::string("H").size(), u8"Í");
				else if (name.find("K") != std::string::npos)
					name = name.replace(name.find("K"), std::string("K").size(), u8"Ê");
				else if (name.find("M") != std::string::npos)
					name = name.replace(name.find("M"), std::string("M").size(), u8"Ì");
				else if (name.find("B") != std::string::npos)
					name = name.replace(name.find("B"), std::string("B").size(), u8"Â");
				else if (name.find(u8"à") != std::string::npos)//start ru2en
					name = name.replace(name.find(u8"à"), std::string(u8"à").size(), "a");
				else if (name.find(u8"À") != std::string::npos)
					name = name.replace(name.find(u8"À"), std::string(u8"À").size(), "A");
				else if (name.find(u8"î") != std::string::npos)
					name = name.replace(name.find(u8"î"), std::string(u8"î").size(), "o");
				else if (name.find(u8"Î") != std::string::npos)
					name = name.replace(name.find(u8"Î"), std::string(u8"Î").size(), "O");
				else if (name.find(u8"å") != std::string::npos)
					name = name.replace(name.find(u8"å"), std::string(u8"å").size(), "e");
				else if (name.find(u8"Å") != std::string::npos)
					name = name.replace(name.find(u8"Å"), std::string(u8"Å").size(), "E");
				else if (name.find(u8"õ") != std::string::npos)
					name = name.replace(name.find(u8"õ"), std::string(u8"õ").size(), "x");
				else if (name.find(u8"Õ") != std::string::npos)
					name = name.replace(name.find(u8"Õ"), std::string(u8"Õ").size(), "X");
				else if (name.find(u8"ñ") != std::string::npos)
					name = name.replace(name.find(u8"ñ"), std::string(u8"ñ").size(), "c");
				else if (name.find(u8"Ñ") != std::string::npos)
					name = name.replace(name.find(u8"Ñ"), std::string(u8"Ñ").size(), "C");
				else if (name.find(u8"Ò") != std::string::npos)
					name = name.replace(name.find(u8"Ò"), std::string(u8"Ò").size(), "T");
				else if (name.find(u8"ð") != std::string::npos)
					name = name.replace(name.find(u8"ð"), std::string(u8"ð").size(), "p");
				else if (name.find(u8"Ð") != std::string::npos)
					name = name.replace(name.find(u8"Ð"), std::string(u8"Ð").size(), "P");
				else if (name.find(u8"Í") != std::string::npos)
					name = name.replace(name.find(u8"Í"), std::string(u8"Í").size(), "H");
				else if (name.find(u8"Ê") != std::string::npos)
					name = name.replace(name.find(u8"Ê"), std::string(u8"Ê").size(), "K");
				else if (name.find(u8"Ì") != std::string::npos)
					name = name.replace(name.find(u8"Ì"), std::string(u8"Ì").size(), "M");
				else if (name.find(u8"Â") != std::string::npos)
					name = name.replace(name.find(u8"Â"), std::string(u8"Â").size(), "B");
				else
					name.append(".");

				std::string cmd = "name \"";
				cmd.append(name.c_str());
				cmd.append("\"");

				g_Engine.pfnClientCmd(cmd.c_str());
				break;
			}
		}
	}
}

void CMiscellaneous::FakeLatency()
{
	g_Miscellaneous.m_bFakeLatencyActive = false;

	if (!cvar.fakelatency_enabled)
		return;

	if (cvar.fakelatency_key_press && !state.fakelatency_active)
		return;

	const auto latency = (double)cvar.fakelatency_value / 1000.0;

	if (latency > 0.0)
	{
		g_Utils.SetFakeLatency(latency);

		g_Miscellaneous.m_bFakeLatencyActive = true;
	}
}

void CMiscellaneous::AutoReload(usercmd_s *cmd)
{
	if (cvar.automatic_reload)
	{
		if (cmd->buttons & IN_ATTACK && g_Weapon.IsGun() && g_Weapon.data.m_iClip < 1)
		{
			cmd->buttons &= ~IN_ATTACK;
			cmd->buttons |= IN_RELOAD;
		}
	}
}

void CMiscellaneous::AutoPistol(usercmd_s* cmd)
{
	if (cvar.automatic_pistol)
	{
		if (cmd->buttons & IN_ATTACK && g_Weapon.IsPistol() && !g_Weapon.data.m_iInReload && !g_Weapon.CanAttack())
		{
			cmd->buttons &= ~IN_ATTACK;
		}
	}
}

void CMiscellaneous::FakeLag(usercmd_s *cmd)
{
	_bFakeLagActive = false;

	if (!cvar.ragebot_active)
		return;

	if (!cvar.fakelag_enabled)
		return;

	if (!cvar.fakelag_while_shooting && cmd->buttons & IN_ATTACK && g_Weapon.CanAttack())
		return;

	if (cvar.fakelag_triggers == 0)
	{
		if (g_Local.m_flHeight > 0.f)
			return;
	}
	else if (cvar.fakelag_triggers == 1)
	{
		if (g_Local.m_flHeight <= 0.f)
			return;
	}

	if (cvar.fakelag_on_enemy_visible)
	{
		auto nEnemies = 0;

		for (auto i = 1; i <= MAX_CLIENTS; ++i)
		{
			if (i == g_Local.m_iIndex)
				continue;

			const auto pPlayer = g_World.GetPlayer(i);

			if (!pPlayer)
				continue;

			if (pPlayer->m_bIsDead)
				continue;

			if (!pPlayer->m_bIsInPVS)
				continue;

			if (!cvar.ragebot_aim_friendly_fire && pPlayer->m_iTeam == g_Local.m_iTeam)
				continue;

			nEnemies++;
		}

		if (nEnemies <= 0)
			return;
	}

	_bFakeLagActive = true;

	static auto nChoked = 0;

	if (cvar.fakelag_type == 0 && cvar.fakelag_variance > 0.f) //Dynamic
	{
		static auto nNotChoked = 0;

		if (nChoked < cvar.fakelag_choke_limit)
		{
			g_Globals.m_bSendCommand = false;
			nChoked++;
			nNotChoked = 0;
		}
		else {
			float flTmp = (cvar.fakelag_choke_limit / 100.f) * cvar.fakelag_variance;

			nNotChoked++;

			if (nNotChoked > flTmp)
				nChoked = 0;
		}
	}
	else if (cvar.fakelag_type == 1) //Maximum
	{
		nChoked++;

		if (nChoked > 0)
			g_Globals.m_bSendCommand = false;

		if (nChoked > cvar.fakelag_choke_limit)
			nChoked = -1; //1 tick valid
	}
	else if (cvar.fakelag_type == 2) //Break lag compensation
	{
		float flLength = g_Local.m_flVelocity * g_Globals.m_flFrameTime;

		if (flLength < 64.0f && g_Local.m_flVelocity > 0.05f && flLength)
		{
			auto nNeedChoke = 64.0f / flLength;

			if (nNeedChoke > cvar.fakelag_choke_limit)
				nNeedChoke = cvar.fakelag_choke_limit;

			if (nChoked < nNeedChoke)
			{
				g_Globals.m_bSendCommand = false;
				nChoked++;
			}
			else
				nChoked = 0;
		}
		else
			_bFakeLagActive = false;
	}
}

void CMiscellaneous::FakeWalk(usercmd_s *cmd)
{
	m_bFakeWalkActive = false;

	static auto replaced = false;
	static auto previous_fps = g_Engine.pfnGetCvarFloat("fps_max");

	if (cvar.ragebot_fake_walk_decrease_fps && replaced)
	{
		g_Engine.Cvar_SetValue("fps_max", previous_fps);
		replaced = false;
	}

	if (!cvar.ragebot_active)
		return;
	
	if (cvar.ragebot_fake_walk_choke_limit <= 0)
		return;

	if (!cvar.ragebot_fake_walk_enabled)
		return;

	if (!cvar.ragebot_fake_walk_key_press || !state.fakewalk_active)
		return;

	static auto bAllowChoke = true;
	static auto nChoked = 0;
	
	if (bAllowChoke)
	{
		if (cmd->sidemove != 0.f || cmd->forwardmove != 0.f)
		{
			if (cvar.ragebot_fake_walk_decrease_fps && !replaced)
			{
				previous_fps = g_Engine.pfnGetCvarFloat("fps_max");
				g_Engine.Cvar_SetValue("fps_max", 20.0f);
				replaced = true;
			}

			g_Globals.m_bSendCommand = false;

			m_bFakeWalkActive = true;

			nChoked++;

			if (nChoked >= cvar.ragebot_fake_walk_choke_limit)
				bAllowChoke = false;
		}
	}
	else {
		cmd->sidemove = 0.f;
		cmd->forwardmove = 0.f;

		if (pmove->velocity.Length2D() <= 0.f)
		{
			bAllowChoke = true;
			nChoked = 0;
		}
	}
}

void CMiscellaneous::AntiAim(usercmd_s *cmd)
{
	if (!cvar.ragebot_active)
		return;

	if (!cvar.antiaim_enabled)
		return;

	if (g_Weapon.IsNade()) // Grenades throw only
	{
		if (!(cmd->buttons & IN_ATTACK))
		{
			if (g_Weapon.data.m_flStartThrow)
				return;

			if (g_Weapon.data.m_flReleaseThrow > 0.f)
				return;
		}
	}
	else if (g_Weapon.IsC4()) // Planting
	{
		
	}
	else // Guns and knife attack tick
	{
		if (g_Weapon.IsNonAttack())
			return;

		if (!cvar.antiaim_on_knife && g_Weapon.IsKnife())
			return;

		if (cmd->buttons & IN_ATTACK && g_Weapon.CanAttack())
			return;
	}

	if (cmd->buttons & IN_USE) //Defusing
		return;

	if (pmove->movetype == MOVETYPE_FLY && g_Local.m_flVelocity > 0.f)
		return;

	auto best_origin = Vector();
	auto best_fov = FLT_MAX;

	for (auto i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (i == g_Local.m_iIndex)
			continue;

		const auto pPlayer = g_World.GetPlayer(i);

		if (!pPlayer)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		if (!pPlayer->m_bIsInPVS)
			continue;

		if (!cvar.antiaim_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (cvar.player_list && pPlayer->m_iPlayerListType == 1)
			continue;

		auto fov = g_Local.m_vecForward.AngleBetween(pPlayer->m_vecOrigin - g_Local.m_vecEye);

		if (fov < best_fov)
		{
			best_fov = fov;
			best_origin = pPlayer->m_vecOrigin;
		}
	}

	if (!_bFakeLagActive)
	{
		static auto nNeedChoke = cvar.antiaim_choke_limit;

		if (nNeedChoke > 0)
		{
			g_Globals.m_bSendCommand = false;
			nNeedChoke--;
		}
		else
			nNeedChoke = cvar.antiaim_choke_limit;
	}

	static auto side = false;

	if (g_Globals.m_bSendCommand)
		side = !side;

	QAngle QAngles(cmd->viewangles);
	QAngle QBackupAngles = QAngles;

	if (!best_origin.IsZero())
		g_Utils.VectorAngles(best_origin - g_Local.m_vecEye, QAngles);

	g_Utils.FixMoveStart(cmd);

	cmd->viewangles = QAngles;

	static auto previous_yaw = cmd->viewangles.y;

	if (g_Local.m_flVelocity > 0.f && !m_bFakeWalkActive)
	{
		if (cvar.antiaim_yaw_while_running == 1)
			cmd->viewangles.y += 180;
		else  if (cvar.antiaim_yaw_while_running == 2)
			cmd->viewangles.y += cvar.antiaim_yaw_static;
		else
			cmd->viewangles.y = QBackupAngles.y;
	}
	else if (!g_Globals.m_bSendCommand)//Yaw
	{
		if (cvar.antiaim_yaw == 1)
			cmd->viewangles.y += 180;
		else if (cvar.antiaim_yaw == 2)
			cmd->viewangles.y = previous_yaw - 180;
		else if (cvar.antiaim_yaw == 3)
			cmd->viewangles.y += cvar.antiaim_yaw_static;
	
		m_vecRealAngles.y = cmd->viewangles.y;
	}
	else { //Fake yaw if choking aka desyns
		if (cvar.antiaim_fake_yaw == 1)
			cmd->viewangles.y += 180;
		else if (cvar.antiaim_fake_yaw == 2)
		{
			if (!side)
				cmd->viewangles.y += 140;
			else
				cmd->viewangles.y -= 140;
		}
		else if (cvar.antiaim_fake_yaw == 3)
			cmd->viewangles.y = g_Local.m_QAngles[1];
		else  if (cvar.antiaim_fake_yaw == 4)
			cmd->viewangles.y += g_Engine.pfnRandomFloat(-180, 180);
		else  if (cvar.antiaim_fake_yaw == 5)
			cmd->viewangles.y += fmod(g_Engine.GetClientTime() * cvar.antiaim_yaw_spin * 360, 360);
		else  if (cvar.antiaim_fake_yaw == 6)
			cmd->viewangles.y += cvar.antiaim_fake_yaw_static;

		previous_yaw = cmd->viewangles.y;
	}

	if (cvar.antiaim_pitch == 1)
		cmd->viewangles.x = 180;
	else if (cvar.antiaim_pitch == 2)
		cmd->viewangles.x = -179.99999f;
	else if (cvar.antiaim_pitch == 3)
		cmd->viewangles.x = 89;
	else if (cvar.antiaim_pitch == 4)
		cmd->viewangles.x = -89;
	else if (cvar.antiaim_pitch == 5)
	{
		if (side)
			cmd->viewangles.x = -89;
		else
			cmd->viewangles.x = 89;
	}
	else if (cvar.antiaim_pitch == 6)
		cmd->viewangles.x = g_Engine.pfnRandomFloat(-180, 180);

	cmd->viewangles.z = cvar.antiaim_roll;

	m_vecRealAngles.x = cmd->viewangles.x;
	m_vecRealAngles.z = cmd->viewangles.z;

	g_Utils.FixMoveEnd(cmd);
}

void CMiscellaneous::ThirdPerson(ref_params_s *pparams)
{
	if (!cvar.visuals)
		return;

	if (!cvar.thirdperson)
		return;

	if (state.visuals_panic || g_Globals.m_bSnapshot || g_Globals.m_bScreenshot)
		return;

	if (cvar.thirdperson_key_toggle && !state.thirdperson_active)
		return;

	if (g_Local.m_bIsDead)
		return;

	pparams->vieworg += pparams->forward * cvar.thirdperson * -1.f;
}

void CMiscellaneous::ScreenFade()
{
	const auto screenfade_limit = (255.f / 100.f) * cvar.screenfade_limit;

	if (client_state)
	{
		if (client_state->sf.fadeEnd > (float)client_state->time)
		{
			_flFadeEnd = (float)client_state->sf.fadeEnd;

			if (screenfade_limit > 0.f)
			{
				if (screenfade_limit >= 255.f)
					client_state->sf.fadeEnd = 0.f;
				else if (client_state->sf.fadealpha > (byte)screenfade_limit)
					client_state->sf.fadealpha = (byte)screenfade_limit;
			}
		}

		if (_flFadeEnd > (float)client_state->time)
			m_flFlashed = (_flFadeEnd - (float)client_state->time) * client_state->sf.fadeSpeed;
		else
			m_flFlashed = 0.f;

		m_flFlashed = ImClamp(m_flFlashed, 0.f, 255.f);
	}
}

void CMiscellaneous::CustomRenderFov(const int value)
{
	if (g_Local.m_iFOV == DEFAULT_FOV)
	{
		int iRenderFov = g_Local.m_iFOV;

		if (value != iRenderFov)
			iRenderFov = value;

		g_ClientUserMsgsMap["SetFOV"]("SetFOV", 1, &iRenderFov);
	}
}

void CMiscellaneous::MaximizeWindowAfterRespawn()
{
	static bool bMaximizeWindowState = false;

	if (g_Local.m_bIsDead && !bMaximizeWindowState)
		bMaximizeWindowState = true;

	else if (!g_Local.m_bIsDead && bMaximizeWindowState)
	{
		SendMessageA(g_Globals.m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		bMaximizeWindowState = false;
	}
}

void CMiscellaneous::Brightness()
{
	static auto replaced = false;
	static float color[4];

	if (cvar.lightmap && !cvar.hide_from_obs && !g_Globals.m_bUnloadLibrary && !state.visuals_panic && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
	{
		if (color[0] != cvar.lightmap_color[0] || color[1] != cvar.lightmap_color[1] || color[2] != cvar.lightmap_color[2] || color[3] != cvar.lightmap_color[3])
		{
			RtlCopyMemory(color, cvar.lightmap_color, sizeof(color));

			g_Engine.pfnSetFilterMode(1);
			g_Engine.pfnSetFilterColor(color[0], color[1], color[2]);
			g_Engine.pfnSetFilterBrightness(color[3]);

			replaced = true;
		}
	}
	else if (replaced && (!cvar.lightmap || g_Globals.m_bUnloadLibrary || cvar.hide_from_obs || state.visuals_panic || g_Globals.m_bSnapshot || g_Globals.m_bScreenshot))
	{
		RtlSecureZeroMemory(color, sizeof(color));

		g_Engine.pfnSetFilterMode(0);
		
		replaced = false;
	}
}