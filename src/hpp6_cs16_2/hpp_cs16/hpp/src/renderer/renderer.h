class CRenderer : public CDrawList
{
public:
	CRenderer();
	~CRenderer();

	void RenderScene(std::function<void()> RenderFn);
};

extern std::unique_ptr<CRenderer> g_pRenderer;