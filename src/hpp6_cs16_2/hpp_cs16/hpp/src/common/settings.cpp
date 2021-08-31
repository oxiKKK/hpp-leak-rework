#include "framework.h"

void CSettings::PushHandleFile(const SettingsFileData& data)
{
	m_FilesData.push_back(data);
}

void CSettings::ProcessFile(const SettingsFileData& data, const std::string& settings_path)
{
	const std::string path = settings_path + data.path;

	m_SimpleIni.LoadFile(path.c_str());

	data.handle_func();

	if (m_operation == SAVE_SETTINGS)
		m_SimpleIni.SaveFile(path.c_str());

	m_SimpleIni.Reset();
}

void CSettings::Load(const std::string& name)
{
	if (m_FilesData.empty())
		return;

	Utils::TraceLog("[event]: Loading settings with name '%s'.\n", name.c_str());

	const std::string settings_path = g_pGlobals->m_sSettingsPath + name;

	CreateDirectory(settings_path.c_str(), 0);

	m_operation = LOAD_SETTINGS;

	for (auto& data : m_FilesData)
		ProcessFile(data, settings_path);

	m_operation = SAVE_SETTINGS;

	for (auto& data : m_FilesData)
	{
		if (!Utils::FileExists(settings_path + data.path))
			ProcessFile(data, settings_path);
	}
}

void CSettings::Save(const std::string& name)
{
	if (m_FilesData.empty())
		return;

	Utils::TraceLog("[event]: Saving settings with name '%s'.\n", name.c_str());

	const std::string settings_path = g_pGlobals->m_sSettingsPath + name;

	CreateDirectory(settings_path.c_str(), 0);

	m_operation = SAVE_SETTINGS;

	for (auto& data : m_FilesData)
		ProcessFile(data, settings_path);
}

void CSettings::Reset(const std::string& name, bool as_default)
{
	Remove(name);

	as_default ? Load(name) : Save(name);
}

void CSettings::Remove(const std::string& name)
{
	Utils::TraceLog("[event]: Removing settings with name '%s'.\n", name.c_str());

	const std::string settings_path = g_pGlobals->m_sSettingsPath + name;

	const DWORD dwAttrib = GetFileAttributes(settings_path.c_str());

	if (dwAttrib == INVALID_FILE_ATTRIBUTES)
		return;

	if (~dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
		return;

	SetLastError(0);

	if (RemoveDirectory(settings_path.c_str()))
		return;

	if (GetLastError() != ERROR_DIR_NOT_EMPTY)
		return;

	if (settings_path.length() + strlen("\\*.*") + 1 > MAX_PATH)
		return;

	char buf[MAX_PATH] = { '\0' };

	sprintf_s(buf, "%s\\*.*", settings_path.c_str());

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(buf, &FindFileData);

	if (!hFind || hFind == INVALID_HANDLE_VALUE)
		return;

	strcpy_s(buf, settings_path.c_str());

	do
	{
		if (!strcmp(FindFileData.cFileName, "."))
			continue;

		if (!strcmp(FindFileData.cFileName, ".."))
			continue;

		if (strlen(buf) + strlen(FindFileData.cFileName) + 1 > MAX_PATH)
			continue;

		char szCurrentFile[MAX_PATH] = { '\0' };
		sprintf_s(szCurrentFile, "%s\\%s", buf, FindFileData.cFileName);

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
				FindFileData.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributes(szCurrentFile, FindFileData.dwFileAttributes);
			}

			Remove(szCurrentFile);
		}
		else
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) || (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
				SetFileAttributes(szCurrentFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFile(szCurrentFile);
		}

	} while (FindNextFile(hFind, &FindFileData));

	FindClose(hFind);

	if (GetLastError() == ERROR_NO_MORE_FILES)
		RemoveDirectory(settings_path.c_str());
}

static void StringToFloatArray(const std::string& input, float* value, size_t size)
{
	const string_array array = Utils::ParseString(input, ',');

	for (size_t i = 0; i < size; i++)
		value[i] = std::stof(array[i]);
}

static std::string FloatArrayToString(float* value, size_t size)
{
	std::string string;

	for (size_t i = 0; i < size; i++)
		string.append(Utils::FormatString("%.0f,", value[i]));

	return string.erase(string.length() - 1);
}

static void ColorByteToFloat(float(&value)[4])
{
	const size_t size = IM_ARRAYSIZE(value);

	for (size_t i = 0; i < size; i++)
		value[i] /= 255.f;
}

