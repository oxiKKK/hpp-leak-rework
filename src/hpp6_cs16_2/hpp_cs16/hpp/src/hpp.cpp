// hpp.cpp: Defines the entry point for the applicaton.

#include "framework.h"

constexpr auto sleep_time = std::chrono::milliseconds::duration(100);

bool FindOffsets()
{
	auto pOffsets = std::make_unique<COffsets>();

	auto time_counter = std::chrono::milliseconds::duration(0);

	while (true)
	{
		std::this_thread::sleep_for(sleep_time);

		time_counter += sleep_time;

		if (time_counter > std::chrono::seconds::duration(10))
			return false;

		if (!pOffsets->FindModules())
			continue;

		if (!pOffsets->FindInterfaces())
			continue;

		break;
	}

	return pOffsets->FindGameOffsets();
}

bool EntryPoint()
{
	if (!FindOffsets())
		return false;

	return true;
}


void MainThread(HINSTANCE hInstance)
{
	if (EntryPoint())
	{

		g_pConsole->DPrintf("> %s: m_hModule: 0x%X\n", __FUNCTION__, g_pGlobals->m_hModule);
		g_pConsole->DPrintf("> %s: m_hWnd: 0x%X\n", __FUNCTION__, g_pGlobals->m_hWnd);
		g_pConsole->DPrintf("> %s: m_dwProcessID: %i\n", __FUNCTION__, g_pGlobals->m_dwProcessId);
		g_pConsole->DPrintf("> %s: m_sSettingsPath: %s\n", __FUNCTION__, g_pGlobals->m_sSettingsPath.c_str());
		g_pConsole->DPrintf("> %s: m_sDebugFile: %s\n", __FUNCTION__, g_pGlobals->m_sDebugFile.c_str());
		g_pConsole->DPrintf("> %s: m_bIsUnloadingLibrary: %i\n", __FUNCTION__, g_pGlobals->m_bIsUnloadingLibrary);

		if (g_pGlobals->m_iGameBuild <= 8308)
		{
			while (g_pGlobals->m_bIsGameHooked)
			{
				if (g_pGlobals->m_bIsUnloadingLibrary)
					break;
			}
		}
		else
		{
			Utils::TraceLog(V("> Version of the game is outdated.\n"));
		}

		if (SetupHooks())
			return;
	}

	DeleteHooks();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return FALSE;

		g_pGlobals = std::make_unique<CGlobals>();

		if (!g_pGlobals->m_hWnd || g_pGlobals->m_hWnd == INVALID_HANDLE_VALUE)
			return FALSE;

		g_pGlobals->m_hModule = hinstDLL;

		//DisableThreadLibraryCalls(hinstDLL);
		//
		//std::thread main_thread(MainThread, hinstDLL);

		CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)MainThread,
			0,
			0,
			NULL);

		return TRUE;
	}

	return FALSE;
}