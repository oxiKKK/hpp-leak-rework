class CDrawList;
class CRenderScene;

class CRenderScene 
{
public:
    void Begin();
	void Render();
    void End();
};

extern CRenderScene g_RenderScene;

typedef int FontFlag;

enum FontFlags 
{
    FontFlags_None = 1 << 0,
    FontFlags_Shadow = 1 << 1,
    FontFlags_Outline = 1 << 2,
    FontFlags_CenterX = 1 << 3,
    FontFlags_CenterY = 1 << 4
};


class CDrawList 
{
public:
    void AddText(float x, float y, const ImColor& color, float fontSize, FontFlag flags, const char* format, ...);
    void AddLine(const ImVec2& from, const ImVec2& to, const ImColor& color, float thickness = 1.f);
    void AddCircle(const ImVec2& position, float radius, const ImColor& color, int segments = 100);
    void AddCircleFilled(const ImVec2& position, float radius, const ImColor& color, int segments = 100);
    void AddRect(const ImVec2& position, const ImVec2& size, const ImColor& color, float rounding = 0.f);
    void AddRectFilled(const ImVec2& position, const ImVec2& size, const ImColor& color, float rounding = 0.f);
    void AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& color, float thickness = 1.f);
    void AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& color);
	void AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& color);

    void DrawFillArea(float x, float y, float w, float h, const ImColor& color);
    void DrawBox(float x, float y, float w, float h, const ImColor& color);
    void DrawBoxOutline(float x, float y, float w, float h, const ImColor& color);
    void DrawCornerBox(float x, float y, float w, float h, const ImColor& color);
    void DrawCornerBoxOutline(float x, float y, float w, float h, const ImColor& color);
    void DrawRoundBox(float x, float y, float w, float h, const ImColor& color);
    void DrawRoundBoxOutline(float x, float y, float w, float h, const ImColor& color);
    void DrawHitbox(float* pflOrigin, float* pflVecScreen, const ImColor& color);
};

extern CDrawList g_DrawList;