static void ColorFloatToByte(float(&value)[4])
{
	const size_t size = IM_ARRAYSIZE(value);

	for (size_t i = 0; i < size; i++)
		value[i] *= 255.f;
}

static std::string ColorToString(float(&value)[4])
{
	ColorFloatToByte(value);

	std::string ret = FloatArrayToString(value, IM_ARRAYSIZE(value));

	ColorByteToFloat(value);

	return ret;
}

static std::string FormatKeyString(std::string& key)
{
	return "|" + key;
}

void CSettings::Section(const std::string& section)
{
	m_section = section;
}

void CSettings::Key(std::string key, bool& value, bool def)
{
	switch (m_operation)
	{
	case LOAD_SETTINGS: value = m_SimpleIni.GetBoolValue(m_section.c_str(), FormatKeyString(key).c_str(), def); break;
	case SAVE_SETTINGS: m_SimpleIni.SetBoolValue(m_section.c_str(), FormatKeyString(key).c_str(), value);
	}
}

void CSettings::Key(std::string key, int& value, int def)
{
	switch (m_operation)
	{
	case LOAD_SETTINGS: value = static_cast<int>(m_SimpleIni.GetLongValue(m_section.c_str(), FormatKeyString(key).c_str(), def)); break;
	case SAVE_SETTINGS: m_SimpleIni.SetLongValue(m_section.c_str(), FormatKeyString(key).c_str(), value);
	}
}

void CSettings::Key(std::string key, float& value, float def)
{
	switch (m_operation)
	{
	case LOAD_SETTINGS: value = static_cast<float>(m_SimpleIni.GetDoubleValue(m_section.c_str(), FormatKeyString(key).c_str(), def)); break;
	case SAVE_SETTINGS: m_SimpleIni.SetDoubleValue(m_section.c_str(), FormatKeyString(key).c_str(), value);
	}
}

void CSettings::Key(std::string key, char* value, const char* def)
{
	switch (m_operation)
	{
	case LOAD_SETTINGS: strcpy(value, m_SimpleIni.GetValue(m_section.c_str(), FormatKeyString(key).c_str(), def)); break;
	case SAVE_SETTINGS: m_SimpleIni.SetValue(m_section.c_str(), FormatKeyString(key).c_str(), value);
	}
}

void CSettings::Key(std::string key, float(&value)[4], const char* def)
{
	switch (m_operation)
	{
	case LOAD_SETTINGS:
		StringToFloatArray(m_SimpleIni.GetValue(m_section.c_str(), FormatKeyString(key).c_str(), def), value, IM_ARRAYSIZE(value));
		ColorByteToFloat(value);
		break;
	case SAVE_SETTINGS: m_SimpleIni.SetValue(m_section.c_str(), FormatKeyString(key).c_str(), ColorToString(value).c_str());
	}
}

void CSettings::Key(std::string key, float(&value)[3], const char* def)
{
	switch (m_operation)
	{
	case LOAD_SETTINGS: StringToFloatArray(m_SimpleIni.GetValue(m_section.c_str(), FormatKeyString(key).c_str(), def), value, 2); value[2] = 1.f; break;
	case SAVE_SETTINGS: m_SimpleIni.SetValue(m_section.c_str(), FormatKeyString(key).c_str(), FloatArrayToString(value, 2).c_str());
	}
}

void CSettings::Key(std::string key, bind_t& value, bind_t def)
{
	switch (m_operation)
	{
	case LOAD_SETTINGS:
	{
		const std::string def_string = std::to_string(def.keynum) + "," + std::to_string(def.type);
		const std::string bind = m_SimpleIni.GetValue(m_section.c_str(), FormatKeyString(key).c_str(), def_string.c_str());
		const std::size_t pos = bind.find(',');

		if (pos == std::string::npos)
		{
			memset(&value, 0, sizeof(value));
			return;
		}

		value.keynum = std::stoi(bind.substr(0, pos));
		value.type = std::stoi(bind.substr(pos + 1));
		break;
	}
	case SAVE_SETTINGS:
	{
		const std::string bind_string = std::to_string(value.keynum) + "," + std::to_string(value.type);
		m_SimpleIni.SetValue(m_section.c_str(), FormatKeyString(key).c_str(), bind_string.c_str());
	}
	}
}

std::unique_ptr<CSettings> g_pSettings;