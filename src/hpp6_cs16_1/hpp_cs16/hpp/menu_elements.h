#define COMBO_ARRAY(arr) arr, IM_ARRAYSIZE(arr)

namespace ImGui
{
	void	Spacing(const size_t count);
	float	GetSameLinePadding();

	class Control
	{
	public:
		static void			SetStringIdentify(const char* str_id);

		static void			PushCondition(const bool condition);
		static void			PopCondition(size_t count = 1);

		static void			PushTextColor(const ImVec4& color);
		static void			PopTextColor(size_t count = 1);

		static void			Text(const char* fmt, ...);
		static bool			Checkbox(bool* v, const char* label, const char* message = (const char*)0);
		static bool			Combo(int* current_item, const char* label, const char* const items[], const int size, const char* message = (const char*)0);
		static bool			SliderFloat(float* v, const char* label, const float min, const float max, const char* format = "%.3f", const float power = 1.0F, const char* message = (const char*)0);
		static bool			SliderInt(int* v, const char* label, const int min, const int max, const char* format = "%.0f", const char* message = (const char*)0);
		static bool			InputText(char* v, const char* label, const size_t size, const ImGuiInputTextFlags flags = 0, const char* message = (const char*)0);
		static bool			ColorEdit3(float* col, const char* label);
		static bool			ColorEdit4(float* col, const char* label);
		static void			ColorButton3(float* col, const char* label, const float sameline = -1.0F);
		static void			ColorButton4(float* col, const char* label, const float sameline = -1.0F);
		static void			KeyAssignment(int* key, const char* label, const bool center = false, const bool panic = false);
		static bool			ListBoxArray(const char* label, int* current_item, std::vector<std::string>& values, const int height_in_items = -1);
		static bool			ComboArray(const char* label, int* current_item, std::vector<std::string>& values, const int height_in_items = -1);

		static void			KeyAssignBegin();
		static void			KeyAssingEnd();
		static bool			IsKeyAssingActive();

	private:
		static const char*	m_pcszIdentifyString;
		static std::deque<bool>	m_dConditions;
		static bool			m_bKeyAssignState;
		static std::deque<ImVec4> m_TextColor;

		static void			ShowTooltipMsg(const char* message);
		static void			ClampColor(float* color, const size_t size);
		static std::string	Base(const char* label, const char* message = (const char*)0);
	};

	class Child
	{
	public:
		static bool			Begin(const char* label, const ImVec2 size);
		static void			End();
	};

	class Tab
	{
	public:
		static void			Begin(bool* v = (bool*)0, const char* label = (const char*)0, int* key = (int*)0, const int state = 0);
		static void			End();
	};

	class ModalPopup
	{
	public:
		static bool			Begin(const char* label, const ImVec2 size);
		static void			End();
	};
}