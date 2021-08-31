typedef void(*PreS_DynamicSound_t)(int, DWORD, char*, float*, float, float, int, int);
typedef void(*R_DrawEntitiesOnList_t)();

bool HookEngine(void);
void UnHooks(bool a_bFreeLibrary = true);

extern cl_clientfunc_t g_Client, *g_pClient;
extern cl_enginefunc_t g_Engine, *g_pEngine;
extern engine_studio_api_t g_Studio, *g_pStudio;
extern StudioModelRenderer_t g_StudioModelRenderer, *g_pStudioModelRenderer;
extern IRunGameEngine* g_pIRunGameEngine;
extern IGameConsole* g_pConsole;
extern IGameUI* g_pGameUI;
extern playermove_t* pmove;
extern PreS_DynamicSound_t g_pPreS_DynamicSound;
extern StudioModelRenderer_d pCStudioModelRenderer;
extern client_state_t *client_state;
extern client_static_t *client_static;
extern extra_player_info_t *g_PlayerExtraInfo;
extern R_DrawEntitiesOnList_t g_pR_DrawEntitiesOnList;

extern SCREENINFO g_Screen;
extern net_status_s g_Status;

extern void oStudioRenderFinal();
extern void oStudioRenderModel();