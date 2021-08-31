#define _CRT_SECURE_NO_WARNINGS
//#define LICENSING
#define DISABLE_SOCKETING

#include <SDKDDKVer.h>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <deque>
#include <vector>
#include <io.h>
#include <gl\GL.h>
#include <Psapi.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <wbemidl.h>
#include <comutil.h>
#include <ctime>
#include <sstream>
#include <wininet.h>
#include <Winuser.h>
#include <chrono>

#include "SimpleIni/SimpleIni.h"
#include "minhook.h"
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_sdl.h"
#include "ImGui\imgui_internal.h"
#include "SimpleIni\SimpleIni.h"
#include "easywsclient.hpp"

using easywsclient::WebSocket;

#ifdef LICENSING
#include "VMProtectSDK.h"
#define VMP_BEGIN(e) VMProtectBegin(e)
#define VMP_VIRTUAL(e) VMProtectBeginVirtualization(e)
#define VMP_MUTATION(e) VMProtectBeginMutation(e)
#define VMP_ULTRA(e) VMProtectBeginUltra(e)
#define VMP_DecryptA(e) VMProtectDecryptStringA(e)
#define VMP_DecryptW(e) VMProtectDecryptStringW(e)
#define VMP_END VMProtectEnd()
#else
#define VMP_BEGIN(e)
#define VMP_VIRTUAL(e)
#define VMP_MUTATION(e)
#define VMP_ULTRA(e)
#define VMP_DecryptA(e) (e)
#define VMP_DecryptW(e) (e)
#define VMP_END
#endif

#define RAD2DEG(x) ((float)(x) * (float)(180.f / IM_PI))
#define DEG2RAD(x) ((float)(x) * (float)(IM_PI / 180.f))

#include "vector.h"
#include "qangle.h"

#pragma comment(lib, "minhook.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")  
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "SDL2.lib")

#include "ValveSDK.h"
#include "funchook.h"
#include "offsets.h"
#include "utils.h"
#include "globals.h"
#include "security.h"
#include "world.h"
#include "cmd_handling.h"
#include "drawlist.h"
#include "menu_themes.h"
#include "menu_elements.h"
#include "menu.h"
#include "drawgui.h"
#include "settings.h"
#include "cvars.h"
#include "client.h"
#include "opengl.h"
#include "usermsg.h"
#include "visuals.h"
#include "kreedz.h"
#include "weapons.h"
#include "aimbot.h"
#include "nospread.h"
#include "enginemsg.h"
#include "knifebot.h"
#include "miscellaneous.h"
#include "spoofer.h"

extern ImFont* Verdana_16px;
extern ImFont* ProggyClean_13px;