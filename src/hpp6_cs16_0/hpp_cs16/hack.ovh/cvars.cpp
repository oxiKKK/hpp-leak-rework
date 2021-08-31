#include "main.h"

std::map<std::string, xcommand_t> g_ClientCommandsMap;

void HookCommands()
{
	HookCommand("snapshot", CL_TakeSnapshot);
	HookCommand("screenshot", CL_TakeSnapshot);
}

void UnHookCommands()
{
	UnHookCommand("snapshot");
	UnHookCommand("screenshot");
}

void AntiScreen()
{
	static int iTimer = 10;

	if (g_Globals.m_bSnapshot || g_Globals.m_bScreenshot)
	{
		iTimer--;

		if (iTimer <= 0)
		{
			if (g_Globals.m_bSnapshot)
			{
				g_ClientCommandsMap["snapshot"]();
				g_Globals.m_bSnapshot = false;
			}
			else if (g_Globals.m_bScreenshot)
			{
				g_ClientCommandsMap["screenshot"]();
				g_Globals.m_bScreenshot = false;
			}

			iTimer = 10;
		}
	}
}

void CL_TakeSnapshot()
{
	g_Globals.m_bSnapshot = true;
}

void CL_TakeScreenshot()
{
	g_Globals.m_bScreenshot = true;
}

bool HookCommand(const char *pszName, xcommand_t pfn)
{
	auto pCmdList = g_Engine.pfnGetCmdList();
	while (pCmdList)
	{
		if (!strcmp(pCmdList->name, pszName))
		{
			g_ClientCommandsMap[pszName] = pCmdList->function;
			pCmdList->function = pfn;
			return true;
		}

		pCmdList = pCmdList->next;
	}

	TraceLog("> %s: failed to hook %s.\n", __FUNCTION__, pszName);

	return false;
}

bool UnHookCommand(const char *pszName)
{
	auto pCmdList = g_Engine.pfnGetCmdList();
	while (pCmdList)
	{
		if (!strcmp(pCmdList->name, pszName))
		{
			pCmdList->function = g_ClientCommandsMap[pszName];
			g_ClientCommandsMap[pszName] = nullptr;
			return true;
		}

		pCmdList = pCmdList->next;
	}

	TraceLog("> %s: failed to unhook %s.\n", __FUNCTION__, pszName);
	return false;
}