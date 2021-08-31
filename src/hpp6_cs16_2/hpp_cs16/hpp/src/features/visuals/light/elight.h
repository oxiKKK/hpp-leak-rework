class CElight
{
public:
	void Draw();

private:
	bool IsPlayer(int index);

	void DrawLight(cl_entity_s* pGameEntity);
};