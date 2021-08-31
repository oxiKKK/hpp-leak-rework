#include "framework.h"

#include "imgui.h"
#include "imgui_impl_opengl2.h"
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

// Include OpenGL header (without an OpenGL loader) requires a bit of fiddling
#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY __stdcall                  // It is customary to use APIENTRY for OpenGL function pointer declarations on all platforms.  Additionally, the Windows OpenGL header needs APIENTRY.
#endif
#if defined(_WIN32) && !defined(WINGDIAPI)
#define WINGDIAPI __declspec(dllimport)     // Some Windows OpenGL headers need this
#endif
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "overlay/images/menu/background_title.h"
#include "overlay/images/menu/tab_rage_icon.h"
#include "overlay/images/menu/tab_legit_icon.h"
#include "overlay/images/menu/tab_visuals_icon.h"
#include "overlay/images/menu/tab_kreedz_icon.h"
#include "overlay/images/menu/tab_misc_icon.h"
#include "overlay/images/menu/tab_configs_icon.h"
#include "overlay/images/menu/tab_console_icon.h"
#include "overlay/images/menu/wpn_pistols_icon.h"
#include "overlay/images/menu/wpn_rifles_icon.h"
#include "overlay/images/menu/wpn_snipers_icon.h"
#include "overlay/images/menu/wpn_shotguns_icon.h"
#include "overlay/images/menu/wpn_smgs_icon.h"

#include "overlay/images/weapons/weapon_ak47.h"
#include "overlay/images/weapons/weapon_aug.h"
#include "overlay/images/weapons/weapon_awp.h"
#include "overlay/images/weapons/weapon_c4.h"
#include "overlay/images/weapons/weapon_deagle.h"
#include "overlay/images/weapons/weapon_elite.h"
#include "overlay/images/weapons/weapon_famas.h"
#include "overlay/images/weapons/weapon_fiveseven.h"
#include "overlay/images/weapons/weapon_flashbang.h"
#include "overlay/images/weapons/weapon_g3sg1.h"
#include "overlay/images/weapons/weapon_galil.h"
#include "overlay/images/weapons/weapon_glock18.h"
#include "overlay/images/weapons/weapon_hegrenade.h"
#include "overlay/images/weapons/weapon_knife.h"
#include "overlay/images/weapons/weapon_m249.h"
#include "overlay/images/weapons/weapon_m4a1.h"
#include "overlay/images/weapons/weapon_m3.h"
#include "overlay/images/weapons/weapon_mac10.h"
#include "overlay/images/weapons/weapon_mp5.h"
#include "overlay/images/weapons/weapon_p228.h"
#include "overlay/images/weapons/weapon_p90.h"
#include "overlay/images/weapons/weapon_scout.h"
#include "overlay/images/weapons/weapon_sg550.h"
#include "overlay/images/weapons/weapon_sg552.h"
#include "overlay/images/weapons/weapon_smokegrenade.h"
#include "overlay/images/weapons/weapon_tmp.h"
#include "overlay/images/weapons/weapon_ump45.h"
#include "overlay/images/weapons/weapon_usp.h"
#include "overlay/images/weapons/weapon_xm1014.h"

#include "overlay/images/defuser/defuser.h"

GLuint g_FontTexture = 0;
ImFont* g_pFontList[MAX_FONTS];
ImageInfo g_pImageList[MAX_IMAGES];

void ImGui_ImplOpenGL2_Init()
{
    GImGui->IO.BackendRendererName = "imgui_impl_opengl2";
	GImGui->IO.RenderDrawListsFn = ImGui_ImplOpenGL2_RenderDrawData;

	GImGui->Style.AntiAliasedFill = true;
	GImGui->Style.AntiAliasedLines = true;
	GImGui->Style.WindowBorderSize = 1.f;
	GImGui->Style.WindowRounding = 0.f;
	GImGui->Style.FramePadding = ImVec2(4, 3);
	GImGui->Style.ItemSpacing = ImVec2(8, 4);
	GImGui->Style.ItemInnerSpacing = ImVec2(6, 4);
	GImGui->Style.TouchExtraPadding = ImVec2();
	GImGui->Style.IndentSpacing = 21.f;
	GImGui->Style.ScrollbarSize = 7.f;
	GImGui->Style.WindowTitleAlign = ImVec2(0.015f, 0.5f);

	ImFontConfig config;

	//config.OversampleH = 3;
	//config.OversampleV = 3;
	config.RasterizerMultiply = 1.125f;

	std::string sFontPath = g_pGlobals->m_sSystemDisk + ":\\Windows\\Fonts\\verdana.ttf";

	g_pFontList[ProggyClean_13px] = GImGui->IO.Fonts->AddFontDefault();

	for (int verdana_fontnum = 1; verdana_fontnum <= 25; verdana_fontnum++)
	{
		g_pFontList[verdana_fontnum] = GImGui->IO.Fonts->AddFontFromFileTTF(sFontPath.c_str(), 
			static_cast<float>(verdana_fontnum), &config, GImGui->IO.Fonts->GetGlyphRangesCyrillic());
	}
}

