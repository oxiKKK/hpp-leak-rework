#include "framework.h"

std::unique_ptr<CMiscellaneous> g_pMiscellaneous;

CMiscellaneous::CMiscellaneous()
{
	// run code one time when you connected to server
}

CMiscellaneous::~CMiscellaneous()
{
	// run code one time when you disconnected from server
}

void CMiscellaneous::NameStealer()
{
	if (!cvars::misc.namestealer)
		return;

	static const std::string english[] = { "A", "a", "E", "e", "O", "o", "X", "x", "C", "c", "B", "K", "H", "P", "p", "T", "M" };
	static const std::string russian[] = { "À", "à", "Å", "å", "Î", "î", "Õ", "õ", "Ñ", "ñ", "Â", "Ê", "Í", "Ð", "ð", "Ò", "Ì" };

	static auto previous_time = client_state->time;

	if (abs(client_state->time - previous_time) > (double)cvars::misc.namestealer_interval)
	{
		previous_time = client_state->time;

		std::deque<std::string> nicknames;

		for (int i = 1; i <= client_state->maxclients; i++)
		{
			if (g_Player[i]->m_ClassId == EClassEntity_BaseLocal)
				continue;

			if (!g_Player[i]->m_bIsConnected)
				continue;

			if (g_Player[i]->m_iTeamNum == TEAM_SPECTATOR || g_Player[i]->m_iTeamNum == TEAM_UNASSIGNED)
				continue;

			if (strlen(g_Player[i]->m_szPrintName))
				nicknames.push_back(g_Player[i]->m_szPrintName);
		}

		while (nicknames.size())
		{
			bool replaced = false;

			int random = g_Engine.pfnRandomLong(0, nicknames.size() - 1);

			assert(random >= 0 && random < nicknames.size());

			std::string nickname = nicknames[random];

			// English to russian
			for (size_t j = 0; j < IM_ARRAYSIZE(english); j++)
			{
				auto pos = nickname.find(english[j]);

				if (pos != std::string::npos)
				{
					nickname = nickname.replace(pos, english[j].size(), russian[j]);
					replaced = true;
					break;
				}
			}
			// Russian to english
			if (!replaced)
			{
				for (size_t j = 0; j < IM_ARRAYSIZE(russian); j++)
				{
					auto pos = nickname.find(russian[j]);

					if (pos != std::string::npos)
					{
						nickname = nickname.replace(pos, russian[j].size(), english[j]);
						replaced = true;
						break;
					}
				}
			}

			if (replaced)
			{
				std::string cmd = "name \"" + nickname + "\"";
				g_Engine.pfnClientCmd(cmd.c_str());
				break;
			}

			nicknames.erase(nicknames.begin() + random);
		}
	}
}

void CMiscellaneous::FakeLatency()
{
	m_bFakeLatencyActive = false;

	if (cvars::misc.fakelatency)
	{
		const auto latency = cvars::misc.fakelatency_amount / 1000.0;

		if (latency > 0.0)
		{
			m_bFakeLatencyActive = true;

			Game::SetFakeLatency(latency);
		}
	}
}

void CMiscellaneous::ChokedCommandsCounter()
{
	static int previous_seq = -1;

	if (client_static->netchan.outgoing_sequence != previous_seq)
	{
		if (client_static->nextcmdtime == FLT_MAX)
			m_iChokedCommands++;
		else if (g_pMiscellaneous->m_iChokedCommands)
			m_iChokedCommands = 0;

		previous_seq = client_static->netchan.outgoing_sequence;
	}

	//g_pEngine->Con_NPrintf(7, "g_pMiscellaneous->m_iChokedCommands: %i", g_pMiscellaneous->m_iChokedCommands);
}

void CMiscellaneous::AutoReload(usercmd_s* cmd)
{
	if (cvars::misc.automatic_reload && g_Weapon.IsGun())
	{
		if (cmd->buttons & IN_ATTACK && g_Weapon->m_iClip < 1)
		{
			cmd->buttons &= ~IN_ATTACK;
			cmd->buttons |= IN_RELOAD;
		}
	}
}

void CMiscellaneous::AutoPistol(usercmd_s* cmd)
{
	if (cvars::misc.automatic_pistol && g_Weapon.IsPistol())
	{
		if (cmd->buttons & IN_ATTACK && !g_Weapon.CanAttack())
		{
			cmd->buttons &= ~IN_ATTACK;
		}
	}
}

void CMiscellaneous::RecordHUDCommands(usercmd_s* cmd)
{
	if (client_static->demorecording)
	{
		if (cmd->buttons & IN_ATTACK && (~m_iHudCommands & IN_ATTACK))
		{
			CL_RecordHUDCommand("+attack");
			m_iHudCommands |= IN_ATTACK;
		}
		else if (!(cmd->buttons & IN_ATTACK) && m_iHudCommands & IN_ATTACK)
		{
			CL_RecordHUDCommand("-attack");
			m_iHudCommands &= ~IN_ATTACK;
		}

		if (cmd->buttons & IN_ATTACK2 && (~m_iHudCommands & IN_ATTACK2))
		{
			CL_RecordHUDCommand("+attack2");
			m_iHudCommands |= IN_ATTACK2;
		}
		else if (!(cmd->buttons & IN_ATTACK2) && m_iHudCommands & IN_ATTACK2)
		{
			CL_RecordHUDCommand("-attack2");
			m_iHudCommands &= ~IN_ATTACK2;
		}
	}
}

float CMiscellaneous::GetInterpAmount(const int& lerp)
{
	assert(lerp >= 0 && lerp <= 100);

	const float maxmove = g_Local->m_flFrameTime * 0.05;

	float diff = (lerp / 1000.0) - m_flPositionAdjustmentInterpAmount;

	diff = std::clamp(diff, -maxmove, maxmove);

	const float interp = (m_flPositionAdjustmentInterpAmount + diff);

	return interp;
}