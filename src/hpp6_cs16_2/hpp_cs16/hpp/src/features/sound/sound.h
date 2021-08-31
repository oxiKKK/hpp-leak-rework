struct sound_resolver_data
{
	int orig_index;
	int resolved_index;
};

class CSound
{
public:
	~CSound();

	bool IsValidSound(char* pszSoundFile, float* pOrigin, int index, DWORD channel);
	bool IsPlayerSound(char* pszSoundFile);
	bool IsWeaponSound(char* pszSoundFile);
	bool IsHitSound(char* pszSoundFile);
	bool IsFilterSound(char* pszSoundFile);

	void UpdateDormantPlayer(int index, Vector origin);
	void UpdatePlayerArmor(int index, char* pszSoundFile);
	void UpdatePlayerWeapon(int index, char* pszSoundFile);
	void UpdatePlayerHealth(int index, char* pszSoundFile);

	void GetResolvedIndex(int index, Vector origin, int& resolved);
	void ClearResolvedPlayer(int index);
	void ClearResolverData();	

private:
	std::vector<sound_resolver_data> m_sound_resolver;
};

extern std::unique_ptr<CSound> g_pSound;