void ImGui_ImplOpenGL2_Shutdown()
{
    ImGui_ImplOpenGL2_DestroyDeviceObjects();
}

void ImGui_ImplOpenGL2_NewFrame()
{
    if (!g_FontTexture)
        ImGui_ImplOpenGL2_CreateDeviceObjects();
}

static void ImGui_ImplOpenGL2_SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(draw_data->DisplayPos.x, draw_data->DisplayPos.x + draw_data->DisplaySize.x, draw_data->DisplayPos.y + draw_data->DisplaySize.y, draw_data->DisplayPos.y, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void ImGui_ImplOpenGL2_RenderDrawData(ImDrawData* draw_data)
{
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;

    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

    ImGui_ImplOpenGL2_SetupRenderState(draw_data, fb_width, fb_height);

    ImVec2 clip_off = draw_data->DisplayPos; 
    ImVec2 clip_scale = draw_data->FramebufferScale;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (pcmd->UserCallback)
            {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplOpenGL2_SetupRenderState(draw_data, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
                }
            }

            idx_buffer += pcmd->ElemCount;
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]); glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

static void CreateTexture(unsigned char* data, int size, ImageInfo* image_info)
{
	GLuint texture;
	int width, height;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	unsigned char* image = SOIL_load_image_from_memory(data, size, &width, &height, 0, SOIL_LOAD_RGBA);

	image_info->size.x = static_cast<float>(width);
	image_info->size.y = static_cast<float>(height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)image_info->size.x, (GLsizei)image_info->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	image_info->data = (ImTextureID)(intptr_t)texture;
}

