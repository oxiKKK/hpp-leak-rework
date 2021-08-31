class CBasePlayer
{
public:
	void AllocateMemory();
	void FreeMemory();

	void Update();
	void Clear();

	CBaseEntInfo* GetBaseInfo(int index);
	CBaseEntInfo* operator[](int index);

private:
	std::unique_ptr<CBaseEntInfo> pBaseInfo[MAX_CLIENTS];

	bool IsPlayerDead(cl_entity_s* pGameEntity);

	void UpdatePlayer(CBaseEntInfo* pPlayer, cl_entity_s* pGameEntity);
	void ClearPlayer(CBaseEntInfo* pPlayer);
};

extern CBasePlayer g_Player;