#include "main.h"

CDrawGUI g_DrawGUI;

int CWindow::_iGlobalIndex;

CWindow::CWindow() :
	_bIsVisible(false),
	_pszName(NULL),
	_iIndex(NULL),
	_Fn(NULL)
{
}

void CWindow::Create(const char* pcszName, void(*fn)())
{
	_pszName = const_cast<char*>(pcszName);
	_Fn = fn;
	_iIndex = _iGlobalIndex++;
}

bool CWindow::IsVisible()
{
	return _bIsVisible;
}

char* CWindow::GetName()
{
	return _pszName;
}

int CWindow::GetIndex()
{
	return _iIndex;
}

void CWindow::Show()
{
	_bIsVisible = true;
}

void CWindow::Hide()
{
	_bIsVisible = false;
}

void CWindow::Toggle()
{
	_bIsVisible = !_bIsVisible;
}

void CWindow::Run()
{
	if (_bIsVisible)
		_Fn();
}

CDrawGUI::CDrawGUI() :
	_bIsDrawing(false)
{
}

void CDrawGUI::Create(const char* pcszName, void(*fn)())
{
	CWindow w;
	w.Create(pcszName, fn);
	_window.push_back(w);
}

std::deque<CWindow>& CDrawGUI::GetWindow()
{
	return _window;
}

CWindow& CDrawGUI::GetWindowByName(const char* pcszName)
{
	for (auto& w : _window)
		if (!strcmp(pcszName, w.GetName()))
			return w;
}

CWindow& CDrawGUI::GetWindowByIndex(int iIndex)
{
	for (auto& w : _window)
		if (iIndex == w.GetIndex())
			return w;
}

bool CDrawGUI::IsDrawing()
{
	return _bIsDrawing;
}

bool CDrawGUI::IsFadeout()
{
	return _bIsFadeout;
}

void CDrawGUI::Draw()
{
	_bIsDrawing = true;
}

void CDrawGUI::Hide()
{
	_bIsDrawing = false;
}

void CDrawGUI::Toggle()
{
	_bIsDrawing = !_bIsDrawing;
}

void CDrawGUI::FadeRunning()
{
	_bIsFadeout = true;
}

void CDrawGUI::FadeStopping()
{
	_bIsFadeout = false;
}

void CDrawGUI::Run()
{
	if (!_bIsDrawing)
		return;

	for (auto& w : _window)
		w.Run();
}

