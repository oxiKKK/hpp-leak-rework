typedef int RenderType_t;

enum RenderType
{
	FLAT = 1,
	DARKENED,
	LIGHTED,
	TEXTURE,
};

enum ColorHealthBased
{
	ColorHealthBased_Disable,
	ColorHealthBased_OnVisible,
	ColorHealthBased_Always
};

enum ColoredModelsMarkers
{
	LOCAL_DESYNC_MARKER = 228,
	PLAYER_MARKER = 1337,
	BACKTRACK_MARKER = 1488,
	HIT_POSITION_MARKER = 777,
	LOCAL_PLAYER_FOR_GAITYAW = 696,
};

class CRenderModels
{
public:
	static bool IsWeaponModelRender(cl_entity_s* pGameEntity);

	static void StudioDrawPlayer(cl_entity_s* pGameEntity);
	static void StudioRenderFinal();

protected:
	void SetupRenderer(cl_entity_s* pGameEntity);
	void RestoreRenderer(cl_entity_s* pGameEntity);

	void SetRenderType(RenderType_t renderType, bool wireframe = false);

	void SetVisible();
	void SetHidden();

	void DormantColor(int index, float* flRenderColor);
};