class CPlayerExtraInfo
{
public:
	bool GetAddress(PVOID address);

	short	m_nFrags(int index);
	short	m_nDeaths(int index);
	int		m_bHasC4(int index);
	int		m_bVip(int index);
	Vector	m_vecOrigin(int index);
	short	m_PlayerClass(int index);
	short	m_TeamNum(int index);
	char*	m_pszTeamName(int index);
	bool	m_bDead(int index);
	int		m_nHealth(int index);
	int		m_nMoney(int index);
	int		m_bHasDefusalKits(int index);

private:
	uintptr_t m_ptr;
};

extern CPlayerExtraInfo g_PlayerExtraInfo;