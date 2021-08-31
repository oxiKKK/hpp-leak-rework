class CVisuals
{
public:
	CVisuals();

	bool m_bThirdPersonState;

	void CreateEntities();

	bool Chams(cl_entity_s* pGameEntity);
	bool Glow(cl_entity_s* pGameEntity);
	void Light();
	void LineSight();
	void Overlay();
	
private:
	std::unique_ptr<CChamsPlayers> m_pChamsPlayers;
	std::unique_ptr<CChamsHitPosition> m_pChamsHitPosition;
	std::unique_ptr<CChamsDesyncAA> m_pChamsDesyncAA;
	std::unique_ptr<CChamsBacktrack> m_pChamsBacktrack;
	std::unique_ptr<CChamsHands> m_pChamsHands;
	std::unique_ptr<CDlight> m_pDlight;
	std::unique_ptr<CElight> m_pElight;
	std::unique_ptr<CDrawPlayers> m_pDrawPlayers;
	std::unique_ptr<CGlowPlayers> m_pGlowPlayers;
	std::unique_ptr<CDrawWorld> m_pDrawWorld;
	std::unique_ptr<CDrawLocal> m_pDrawLocal;
};

extern std::unique_ptr<CVisuals> g_pVisuals;