namespace Menu
{
	enum TabsList
	{
		Tab_RageBot,
		Tab_LegitBot,
		Tab_Visuals,
		Tab_Kreedz,
		Tab_Misc,
		Tab_GUI,
		Tab_Settings
	};

	enum WeaponGroupsList
	{
		WeaponGroup_Pistol,
		WeaponGroup_SubMachineGun,
		WeaponGroup_Rifle,
		WeaponGroup_Shotgun,
		WeaponGroup_Sniper
	};
	
	enum FilesList
	{
		File_RageBot,
		File_LegitBot,
		File_Visuals,
		File_Kreedz,
		File_Misc,
		File_Gui
	};

	static const char* const pcszFilesList[] = 
	{ 
		"ragebot.ini", 
		"legitbot.ini", 
		"visuals.ini", 
		"kreedz.ini", 
		"misc.ini", 
		"gui.ini" 
	};

	class Tab
	{
	public:
		static TabsList GetCurrentTab();
		static void ClearCurrentWeaponId();

		static bool Labels();
		static void RageBot();
		static void LegitBot();
		static void Visuals();
		static void Misc();
		static void Kreedz();
		static void GUI();
		static void Settings();

	private:
		static TabsList m_CurrentTabId;
		static WeaponIdType m_CurrentWeaponId;
	};

	class Settings
	{
	public:
		static void	Load();
		static void Save();
		static void	RestorePopupOpen();
		static void SaveAsPopupOpen();
		static void RemovePopupOpen();
		static void RenamePopupOpen();
		static void OpenSettingsFolder();
		static void OpenSettingsFile();
		static void OpenSettingsPath();
	};

	class ModalPopup
	{
	public:
		static void SettingsSaveAs();
		static void SettingsRename();
		static void SettingsRemove();
		static void SettingsRestore();
	};

	struct window_parameters_s
	{
		ImVec2	WindowSize;
		ImVec2	ModalSize;
		ImVec2	ModalButtonSize;
		char*	Title;
		float	ChildWidth;
		float	NextChildPosX;
		float	ChildCenterPos;
		float	ItemWidth;
	};

	void RefreshSettings();

	void DrawCursor();
	void DrawMenuFade();
	void DrawMenu();
	void DrawPlayerList();

	extern char szSettingsBuffer[40];
	extern unsigned int iSelectedSettings;
	extern unsigned int iSelectedIniFile;
	extern std::vector<std::string> sSettingsList;
	extern bool bUpdateFileContent;
}