#include "Main.h"

HINSTANCE g_hInstance = nullptr;
WNDCLASSEXA g_WndClass;
HWND g_hWnd = nullptr;
MSG g_Msg;
LicenseInfo g_LicenseInfo;
IDirect3D9* g_pD3D9 = nullptr;
D3DPRESENT_PARAMETERS g_D3DPresentParam;
LPDIRECT3DDEVICE9 g_lpD3D9Device = nullptr;
IDirect3DTexture9* g_pD3DTexture_TitleLogo = nullptr;
IDirect3DTexture9* g_pD3DTexture_Logotype = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void InitializeWindow()
{
	const size_t cbSize = sizeof(WNDCLASSEXA);

	RtlSecureZeroMemory(&g_WndClass, cbSize);

	g_WndClass = { cbSize, CS_CLASSDC, WndProc, 0, 0, g_hInstance, nullptr, 
		LoadCursorA(NULL, IDC_ARROW), nullptr, nullptr, _T("Launcher"), nullptr };

	if (!RegisterClassExA(&g_WndClass))
	{
		MessageBoxA(g_hWnd, "Can not register class", "Error", MB_OK);
		Shutdown();
	}

	const int x = GetSystemMetrics(SM_CXSCREEN) / 2 - WinWidth / 2;
	const int y = GetSystemMetrics(SM_CYSCREEN) / 2 - WinHeight / 2;

	g_hWnd = CreateWindowExA(0, g_WndClass.lpszClassName, _T("Hpp Hack"), WS_POPUP,
		x, y, WinWidth, WinHeight, nullptr, nullptr, g_WndClass.hInstance, nullptr);

	if (!g_hWnd)
	{
		MessageBoxA(g_hWnd, "Can not create window", "Error", MB_OK);
		Shutdown();
	}
}

#define CreateD3DTexture(pDevice, pSrcData, Width, Height, ppTexture) {\
	D3DXCreateTextureFromFileInMemoryEx(pDevice, &pSrcData, sizeof(pSrcData), Width, Height, D3DX_DEFAULT,\
		0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, nullptr, nullptr, ppTexture);\
}

void InitializeDirect3D() noexcept
{
	
	RtlSecureZeroMemory(&g_D3DPresentParam, sizeof(g_D3DPresentParam));

	g_D3DPresentParam.Windowed = TRUE;
	g_D3DPresentParam.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_D3DPresentParam.EnableAutoDepthStencil = TRUE;
	g_D3DPresentParam.AutoDepthStencilFormat = D3DFMT_D16;
	g_D3DPresentParam.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	g_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	g_pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_D3DPresentParam, &g_lpD3D9Device);

	CreateD3DTexture(g_lpD3D9Device, IMG_TitleLogo, 42, 51, &g_pD3DTexture_TitleLogo);
	CreateD3DTexture(g_lpD3D9Device, IMG_Logotype, 229, 290, &g_pD3DTexture_Logotype);
}