bool ImGui_ImplOpenGL2_CreateFontsTexture()
{
    unsigned char* pixels;
    int width, height;

    GImGui->IO.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    GLint last_texture;

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	GImGui->IO.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

	CreateTexture(menu_title_bytes, sizeof(menu_title_bytes), &g_pImageList[MenuTitleBg]);

	CreateTexture(rage_icon_bytes, sizeof(rage_icon_bytes), &g_pImageList[TabRageIcon]);
	CreateTexture(legit_icon_bytes, sizeof(legit_icon_bytes), &g_pImageList[TabLegitIcon]);
	CreateTexture(visuals_icon_bytes, sizeof(visuals_icon_bytes), &g_pImageList[TabVisualsIcon]);
	CreateTexture(kreedz_icon_bytes, sizeof(kreedz_icon_bytes), &g_pImageList[TabKreedzIcon]);
	CreateTexture(misc_icon_bytes, sizeof(misc_icon_bytes), &g_pImageList[TabMiscIcon]);
	CreateTexture(configs_icon_bytes, sizeof(configs_icon_bytes), &g_pImageList[TabConfigsIcon]);
	CreateTexture(console_icon_bytes, sizeof(console_icon_bytes), &g_pImageList[TabConsoleIcon]);

	CreateTexture(wpn_pistols_icon_bytes, sizeof(wpn_pistols_icon_bytes), &g_pImageList[WpnPistolsIcon]);
	CreateTexture(wpn_rifles_icon_bytes, sizeof(wpn_rifles_icon_bytes), &g_pImageList[WpnRiflesIcon]);
	CreateTexture(wpn_snipers_icon_bytes, sizeof(wpn_snipers_icon_bytes), &g_pImageList[WpnSnipersIcon]);
	CreateTexture(wpn_shotguns_icon_bytes, sizeof(wpn_shotguns_icon_bytes), &g_pImageList[WpnShotgunsIcon]);
	CreateTexture(wpn_smgs_icon_bytes, sizeof(wpn_smgs_icon_bytes), &g_pImageList[WpnSmgsIcon]);

	CreateTexture(weapon_ak47_bytes, sizeof(weapon_ak47_bytes), &g_pImageList[WPN_AK47]);
	CreateTexture(weapon_aug_bytes, sizeof(weapon_aug_bytes), &g_pImageList[WPN_AUG]);
	CreateTexture(weapon_awp_bytes, sizeof(weapon_awp_bytes), &g_pImageList[WPN_AWP]);
	CreateTexture(weapon_c4_bytes, sizeof(weapon_c4_bytes), &g_pImageList[WPN_C4]);
	CreateTexture(weapon_deagle_bytes, sizeof(weapon_deagle_bytes), &g_pImageList[WPN_DEAGLE]);
	CreateTexture(weapon_elite_bytes, sizeof(weapon_elite_bytes), &g_pImageList[WPN_ELITE]);
	CreateTexture(weapon_famas_bytes, sizeof(weapon_famas_bytes), &g_pImageList[WPN_FAMAS]);
	CreateTexture(weapon_fiveseven_bytes, sizeof(weapon_fiveseven_bytes), &g_pImageList[WPN_FIVESEVEN]);
	CreateTexture(weapon_flashbang_bytes, sizeof(weapon_flashbang_bytes), &g_pImageList[WPN_FLASHBANG]);
	CreateTexture(weapon_g3sg1_bytes, sizeof(weapon_g3sg1_bytes), &g_pImageList[WPN_G3SG1]);
	CreateTexture(weapon_galil_bytes, sizeof(weapon_galil_bytes), &g_pImageList[WPN_GALIL]);
	CreateTexture(weapon_glock18_bytes, sizeof(weapon_glock18_bytes), &g_pImageList[WPN_GLOCK18]);
	CreateTexture(weapon_hegrenade_bytes, sizeof(weapon_hegrenade_bytes), &g_pImageList[WPN_HEGRENADE]);
	CreateTexture(weapon_knife_bytes, sizeof(weapon_knife_bytes), &g_pImageList[WPN_KNIFE]);
	CreateTexture(weapon_m249_bytes, sizeof(weapon_m249_bytes), &g_pImageList[WPN_M249]);
	CreateTexture(weapon_m4a1_bytes, sizeof(weapon_m4a1_bytes), &g_pImageList[WPN_M4A1]);
	CreateTexture(weapon_m3_bytes, sizeof(weapon_m3_bytes), &g_pImageList[WPN_M3]);
	CreateTexture(weapon_mac10_bytes, sizeof(weapon_mac10_bytes), &g_pImageList[WPN_MAC10]);
	CreateTexture(weapon_mp5_bytes, sizeof(weapon_mp5_bytes), &g_pImageList[WPN_MP5N]);
	CreateTexture(weapon_p228_bytes, sizeof(weapon_p228_bytes), &g_pImageList[WPN_P228]);
	CreateTexture(weapon_p90_bytes, sizeof(weapon_p90_bytes), &g_pImageList[WPN_P90]);
	CreateTexture(weapon_scout_bytes, sizeof(weapon_scout_bytes), &g_pImageList[WPN_SCOUT]);
	CreateTexture(weapon_sg550_bytes, sizeof(weapon_sg550_bytes), &g_pImageList[WPN_SG550]);
	CreateTexture(weapon_sg552_bytes, sizeof(weapon_sg552_bytes), &g_pImageList[WPN_SG552]);
	CreateTexture(weapon_smokegrenade_bytes, sizeof(weapon_smokegrenade_bytes), &g_pImageList[WPN_SMOKEGRENADE]);
	CreateTexture(weapon_tmp_bytes, sizeof(weapon_tmp_bytes), &g_pImageList[WPN_TMP]);
	CreateTexture(weapon_ump45_bytes, sizeof(weapon_ump45_bytes), &g_pImageList[WPN_UMP45]);
	CreateTexture(weapon_usp_bytes, sizeof(weapon_usp_bytes), &g_pImageList[WPN_USP]);
	CreateTexture(weapon_xm1014_bytes, sizeof(weapon_xm1014_bytes), &g_pImageList[WPN_XM1014]);

	CreateTexture(defuser_bytes, sizeof(defuser_bytes), &g_pImageList[DefuserIcon]);

    glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

void ImGui_ImplOpenGL2_DestroyFontsTexture()
{
    if (g_FontTexture)
    {
        glDeleteTextures(1, &g_FontTexture);
		GImGui->IO.Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

bool ImGui_ImplOpenGL2_CreateDeviceObjects()
{
    return ImGui_ImplOpenGL2_CreateFontsTexture();
}

void ImGui_ImplOpenGL2_DestroyDeviceObjects()
{
    ImGui_ImplOpenGL2_DestroyFontsTexture();
}
