#include "main.h"

CKreedz g_Kreedz;

void CKreedz::Run(usercmd_s* cmd)
{
	if (cvar.kreedz == 1)
	{
		RageBunnyHop(cmd);
	}
	else if (cvar.kreedz == 2)
	{

	}
}

void CKreedz::RageBunnyHop(usercmd_s* cmd)
{
	if (cvar.ragekreedz_bunnyhop)
	{
		if (cmd->buttons&IN_JUMP)
		{
			cmd->buttons &= ~IN_JUMP;

			if (pmove->flags & FL_ONGROUND || pmove->waterlevel >= 2)
			{
				cmd->buttons |= IN_JUMP;
			}
		}
	}
}