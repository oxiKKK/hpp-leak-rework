class CChamsHands : private CRenderModels
{
public:
	bool Draw(cl_entity_s* pGameEntity);

private:
	bool IsHands(cl_entity_s* pGameEntity);

	void DrawHands();
};