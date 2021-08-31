typedef int OperationFlag_t;

enum OperationFlags
{
	LOAD_SETTINGS,
	SAVE_SETTINGS
};

struct SettingsFileData
{
	std::function<void()> handle_func;
	std::string path;
};

class CSettings
{
public:
	void PushHandleFile(const SettingsFileData& data);

	void Load(const std::string& name);
	void Save(const std::string& name);
	void Reset(const std::string& name, bool as_default = false);
	void Remove(const std::string& name);

	void Section(const std::string& section);

	void Key(std::string key, bool& value, bool def = false);
	void Key(std::string key, int& value, int def = 0);
	void Key(std::string key, float& value, float def = 0.f);
	void Key(std::string key, char* value, const char* def = "");
	void Key(std::string key, float(&value)[4], const char* def = "255,255,255,255");
	void Key(std::string key, float(&value)[3], const char* def = "100,100");
	void Key(std::string key, bind_t& value, bind_t def = bind_t{ 0, 0 });

private:
	std::vector<SettingsFileData> m_FilesData;

	CSimpleIni m_SimpleIni;

	OperationFlag_t m_operation;
	std::string m_section;

	void ProcessFile(const SettingsFileData& data, const std::string& settings_path);
};

extern std::unique_ptr<CSettings> g_pSettings;