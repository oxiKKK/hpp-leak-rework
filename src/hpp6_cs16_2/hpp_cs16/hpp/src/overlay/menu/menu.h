static const char* weapons_pistols[] = { "subsection", "glock18", "usp", "p228", "deagle", "elite", "fiveseven" };
static const char* weapons_submachineguns[] = { "subsection", "m249", "tmp", "p90", "mp5", "mac10", "ump45" };
static const char* weapons_rifles[] = { "subsection", "m4a1", "galil", "famas", "aug", "ak47", "sg552" };
static const char* weapons_shotguns[] = { "subsection", "xm1014", "m3" };
static const char* weapons_snipers[] = { "subsection", "awp", "scout", "g3sg1", "sg550" };

// NOTE: переместить
struct ExampleAppConsole
{
	char                  InputBuf[256];
	ImVector<char*>       Items;
	ImVector<const char*> Commands;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter       Filter;
	bool                  AutoScroll;
	bool                  ScrollToBottom;

	ExampleAppConsole()
	{
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		Commands.push_back("HELP");
		Commands.push_back("HISTORY");
		Commands.push_back("CLEAR");
		Commands.push_back("CLASSIFY");  // "classify" is only here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
		AutoScroll = true;
		ScrollToBottom = true;
	}
	~ExampleAppConsole()
	{
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	// Portable helpers
	static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
	static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
	static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
	static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str&& str_end[-1] == ' ') str_end--; *str_end = 0; }

	void    ClearLog()
	{
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
		ScrollToBottom = true;
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(Strdup(buf));
		if (AutoScroll)
			ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open)
	{
		const float footer_height_to_reserve = ImGui::GetFrameHeightWithSpacing() - 4.f; // 1 separator, 1 input text
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_WindowBg));
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
		ImGui::SameLine(4);
		ImGui::BeginGroup();
		ImGui::PopStyleColor();
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		for (int i = 0; i < Items.Size; i++)
		{
			const char* item = Items[i];
			if (!Filter.PassFilter(item))
				continue;

			// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
			bool pop_color = false;
			if (strstr(item, "[error]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; }
			else if (strstr(item, "[warning]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.4f, 1.0f)); pop_color = true; }
			else if (strstr(item, "[event]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f)); pop_color = true; }
			else if (strstr(item, "[debug]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f)); pop_color = true; }
			else if (strncmp(item, "# ", 2) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
			ImGui::TextUnformatted(item);
			if (pop_color)
				ImGui::PopStyleColor();
		}

		if (ScrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;
		ImGui::PopStyleVar();
		const ImGuiStyle backup_style = GImGui->Style;
		GImGui->Style.ItemSpacing = ImVec2(0, 1);

		ImVec2 window_pos = GImGui->CurrentWindow->Pos;
		ImVec2 window_size = GImGui->CurrentWindow->Size;

		ImVec2 min_top = window_pos;
		ImVec2 max_top = min_top + ImVec2(window_size.x, min(GImGui->CurrentWindow->Scroll.y, 20));
		ImVec2 min_bottom = min_top + ImVec2(0, window_size.y - min(GImGui->CurrentWindow->ScrollMax.y - GImGui->CurrentWindow->Scroll.y, 20));
		ImVec2 max_bottom = min_bottom + ImVec2(window_size.x, 20);

		if (GImGui->CurrentWindow->ScrollbarY)
		{
			ImColor color_windowbg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

			ImColor color_top = color_windowbg;
			ImColor color_bottom = color_top;
			color_bottom.value.w = 0.00f;

			if (GImGui->CurrentWindow->Scroll.y > 1.0f)
				GImGui->CurrentWindow->DrawList->AddRectFilledMultiColor(min_top, max_top, color_top, color_top, color_bottom, color_bottom);

			if ((ImGui::GetScrollMaxY() - GImGui->CurrentWindow->Scroll.y) > 1.f)
				GImGui->CurrentWindow->DrawList->AddRectFilledMultiColor(min_bottom, max_bottom, color_bottom, color_bottom, color_top, color_top);
		}
		ImGui::EndGroup();
		ImGui::EndChild();
		GImGui->Style = backup_style;

		// Command-line
		bool reclaim_focus = false;
		ImGui::SetNextItemWidth(GImGui->CurrentWindow->Size.x);
		if (ImGui::InputText("##Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* s = InputBuf;
			Strtrim(s);
			if (s[0])
				ExecCommand(s);
			strcpy(s, "");
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	}

	void    ExecCommand(const char* command_line)
	{
		AddLog("# %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}
		History.push_back(Strdup(command_line));

		// Process command
		if (Stricmp(command_line, "CLEAR") == 0)
		{
			ClearLog();
		}
		else if (Stricmp(command_line, "HELP") == 0)
		{
			AddLog("Commands:");
			for (int i = 0; i < Commands.Size; i++)
				AddLog("- %s", Commands[i]);
		}
		else if (Stricmp(command_line, "HISTORY") == 0)
		{
			int first = History.Size - 10;
			for (int i = first > 0 ? first : 0; i < History.Size; i++)
				AddLog("%3d: %s\n", i, History[i]);
		}
		else
		{
			AddLog("Unknown command: '%s'\n", command_line);
		}

		// On commad input, we scroll to bottom even if AutoScroll==false
		ScrollToBottom = true;
	}

	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int     TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < Commands.Size; i++)
				if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(Commands[i]);

			if (candidates.Size == 0)
			{
				// No match
				AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				AddLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("- %s\n", candidates[i]);
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
		}
		return 0;
	}
};

extern ExampleAppConsole g_Console;

class CMenu
{
public:
	CMenu();

	bool IsOpened();

	void Open();
	void Close();
	void Toggle();
	
	void CursorState();
	void DrawFadeBg();
	void Draw();

private:
	std::unique_ptr<CMenuGui> m_pGui;
	std::unique_ptr<CMenuPopupModal> m_pPopupModal;
	std::unique_ptr<CMenuHotkeys> m_pHotkeys;
	std::unique_ptr<CMenuSettingsList> m_pSettingsList;
	std::unique_ptr<CMenuFadeBg> m_pFadeBg;
	std::unique_ptr<CMenuThemes> m_pThemes;

	bool m_bIsOpened;

	ImVec2 m_WindowPos;
	ImVec2 m_WindowSize;
	ImVec2 m_WindowCanvasPos;
	ImVec2 m_WindowCanvasSize;

	int m_iSelectedTab;
	int m_iHoveredTab;

	void DialogSettingsSaveAs(char* buf, size_t size);
	void DialogSettingsRename(char* buf, size_t size);
	void DialogSettingsReset();
	void DialogSettingsRemove();
	void DialogFindAndReplace(char* buf);

	void DrawRage();
	void DrawLegit();
	void DrawVisuals();
};

extern std::unique_ptr<CMenu> g_pMenu;