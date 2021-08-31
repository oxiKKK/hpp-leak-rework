class CPlayerInfoList
{
public:
	char*	m_pszName(int index);
	short	m_nPing(int index);
	byte	m_nPacketLoss(int index);
	uint64	m_nSteamID(int index);

private:
	hud_player_info_t GetPlayerInfo(int index);
};

extern CPlayerInfoList g_PlayerInfoList;