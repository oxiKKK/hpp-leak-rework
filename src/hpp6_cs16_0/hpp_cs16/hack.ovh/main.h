#include "gsl\gsl_algorithm.h" // copy
#include "gsl\gsl_assert.h"    // Ensures/Expects
#include "gsl\gsl_byte.h"      // byte
#include "gsl\gsl_util.h"      // finally()/narrow()/narrow_cast()...
#include "gsl\multi_span.h"    // multi_span, strided_span...
#include "gsl\pointers.h"      // owner, not_null
#include "gsl\span.h"          // span
#include "gsl\string_span.h"   // zstring, string_span, zstring_builder...

#include <SDKDDKVer.h>
#include <windows.h>
#include <deque>
#include <vector>
#include <fstream>
#include <Psapi.h>
#include <string>
#include <io.h>
#include <gl\GL.h>
#include <wininet.h>
#include <wbemidl.h>
#include <comutil.h>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <tlhelp32.h>

#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_sdl.h"
#include "ImGui\imgui_internal.h"

#include "SimpleIni\SimpleIni.h"

#define RAD2DEG(x) ((float)(x) * (float)(180 / IM_PI))
#define DEG2RAD(x) ((float)(x) * (float)(IM_PI / 180))

#define M_PI 3.14159265358979323846

typedef unsigned int uint;

//#include "smaz\smaz.h"
#include "vector.h"
#include "qangle.h"
#include "ValveSDK.h"
#include "security.h"
#include "minhook.h"
#include "Themida\ThemidaSDK.h"
#include "offsets.h"
#include "settings.h"
#include "client.h"
#include "drawlist.h"
#include "opengl.h"
#include "hooks.h"
#include "globals.h"
#include "world.h"
#include "utils.h"
#include "drawgui.h"
#include "usermsg.h"
#include "svcmsg.h"
#include "weapons.h"
#include "visuals.h"
#include "nospread.h"
#include "aimbot.h"
#include "cvars.h"
#include "misc.h"
#include "knifebot.h"
#include "kreedz.h"

#pragma comment(lib, "minhook.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "comsuppw.lib")  
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ws2_32.lib")

extern HWND g_hWnd;
extern WNDPROC g_WndProc_o;

extern ImFont* Verdana_16px;
extern ImFont* ProggyClean_13px;

//#define LICENSE