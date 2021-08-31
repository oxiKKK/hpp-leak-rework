BOOL InstallHooks(void);
void DeleteHooks(void);
void R_DrawEntitiesOnList_Hooked(void);
void CL_RecordHUDCommand_Hooked(const char *cmdname);

typedef void(*PreS_DynamicSound_t)(int, DWORD, char*, float*, float, float, int, int);
typedef void(*R_DrawEntitiesOnList_t)();
typedef void(*CL_RecordHUDCommand_t)(const char *cmdname);
typedef int(*InitiateGameConnection_t)(void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure);

extern cl_clientfunc_t g_Client, *g_pClient;
extern cl_enginefunc_t g_Engine, *g_pEngine;
extern engine_studio_api_t g_Studio, *g_pStudio;
extern StudioModelRenderer_t g_StudioModelRenderer, *g_pStudioModelRenderer;
extern IRunGameEngine* g_pIRunGameEngine;
extern IGameConsole* g_pConsole;
extern IGameUI* g_pIGameUI;
extern playermove_t* pmove;
extern PreS_DynamicSound_t g_pPreS_DynamicSound;
extern R_DrawEntitiesOnList_t g_pR_DrawEntitiesOnList;
extern StudioModelRenderer_dt* pCStudioModelRenderer;
extern client_state_t *client_state;
extern client_static_t *client_static;
extern extra_player_info_t *g_PlayerExtraInfo;
extern char **keybindings;
extern particle_t **particles;
extern ISteamClient *g_pISteamClient;
extern ISteamScreenshots *g_pISteamScreenshots;
extern ISteamUtils *g_pISteamUtils;
extern ISteamFriends *g_pISteamFriends;
extern ISteamUser *g_pISteamUser;
extern CL_RecordHUDCommand_t g_pCL_RecordHUDCommand;
extern InitiateGameConnection_t g_pInitiateGameConnection;

extern SCREENINFO g_Screen;
extern net_status_s g_Status;

extern WNDPROC g_pWndProc;

extern std::deque<my_sound_t> g_Sounds;
extern std::deque<CIncomingSequence> sequences;