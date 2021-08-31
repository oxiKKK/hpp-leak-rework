#include "framework.h"

std::map<std::string, xcommand_t> g_ClientCommandsMap;

static void CL_TakeSnapshot()
{
	if (cvars::visuals.antiscreen)
	{
		g_pGlobals->m_bSnapshot = true;
	}
	else
	{
		g_ClientCommandsMap["snapshot"]();
	}
}

static void CL_TakeScreenshot()
{
	if (cvars::visuals.antiscreen)
	{
		g_pGlobals->m_bScreenshot = true;
	}
	else
	{
		g_ClientCommandsMap["screenshot"]();
	}
}

static void InitClientCommandsMap()
{
	auto* pCmdList = g_Engine.pfnGetCmdList();

	while (pCmdList)
	{
		g_ClientCommandsMap[pCmdList->name] = pCmdList->function;
		pCmdList = pCmdList->next;
	}
}

static bool HookCommand(const std::string& name, const xcommand_t& pfn)
{
	auto* pCmdList = g_Engine.pfnGetCmdList();

	while (pCmdList)
	{
		if (!name.compare(pCmdList->name))
		{
			pCmdList->function = pfn;
			return true;
		}

		pCmdList = pCmdList->next;
	}

	Utils::TraceLog(V("> %s: failed to hook %s.\n"), V(__FUNCTION__), name.c_str());
	return false;
}

static bool UnHookCommand(const std::string& name)
{
	auto* pCmdList = g_Engine.pfnGetCmdList();

	while (pCmdList)
	{
		if (!name.compare(pCmdList->name))
		{
			pCmdList->function = g_ClientCommandsMap[name];
			return true;
		}

		pCmdList = pCmdList->next;
	}

	Utils::TraceLog(V("> %s: failed to unhook %s.\n"), V(__FUNCTION__), name.c_str());
	return false;
}

bool HookCommands()
{
	InitClientCommandsMap();

	if (!HookCommand("snapshot", CL_TakeSnapshot))
		return false;

	if (!HookCommand("screenshot", CL_TakeSnapshot))
		return false;

	return true;
}

void UnHookCommands()
{
	UnHookCommand("snapshot");
	UnHookCommand("screenshot");
}