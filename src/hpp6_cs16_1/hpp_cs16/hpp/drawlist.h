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
	void AddText(float x, float y, const ImColor& col, const float size, FontFlag flags, const char* format, ...);
	void AddLine(const ImVec2& from, const ImVec2& to, const ImColor& col, const float thickness = 1.0F);
	void AddCircle(const ImVec2& position, float radius, const ImColor& col, const int segments = 100);
	void AddCircleFilled(const ImVec2& position, const float radius, const ImColor& col, const int segments = 100);
	void AddRect(const ImVec2& position, const ImVec2& size, const ImColor& col, const float rounding = 0.0F);
	void AddRectFilled(const ImVec2& position, const ImVec2& size, const ImColor& col, const float rounding = 0.0F);
	void AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col, const float thickness = 1.0F);
	void AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col);
	void AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& col, const float thickness = 1.0F);
	void AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& color);
};

class CDrawing : public CDrawList
{
public:
	void RectFilled(const float x, const float y, const float w, const float h, const ImColor& col);
	void Rect(const float x, const float y, const float w, const float h, const ImColor& col);
	void RectOutlined(const float x, const float y, const float w, const float h, const ImColor& col);
	void RectCorners(const float x, const float y, const float w, const float h, const ImColor& col);
	void RectCornersOutlined(const float x, const float y, const float w, const float h, const ImColor& col);
	void RectRound(const float x, const float y, const float w, const float h, const ImColor& col);
	void RectRoundOutlined(const float x, const float y, const float w, const float h, const ImColor& col);
	void LineByOrigin(const Vector& from, const Vector& to, const ImColor& col);
};

extern CDrawing g_Drawing;