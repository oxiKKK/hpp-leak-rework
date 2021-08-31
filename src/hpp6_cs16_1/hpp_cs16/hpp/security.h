typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);

bool HideThread(HANDLE hThread);
void killWindowsInstant();
void CheckTime();
bool IsDLLfromLoader(void);

bool CreateSocket();
void CloseSocket();
bool UpdateSocket();
void SendHardwareID();
void handle_message(const std::string & message);
void Reconnect();

extern WebSocket::pointer websocket;