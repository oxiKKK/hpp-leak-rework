// hpp.h: include file for standart system include files or include files for specific project.

#pragma once

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely used components from Windows headers.

#include <Windows.h>
#include <TlHelp32.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <mutex>
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <deque>
#include <map>
#include <ctime>
#include <thread>
#include <functional>
#include <wininet.h>
#include <wbemidl.h>
#include <comutil.h>
#include <tchar.h>
#include <io.h>
#include <shellapi.h>
#include <assert.h>
#include <bcrypt.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fcntl.h>
#include <regex>
#include <psapi.h>
#include <gl/GL.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib") 

//#define LICENSING
#define DEBUG

#if defined(LICENSING)
#include "VMProtectSDK\VMProtectSDK.h"
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

#define MAKE_UNIQUE_PTR(c, p) if (!p.get()) p = std::make_unique<c>()

#define DELETE_PTR(p) delete p; p = nullptr
#define DELETE_ARR_PTR(p) delete[] p; p = nullptr
#define DELETE_UNIQUE_PTR(p) if (p.get()) p.reset()

#define IS_NULLPTR(p) (p.get() == nullptr)

#define DEBUG_VALUE_FLOAT(num, step, def)\
	static float debug_value = def;\
	g_Engine.Con_NPrintf(num, "Debug value float: %.3f", debug_value);\
	if (GetAsyncKeyState(VK_ADD) & 0x1) debug_value += step;\
	if (GetAsyncKeyState(VK_SUBTRACT) & 0x1) debug_value -= step;

#define DEBUG_VALUE_INT(num, step, def)\
	static int debug_value = def;\
	g_Engine.Con_NPrintf(num, "Debug value int: %i", debug_value);\
	if (GetAsyncKeyState(VK_ADD) & 0x1) debug_value += step;\
	if (GetAsyncKeyState(VK_SUBTRACT) & 0x1) debug_value -= step;

#include "ImGui/imconfig.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_user.h"
#include "ImGui/imgui_impl_opengl2.h"
#include "ImGui/imgui_impl_win32.h"

#define RAD2DEG(x) ((float)(x) * (float)(180.f / IM_PI))
#define DEG2RAD(x) ((float)(x) * (float)(IM_PI / 180.f))

#include "math/vector.h"
#include "math/qangle.h"
#include "math/matrix3x4.h"

#include "ValveSDK/sdk.h"

using namespace vgui;

#include "MinHook/MinHook.h"
#include "MinHook/funchook.h"
#include "VmtHook/VmtHook.h"


#include "Soil/glcorearb.h"
#include "Soil/soil.h"

#include "SimpleIni/SimpleIni.h"

#include "common/cvars.h"
#include "common/cmds.h"
#include "common/globals.h"
#include "common/offsets.h"
#include "common/settings.h"

#include "tools/build.h"
#include "tools/math.h"
#include "tools/game.h"
#include "tools/physent.h"
#include "tools/utils.h"

#include "messages/usermsg.h"
#include "messages/enginemsg.h"

#include "world/hud_player_info.h"
#include "world/extra_player_info.h"
#include "world/baseinfo.h"
#include "world/baselocal.h"
#include "world/baseplayer.h"
#include "world/baseentity.h"
#include "world/baseweapon.h"
#include "world/hitregister.h"
#include "world/sequences.h"

#include "renderer/drawlist.h"
#include "renderer/renderer.h"

#include "features/legitbot/legitbot.h"
#include "features/knifebot/knifebot.h"
#include "features/nospread/nospread.h"
#include "features/miscellaneous/miscellaneous.h"
#include "features/miscellaneous/sandbox.h"
#include "features/miscellaneous/revemu.h"
#include "features/ragebot/ragebot.h"
#include "features/sound/sound.h"
#include "features/visuals/crosshair/default_crosshair.h"
#include "features/visuals/overlay/selection_players.h"
#include "features/visuals/overlay/draw_players_out_of_fov.h"
#include "features/visuals/overlay/draw_players_sound.h"
#include "features/visuals/overlay/draw_players.h"
#include "features/visuals/overlay/draw_world.h"
#include "features/visuals/overlay/draw_local.h"
#include "features/visuals/glow/glow_players.h"
#include "features/visuals/chams/render_models.h"
#include "features/visuals/chams/chams_players.h"
#include "features/visuals/chams/chams_hit_position.h"
#include "features/visuals/chams/chams_desync_aa.h"
#include "features/visuals/chams/chams_backtrack.h"
#include "features/visuals/chams/chams_hands.h"
#include "features/visuals/light/dlight.h"
#include "features/visuals/light/elight.h"
#include "features/visuals/visuals.h"
#include "features/kreedz/kreedz.h"

#include "overlay/notifications/notifications.h"
#include "overlay/menu/gui/menu_fade_bg.h"
#include "overlay/menu/gui/menu_popup_modal.h"
#include "overlay/menu/gui/menu_gui.h"
#include "overlay/menu/gui/menu_themes.h"
#include "overlay/menu/common/menu_settings_list.h"
#include "overlay/menu/common/menu_hotkeys.h"
#include "overlay/menu/menu.h"

#include "client.h"
#include "opengl.h"