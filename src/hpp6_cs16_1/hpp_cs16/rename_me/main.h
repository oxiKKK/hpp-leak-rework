#define _CRT_SECURE_NO_WARNINGS
#define LICENSING

#include "gsl\gsl_algorithm.h" // copy
#include "gsl\gsl_assert.h"    // Ensures/Expects
#include "gsl\gsl_byte.h"      // byte
#include "gsl\gsl_util.h"      // finally()/narrow()/narrow_cast()...
#include "gsl\multi_span.h"    // multi_span, strided_span...
#include "gsl\pointers.h"      // owner, not_null
#include "gsl\span.h"          // span
#include "gsl\string_span.h"   // zstring, string_span, zstring_builder...

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <wbemidl.h>
#include <comutil.h>
#include <ctime>
#include <sstream>
#include <tlhelp32.h>
#include <deque>
#include <vector>
#include <memory>
#include <d3dx9/d3dx9.h>
#include <d3d9.h>
#include <io.h>
#include <tchar.h>
#include <regex>
#include <WinInet.h>

#pragma comment(lib, "ws2_32")

#if defined(LICENSING)
#include "VMProtectSDK.h"
#endif

#if defined(LICENSING)
#define VM	VMProtectBeginMutation
#define VV	VMProtectBeginVirtualization
#define VU	VMProtectBeginUltra
#define VE	VMProtectEnd
#define V	VMProtectDecryptStringA
#define W_V VMProtectDecryptStringW
#else
#define VM(e)
#define VV(e)
#define VU(e)
#define VE() 
#define V(e) e
#define W_V(e) e
#endif

extern HINSTANCE g_hInstance;
extern WNDCLASSEXA g_WndClass;
extern HWND g_hWnd;
extern LPDIRECT3DDEVICE9 g_lpD3D9Device;
extern IDirect3DTexture9* g_pD3DTexture_TitleLogo;
extern IDirect3DTexture9* g_pD3DTexture_Logotype;

#include "includes/easywsclient/easywsclient.hpp"

using easywsclient::WebSocket;

#include "IMG_TitleLogo.h"
#include "IMG_LogoType.h"

#include "ImGui\imgui.h"
#include "ImGui\imgui_internal.h"
#include "ImGui\imgui_impl_dx9.h"

#include "window.h"
#include "build.h"
#include "license.h"
#include "WindowForm.h"

extern ImFont* Lucida11px;
extern ImFont* Lucida18px;

struct LicenseInfo
{
	bool m_bIsUserValid;
	bool m_bIsUpdated;
	bool m_bForceExit;
	DWORD m_dwProcessId;
	std::string m_sHWID;
	std::string m_sMainText;
};

extern LicenseInfo g_LicenseInfo;

#define WinWidth	700
#define WinHeight	350

#define VERSION		71

__inline bool FileExists(const char* _FileName) noexcept
{
	return _access(_FileName, 0) != -1;
}

void	Shutdown();