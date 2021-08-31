#include "main.h"
 
std::map<std::string, xcommand_t> g_ClientCommandsMap;

CCvars cvar;
CStates state;

extern void Scroll_JumpPressed(void);
extern void Scroll_JumpReleased(void);

CStates::CStates() 
{ 
	RtlSecureZeroMemory(this, sizeof(*this)); 
}

CCvars::CCvars()
{
	RtlSecureZeroMemory(this, sizeof(*this));
}

static void CL_TakeSnapshot()
{
	g_Globals.m_bSnapshot = true;
}

static void CL_TakeScreenshot()
{
	g_Globals.m_bScreenshot = true;
}

static void JumpPressed()
{
	Scroll_JumpPressed();

	g_ClientCommandsMap["+jump"]();
}

static void JumpReleased()
{
	Scroll_JumpReleased();

	g_ClientCommandsMap["-jump"]();
}

bool HookCommands(void)
{
	if (!HookCommand("snapshot", CL_TakeSnapshot))
		return false;

	if (!HookCommand("screenshot", CL_TakeSnapshot))
		return false;

	if (!HookCommand("+jump", JumpPressed))
		return false;

	if (!HookCommand("-jump", JumpReleased))
		return false;

	return true;
}

void UnHookCommands(void)
{
	UnHookCommand("snapshot");
	UnHookCommand("screenshot");
	UnHookCommand("-jump");
	UnHookCommand("+jump");
}

void AntiScreen()
{
	static auto timer = 10;

	if (g_Globals.m_bSnapshot || g_Globals.m_bScreenshot)
	{
		if ((--timer) <= 0)
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

			timer = 10;
		}
	}
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