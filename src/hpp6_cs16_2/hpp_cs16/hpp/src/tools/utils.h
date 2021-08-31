using string_array = std::vector<std::string>;

class Utils
{
public:
	static std::string		GetExeDirectory(HMODULE hModule = NULL);
	static bool				FileExists(std::string path);
	static std::string		FormatString(const char* fmt, ...);
	static void				TraceLog(const char* fmt, ...);
	static double			Duration(double current, double previous);
	static void				StringReplace(char* buf, const char* search, const char* replace);
	static std::string		GenRandomString();
	static void				RainbowColor(float speed, float alpha, float* out);
	static string_array		ParseString(std::string s, char delim);
};