typedef unsigned int FontFlags_t;

enum FontFlagList
{
	FontFlags_None = 1 << 0,
	FontFlags_Shadow = 1 << 1,
	FontFlags_Outline = 1 << 2,
	FontFlags_CenterX = 1 << 3,
	FontFlags_CenterY = 1 << 4,
};

struct DrawListLineData
{
	ImVec2 a, b;
	ImColor col;
};

struct DrawListRectData
{
	ImVec2 a, b;
	ImColor col;
	bool outline;
	float rounding;
	int rounding_corners_flags;
	float thickness;
};

struct DrawListRectCornerData
{
	ImVec2 a, b;
	ImColor col;
	bool outline;
};

struct DrawListRectFilledData
{
	ImVec2 a, b;
	ImColor col;
	float rounding;
	int rounding_corners_flags;
};

struct DrawListRectFilledMultiColorData
{
	ImVec2 a, c;
	ImColor col_upr_left;
	ImColor col_upr_right;
	ImColor col_bot_right;
	ImColor col_bot_left;
};

struct DrawListQuadData
{
	ImVec2 a, b;
	ImVec2 c, d;
	ImColor col;
	float thickness;
};

struct DrawListQuadFilledData
{
	ImVec2 a, b;
	ImVec2 c, d;
	ImColor col;
};

struct DrawListTriangleData
{
	ImVec2 a, b, c;
	ImColor col;
	float thickness;
};

struct DrawListTriangleFilledData
{
	ImVec2 a, b, c;
	ImColor col;
};

struct DrawListCircleData
{
	ImVec2 centre;
	float radius;
	ImColor col;
	int num_segments;
	float thickness;
};

struct DrawListCircleFilledData
{
	ImVec2 centre;
	float radius;
	ImColor col;
	int num_segments;
};

struct DrawListTextData
{
	ImVec2 pos;
	ImColor col;
	FontFlags_t flags;
	std::string fmt;
};

struct DrawListTextWrappedData
{
	ImVec2 pos;
	ImColor col;
	float wrap_width;
	FontFlags_t flags;
	std::string fmt;
};

struct DrawListImageData
{
	ImTextureID user_texture_id;
	ImVec2 a, b;
	ImVec2 uv_a, uv_b;
	ImColor col;
};

class CDrawList
{
public:
	void AddLine(const ImVec2& a, const ImVec2& b, const ImColor& col);
	void AddLine(const Vector2D& a, const Vector2D& b, const ImColor& col, float thickness = 1.f);
	void AddRect(const ImVec2& a, const ImVec2& b, const ImColor& col, bool outline = false, float rounding = 0.f, int rounding_corners_flags = ImDrawCornerFlags_All, float thickness = 1.f);
	void AddRectCorner(const ImVec2& a, const ImVec2& b, const ImColor& col, bool outline = false);
	void AddRectFilled(const ImVec2& a, const ImVec2& b, const ImColor& col, float rounding = 0.f, int rounding_corners_flags = ImDrawCornerFlags_All);
	void AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImColor col_upr_left, ImColor col_upr_right, ImColor col_bot_right, ImColor col_bot_left);
	void AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col, float thickness = 1.f);
	void AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col);
	void AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& col, float thickness = 1.f);
	void AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& col);
	void AddCircle(const ImVec2& centre, float radius, const ImColor& col, int num_segments = 12, float thickness = 1.f);
	void AddCircleFilled(const ImVec2& centre, float radius, const ImColor& col, int num_segments = 12);
	void AddText(const ImVec2& pos, const ImColor& col, FontFlags_t flags, const char* fmt, ...);
	void AddTextWrapped(const ImVec2& pos, const ImColor& col, float wrap_width, FontFlags_t flags, const char* fmt, ...);
	void AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a = ImVec2(), const ImVec2& uv_b = ImVec2(1, 1), const ImColor& col = ImColor(IM_COL32_WHITE));

	void PushLine(const ImVec2& a, const ImVec2& b, const ImColor& col);
	void PushRect(const ImVec2& a, const ImVec2& b, const ImColor& col, bool outline = false, float rounding = 0.f, int rounding_corners_flags = ImDrawCornerFlags_All, float thickness = 1.f);
	void PushRectCorner(const ImVec2& a, const ImVec2& b, const ImColor& col, bool outline = false);
	void PushRectFilled(const ImVec2& a, const ImVec2& b, const ImColor& col, float rounding = 0.f, int rounding_corners_flags = ImDrawCornerFlags_All);
	void PushRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImColor col_upr_left, ImColor col_upr_right, ImColor col_bot_right, ImColor col_bot_left);
	void PushQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col, float thickness = 1.f);
	void PushQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col);
	void PushTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& col, float thickness = 1.f);
	void PushTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& col);
	void PushCircle(const ImVec2& centre, float radius, const ImColor& col, int num_segments = 12, float thickness = 1.f);
	void PushCircleFilled(const ImVec2& centre, float radius, const ImColor& col, int num_segments = 12);
	void PushText(const ImVec2& pos, const ImColor& col, FontFlags_t flags, const char* fmt, ...);
	void PushTextWrapped(const ImVec2& pos, const ImColor& col, float wrap_width, FontFlags_t flags, const char* fmt, ...);
	void PushImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a = ImVec2(), const ImVec2& uv_b = ImVec2(1, 1), const ImColor& col = ImColor(IM_COL32_WHITE));

protected:
	using AddLineList_t = std::vector<DrawListLineData>;
	using AddRectList_t = std::vector<DrawListRectData>;
	using AddRectCornerList_t = std::vector<DrawListRectCornerData>;
	using AddRectFilledList_t = std::vector<DrawListRectFilledData>;
	using AddRectFilledMultiColor_t = std::vector<DrawListRectFilledMultiColorData>;
	using AddQuadList_t = std::vector<DrawListQuadData>;
	using AddQuadFilledList_t = std::vector<DrawListQuadFilledData>;
	using AddTriangleList_t = std::vector<DrawListTriangleData>;
	using AddTriangleFilledList_t = std::vector<DrawListTriangleFilledData>;
	using AddCircleList_t = std::vector<DrawListCircleData>;
	using AddCircleFilledList_t = std::vector<DrawListCircleFilledData>;
	using AddTextList_t = std::vector<DrawListTextData>;
	using AddTextWrappedList_t = std::vector<DrawListTextWrappedData>;
	using AddImageList_t = std::vector<DrawListImageData>;

	AddLineList_t m_add_line_list;
	AddRectList_t m_add_rect_list;
	AddRectCornerList_t m_add_rect_corner_list;
	AddRectFilledList_t m_add_rect_filled_list;
	AddRectFilledMultiColor_t m_add_rect_filled_multi_color_list;
	AddQuadList_t m_add_quad_list;
	AddQuadFilledList_t m_add_quad_filled_list;
	AddTriangleList_t m_add_triangle_list;
	AddTriangleFilledList_t m_add_triangle_filled_list;
	AddCircleList_t m_add_circle_list;
	AddCircleFilledList_t m_add_circle_filled_list;
	AddTextList_t m_add_text_list;
	AddTextWrappedList_t m_add_text_wrapped_list;
	AddImageList_t m_add_image_list;
};