void Shutdown()
{
	g_pWindow->Clear();
	ImGui_ImplDX9_Shutdown();
	g_lpD3D9Device->Release();
	g_pD3D9->Release();

	UnregisterClassA(g_WndClass.lpszClassName, g_WndClass.hInstance);
	TerminateProcess(GetCurrentProcess(), 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		Shutdown();
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

bool CheckForUpdates()
{
	std::string sUrl = "/hpp/loader.php?version=" + std::to_string(VERSION);

	const std::string sResponse = GetUrlData(sUrl);

	if (!sResponse.size())
		return false;

	const int iCurrentVersion = std::stoi(sResponse);

	if (iCurrentVersion <= VERSION)
		return false;

	return true;
}

DWORD GetLicenseInfoThread(LPVOID)
{
	char cBuffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(g_hInstance, cBuffer, sizeof(cBuffer));

	strcat(cBuffer, ".tmp");

	if (FileExists(cBuffer))
		remove(cBuffer);

	g_LicenseInfo.m_bIsAviableUpdate = CheckForUpdates();
	g_LicenseInfo.m_sHardwareID = GetHardwareID();
	g_LicenseInfo.m_bIsUpdated = false;

	if (g_LicenseInfo.m_bIsAviableUpdate)
	{
		std::string sResponse = {};
		std::string sFileName = {};

		RtlSecureZeroMemory(cBuffer, sizeof(cBuffer));
		GetModuleFileNameA(g_hInstance, cBuffer, sizeof(cBuffer));

		sFileName = cBuffer;

		if (rename(sFileName.c_str(), std::string(sFileName + ".tmp").c_str()))
		{
			MessageBoxA(g_hWnd, "Can not update launcher #1", "Error", MB_OK);
			Shutdown();
		}

		FILE* pFile = fopen(sFileName.c_str(), "wb+");

		if (pFile == nullptr)
		{
			MessageBoxA(g_hWnd, "Can not update launcher #2", "Error", MB_OK);
			Shutdown();
		}

		sResponse.append(GetUrlData("/hpp/loader.php?download=yes"));

		fwrite(sResponse.c_str(), sizeof(char), sResponse.size(), pFile);
		fclose(pFile);

		sResponse.clear();

		ShellExecuteA(nullptr, nullptr, sFileName.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		Shutdown();
	}

	while (true)
	{
		if (!g.m_bIsDownloadThreadStart)
		{
			if (!g_LicenseInfo.m_bIsGetBuild)
			{
				GetBuilds();
			}
		}

		if (!g_LicenseInfo.m_bIsUpdated)
			g_LicenseInfo.m_bIsUpdated = true;

		Sleep(10000);
	}

	return 0;
}

bool HideThread(HANDLE hThread) noexcept
{
	HMODULE NtDll = GetModuleHandleA("ntdll.dll");

	if (!NtDll)
		return false;

	typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);
	pNtSetInformationThread NtSIT = reinterpret_cast<pNtSetInformationThread>(GetProcAddress(NtDll, "NtSetInformationThread"));

	if (!NtSIT)
		return false;

	const NTSTATUS status = hThread ? NtSIT(hThread, 0x11, nullptr, 0) : NtSIT(GetCurrentThread(), 0x11, nullptr, 0);

	return status == 0;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	g_hInstance = hInstance;

	InitializeWindow();
	InitializeDirect3D();

	if (ImGui_ImplDX9_Init())
	{
		HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)GetLicenseInfoThread, nullptr, 0, nullptr);

		if (hThread && !HideThread(hThread))
		{
			Shutdown();
			return 0;
		}

		g_pWindow->PushWindow("StartPoint", StartPoint);
		g_pWindow->PushWindow("HardwareID", HardwareID);
		g_pWindow->PushWindow("General", General);

		ShowWindow(g_hWnd, SW_SHOWNORMAL);
		UpdateWindow(g_hWnd);

		while (g_Msg.message != WM_QUIT)
		{
			if (PeekMessageA(&g_Msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&g_Msg);
				DispatchMessageA(&g_Msg);
				continue;
			}

			ImGui_ImplDX9_NewFrame();
			ImGui::SetNextWindowPos(ImVec2());
			ImGui::SetNextWindowSize(ImVec2(WinWidth, WinHeight));

			g_pWindow->RenderWindow();

			g_lpD3D9Device->SetRenderState(D3DRS_ZENABLE, false);
			g_lpD3D9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			g_lpD3D9Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
			g_lpD3D9Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(255, 255, 255, 255), 1, 0);

			if (g_lpD3D9Device->BeginScene() >= S_OK)
			{
				ImGui::Render();
				g_lpD3D9Device->EndScene();
			}

			HRESULT result = g_lpD3D9Device->Present(nullptr, nullptr, nullptr, nullptr);

			if (result == D3DERR_DEVICELOST && g_lpD3D9Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			{
				ImGui_ImplDX9_InvalidateDeviceObjects();
				g_lpD3D9Device->Reset(&g_D3DPresentParam);
				ImGui_ImplDX9_CreateDeviceObjects();
			}
		}
	}
	else
		MessageBoxA(g_hWnd, "Can not initialize imgui", "Error", MB_OK);

	Shutdown();

	return 0;
}