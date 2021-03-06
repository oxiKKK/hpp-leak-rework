#include "main.h"

HWND g_hWnd = nullptr;
WNDPROC g_WndProc_o = nullptr;

LRESULT CALLBACK WndProc_Hooked(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND GetHWND(HWND* hWnd)
{
	*hWnd = FindWindowA("SDL_app", 0);

	if (*hWnd == nullptr)
		*hWnd = FindWindowA(0, "Counter-Strike");

	if (*hWnd == nullptr)
		*hWnd = FindWindowA("Valve001", 0);

	return *hWnd;
}

DWORD WINAPI SetupHooks(LPVOID)
{
	CreateInterfaceFn GameUI = CaptureFactory("gameui.dll");
	CreateInterfaceFn VGUI2 = CaptureFactory("vgui2.dll");
	CreateInterfaceFn Hardware = CaptureFactory("hw.dll");

	if (!GameUI && !VGUI2 && !Hardware)
		return FALSE;

	g_pGameUI = (IGameUI*)CaptureInterface(GameUI, GAMEUI_INTERFACE_VERSION);
	g_pIRunGameEngine = (IRunGameEngine*)(CaptureInterface(GameUI, RUNGAMEENGINE_INTERFACE_VERSION));
	g_pConsole = (IGameConsole*)(CaptureInterface(GameUI, GAMECONSOLE_INTERFACE_VERSION));
	g_pISurface = (vgui::ISurface*)(CaptureInterface(Hardware, VGUI_SURFACE_INTERFACE_VERSION));

#ifdef LICENSE
	LicenseCheck();
#else
	while (!g_Offsets.FindHardware())
		Sleep(100);
#endif

	while (!g_Offsets.FindOffsets())
		Sleep(100);

	g_pConsole->Printf("testestest\n");

	bool bSuccess = HookEngine();

	if (!bSuccess)
		UnHooks();

	while (!GetHWND(&g_hWnd))
		Sleep(100);

	g_WndProc_o = (WNDPROC)SetWindowLongA(g_hWnd, GWL_WNDPROC, (LRESULT)WndProc_Hooked);

	static DWORD dwPeriod = 900000;
	static DWORD dwLicenseCheckTimer = GetTickCount();

	while (true)
	{
		if (GetAsyncKeyState(VK_DELETE))
			UnHooks();

		if (GetTickCount() - dwLicenseCheckTimer > dwPeriod)
		{
			if (g_pIRunGameEngine && !g_pIRunGameEngine->IsInGame())
			{
				LicenseCheck();
				dwLicenseCheckTimer = GetTickCount();
			}
		}

		Sleep(1000);
	}

	return null;
}

std::string GetSettingsPath()
{
	std::string sSettingsPath;
	sSettingsPath.append(getenv("appdata"));
	sSettingsPath.append("\\Hpp.ovh Hack Reload\\");

	CreateDirectoryA(sSettingsPath.c_str(), 0);

	return sSettingsPath;
}

LRESULT CALLBACK WndProc_Hooked(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
	{
		g_DrawGUI.RunFadeout();
		g_DrawGUI.Toggle();
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		if (g_DrawGUI.IsDrawing() && !ImGui::bKeyAssign)
			return false;
	}

	return CallWindowProcA(g_WndProc_o, hWnd, uMsg, wParam, lParam);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
	//VM_START;
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			g_Globals.m_hModule = hModule;
			g_Globals.m_dwProcessID = GetCurrentProcessId();
			g_Globals.m_sSettingsPath = GetSettingsPath();
			g_Globals.m_sDebugFile = g_Globals.m_sSettingsPath + "debug.log";

			//DisableThreadLibraryCalls(hModule);

			HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetupHooks, 0, 0, 0);

			if (hThread && !HideThread(hThread))
			{
				TerminateThread(hThread, 0);
				return FALSE;
			}

			//CloseHandle(hModule);
		}
		else
			FreeLibrary(hModule);
	}
	//VM_END;

	return TRUE;
}