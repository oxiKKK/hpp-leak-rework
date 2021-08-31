#pragma once

enum EFontList
{
	ProggyClean_13px,
	Verdana_1px,
	Verdana_2px,
	Verdana_3px,
	Verdana_4px,
	Verdana_5px,
	Verdana_6px,
	Verdana_7px,
	Verdana_8px,
	Verdana_9px,
	Verdana_10px,
	Verdana_11px,
	Verdana_12px,
	Verdana_13px,
	Verdana_14px,
	Verdana_15px,
	Verdana_16px,
	Verdana_17px,
	Verdana_18px,
	Verdana_19px,
	Verdana_20px,
	Verdana_21px,
	Verdana_22px,
	Verdana_23px,
	Verdana_24px,
	Verdana_25px,
	MAX_FONTS
};

enum EFontOffsetList
{
	FontProggyCleanOffset = 0,
	FontVerdanaOffset = 1
};

enum EImageList
{
	MenuTitleBg,

	TabRageIcon,
	TabLegitIcon,
	TabVisualsIcon,
	TabKreedzIcon,
	TabMiscIcon,
	TabConfigsIcon,
	TabConsoleIcon,

	WpnPistolsIcon,
	WpnSmgsIcon,
	WpnRiflesIcon,
	WpnShotgunsIcon,
	WpnSnipersIcon,

	WPN_NONE,
	WPN_P228,
	WPN_GLOCK,
	WPN_SCOUT,
	WPN_HEGRENADE,
	WPN_XM1014,
	WPN_C4,
	WPN_MAC10,
	WPN_AUG,
	WPN_SMOKEGRENADE,
	WPN_ELITE,
	WPN_FIVESEVEN,
	WPN_UMP45,
	WPN_SG550,
	WPN_GALIL,
	WPN_FAMAS,
	WPN_USP,
	WPN_GLOCK18,
	WPN_AWP,
	WPN_MP5N,
	WPN_M249,
	WPN_M3,
	WPN_M4A1,
	WPN_TMP,
	WPN_G3SG1,
	WPN_FLASHBANG,
	WPN_DEAGLE,
	WPN_SG552,
	WPN_AK47,
	WPN_KNIFE,
	WPN_P90,

	DefuserIcon,

	MAX_IMAGES
};

enum EImageOffsetsList
{
	IconsTabsOffset = 1,
	IconsWpnsOffset = 8,
	IconsEspWpnsOffset = 13
};

struct ImageInfo
{
	ImTextureID data;
	ImVec2 size;
};

extern ImFont* g_pFontList[MAX_FONTS];
extern ImageInfo g_pImageList[MAX_IMAGES];

IMGUI_IMPL_API void ImGui_ImplOpenGL2_Init();
IMGUI_IMPL_API void ImGui_ImplOpenGL2_Shutdown();
IMGUI_IMPL_API void ImGui_ImplOpenGL2_NewFrame();
IMGUI_IMPL_API void ImGui_ImplOpenGL2_RenderDrawData(ImDrawData* draw_data);
IMGUI_IMPL_API bool ImGui_ImplOpenGL2_CreateFontsTexture();
IMGUI_IMPL_API void ImGui_ImplOpenGL2_DestroyFontsTexture();
IMGUI_IMPL_API bool ImGui_ImplOpenGL2_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplOpenGL2_DestroyDeviceObjects();