//#include "main.h"
//
//CDrawGUI g_DrawGUI;
//
//static std::vector<std::string> g_sSettingsList;
//static unsigned int g_iSettingsSelect = 0;
//static bool g_bIsUpdateFile = true;
//
//static auto vector_getter = [](void* data, int idx, const char** out_text)
//{
//	if (out_text == nullptr)
//		return false;
//
//	const auto& vector = *static_cast<std::vector<std::string>*>(data);
//	*out_text = vector.at(idx).c_str();
//
//	return true;
//};
//
//namespace ImGui
//{
//	bool bKeyAssign = false;
//
//	static void Spacing(size_t count)
//	{
//		while (--count + 1)
//			Spacing();
//	}
//
//	static bool ListBoxArray(const char* label, uint* idx, std::vector<std::string>& values, int height_in_items = -1)
//	{
//		return ListBox(label, reinterpret_cast<int*>(idx), vector_getter, &values, values.size(), height_in_items);
//	}
//
//	static bool TabLabels(std::vector<std::string>& labels, int& idx)
//	{
//		if (!labels.size())
//			return false;
//
//		ImGuiStyle& style = GetStyle();
//
//		const auto ItemSpacing = style.ItemSpacing;
//		const auto FrameRounding = style.FrameRounding;
//		const auto ColorButton = style.Colors[ImGuiCol_Button];
//		const auto ColorButtonActive = style.Colors[ImGuiCol_ButtonActive];
//		const auto ColorButtonHovered = style.Colors[ImGuiCol_ButtonHovered];
//		const auto ColorBorder = style.Colors[ImGuiCol_Border];
//		const auto ColorScrollbarBg = style.Colors[ImGuiCol_ScrollbarBg];
//
//		style.ItemSpacing = ImVec2(0, 1);
//		style.FrameRounding = 0.f;
//		style.Colors[ImGuiCol_ButtonHovered] = ColorButtonHovered;
//		style.Colors[ImGuiCol_ButtonActive] = ColorButtonActive;
//		style.Colors[ImGuiCol_Border] = ImColor();
//
//		bool bPressed = false;
//
//		for (size_t i = 0; i < labels.size(); ++i)
//		{
//			style.Colors[ImGuiCol_Button] = i != idx ? ColorScrollbarBg : ColorButton;
//
//			if (i != 0)
//				SameLine();
//
//			if (bPressed = Button(labels.at(i).c_str(), ImVec2(GetWindowSize().x / labels.size(), 18)))
//				idx = i;
//		}
//
//		Spacing(2);
//
//		style.Colors[ImGuiCol_Button] = ColorButton;
//		style.Colors[ImGuiCol_ButtonActive] = ColorButtonActive;
//		style.Colors[ImGuiCol_ButtonHovered] = ColorButtonHovered;
//		style.Colors[ImGuiCol_Border] = ColorBorder;
//		style.ItemSpacing = ItemSpacing;
//		style.FrameRounding = FrameRounding;
//
//		return bPressed;
//	}
//
//	static void KeyAssignment(const char* label, int* key, float item_offset = 0.f)
//	{
//		if (key == nullptr)
//			return;
//
//		const ImFont* pFont = GetIO().Fonts->Fonts[0];
//
//		if (pFont == nullptr)
//			return;
//
//		const std::string c_sKeyString = g_Utils.KeyToString(*key);
//
//		SameLine((item_offset - 12.f) * 2.f - pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, c_sKeyString.c_str()).x - 9.f);
//		PushStyleColor(ImGuiCol_Border, ImVec4());
//
//		if (Button(std::string(c_sKeyString + label).c_str()))
//		{
//			OpenPopup("##KeyAssignment");
//			bKeyAssign = true;
//		}
//
//		PopStyleColor();
//		SetNextWindowSize(ImVec2(125, 40));
//
//		if (BeginPopupModal("##KeyAssignment", static_cast<bool*>(nullptr),
//			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
//		{
//			Spacing(4);
//
//			const char* pcszMessage = "Press any key";
//			SameLine(((GetWindowWidth()) - pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, pcszMessage).x) * 0.5f);
//			Text(pcszMessage);
//
//			int nKeyASCII = 0;
//			g_Utils.ConvertToASCII(&nKeyASCII);
//
//			if (nKeyASCII || !bKeyAssign)
//			{
//				*key = nKeyASCII;
//				CloseCurrentPopup();
//			}
//
//			EndPopup();
//		}
//	}
//
//	static std::string ControlBase(const char* tab_name, const char* label, float item_offset = 0.f, const char* tooltip_text = nullptr)
//	{
//		std::string sLabel;
//
//		ImGui::Text(label);
//
//		if (tooltip_text && ImGui::IsItemHovered())
//		{
//			const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];
//
//			if (pFont == nullptr)
//				return sLabel;
//
//			const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, tooltip_text);
//			const auto offset_padding = 8.f;
//
//			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//			ImGui::SetNextWindowSize(ImVec2(text_size.x + offset_padding, text_size.y + offset_padding));
//			ImGui::BeginTooltip();
//			ImGui::Spacing(2);
//			ImGui::SameLine(offset_padding * 0.5f);
//			ImGui::Text(tooltip_text);
//			ImGui::EndTooltip();
//			ImGui::PopStyleColor();
//		}
//
//		ImGui::SameLine(item_offset);
//
//		sLabel.append(tab_name);
//		sLabel.append("_");
//		sLabel.append(label);
//
//		return sLabel;
//	}
//
//	static bool Checkbox(const char* tab_name, const char* label, bool* v, float item_offset = 0.f, const char* tooltip_text = nullptr)
//	{
//		return Checkbox(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v);
//	}
//
//	static bool Combo(const char* tab_name, const char* label, int* v, const char* const items[],
//		int count, float item_offset = 0.f, const char* tooltip_text = nullptr)
//	{
//		return Combo(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v, items, count, -1);
//	}
//
//	static bool SliderFloat(const char* tab_name, const char* label, float* v, float v_min, float v_max,
//		const char* format = "%.3f", float power = 1.f, float item_offset = 0.f, const char* tooltip_text = nullptr)
//	{
//		if (*v < v_min)
//			*v = v_min;
//		else if (*v > v_max)
//				*v = v_max;
//
//		return SliderFloat(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v, v_min, v_max, format, power);
//	}
//
//	static bool SliderInt(const char* tab_name, const char* label, int* v, int v_min, int v_max,
//		const char* format = "%.3f", float item_offset = 0.f, const char* tooltip_text = nullptr)
//	{
//		if (*v < v_min)
//			*v = v_min;
//		else if (*v > v_max)
//			*v = v_max;
//
//		return SliderInt(ControlBase(tab_name, label, item_offset, tooltip_text).c_str(), v, v_min, v_max, format);
//	}
//
//	static bool ColorEdit3(const char* tab_name, const char* label, float* col, ImGuiColorEditFlags flags = 0, float item_offset = 0.f)
//	{
//		return ColorEdit3(ControlBase(tab_name, label, item_offset, nullptr).c_str(), col, flags);
//	}
//
//	static bool ColorEdit4(const char* tab_name, const char* label, float* col, ImGuiColorEditFlags flags = 0, float item_offset = 0.f)
//	{
//		return ColorEdit4(ControlBase(tab_name, label, item_offset, nullptr).c_str(), col, flags);
//	}
//
//	static void ColorButton3(const char* label, float* col, float item_offset = 0.f)
//	{
//		const ImGuiContext& g = *GImGui;
//		SameLine(item_offset);
//
//		if (ColorButton(label, ImColor(col), 0, ImVec2(32, 0)))
//		{
//			const ImGuiWindow* window = GetCurrentWindow();
//			OpenPopup(label);
//			SetNextWindowPos(ImVec2(window->DC.LastItemRect.GetBL().x + -1.f,
//				window->DC.LastItemRect.GetBL().y + g.Style.ItemSpacing.y));
//		}
//
//		if (BeginPopup(label))
//		{
//			ColorPicker3("##picker", col, ImGuiColorEditFlags_PickerHueBar);
//			EndPopup();
//		}
//	}
//
//	static void ColorButton4(const char* label, float* col, float item_offset = 0.f)
//	{
//		ImGuiContext& g = *GImGui;
//		SameLine(item_offset);
//
//		if (ColorButton(label, ImColor(col), ImGuiColorEditFlags_AlphaPreview, ImVec2(32, 0)))
//		{
//			const ImGuiWindow* window = GetCurrentWindow();
//			g.ColorPickerRef = ImColor(col);
//			OpenPopup(label);
//			SetNextWindowPos(ImVec2(window->DC.LastItemRect.GetBL().x + -1.f,
//				window->DC.LastItemRect.GetBL().y + g.Style.ItemSpacing.y));
//		}
//
//		if (BeginPopup(label))
//		{
//			ColorPicker4("##picker", col, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar |
//				ImGuiColorEditFlags_AlphaPreviewHalf, &g.ColorPickerRef.x);
//			EndPopup();
//		}
//	}
//}
//
//struct ExampleAppConsole
//{
//	char                  InputBuf[256];
//	ImVector<char*>       Items;
//	bool                  ScrollToBottom;
//	ImVector<char*>       History;
//	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
//	ImVector<const char*> Commands;
//
//	ExampleAppConsole()
//	{
//		ClearLog();
//		memset(InputBuf, 0, sizeof(InputBuf));
//		HistoryPos = -1;
//		Commands.push_back("HELP");
//		Commands.push_back("HISTORY");
//		Commands.push_back("CLEAR");
//		Commands.push_back("CLASSIFY");  // "classify" is only here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
//		AddLog("Welcome to Dear ImGui!");
//	}
//	~ExampleAppConsole()
//	{
//		ClearLog();
//		for (int i = 0; i < History.Size; i++)
//			free(History[i]);
//	}
//
//	// Portable helpers
//	static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
//	static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
//	static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buff = malloc(len); return (char*)memcpy(buff, (const void*)str, len); }
//	static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }
//
//	void    ClearLog()
//	{
//		for (int i = 0; i < Items.Size; i++)
//			free(Items[i]);
//		Items.clear();
//		ScrollToBottom = true;
//	}
//
//	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
//	{
//		// FIXME-OPT
//		char buf[1024];
//		va_list args;
//		va_start(args, fmt);
//		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
//		buf[IM_ARRAYSIZE(buf) - 1] = 0;
//		va_end(args);
//		Items.push_back(Strdup(buf));
//		ScrollToBottom = true;
//	}
//
//	void    Draw(const char* title, bool* p_open)
//	{
//		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
//		if (!ImGui::Begin(title, p_open))
//		{
//			ImGui::End();
//			return;
//		}
//
//		// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
//		// Here we create a context menu only available from the title bar.
//		if (ImGui::BeginPopupContextItem())
//		{
//			if (ImGui::MenuItem("Close Console"))
//				*p_open = false;
//			ImGui::EndPopup();
//		}
//
//		ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
//		ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");
//
//		// TODO: display items starting from the bottom
//
//		if (ImGui::SmallButton("Add Dummy Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); } ImGui::SameLine();
//		if (ImGui::SmallButton("Add Dummy Error")) { AddLog("[error] something went wrong"); } ImGui::SameLine();
//		if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
//		bool copy_to_clipboard = ImGui::SmallButton("Copy"); ImGui::SameLine();
//		if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;
//		//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }
//
//		ImGui::Separator();
//
//		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
//		static ImGuiTextFilter filter;
//		filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
//		ImGui::PopStyleVar();
//		ImGui::Separator();
//
//		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
//		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
//		if (ImGui::BeginPopupContextWindow())
//		{
//			if (ImGui::Selectable("Clear")) ClearLog();
//			ImGui::EndPopup();
//		}
//
//		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
//		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
//		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
//		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
//		//     ImGuiListClipper clipper(Items.Size);
//		//     while (clipper.Step())
//		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
//		// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
//		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
//		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
//		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
//		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
//		if (copy_to_clipboard)
//			ImGui::LogToClipboard();
//		ImVec4 col_default_text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
//		for (int i = 0; i < Items.Size; i++)
//		{
//			const char* item = Items[i];
//			if (!filter.PassFilter(item))
//				continue;
//			ImVec4 col = col_default_text;
//			if (strstr(item, "[error]")) col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
//			else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
//			ImGui::PushStyleColor(ImGuiCol_Text, col);
//			ImGui::TextUnformatted(item);
//			ImGui::PopStyleColor();
//		}
//		if (copy_to_clipboard)
//			ImGui::LogFinish();
//		if (ScrollToBottom)
//			ImGui::SetScrollHere(1.0f);
//		ScrollToBottom = false;
//		ImGui::PopStyleVar();
//		ImGui::EndChild();
//		ImGui::Separator();
//
//		// Command-line
//		bool reclaim_focus = false;
//		if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
//		{
//			char* s = InputBuf;
//			Strtrim(s);
//			if (s[0])
//				ExecCommand(s);
//			strcpy(s, "");
//			reclaim_focus = true;
//		}
//
//		// Auto-focus on window apparition
//		ImGui::SetItemDefaultFocus();
//		if (reclaim_focus)
//			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
//
//		ImGui::End();
//	}
//
//	void    ExecCommand(const char* command_line)
//	{
//		AddLog("# %s\n", command_line);
//
//		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
//		HistoryPos = -1;
//		for (int i = History.Size - 1; i >= 0; i--)
//			if (Stricmp(History[i], command_line) == 0)
//			{
//				free(History[i]);
//				History.erase(History.begin() + i);
//				break;
//			}
//		History.push_back(Strdup(command_line));
//
//		// Process command
//		if (Stricmp(command_line, "CLEAR") == 0)
//		{
//			ClearLog();
//		}
//		else if (Stricmp(command_line, "HELP") == 0)
//		{
//			AddLog("Commands:");
//			for (int i = 0; i < Commands.Size; i++)
//				AddLog("- %s", Commands[i]);
//		}
//		else if (Stricmp(command_line, "HISTORY") == 0)
//		{
//			int first = History.Size - 10;
//			for (int i = first > 0 ? first : 0; i < History.Size; i++)
//				AddLog("%3d: %s\n", i, History[i]);
//		}
//		else
//		{
//			AddLog("Unknown command: '%s'\n", command_line);
//		}
//	}
//
//	static int TextEditCallbackStub(ImGuiTextEditCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
//	{
//		ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
//		return console->TextEditCallback(data);
//	}
//
//	int     TextEditCallback(ImGuiTextEditCallbackData* data)
//	{
//		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
//		switch (data->EventFlag)
//		{
//		case ImGuiInputTextFlags_CallbackCompletion:
//		{
//			// Example of TEXT COMPLETION
//
//			// Locate beginning of current word
//			const char* word_end = data->Buf + data->CursorPos;
//			const char* word_start = word_end;
//			while (word_start > data->Buf)
//			{
//				const char c = word_start[-1];
//				if (c == ' ' || c == '\t' || c == ',' || c == ';')
//					break;
//				word_start--;
//			}
//
//			// Build a list of candidates
//			ImVector<const char*> candidates;
//			for (int i = 0; i < Commands.Size; i++)
//				if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
//					candidates.push_back(Commands[i]);
//
//			if (candidates.Size == 0)
//			{
//				// No match
//				AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
//			}
//			else if (candidates.Size == 1)
//			{
//				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
//				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
//				data->InsertChars(data->CursorPos, candidates[0]);
//				data->InsertChars(data->CursorPos, " ");
//			}
//			else
//			{
//				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
//				int match_len = (int)(word_end - word_start);
//				for (;;)
//				{
//					int c = 0;
//					bool all_candidates_matches = true;
//					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
//						if (i == 0)
//							c = toupper(candidates[i][match_len]);
//						else if (c == 0 || c != toupper(candidates[i][match_len]))
//							all_candidates_matches = false;
//					if (!all_candidates_matches)
//						break;
//					match_len++;
//				}
//
//				if (match_len > 0)
//				{
//					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
//					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
//				}
//
//				// List matches
//				AddLog("Possible matches:\n");
//				for (int i = 0; i < candidates.Size; i++)
//					AddLog("- %s\n", candidates[i]);
//			}
//
//			break;
//		}
//		case ImGuiInputTextFlags_CallbackHistory:
//		{
//			// Example of HISTORY
//			const int prev_history_pos = HistoryPos;
//			if (data->EventKey == ImGuiKey_UpArrow)
//			{
//				if (HistoryPos == -1)
//					HistoryPos = History.Size - 1;
//				else if (HistoryPos > 0)
//					HistoryPos--;
//			}
//			else if (data->EventKey == ImGuiKey_DownArrow)
//			{
//				if (HistoryPos != -1)
//					if (++HistoryPos >= History.Size)
//						HistoryPos = -1;
//			}
//
//			// A better implementation would preserve the data on the current input line along with cursor position.
//			if (prev_history_pos != HistoryPos)
//			{
//				const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
//				data->DeleteChars(0, data->BufTextLen);
//				data->InsertChars(0, history_str);
//			}
//		}
//		}
//		return 0;
//	}
//};
//
//int window::_iIndex = -1;
//
//void window::PushWindow(const char* name, void(*func)(), bool show_default)
//{
//	window w;
//	show_default ? w.Open() : w.Close();
//	w.Push(const_cast<char*>(name), func);
//	g_DrawGUI.GetWindow().push_back(w);
//}
//
//int window::FindIndexByName(const char* name)
//{
//	for (auto& w : g_DrawGUI.GetWindow())
//		if (!strcmp(name, w.GetName()))
//			return w.GetIndex();
//
//	return -1;
//}
//
//std::deque<window>& CDrawGUI::GetWindow()
//{
//	return _window;
//}
//
//void CDrawGUI::NewFrame()
//{
//	static auto ShowCursorState = false;
//	static auto FixCenterCursor = 0;
//
//	auto& io = ImGui::GetIO();
//	io.MouseDrawCursor = _bIsDrawing;
//
//	if (cvar.obs_bypass && g_pIGameUI->IsGameUIActive())
//		io.MouseDrawCursor = false;
//
//	if (_bIsDrawing)
//	{
//		g_pISurface->SetCursor(dc_arrow);
//		g_pISurface->UnlockCursor();
//
//		if (!g_pIGameUI->IsGameUIActive())
//		{
//			if (!ShowCursorState)
//			{
//				ShowCursor(false);
//				ShowCursorState = true;
//			}
//
//			if (FixCenterCursor)
//			{
//				g_pISurface->SurfaceSetCursorPos(g_Screen.iWidth / 2, g_Screen.iHeight / 2);
//				--FixCenterCursor;
//			}
//		}
//	}
//	else if (ShowCursorState)
//	{
//		FixCenterCursor = 2;
//
//		if (!g_pIGameUI->IsGameUIActive())
//		{
//			g_pISurface->SetCursor(dc_none);
//			g_pISurface->LockCursor();
//		}
//
//		ShowCursor(true);
//		ShowCursorState = false;
//	}
//
//
//	/*static bool bState = false;
//
//	if (IsDrawing() && !g_pISurface->IsCursorVisible())
//	{
//		ImGui::GetIO().MouseDrawCursor = _bIsDrawing;
//		g_pISurface->SetCursor(dc_arrow);
//		g_pISurface->UnlockCursor();
//		bState = true;
//	}
//	else if(!IsDrawing() && bState)
//	{
//		bState = false;
//		g_pISurface->SetCursor(dc_none);
//		g_pISurface->LockCursor();
//		ImGui::GetIO().MouseDrawCursor = false;
//	}*/
//	//static bool bState = false;
//	/*static auto nFixCenterCursor = 0;
//
//	static cvar_s *m_rawinput = nullptr;
//
//	if (!m_rawinput)
//		m_rawinput = g_Engine.pfnGetCvarPointer("m_rawinput");
//
//	ImGui::GetIO().MouseDrawCursor = false;
//
//	if (!g_pIGameUI->IsGameUIActive())
//	{
//		//g_Engine.Con_NPrintf(1, "!IsGameUIActive");
//
//		if (IsDrawing())
//		{
//			ImGui::GetIO().MouseDrawCursor = _bIsDrawing;
//
//			if (m_rawinput->value == 0)
//			{
//				g_pISurface->SetCursor(dc_user);
//				g_pISurface->UnlockCursor();
//			}
//		//	g_Engine.Con_NPrintf(2, "!IsGameUIActive IsDrawing");
//			if(nFixCenterCursor)
//			{
//				g_pISurface->SurfaceSetCursorPos(g_Screen.iWidth / 2, g_Screen.iHeight / 2);
//				nFixCenterCursor--;
//			}
//		}
//		else {
//		//	g_Engine.Con_NPrintf(2, "!IsGameUIActive !IsDrawing");
//			if (m_rawinput->value == 0)
//			{
//				g_pISurface->SetCursor(dc_none);
//				g_pISurface->LockCursor();
//			}
//			nFixCenterCursor = 2;
//		}
//	}
//	/*if (ImGui::GetIO().MouseDrawCursor = _bIsDrawing)
//	{
//		if (!g_pIGameUI->IsGameUIActive())
//		{
//			g_pConsole->DPrintf("[hpp] 1\n");
//			g_pISurface->SetCursor(dc_arrow);
//		}
//	}*/
//	/*if (ImGui::GetIO().MouseDrawCursor = _bIsDrawing)
//	{
//		if (bState)
//		{
//		//	ShowCursor(FALSE);
//
//			//if(cvar.debug_console)
//			//	g_pConsole->DPrintf("[hpp] ShowCursos false.\n");
//		}
//
//		//g_pISurface->SetCursor(dc_arrow);
//		g_pISurface->SetCursor(dc_user);
//		g_pISurface->UnlockCursor();
//		bState = false;
//
//		if (nFixCenterCursor)
//		{
//			g_pISurface->SurfaceSetCursorPos(g_Screen.iWidth / 2, g_Screen.iHeight / 2);
//			nFixCenterCursor--;
//		}
//	}
//	else if(!g_pIGameUI->IsGameUIActive() && !bState) 
//	{
//		g_pISurface->SetCursor(dc_none);
//		g_pISurface->LockCursor();
//		bState = true;
//		nFixCenterCursor = 2;	// 2 frames min
//		//ShowCursor(TRUE);
//
//		//if (cvar.debug_console)
//		//	g_pConsole->DPrintf("[hpp] ShowCursos true.\n");
//	}
//	else if (g_pIGameUI->IsGameUIActive() && !bState)
//	{
//	//	ShowCursor(TRUE);
//		bState = true;
//		//nFixCenterCursor = 2;	// 2 frames min
//
//		//if (cvar.debug_console)
//		//	g_pConsole->DPrintf("[hpp] ShowCursos true #2.\n");
//
//		//g_pISurface->SetCursor(dc_arrow);
//		//g_pISurface->UnlockCursor();
//	}*/
//	/*static auto bCursorState = false;
//	static auto nFixCenterCursor = 0;
//
//	if (ImGui::GetIO().MouseDrawCursor = _bIsDrawing)
//	{
//		if (!bCursorState)
//		{
//			ShowCursor(FALSE);
//			bCursorState = true;
//		}
//
//		g_pISurface->SetCursor(dc_arrow);
//		g_pISurface->UnlockCursor();
//
//		if (!g_pGameUI->IsGameUIActive() && nFixCenterCursor)
//		{
//			g_pISurface->SurfaceSetCursorPos(g_Screen.iWidth / 2, g_Screen.iHeight / 2);
//			--nFixCenterCursor;
//		}
//	}
//	else
//	{
//		if (g_pIRunGameEngine->IsInGame() && !g_pGameUI->IsGameUIActive())
//			nFixCenterCursor = 2;	// 2 frames min
//
//		if (bCursorState)
//		{
//			if (!g_pGameUI->IsGameUIActive())
//			{
//				g_pISurface->SetCursor(dc_none);
//				g_pISurface->LockCursor(); 
//			}
//
//			ShowCursor(TRUE);
//			bCursorState = false;
//		}
//	}
//}
//
//
//void CDrawGUI::Fadeout()
//{
//	static auto frames = 0.f;
//
//	const auto width = static_cast<float>(g_Screen.iWidth);
//	const auto height = static_cast<float>(g_Screen.iHeight);
//	const auto max_alpha = 0.45f;
//	const auto fade_duration = 20.f;
//
//	if (_bIsFadeout)
//	{
//		frames += 1.f * (100.f / ImGui::GetIO().Framerate);
//
//		if (frames <= fade_duration)
//		{
//			const auto f1 = _bIsDrawing ? 0.f : max_alpha;
//			const auto f3 = _bIsDrawing ? max_alpha : 0.f;
//
//			g_Drawing.AddRectFilled(ImVec2(), ImVec2(width, height), ImVec4(0, 0, 0,
//				g_Utils.fInterp(1.f, frames, fade_duration, f1, f3)));
//		}
//		else
//		{
//			g_Drawing.AddRectFilled(ImVec2(), ImVec2(width, height),
//				ImVec4(0, 0, 0, _bIsDrawing ? max_alpha : 0.f));
//
//			_bIsFadeout = false;
//			frames = 0.f;
//		}
//	}
//	else
//	{
//		if (_bIsDrawing)
//			g_Drawing.AddRectFilled(ImVec2(), ImVec2(width, height), ImVec4(0, 0, 0, max_alpha));
//	}
//}
//
//void CDrawGUI::Render()
//{
//	if (!_bIsDrawing)
//		return;
//
//	for (auto& w : _window)
//		w.Draw();
//}
//
//typedef void(*LPSEARCHFUNC)(LPCTSTR a_lpcszFileName);
//static inline void SearchFiles(LPCTSTR a_lpcszFileName, LPSEARCHFUNC a_lpSearchFunc)
//{
//	if (a_lpSearchFunc == nullptr)
//		return;
//
//	WIN32_FIND_DATA FindFileData;
//	HANDLE hFileFind = FindFirstFileA(a_lpcszFileName, &FindFileData);
//
//	if (hFileFind != INVALID_HANDLE_VALUE)
//	{
//		do {
//			if (!strstr(&FindFileData.cFileName[0], "."))
//			{
//				char szBuffer[MAX_PATH] = { 0 };
//				LPSTR lpszPart = nullptr;
//
//				GetFullPathNameA(a_lpcszFileName, sizeof(szBuffer), &szBuffer[0], &lpszPart);
//				strcpy(lpszPart, &FindFileData.cFileName[0]);
//				a_lpSearchFunc(&FindFileData.cFileName[0]);
//			}
//
//		} while (FindNextFileA(hFileFind, &FindFileData));
//
//		FindClose(hFileFind);
//	}
//}
//
//static void ReadSettings(const char* a_pcszFileName)
//{
//	if (!strstr(a_pcszFileName, "*.ini"))
//		g_sSettingsList.push_back(a_pcszFileName);
//}
//
//static void RefreshSettings()
//{
//	g_sSettingsList.clear();
//
//	const std::string c_sPath = g_Globals.m_sSettingsPath + "*";
//
//	SearchFiles(c_sPath.c_str(), ReadSettings);
//}
//
//static inline void ReadFromFile(const char* a_pcszFileName, char* a_pszBuffer)
//{
//	std::ifstream input(a_pcszFileName);
//	std::size_t nSize = 0;
//
//	char ch;
//	while ((ch = (char)input.get()) != -1)
//		++nSize;
//
//	input.close();
//
//	FILE* pFile = fopen(a_pcszFileName, "r");
//
//	if (pFile)
//	{
//		fread(a_pszBuffer, nSize, 1, pFile);
//		fclose(pFile);
//	}
//}
//
//DWORD LoadSettingsThread(LPVOID)
//{
//	g_Settings.Load(g_sSettingsList.at(g_iSettingsSelect).c_str());
//	g_bIsUpdateFile = true;
//	return null;
//}
//
//DWORD SaveSettingsThread(LPVOID)
//{
//	g_Settings.Save(g_sSettingsList.at(g_iSettingsSelect).c_str());
//	g_bIsUpdateFile = true;
//	return null;
//}
//
//DWORD RestoreSettingsAsDefaultThread(LPVOID)
//{
//	g_Settings.Restore(g_sSettingsList.at(g_iSettingsSelect).c_str(), true);
//	g_bIsUpdateFile = true;
//	return null;
//}
//
//DWORD RestoreSettingsWithSavingThread(LPVOID)
//{
//	g_Settings.Restore(g_sSettingsList.at(g_iSettingsSelect).c_str());
//	g_bIsUpdateFile = true;
//	return null;
//}
//
//enum WindowTabs
//{
//	WindowTab_RageBot,
//	WindowTab_LegitBot,
//	WindowTab_Visuals,
//	WindowTab_Kreedz,
//	WindowTab_Misc,
//	WindowTab_GUI,
//	WindowTab_Settings
//};
//
//void CheatMenu()
//{
//	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];
//
//	if (pFont == nullptr)
//		return;
//
//	const auto window_size = ImVec2(800, 550);
//	const auto flags = ImGuiWindowFlags_NoResize |
//		ImGuiWindowFlags_NoCollapse |
//		ImGuiWindowFlags_NoScrollWithMouse |
//		ImGuiWindowFlags_NoTitleBar |
//		ImGuiWindowFlags_ShowBorders;
//
//	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
//	ImGui::SetNextWindowSize(window_size);
//
//	if (ImGui::Begin("##Menu", static_cast<bool*>(nullptr), flags))
//	{
//		std::vector<std::string> sTabNames;
//
//		sTabNames.push_back("RageBot");
//		sTabNames.push_back("LegitBot");
//		sTabNames.push_back("Visuals");
//		sTabNames.push_back("Kreedz");
//		sTabNames.push_back("Misc");
//		sTabNames.push_back("GUI");
//		sTabNames.push_back("Settings");
//
//		const auto offset_x = 12.f;
//		const auto colums_count = 2;
//		const auto child_width = (window_size.x - offset_x * (colums_count + 1)) / colums_count;
//		const auto next_child_pos_x = (window_size.x + offset_x) / colums_count;
//		const auto item_space = child_width * 0.5f;
//		const auto item_width = item_space - offset_x * 2.f;
//		const auto title = "Hpp Hack for Counter-Strike 1.6";
//		const auto text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, title);
//
//		static int iTabSelected = WindowTab_RageBot;
//
//		ImGui::Spacing(2);
//		ImGui::SameLine((window_size.x - text_size.x) * 0.5f);
//		ImGui::PushFont(Verdana_16px);
//		ImGui::Text(title);
//		ImGui::PopFont();
//		ImGui::Separator();
//		ImGui::TabLabels(sTabNames, iTabSelected);
//		ImGui::Separator();
//		ImGui::BeginChild("##Controls");
//		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1));
//
//		switch (iTabSelected)
//		{
//		case WindowTab_RageBot:
//			ImGui::Spacing(3);
//			ImGui::SameLine(offset_x);
//			ImGui::Checkbox("##RageBot", "Active", &cvar.ragebot_active, 60);
//			ImGui::Spacing(4);
//			ImGui::SameLine(offset_x);
//			ImGui::BeginGroup();
//
//			if (cvar.ragebot_active)
//				cvar.legitbot_active = false;
//
//			// block: aimbot
//			{
//				const char* str_id = "##RageBot(Aimbot)";
//				const char* const multipoint_combo[] = { "Off", "Low", "Medium", "High" };
//				const char* const hitbox_combo[] = { "Head", "Neck", "Chest", "Stomach", "All hitboxes", "Vital hitboxes" };
//
//				ImGui::Text("Aimbot:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##RageBot_Aimbot", ImVec2(child_width, 260), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Enabled", &cvar.ragebot_aim_enabled, item_space, "Global enable/disable ragebot.");
//				ImGui::Checkbox(str_id, "Friendly fire", &cvar.ragebot_aim_friendlyfire, item_space, "Fire in your teammates.");
//				ImGui::Combo(str_id, "Hitbox", &cvar.ragebot_aim_hitbox, &hitbox_combo[0], IM_ARRAYSIZE(hitbox_combo), item_space, "Hitbox or group of hitboxes for aiming.");
//				ImGui::Combo(str_id, "Multi-point", &cvar.ragebot_aim_multipoint, &multipoint_combo[0], IM_ARRAYSIZE(multipoint_combo), item_space);
//				ImGui::SliderFloat(str_id, "Multi-point scale", &cvar.ragebot_aim_multipoint_scale, 1, 100, "%.f%%", 1, item_space);
//				ImGui::Checkbox(str_id, "Automatic penetration", &cvar.ragebot_aim_auto_penetration, item_space, "Automatic penetration through walls.");
//				ImGui::Checkbox(str_id, "Automatic scope", &cvar.ragebot_aim_auto_scope, item_space, "Automatic opening scope on snipers weapons.");
//				ImGui::Checkbox(str_id, "Silent aim", &cvar.ragebot_aim_silent, item_space, "Silent aim (hidden only for you).");
//				ImGui::Checkbox(str_id, "Perfect silent aim", &cvar.ragebot_aim_perfect_silent, item_space, "Perfect silent aim (hidden for all).");
//				ImGui::SliderFloat(str_id, "Backtrack (ping spike)", &cvar.ragebot_backtrack, 0, 500, "%.0f ms", 1, item_space, "Returns a target a few milliseconds ago.\nIf your ping is very different from this value, reduce it.");
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::Spacing();
//
//			// block: fake lag
//			{
//				const char* str_id = "##RageBot(Fake lag)";
//				const char* const triggers_combo[] = { "On land", "In air", "On land + In air" };
//				const char* const type_combo[] = { "Dynamic", "Maximum", "Flucture", "Break lag compensation" };
//
//				ImGui::Text("Fake lag:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##RageBot_Fakelag", ImVec2(child_width, 140), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Enabled", &cvar.fakelag_enabled, item_space, "Global enable/disable fake lag.");
//				ImGui::Combo(str_id, "Type", &cvar.fakelag_type, &type_combo[0], IM_ARRAYSIZE(type_combo), item_space, "Type of fake lagging.");
//				ImGui::Combo(str_id, "Triggers", &cvar.fakelag_triggers, &triggers_combo[0], IM_ARRAYSIZE(triggers_combo), item_space);	// need desk
//				ImGui::SliderFloat(str_id, "Variance", &cvar.fakelag_variance, 1, 100, "%.f%%", 1, item_space);			// need desc
//				ImGui::SliderFloat(str_id, "Choke limit", &cvar.fakelag_choke_limit, 1, 64, "%.f", 1, item_space);			// need desc
//				ImGui::Checkbox(str_id, "Fake lag while shooting", &cvar.fakelag_while_shooting, item_space);
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			ImGui::SameLine(next_child_pos_x);
//			ImGui::BeginGroup();
//
//			// block: other
//			{
//				const char* str_id = "##RageBot(Other)";
//				const char* const nospread_combo[] = { "Off", "Default", "Perfect Pitch / Yaw", "Transpose Pitch / Yaw / Roll", "Perfect Pitch / Yaw / Roll" };
//
//				ImGui::Text("Other:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##RageBot_Other", ImVec2(child_width, 140), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Remove recoil", &cvar.ragebot_remove_recoil, item_space, "Disables the recoil of weapons.");
//				ImGui::Combo(str_id, "Remove spread", &cvar.ragebot_remove_spread, &nospread_combo[0], IM_ARRAYSIZE(nospread_combo), item_space, "Disables the spread of weapons.");
//
//				ImGui::TextColored(ImVec4(1, 1, 0, 0.8f), "Anti-aim resolver");
//				//ImGui::SameLine(child_width / 2);
//				//ImGui::Checkbox("##RageBot(Anti-aim resolver)", &cvar.ragebot_anti_aim_resolver);
//
//				ImGui::TextColored(ImVec4(1, 1, 0, 0.8f), "Yaw / Roll aiming");
//				//ImGui::SameLine(child_width / 2);
//				//ImGui::Checkbox("##RageBot(Yaw / Roll aiming)", &cvar.ragebot_anti_aim_resolver);
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::Spacing();
//
//			// block: anti-aims
//			{
//				const char* str_id = "##RageBot(Anti-aimbot)";
//				const char* const pitch_combo[] = { "Off", "Fakedown", "Down", "Up", "Jitter", "Minimal", "Random" };
//				const char* const yaw_combo[] = { "Off", "180", "180 Z", "180 Jitter", "Jitter", "Spin", "Sideways", "Random", "Static" };
//				const char* const yaw_while_running_combo[] = { "Off", "180", "180 Jitter", "Jitter", "Spin", "Sideways", "Random", "Static" };
//				const char* const fake_yaw_combo[] = { "Off", "180", "Sideways", "Local view", "Random", "Static" };
//				const char* const edge_triggers_combo[] = { "Standing", "On land", "Always" };
//
//				ImGui::Text("Anti-aimbot:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##RageBot_Anti-aimbot", ImVec2(child_width, 260), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Enabled", &cvar.antiaim_enabled, item_space, "Global enable/disable anti-aimbot.");
//				ImGui::Checkbox(str_id, "Teammates", &cvar.antiaim_teammates, item_space);
//				ImGui::Combo(str_id, "Pitch", &cvar.antiaim_pitch, &pitch_combo[0], IM_ARRAYSIZE(pitch_combo), item_space);	// need desk
//				ImGui::Combo(str_id, "Yaw", &cvar.antiaim_yaw, &yaw_combo[0], IM_ARRAYSIZE(yaw_combo), item_space);	// need desk
//				ImGui::SliderFloat(str_id, "Yaw static", &cvar.antiaim_yaw_static, -180, 180, u8"%.f°", 1.4f, item_space);			// need desc
//				ImGui::SliderInt(str_id, "Spin speed", &cvar.antiaim_yaw_spin, 1, 128, "%.f", item_space);			// need desc
//				ImGui::Combo(str_id, "Yaw while running", &cvar.antiaim_yaw_while_running, &yaw_while_running_combo[0], IM_ARRAYSIZE(yaw_while_running_combo), item_space);	// need desk
//				ImGui::Combo(str_id, "Fake yaw", &cvar.antiaim_fake_yaw, &fake_yaw_combo[0], IM_ARRAYSIZE(fake_yaw_combo), item_space);	// need desk
//				ImGui::SliderFloat(str_id, "Fake yaw static", &cvar.antiaim_fake_yaw_static, -180, 180, u8"%.f°", 1.4f, item_space);			// need desc
//				ImGui::Checkbox(str_id, "Edge", &cvar.antiaim_edge, item_space);		// need desc
//				ImGui::SliderFloat(str_id, "Edge distance", &cvar.antiaim_edge_distance, 32, 256, "%.f", 1, item_space);			// need desc
//				ImGui::SliderFloat(str_id, "Edge yaw offset", &cvar.antiaim_edge_offset, -180, 180, u8"%.f°", 1.4f, item_space);			// need desc
//				ImGui::Combo(str_id, "Edge triggers", &cvar.antiaim_edge_triggers, &edge_triggers_combo[0], IM_ARRAYSIZE(edge_triggers_combo), item_space);	// need desk
//				ImGui::Checkbox(str_id, "On knife", &cvar.antiaim_on_knife, item_space);		// need desc
//				ImGui::SliderFloat(str_id, "Choke limit", &cvar.antiaim_choke_limit, 0, 64, "%.f", 1, item_space, "Choke limit (if zero no faking)");
//				ImGui::SliderFloat(str_id, "Roll angle", &cvar.antiaim_roll, -180, 180, u8"%.f°", 1.4f, item_space, "Roll angle (only special servers)");
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			break;
//
//		case WindowTab_LegitBot:
//		{
//			ImGui::Spacing(3);
//			ImGui::SameLine(offset_x);
//			ImGui::Checkbox("##LegitBot", "Active", &cvar.legitbot_active, 60);
//			ImGui::SameLine(390);
//
//			if (cvar.legitbot_active)
//				cvar.ragebot_active = false;
//
//			enum { Pistol, SubMachineGun, Rifle, Shotgun, Sniper };
//			const char* const weapon_groups_combo[] = { "Pistol", "SubMachineGun", "Rifle", "Shotgun", "Sniper" };
//
//			static std::array<int, IM_ARRAYSIZE(weapon_groups_combo)> current_weapon;
//			static int current_weapon_group = 0;
//
//			ImGui::PushItemWidth(150);
//			ImGui::Combo("##LegitBot", "Group", &current_weapon_group, &weapon_groups_combo[0], IM_ARRAYSIZE(weapon_groups_combo));
//			ImGui::SameLine();
//			ImGui::Text("Weapon");
//			ImGui::SameLine();
//
//			if (current_weapon_group == Pistol)
//			{
//				const char* const weapons_combo[] = { "glock18", "usp", "p228", "deagle", "elite", "fiveseven" };
//				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
//			}
//			else if (current_weapon_group == SubMachineGun)
//			{
//				const char* const weapons_combo[] = { "m249", "tmp", "p90", "mp5n", "mac10", "ump45" };
//				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
//			}
//			else if (current_weapon_group == Rifle)
//			{
//				const char* const weapons_combo[] = { "m4a1", "galil", "famas", "aug", "ak47", "sg552" };
//				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
//			}
//			else if (current_weapon_group == Shotgun)
//			{
//				const char* const weapons_combo[] = { "xm1014", "m3" };
//				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
//			}
//			else if (current_weapon_group == Sniper)
//			{
//				const char* const weapons_combo[] = { "awp", "scout", "g3sg1", "sg550" };
//				ImGui::Combo("##LegitBot_Weapon", &current_weapon.at(current_weapon_group), &weapons_combo[0], IM_ARRAYSIZE(weapons_combo));
//			}
//
//			ImGui::PopItemWidth();
//			ImGui::Spacing(4);
//			ImGui::SameLine(offset_x);
//			ImGui::BeginGroup();
//
//			int iWeaponID = 0;
//
//			if (current_weapon_group == Pistol)
//			{
//				switch (current_weapon.at(current_weapon_group))
//				{
//				case 0: iWeaponID = WEAPON_GLOCK18; break;
//				case 1: iWeaponID = WEAPON_USP; break;
//				case 2: iWeaponID = WEAPON_P228; break;
//				case 3: iWeaponID = WEAPON_DEAGLE; break;
//				case 4: iWeaponID = WEAPON_ELITE; break;
//				case 5: iWeaponID = WEAPON_FIVESEVEN;
//				}
//			}
//			else if (current_weapon_group == SubMachineGun)
//			{
//				switch (current_weapon.at(current_weapon_group))
//				{
//				case 0: iWeaponID = WEAPON_M249; break;
//				case 1: iWeaponID = WEAPON_TMP; break;
//				case 2: iWeaponID = WEAPON_P90; break;
//				case 3: iWeaponID = WEAPON_MP5N; break;
//				case 4: iWeaponID = WEAPON_MAC10; break;
//				case 5: iWeaponID = WEAPON_UMP45;
//				}
//			}
//			else if (current_weapon_group == Rifle)
//			{
//				switch (current_weapon.at(current_weapon_group))
//				{
//				case 0: iWeaponID = WEAPON_M4A1; break;
//				case 1: iWeaponID = WEAPON_GALIL; break;
//				case 2: iWeaponID = WEAPON_FAMAS; break;
//				case 3: iWeaponID = WEAPON_AUG; break;
//				case 4: iWeaponID = WEAPON_AK47; break;
//				case 5: iWeaponID = WEAPON_SG552;
//				}
//			}
//			else if (current_weapon_group == Shotgun)
//			{
//				switch (current_weapon.at(current_weapon_group))
//				{
//				case 0: iWeaponID = WEAPON_XM1014; break;
//				case 1: iWeaponID = WEAPON_M3;
//				}
//			}
//			else if (current_weapon_group == Sniper)
//			{
//				switch (current_weapon.at(current_weapon_group))
//				{
//				case 0: iWeaponID = WEAPON_AWP; break;
//				case 1: iWeaponID = WEAPON_SCOUT; break;
//				case 2: iWeaponID = WEAPON_G3SG1; break;
//				case 3: iWeaponID = WEAPON_SG550;
//				}
//			}
//
//			// block: Aimbot
//			{
//				const char* str_id = "##LegitBot(Aimbot)";
//				const char* const aim_accuracy_combo[] = { "None", "Bound box", "Recoil", "Recoil + Spread" };
//				const char* const psilent_type_combo[] = { "Manual", "Automatic fire" };
//				const char* const psilent_triggers_combo[] = { "Standing", "On land", "In air", "Always" };
//
//				ImGui::Text("Aimbot:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##LegitBot_Aimbot", ImVec2(child_width, 425), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Enabled", &cvar.legitbot[iWeaponID].aim, item_space, "Global enable/disable legitbot.");
//				ImGui::Checkbox(str_id, "Head", &cvar.legitbot[iWeaponID].aim_head, item_space);		// need desk
//				ImGui::Checkbox(str_id, "Chest", &cvar.legitbot[iWeaponID].aim_chest, item_space);		// need desk
//				ImGui::Checkbox(str_id, "Stomach", &cvar.legitbot[iWeaponID].aim_stomach, item_space);		// need desk
//				ImGui::Checkbox(str_id, "Arms", &cvar.legitbot[iWeaponID].aim_arms, item_space);		// need desk
//				ImGui::Checkbox(str_id, "Legs", &cvar.legitbot[iWeaponID].aim_legs, item_space);		// need desk
//				ImGui::SliderInt(str_id, "Reaction time", &cvar.legitbot[iWeaponID].aim_reaction_time, 0, 1000, "%.0f ms", item_space, "Delay before aiming to the next player (in milliseconds) after killing the previous one.");
//				ImGui::SliderFloat(str_id, "Maximum FOV", &cvar.legitbot[iWeaponID].aim_fov, 0, 180, u8"%.1f°", 2, item_space, "Maximal field of view of the aimbot.");
//				ImGui::SliderFloat(str_id, "Smooth (auto aim)", &cvar.legitbot[iWeaponID].aim_smooth, 0, 300, "%.2f", 2, item_space);		// need desc
//				ImGui::SliderFloat(str_id, "Smooth in attack", &cvar.legitbot[iWeaponID].aim_smooth_in_attack, 0, 300, "%.2f", 2, item_space);		// need desc
//				ImGui::SliderFloat(str_id, "Smooth scale - FOV", &cvar.legitbot[iWeaponID].aim_smooth_scale_fov, 0, 100, "%.0f%%", 1, item_space);		// need desc
//				ImGui::SliderInt(str_id, "Recoil compensation pitch", &cvar.legitbot[iWeaponID].aim_recoil_pitch, 0, 100, "%.0f%%", item_space);
//				ImGui::SliderInt(str_id, "Recoil compensation yaw", &cvar.legitbot[iWeaponID].aim_recoil_yaw, 0, 100, "%.0f%%", item_space);
//				ImGui::SliderFloat(str_id, "Recoil comp. maximum FOV", &cvar.legitbot[iWeaponID].aim_recoil_fov, 0, 180, u8"%.1f°", 2, item_space, "Maximal field of view of the aimbot when working recoil");
//				ImGui::SliderFloat(str_id, "Perfect silent aim max. angle", &cvar.legitbot[iWeaponID].aim_psilent_angle, 0, 1, u8"%.1f°", 1, item_space);	// need desc
//				ImGui::Combo(str_id, "Perfect silent type", &cvar.legitbot[iWeaponID].aim_psilent_type, &psilent_type_combo[0], IM_ARRAYSIZE(psilent_type_combo), item_space);	// need desk
//				ImGui::Combo(str_id, "Perfect silent triggers", &cvar.legitbot[iWeaponID].aim_psilent_triggers, &psilent_triggers_combo[0], IM_ARRAYSIZE(psilent_triggers_combo), item_space);	// need desk
//				ImGui::Checkbox(str_id, "Automatic penetration", &cvar.legitbot[iWeaponID].aim_penetration, item_space, "Automatic penetration through walls.");		// need desk
//				ImGui::Combo(str_id, "Accuracy boost", &cvar.legitbot[iWeaponID].aim_accuracy, &aim_accuracy_combo[0], IM_ARRAYSIZE(aim_accuracy_combo), item_space);	// need desk
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			ImGui::SameLine(next_child_pos_x);
//			ImGui::BeginGroup();
//
//			// block: triggerbot
//			{
//				const char* str_id = "##LegitBot(Triggerbot)";
//				const char* const trigger_accuracy_combo[] = { "None", "Recoil", "Recoil + Spread" };
//
//				ImGui::Text("Triggerbot:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##LegitBot_Triggerbot", ImVec2(child_width, 240), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Enabled", &cvar.legitbot[iWeaponID].trigger, item_space, "Global enable/disable triggerbot.");
//				ImGui::KeyAssignment("##TriggerKey", &cvar.legitbot_trigger_key_toggle, item_space);
//				ImGui::Checkbox(str_id, "Head", &cvar.legitbot[iWeaponID].trigger_head, item_space);		// need desk
//				ImGui::Checkbox(str_id, "Chest", &cvar.legitbot[iWeaponID].trigger_chest, item_space);	// need desk
//				ImGui::Checkbox(str_id, "Stomach", &cvar.legitbot[iWeaponID].trigger_stomach, item_space);	// need desk
//				ImGui::Checkbox(str_id, "Arms", &cvar.legitbot[iWeaponID].trigger_arms, item_space);	// need desk
//				ImGui::Checkbox(str_id, "Legs", &cvar.legitbot[iWeaponID].trigger_legs, item_space);	// need desk
//				ImGui::Checkbox(str_id, "Automatic penetration", &cvar.legitbot[iWeaponID].trigger_penetration, item_space, "Automatic penetration through walls.");
//				ImGui::Combo(str_id, "Accuracy boost", &cvar.legitbot[iWeaponID].trigger_accuracy, &trigger_accuracy_combo[0], IM_ARRAYSIZE(trigger_accuracy_combo), item_space); // need desk
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//
//			ImGui::Spacing();
//
//			// block: Other
//			{
//				const char* str_id = "##LegitBot(Other)";
//
//				ImGui::Text("Other:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##LegitBot_Other", ImVec2(child_width, 160), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Friendly fire", &cvar.legitbot_friendlyfire, item_space, "Fire in your teammates.");
//				ImGui::SliderFloat(str_id, "Backtrack (ping spike)", &cvar.legitbot_backtrack, 0, 500, "%.0f ms", 1, item_space, "Returns a target a few milliseconds ago.\nIf your ping is very different from this value, reduce it.");
//				ImGui::SliderFloat(str_id, "Triggerbot hitbox scale", &cvar.legitbot_trigger_hitbox_scale, 1, 200, "%.f%%", 1.f, item_space);		// need desc
//				ImGui::Checkbox(str_id, "Triggerbot only scoped", &cvar.legitbot_trigger_only_scoped, item_space);  	// need desc
//				ImGui::Checkbox(str_id, "Triggerbot predict spread pistols", &cvar.legitbot_trigger_predict_spread_on_pistols, item_space, "Triggerbot predict spread pistols. #testing");
//				ImGui::Checkbox(str_id, "Legitbot automatic scope", &cvar.legitbot_automatic_scope, item_space);
//				ImGui::Checkbox(str_id, "Legitbot dependence fps", &cvar.legitbot_dependence_fps, item_space, "100 fps = smooth value");
//				
//				
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			break;
//		}
//
//		case WindowTab_Visuals:
//			ImGui::Spacing(3);
//			ImGui::SameLine(12);
//			ImGui::Checkbox("##Visuals", "Active", &cvar.visuals, 60);
//			ImGui::Spacing(4);
//			ImGui::SameLine(12);
//			ImGui::BeginGroup();
//
//			// block: ESP
//			{
//				const char* str_id = "##Visuals(ESP)";
//				const char* const player_box_combo[] = { "Off", "Default box", "Corner box", "Rounded box" };
//				const char* const position_combo[] = { "Off", "Left", "Right", "Top", "Bottom" };
//				const char* const player_distance_combo[] = { "In units", "In meters" };
//				const char* const font_outline_combo[] = { "Off", "Shadow", "Outline" };
//
//				ImGui::Text("Player ESP:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Visuals_ESP", ImVec2(child_width, 250), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Enabled", &cvar.esp_player_enabled, item_space);
//				ImGui::Checkbox(str_id, "Teammates", &cvar.esp_player_teammates, item_space);	// need desc
//				ImGui::Combo(str_id, "Bounding box", &cvar.esp_player_box, &player_box_combo[0], IM_ARRAYSIZE(player_box_combo), item_space);	// need desk
//				ImGui::Text("Bounding box colors"); 
//				ImGui::ColorButton4("##Terrorist color", &cvar.esp_player_box_color_t[0], item_space); ImGui::SameLine(); ImGui::Text("T");
//				ImGui::ColorButton4("##Counter-Terrorist color", &cvar.esp_player_box_color_ct[0]); ImGui::SameLine(); ImGui::Text("CT");
//				ImGui::Checkbox(str_id, "Bounding box outline", &cvar.esp_player_box_outline, item_space);		// need desk
//				ImGui::Combo(str_id, "Name", &cvar.esp_player_name, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Show the name of the player on the screen.");
//				ImGui::Text("Background color"); ImGui::ColorButton4("##Background color", &cvar.esp_player_background_color[0], item_space);
//				ImGui::Combo(str_id, "Weapon", &cvar.esp_player_weapon, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Show the player's weapon on the screen.");
//				ImGui::Combo(str_id, "Distance", &cvar.esp_player_distance, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Show the distance to the player on the screen.");
//				ImGui::Combo(str_id, "Distance measuring", &cvar.esp_player_distance_measure, &player_distance_combo[0], IM_ARRAYSIZE(player_distance_combo), item_space);
//				ImGui::Combo(str_id, "Health bar", &cvar.esp_player_health, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Draw the health bar.");
//				ImGui::Checkbox(str_id, "Show numeric on health bar", &cvar.esp_player_health_value, item_space, "Show health points on the health bar.");
//				ImGui::Combo(str_id, "Armor bar", &cvar.esp_player_armor, &position_combo[0], IM_ARRAYSIZE(position_combo), item_space, "Draw the armor bar.");
//				ImGui::Checkbox(str_id, "Skeleton", &cvar.esp_player_skeleton, item_space, "Draw skeleton of the player.");
//				ImGui::ColorButton4("##Skeleton color", &cvar.esp_player_skeleton_color[0]);
//				ImGui::Checkbox(str_id, "Out of POV arrow", &cvar.esp_player_out_of_pov_arrow, item_space);	 // need desc
//				ImGui::SliderFloat(str_id, "Out of POV arrow radius", &cvar.esp_player_out_of_pov_arrow_radius, 100, 500, "%.0f", 1, item_space);		// need desc
//				ImGui::SliderFloat(str_id, "History time", &cvar.esp_player_history_time, 0.0f, 4.0f, "%.1f sec", 1.0f, item_space);		// need desc
//				ImGui::Checkbox(str_id, "Fade out dormant", &cvar.esp_player_fadeout_dormant, item_space, "Enable soft fade out if the player is not valid.");		// need desc
//				ImGui::Checkbox(str_id, "Visualize sounds", &cvar.esp_sound, item_space);		// need desc
//				ImGui::SliderFloat(str_id, "Sounds fade out time", &cvar.esp_sound_fadeout_time, 0.0f, 4.0f, "%.1f sec", 1.0f, item_space);		// need desc
//				ImGui::SliderFloat(str_id, "Sounds circle size", &cvar.esp_sound_circle_size, 1, 50, "%.1f", 1, item_space);		// need desc
//				ImGui::Text("Sound colors"); ImGui::ColorButton4("##Sounds Terrorist color", &cvar.esp_sound_color_t[0], item_space); ImGui::SameLine(); ImGui::Text("T");
//				ImGui::ColorButton4("##Sounds Counter-Terrorist color", &cvar.esp_sound_color_ct[0], 0.f); ImGui::SameLine(); ImGui::Text("CT");
//				ImGui::SliderFloat(str_id, "Font size", &cvar.esp_font_size, 0.1f, 18, "%.1f px", 1, item_space);		// need desc
//				ImGui::Combo(str_id, "Font outline", &cvar.esp_font_outline, &font_outline_combo[0], IM_ARRAYSIZE(font_outline_combo), item_space);	// need desk
//				ImGui::ColorEdit4(str_id, "Font color", &cvar.esp_font_color[0], ImGuiColorEditFlags_NoPicker, item_space);
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::Spacing();
//
//			// block: Colored models
//			{
//				const char* str_id = "##Visuals(Colored models)";
//				const char* const colored_models_type[] = { "Off", "Flat", "Wireframe", "Texture", "Material" };
//
//				ImGui::Text("Colored models and glow: (no obs proof)");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Visuals_Colored models", ImVec2(child_width, 150), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Enabled", &cvar.colored_models_enabled, item_space, "Global enable/disable colored models");
//				ImGui::Combo(str_id, "Player", &cvar.colored_models_player, &colored_models_type[0], IM_ARRAYSIZE(colored_models_type), item_space);	  // need desk
//				ImGui::Checkbox(str_id, "Player (behind wall)", &cvar.colored_models_player_behind_wall, item_space);	// need desk
//				ImGui::Checkbox(str_id, "Teammates", &cvar.colored_models_player_teammates, item_space);	// need desk
//				ImGui::Text("Visible colors"); ImGui::ColorButton3("##Terrorist visible color", &cvar.colored_models_color_t[0], item_space); ImGui::SameLine(); ImGui::Text("T");
//				ImGui::ColorButton3("##Counter-Terrorist visible color", &cvar.colored_models_color_ct[0]); ImGui::SameLine(); ImGui::Text("CT");
//				ImGui::Text("Hidden colors"); ImGui::ColorButton3("##Terrorist hidden color", &cvar.colored_models_color_t_behind_wall[0], item_space); ImGui::SameLine(); ImGui::Text("T");
//				ImGui::ColorButton3("##Counter-Terrorist hidden color", &cvar.colored_models_color_ct_behind_wall[0]); ImGui::SameLine(); ImGui::Text("CT");
//				ImGui::Combo(str_id, "Hands", &cvar.colored_models_hands, &colored_models_type[0], IM_ARRAYSIZE(colored_models_type), item_space);	  // need desk
//				ImGui::Text("Hands color"); ImGui::ColorButton3("##Hands color", &cvar.colored_models_hands_color[0], item_space);
//				ImGui::Combo(str_id, "Dropped weapons", &cvar.colored_models_dropped_weapons, &colored_models_type[0], IM_ARRAYSIZE(colored_models_type), item_space);	  // need desk
//				ImGui::Text("Dropped weapons color"); ImGui::ColorButton3("##Dropped weapons color", &cvar.colored_models_dropped_weapons_color[0], item_space);
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			ImGui::SameLine(next_child_pos_x);
//			ImGui::BeginGroup();
//
//			// block: Other
//			{
//				const char* str_id = "##Visuals(Other)";
//
//				ImGui::Text("Other:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Visuals_Other", ImVec2(child_width, 180), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Dropped weapons", &cvar.world_dropped_weapons, item_space, "Show weapons on the map.");		// need desc
//				ImGui::Checkbox(str_id, "Thrown grenades", &cvar.world_thrown_grenades, item_space);		// need desc
//				/*ImGui::Checkbox(str_id, "Bomb", &cvar.esp_bomb, item_space);		// need desc
//				ImGui::ColorButton4("##Bomb color", &cvar.esp_bomb_color[0]);*/
//				//ImGui::Checkbox(str_id, "Snipers crosshair", &cvar.crosshair_snipers, item_space);	// need desc
//				//ImGui::ColorButton4("##Snipers crosshair color", &cvar.crosshair_snipers_color[0]);
//				ImGui::Checkbox(str_id, "Legitbot circle FOV", &cvar.legitbot_fov_circle, item_space);	// need desc
//				ImGui::ColorButton4("##Legitbot circle FOV color", &cvar.legitbot_fov_circle_color[0]);
//				ImGui::Checkbox(str_id, "Spread circle FOV", &cvar.spread_fov_circle, item_space);	// need desc
//				ImGui::ColorButton4("##Spread circle FOV color", &cvar.spread_fov_circle_color[0]);
//				ImGui::Checkbox(str_id, "Punch recoil", &cvar.punch_recoil, item_space);	// need desc
//				ImGui::ColorButton4("##Punch recoil color", &cvar.punch_recoil_color[0]);
//				//ImGui::Checkbox(str_id, "Grenade trail", &cvar.grenade_trail, item_space);	// need desc
//				//ImGui::ColorButton4("##Grenade trail color", &cvar.grenade_trail_color[0]);
//				/*ImGui::Checkbox(str_id, "Bullet impacts", &cvar.bullet_impacts, item_space);	// need desc
//				ImGui::ColorButton4("##Bullet impacts color", &cvar.bullet_impacts_color[0]);*/
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::Spacing();
//
//			// block: Effects
//			{
//				const char* str_id = "##Visuals(Effects)";
//
//				ImGui::Text("Effects:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Visuals_Effects", ImVec2(child_width, 220), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				//ImGui::Checkbox(str_id, "Remove visual recoil", &cvar.remove_visual_recoil, item_space);	// need desc
//				ImGui::SliderInt(str_id, "Force third person", &cvar.thirdperson, 0, 512, "%.0f", item_space);	// need desc
//				ImGui::Checkbox(str_id, "Brightness", &cvar.brightness, item_space);	// need desc
//				//ImGui::ColorButton4("##Brightness color", &cvar.brightness_color[0]);
//				ImGui::Checkbox(str_id, "Disable render teammates", &cvar.disable_render_teammates, item_space);	// need desc
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			break;
//
//		case WindowTab_Kreedz:
//		{
//			ImGui::Spacing(3);
//			ImGui::SameLine(12);
//			ImGui::Checkbox("##Kreedz", "Active", &cvar.kreedz);
//			ImGui::Spacing(4);
//			ImGui::SameLine(12);
//			ImGui::BeginGroup();
//
//			// block: Kreedz
//			{
//				const char* str_id = "##Kreedz";
//
//				ImGui::Text("Kreedz:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Kreedz", ImVec2(child_width, 425), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "Bunnyhop", &cvar.kreedz_bunnyhop, item_space);	// need desc
//				ImGui::KeyAssignment("##BhopKey", &cvar.kreedz_bunnyhop_key, item_space);
//				ImGui::SliderInt(str_id, "Bunnyhop ideal percent", &cvar.kreedz_bunnyhop_ideal_percent, 0, 100, "%.0f", item_space); // need desc
//				ImGui::SliderInt(str_id, "Bunnyhop in a row", &cvar.kreedz_bunnyhop_in_a_row, 0, 18, "%.0f", item_space); // need desc
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			ImGui::SameLine(next_child_pos_x);
//			ImGui::BeginGroup();
//
//			// block: Other
//			{
//				ImGui::Text("");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Kreedz_Legit", ImVec2(child_width, 425), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				// here
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			break;
//		}
//		case WindowTab_Misc:
//			ImGui::Spacing(4);
//			ImGui::SameLine(12);
//			ImGui::BeginGroup();
//
//			// block: Miscellaneous
//			{
//				const char* str_id = "##Miscellaneous(Miscellaneous)";
//				const char* const knifebot_type[] = { "Stab", "Slash" };
//
//				ImGui::Text("Miscellaneous:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Miscellaneous_Miscellaneous", ImVec2(child_width, 452), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				
//				ImGui::Checkbox(str_id, "Screen log", &cvar.screen_log, item_space);	// need desc
//			//	ImGui::Checkbox(str_id, "Disable screenshake", &cvar.disable_screenshake, item_space);	// need desc
//				ImGui::Checkbox(str_id, "HUD clear", &cvar.hud_clear, item_space);	// need desc
//				ImGui::Checkbox(str_id, "Automatic pistol", &cvar.automatic_pistol, item_space);	// need desc
//				ImGui::Checkbox(str_id, "Automatic reload", &cvar.automatic_reload, item_space);	// need desc
//				ImGui::Checkbox(str_id, "Knifebot", &cvar.knifebot, item_space);	// need desc
//				ImGui::Checkbox(str_id, "Knifebot friendly fire", &cvar.knifebot_friendly_fire, item_space);	// need desc
//				ImGui::Combo(str_id, "Knifebot type", &cvar.knifebot_type, &knifebot_type[0], IM_ARRAYSIZE(knifebot_type), item_space);	  // need desk
//				ImGui::SliderFloat(str_id, "Knifebot backtrack (ping spike)", &cvar.knifebot_backtrack, 0, 500, "%.0f ms", 1, item_space, "Returns a target a few milliseconds ago.\nIf your ping is very different from this value, reduce it.");
//				ImGui::SliderFloat(str_id, "Knifebot distance stab", &cvar.knifebot_distance_stab, 0, 100, "%.2f", 1, item_space);
//				ImGui::SliderFloat(str_id, "Knifebot distance slash", &cvar.knifebot_distance_slash, 0, 100, "%.2f", 1, item_space);
//				ImGui::SliderFloat(str_id, "Knifebot maximum FOV", &cvar.knifebot_fov, 0, 180, u8"%.1f°", 2, item_space, "Maximal field of view of the knifebot.");
//
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			ImGui::SameLine(next_child_pos_x);
//			ImGui::BeginGroup();
//
//			// block: Other
//			{
//				const char* str_id = "##Miscellaneous(Other)";
//
//				ImGui::Text("Other:");
//				ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
//				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
//				ImGui::BeginChild("##Miscellaneous_Other", ImVec2(child_width, 452), true);
//				ImGui::PopStyleColor(2);
//				ImGui::PushItemWidth(item_width);
//				ImGui::Spacing();
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Spacing(3);
//
//				ImGui::Checkbox(str_id, "OBS bypass", &cvar.obs_bypass, item_space);
//				ImGui::Checkbox(str_id, "Demochecker bypass", &cvar.demochecker_bypass, item_space, "Bypassing bad angles detected by demochecker. (Accuracy drops)");	// need desc
//				ImGui::Checkbox(str_id, "Debug console", &cvar.debug_console, item_space, "Trash :D");	// need desc
//				ImGui::Checkbox(str_id, "Debug visuals", &cvar.debug_visuals, item_space);		// need desc
//
//				ImGui::Spacing(3);
//				ImGui::PopItemWidth();
//				ImGui::EndGroup();
//				ImGui::EndChild();
//			}
//
//			ImGui::EndGroup();
//			break;
//
//		case WindowTab_GUI:
//			break;
//
//		case WindowTab_Settings:
//			static char szFind[100] = { 0 };
//			static char szReplace[100] = { 0 };
//			static char szCreateSettingsBuff[100] = { 0 };
//			static char szRenameSettingsBuff[100] = { 0 };
//			static char szSettingsBuff[104448] = { 0 };
//
//			const std::array<const char* const, 6> apcszIniFilesList =
//			{
//				"ragebot.ini",
//				"legitbot.ini",
//				"visuals.ini",
//				"kreedz.ini",
//				"misc.ini",
//				"gui.ini"
//			};
//
//			static auto iIniFileSelected = 0;
//
//			ImGui::Spacing(2);
//			ImGui::SameLine(12);
//			ImGui::BeginGroup();
//			ImGui::Text("Settings:");
//			ImGui::PushItemWidth(216);
//
//			RefreshSettings();
//
//			if (ImGui::ListBoxArray("##SettingsList", &g_iSettingsSelect, g_sSettingsList, 26))
//				g_bIsUpdateFile = true;
//
//			ImGui::PopItemWidth();
//			ImGui::EndGroup();
//			ImGui::SameLine();
//			ImGui::BeginGroup();
//			ImGui::Text("Files:");
//			ImGui::PushItemWidth(216);
//
//			if (ImGui::ListBox("##IniFiles", &iIniFileSelected, &apcszIniFilesList.at(0), apcszIniFilesList.size()))
//				g_bIsUpdateFile = true;
//
//			ImGui::PopItemWidth();
//			ImGui::EndGroup();
//			ImGui::SameLine();
//			ImGui::BeginGroup();
//			ImGui::Spacing(4);
//			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//
//			if (ImGui::Button("Load settings##Settings", ImVec2(160, 22)))
//				CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)LoadSettingsThread, nullptr, 0, nullptr);
//
//			ImGui::SameLine();
//
//			if (ImGui::Button("Rename settings##Settings", ImVec2(160, 22)))
//			{
//				if (_stricmp("Default", g_sSettingsList.at(g_iSettingsSelect).c_str()))
//				{
//					RtlSecureZeroMemory(&szRenameSettingsBuff[0], sizeof(szRenameSettingsBuff));
//					strcpy(&szRenameSettingsBuff[0], g_sSettingsList.at(g_iSettingsSelect).c_str());
//
//					ImGui::OpenPopup("Rename settings##Modal");
//				}
//
//				g_bIsUpdateFile = true;
//			}
//
//			if (ImGui::Button("Save settings##Settings", ImVec2(160, 22)))
//				CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)SaveSettingsThread, nullptr, 0, nullptr);
//
//			ImGui::SameLine();
//
//			if (ImGui::Button("Create & Save settings##Settings", ImVec2(160, 22)))
//			{
//				RtlSecureZeroMemory(&szCreateSettingsBuff[0], sizeof(szCreateSettingsBuff));
//				ImGui::OpenPopup("Create & Save##Modal");
//			}
//
//			ImGui::Spacing(2);
//
//			if (ImGui::Button("Restore settings##Settings", ImVec2(160, 22)))
//			{
//				ImGui::OpenPopup("Restore settings##Modal");
//				g_bIsUpdateFile = true;
//			}
//
//			ImGui::SameLine();
//
//			if (ImGui::Button("Remove settings##Settings", ImVec2(160, 22)))
//			{
//				if (_stricmp("Default", g_sSettingsList.at(g_iSettingsSelect).c_str()))
//					ImGui::OpenPopup("Remove settings##Modal");
//
//				g_bIsUpdateFile = true;
//			}
//
//			if (ImGui::Button("Open settings folder##Settings", ImVec2(160, 21)))
//			{
//				ShellExecuteA(nullptr, "open", g_Globals.m_sSettingsPath.c_str(), nullptr, nullptr, SW_RESTORE);
//				g_bIsUpdateFile = true;
//			}
//
//			ImGui::SameLine();
//
//			const std::string c_sPath = g_Globals.m_sSettingsPath +
//				g_sSettingsList.at(g_iSettingsSelect) + "\\" + apcszIniFilesList.at(iIniFileSelected);
//
//			if (ImGui::Button("Open selected file##Settings", ImVec2(160, 21)))
//			{
//				ShellExecuteA(nullptr, "open", c_sPath.c_str(), nullptr, nullptr, SW_RESTORE);
//				g_bIsUpdateFile = true;
//			}
//
//			ImGui::PopStyleColor();
//			ImGui::Spacing(2);
//			ImGui::SameLine(-224);
//
//			if (g_bIsUpdateFile)
//			{
//				RtlSecureZeroMemory(&szSettingsBuff[0], sizeof(szSettingsBuff));
//
//				if (g_Utils.FileExists(c_sPath.c_str()))
//					ReadFromFile(c_sPath.c_str(), &szSettingsBuff[0]);
//
//				g_bIsUpdateFile = false;
//			}
//
//			ImGui::PushFont(ProggyClean_13px);
//			ImGui::InputTextMultiline("##src", &szSettingsBuff[0], sizeof(szSettingsBuff), ImVec2(551, 302), ImGuiInputTextFlags_AllowTabInput);
//			ImGui::PopFont();
//			ImGui::Spacing(2);
//			ImGui::SameLine(-224);
//			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//
//			if (ImGui::Button("Save##File", ImVec2(80, 21)) && strlen(&szSettingsBuff[0]) > 0)
//			{
//				FILE* pFile = fopen(c_sPath.data(), "w");
//
//				if (pFile)
//				{
//					fwrite(&szSettingsBuff[0], sizeof(szSettingsBuff), 1, pFile);
//					fclose(pFile);
//				}
//			}
//
//			ImGui::PopStyleColor();
//			ImGui::SameLine();
//			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
//			ImGui::PushItemWidth(184);
//			ImGui::InputText("##replace_from", &szFind[0], sizeof(szFind), ImGuiInputTextFlags_AutoSelectAll);
//			ImGui::SameLine();
//			ImGui::InputText("##replace_to", &szReplace[0], sizeof(szReplace), ImGuiInputTextFlags_AutoSelectAll);
//			ImGui::PopItemWidth();
//			ImGui::PopStyleVar();
//			ImGui::SameLine();
//			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//
//			if (ImGui::Button("Replace##Settings", ImVec2(80, 21)))
//				g_Utils.StringReplace(&szSettingsBuff[0], &szFind[0], &szReplace[0]);
//
//			ImGui::PopStyleColor();
//			ImGui::EndGroup();
//
//			ImGui::Spacing(2);
//			ImGui::SameLine(offset_x);
//			ImGui::PushItemWidth(window_size.x - offset_x * 2.f);
//			ImGui::InputText("##AppData", const_cast<char*>(g_Globals.m_sSettingsPath.c_str()),
//				g_Globals.m_sSettingsPath.size() + 1, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
//			ImGui::PopItemWidth();
//			ImGui::PopStyleVar();
//
//			const auto modal_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
//			const auto modal_size = ImVec2(250, 80);
//			const auto button_size = ImVec2(109, 20);
//
//			// Popup Modal: "Create & Save" button
//			ImGui::SetNextWindowSize(modal_size);
//
//			if (ImGui::BeginPopupModal("Create & Save##Modal", static_cast<bool*>(nullptr), modal_flags))
//			{
//				ImGui::Spacing(3);
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Text("Settings name:");
//				ImGui::PushItemWidth(226);
//				ImGui::InputText("##CreateSettings", &szCreateSettingsBuff[0], sizeof(szCreateSettingsBuff));
//				ImGui::PopItemWidth();
//				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//
//				if (ImGui::Button("Create", button_size) && strlen(&szCreateSettingsBuff[0]) > 0)
//				{
//					if (_stricmp(&szCreateSettingsBuff[0], "Default"))
//					{
//						CreateDirectoryA(std::string(g_Globals.m_sSettingsPath + &szCreateSettingsBuff[0]).c_str(), nullptr);
//
//						RefreshSettings();
//
//						for (size_t i = 0; i < g_sSettingsList.size(); ++i)
//						{
//							if (!strcmp(g_sSettingsList.at(i).c_str(), &szCreateSettingsBuff[0]))
//							{
//								g_iSettingsSelect = i;
//								break;
//							}
//						}
//
//						CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)SaveSettingsThread, nullptr, 0, nullptr);
//					}
//
//					ImGui::CloseCurrentPopup();
//				}
//
//				ImGui::SameLine();
//
//				if (ImGui::Button("Cancel", button_size))
//					ImGui::CloseCurrentPopup();
//
//				ImGui::PopStyleColor();
//				ImGui::EndGroup();
//				ImGui::EndPopup();
//			}
//
//			ImGui::SetNextWindowSize(modal_size);
//
//			// Popup Modal: "Rename settings" button
//			if (ImGui::BeginPopupModal("Rename settings##Modal", static_cast<bool*>(nullptr), modal_flags))
//			{
//				ImGui::Spacing(3);
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::Text("Settings name:");
//				ImGui::PushItemWidth(226);
//				ImGui::InputText("##RenameSettings", &szRenameSettingsBuff[0], sizeof(szRenameSettingsBuff));
//				ImGui::PopItemWidth();
//				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//
//				if (ImGui::Button("Rename", button_size) && strlen(&szRenameSettingsBuff[0]) > 0)
//				{
//					if (_stricmp(g_sSettingsList.at(g_iSettingsSelect).c_str(), "Default"))
//					{
//						const auto src = g_Globals.m_sSettingsPath + g_sSettingsList.at(g_iSettingsSelect);
//						const auto dest = g_Globals.m_sSettingsPath + &szRenameSettingsBuff[0];
//
//						MoveFileA(src.c_str(), dest.c_str());
//
//						g_bIsUpdateFile = true;
//					}
//
//					ImGui::CloseCurrentPopup();
//				}
//
//				ImGui::SameLine();
//
//				if (ImGui::Button("Cancel", button_size))
//					ImGui::CloseCurrentPopup();
//
//				ImGui::PopStyleColor();
//				ImGui::EndGroup();
//				ImGui::EndPopup();
//			}
//
//			ImGui::SetNextWindowSize(modal_size);
//
//			// Popup Modal: "Remove settings" button
//			if (ImGui::BeginPopupModal("Remove settings##Modal", static_cast<bool*>(nullptr), modal_flags))
//			{
//				const auto msg = "Are you sure you want to delete?";
//				const auto msg_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, msg);
//
//				ImGui::Spacing(6);
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::SameLine(((ImGui::GetWindowWidth() - offset_x * 2.f) - msg_size.x) * 0.5f);
//				ImGui::Text(msg);
//				ImGui::Spacing(2);
//				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//
//				if (ImGui::Button("Delete", button_size))
//				{
//					g_Settings.Remove(std::string(g_Globals.m_sSettingsPath + g_sSettingsList.at(g_iSettingsSelect)).c_str());
//
//					g_bIsUpdateFile = true;
//
//					if (g_iSettingsSelect >= g_sSettingsList.size())
//						g_iSettingsSelect = g_sSettingsList.size() - 1;
//
//					ImGui::CloseCurrentPopup();
//				}
//
//				ImGui::SameLine();
//
//				if (ImGui::Button("Cancel", button_size))
//					ImGui::CloseCurrentPopup();
//
//				ImGui::PopStyleColor();
//				ImGui::EndGroup();
//				ImGui::EndPopup();
//			}
//
//			ImGui::SetNextWindowSize(modal_size);
//
//			// Popup Modal: "Restore settings" button
//			if (ImGui::BeginPopupModal("Restore settings##Modal", static_cast<bool*>(nullptr), modal_flags))
//			{
//				const auto msg = "Choose the type of restore";
//				const auto msg_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, msg);
//
//				ImGui::Spacing(6);
//				ImGui::SameLine(offset_x);
//				ImGui::BeginGroup();
//				ImGui::SameLine(((ImGui::GetWindowWidth() - offset_x * 2.f) - msg_size.x) * 0.5f);
//				ImGui::Text(msg);
//				ImGui::Spacing(2);
//				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
//
//				if (ImGui::Button("Set as default", button_size))
//				{
//					CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)RestoreSettingsAsDefaultThread, nullptr, 0, nullptr);
//					ImGui::CloseCurrentPopup();
//				}
//
//				ImGui::SameLine();
//
//				if (ImGui::Button("Save settings", button_size))
//				{
//					CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)RestoreSettingsWithSavingThread, nullptr, 0, nullptr);
//					ImGui::CloseCurrentPopup();
//				}
//
//				ImGui::PopStyleColor();
//				ImGui::EndGroup();
//				ImGui::EndPopup();
//			}
//		}
//
//		ImGui::EndChild();
//		ImGui::End();
//	}
//}
//
//void ShowExampleAppConsole()
//{
//	static ExampleAppConsole console;
//	static auto index = -1;
//
//	if (index == -1)
//		index = window::FindIndexByName("AppConsole");
//	else if (g_DrawGUI.GetWindow()[index].IsOpened())
//		console.Draw("Example: Console", (bool*)0);
//}