class CIni
{
private:
	CSimpleIniA*		_Ini;
	const char*			_pcszSection;

	// Convert float array to write result string to file.			
	static std::string	_ColorToString(float(&a_flColor)[4]);
	static std::string	_ColorToString(float(&a_flColor)[3]);

	// Convert byte format (255) to float (1.0).
	static void			_ColorByteToFloat(float(&a_flColor)[4]) noexcept;
	static void			_ColorByteToFloat(float(&a_flColor)[3]) noexcept;

	// Convert float format (1.0) to byte (255).
	static void			_ColorFloatToByte(float(&a_flColor)[4]) noexcept;
	static void			_ColorFloatToByte(float(&a_flColor)[3]) noexcept;

	// Convert string to array.
	static void			_StringToArray(const char* a_pcszString, float(&a_flVariable)[3]) noexcept;
	static void			_StringToArray(const char* a_pcszString, float(&a_flVariable)[4]) noexcept;
	static void			_StringToArray(const char* a_pcszString, int(&a_iVariable)[2]) noexcept;

protected:
	void				SetFile(CSimpleIniA* a_Ini) noexcept;
	void				SetSection(const char* a_pcszSection) noexcept;

	bool				GetBoolValue(std::string a_sKey, bool a_bDefault = false);
	int					GetIntValue(std::string a_sKey, int a_iDefault = 0);
	float				GetFloatValue(std::string a_sKey, float a_flDefault = 0.f);
	void				GetStringColor(std::string a_sKey, float(&a_flColor)[4], const char* a_pcszDefault = "255,255,255,255");
	void				GetStringColor(std::string a_sKey, float(&a_flColor)[3], const char* a_pcszDefault = "255,255,255");

	void				SetBoolValue(std::string a_sKey, bool a_bValue = false);
	void				SetIntValue(std::string a_sKey, int a_iValue = 0);
	void				SetFloatValue(std::string a_sKey, float a_flValue = 0.f);
	void				SetStringColor(std::string a_sKey, float(&a_flColor)[4]);
	void				SetStringColor(std::string a_sKey, float(&a_flColor)[3]);

	void				Clear() noexcept;
};

class CSettings : public CIni
{
private:
	enum IniFiles
	{
		IniFile_RageBot,
		IniFile_LegitBot,
		IniFile_Visuals,
		IniFile_Kreedz,
		IniFile_Misc,
		IniFile_GUI,
		IniFile_MAX_FILES
	};

	enum SettingsParts
	{
		SettingsPart_RageBot = 1 << 0,
		SettingsPart_LegitBot = 1 << 1,
		SettingsPart_Visuals = 1 << 2,
		SettingsPart_Kreedz = 1 << 3,
		SettingsPart_Misc = 1 << 4,
		SettingsPart_GUI = 1 << 5,
		SettingsPart_All = 1 << 6
	};

	static const char*	_pcszIniFile[IniFile_MAX_FILES];
	static CSimpleIniA	_Ini[IniFile_MAX_FILES];

	static std::string	_GetSection(unsigned int a_nWeaponIndex);
public:
	void				Load(const char* a_pcszSettingsName, unsigned int a_iSettingsPart = SettingsPart_All);
	void				Save(const char* a_pcszSettingsName, unsigned int a_iSettingsPart = SettingsPart_All);
	void				Restore(const char* a_pcszSettingsName, bool a_bSetAsDefault = false);
	bool				Delete(const char* a_szSettingsName);
};

extern CSettings g_Settings;