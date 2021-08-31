int MSG_TeamInfo(const char* pszName, int iSize, void* pbuf);
int MSG_ResetHUD(const char* pszName, int iSize, void* pbuf);
int MSG_Battery(const char* pszName, int iSize, void* pbuf);
int MSG_BombDrop(const char* pszName, int iSize, void* pbuf);
int MSG_Health(const char* pszName, int iSize, void* pbuf);
int MSG_DeathMsg(const char* pszName, int iSize, void* pbuf);
int MSG_SetFOV(const char* pszName, int iSize, void* pbuf);

bool HookUserMsg(const char *pszMsgName, pfnUserMsgHook pfn);
bool UnHookUserMsg(const char *pszMsgName);
void HookUserMessages();
void UnHookUserMessages();

extern UserMsg g_pClientUserMsgs;