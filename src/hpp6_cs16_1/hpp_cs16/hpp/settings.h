class CSettings
{
public:
	void	RefreshGlobals();
	void	ApplyingGlobals();	

	void    Load(const char* pcszSettingsName);
	void    Save(const char* pcszSettingsName);
	void    Restore(const char* pcszSettingsName, bool bSetAsDefault = false);
	void    Remove(const char* pcszSettingsName);
};

extern CSettings g_Settings;