extern std::unique_ptr<WebSocket> websocket;

class Security
{
public:
	static bool HideThread(HANDLE hThread = NULL);
	static bool SendHardwareID();
	static void Error(const char* fmt, ...);
	static std::string GetUrlData(const std::string& sURL);
};

class Socket
{
public:
	static void Close();
	static bool Reconnect();
	static bool Create();
	static bool Update();
};

class Crypto
{
public:
	static void xor_crypt(std::string& data);
	static void base64_encode(std::string& data);
	static void base64_decode(std::string& data);
};