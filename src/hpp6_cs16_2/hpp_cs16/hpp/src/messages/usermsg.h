extern std::map<std::string, pfnUserMsgHook> g_ClientUserMsgsMap;

void InitClientUserMsgMap();

bool HookUserMessages();
void UnHookUserMessages();