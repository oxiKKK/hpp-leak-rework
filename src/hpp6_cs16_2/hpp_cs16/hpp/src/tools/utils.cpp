#include "framework.h"

std::string Utils::GetExeDirectory(HMODULE hModule)
{
	char szFileName[MAX_PATH] = { 0 };

	if (!GetModuleFileName(hModule, szFileName, MAX_PATH))
		return {};

	return szFileName;
}

bool Utils::FileExists(std::string path)
{
	return _access(path.c_str(), 0) != -1;
}

std::string Utils::FormatString(const char* fmt, ...)
{
	char buf[4096] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, fmt, args);
	va_end(args);

	return buf;
}

void Utils::TraceLog(const char* fmt, ...)
{
	char buf[4096] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, fmt, args);
	va_end(args);

	char timestring[9];
	_strtime_s(timestring);

	FILE* fp = fopen(g_pGlobals->m_sDebugFile.c_str(), "a+");

	if (fp)
	{
		fprintf(fp, FormatString("[%s] %s", timestring, buf).c_str());
		fclose(fp);
	}
	
	if (g_pConsole && !cvars::visuals.streamer_mode)
		g_pConsole->DPrintf(buf);

	g_Console.AddLog(FormatString("[%s] %s", timestring, buf).c_str());
}


double Utils::Duration(double current, double previous)
{
	return (current - previous) / (double)CLOCKS_PER_SEC;
}

void Utils::StringReplace(char* buf, const char* search, const char* replace)
{
	if (!strlen(buf) || !strlen(search))
		return;

	char* p = buf;

	const auto l1 = strlen(search);
	const auto l2 = strlen(replace);

	while ((p = strstr(p, search)) != 0)
	{
		memmove(p + l2, p + l1, strlen(p + l1) + 1U);
		memcpy(p, replace, l2);

		p += l2;
	}
}

std::string Utils::GenRandomString()
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	char s[50];
	for (size_t i = 0; i < sizeof(s) - 1; i++)
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	s[49] = 0;

	return std::string(s);
}

void Utils::RainbowColor(float speed, float alpha, float* out)
{
	if (out[0] > 0.00f && out[2] <= 0.00f)
	{
		out[0] -= speed * g_pGlobals->m_flFrameTime;
		out[1] += speed * g_pGlobals->m_flFrameTime;
	}
	else if (out[1] > 0.00f && out[0] <= 0.00f)
	{
		out[1] -= speed * g_pGlobals->m_flFrameTime;
		out[2] += speed * g_pGlobals->m_flFrameTime;
	}
	else if (out[2] > 0.00f && out[1] <= 0.00f)
	{
		out[0] += speed * g_pGlobals->m_flFrameTime;
		out[2] -= speed * g_pGlobals->m_flFrameTime;
	}
	else
	{
		out[0] = 1.00f;
		out[1] = 0.00f;
		out[2] = 0.00f;
	}

	out[3] = alpha;
}

static string_array& split(std::string s, char delim, string_array& elems)
{
	std::stringstream ss(s);
	std::string item;

	while (std::getline(ss, item, delim))
		elems.push_back(item);

	return elems;
}

string_array Utils::ParseString(std::string s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}