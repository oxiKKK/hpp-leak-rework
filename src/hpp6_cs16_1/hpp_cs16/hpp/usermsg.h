#define PLAYER_DEAD (1<<0)
#define PLAYER_HAS_C4 (1<<1)
#define PLAYER_VIP (1<<2)

int MSG_TeamInfo(const char* pszName, int iSize, void* pbuf);
int MSG_ResetHUD(const char* pszName, int iSize, void* pbuf);
int MSG_Battery(const char* pszName, int iSize, void* pbuf);
int MSG_BombDrop(const char* pszName, int iSize, void* pbuf);
int MSG_Health(const char* pszName, int iSize, void* pbuf);
int MSG_DeathMsg(const char* pszName, int iSize, void* pbuf);
int MSG_SetFOV(const char* pszName, int iSize, void* pbuf);
int MSG_MOTD(const char* pszName, int iSize, void* pbuf);
int MSG_ScoreAttrib(const char* pszName, int iSize, void* pbuf);

bool HookUserMsg(const char *pszMsgName, pfnUserMsgHook pfn);
bool UnHookUserMsg(const char *pszMsgName);
bool HookUserMessages(void);
void UnHookUserMessages(void);

extern UserMsg g_pClientUserMsgs;
extern std::map<std::string, pfnUserMsgHook> g_ClientUserMsgsMap;