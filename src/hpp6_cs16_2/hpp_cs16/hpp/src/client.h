extern cl_clientfunc_t g_Client, *g_pClient;
extern cl_enginefunc_t g_Engine, *g_pEngine;
extern engine_studio_api_t g_Studio, *g_pStudio;
extern r_studio_interface_t g_StudioAPI, * g_pStudioAPI;
extern StudioModelRenderer_t g_StudioModelRenderer, *g_pStudioModelRenderer;
extern playermove_s* pmove;
extern client_state_t* client_state;
extern client_static_t* client_static;
extern StudioModelRenderer_d pCStudioModelRenderer;
extern svc_func_t* g_pClientEngineMsgs;

extern IRunGameEngine* g_pIRunGameEngine;
extern IGameConsole* g_pConsole;
extern IGameUI* g_pIGameUI;
extern IGameUIFuncs* g_pIGameUIFuncs;
extern ISurface* g_pISurface;
extern ISteamClient* g_pISteamClient;
extern ISteamScreenshots* g_pISteamScreenshots;
extern ISteamUtils* g_pISteamUtils;
extern ISteamFriends* g_pISteamFriends;
extern ISteamUser* g_pISteamUser;

extern void CL_RecordHUDCommand(const char* cmdname);
extern void CL_RunUsercmd(local_state_t* from, local_state_t* to, usercmd_t* u, qboolean runfuncs, double* time, unsigned int random_seed);

bool SetupHooks();
void DeleteHooks();

template<class T> static void MemoryAdjust(std::unique_ptr<T>& ptr, bool state = true)
{
	if (state) { MAKE_UNIQUE_PTR(T, ptr); }
	else { DELETE_UNIQUE_PTR(ptr); }
}