//#define array_size(_arr) (size_t)(sizeof(_arr) / sizeof(*_arr))

constexpr size_t cmd_list_size = 16;

struct cmd_list_s
{
	const char* name;
	unsigned short button;
};

class cmd
{
public:
	static void get_pointer(usercmd_s* cmd);
	static cmd_list_s* get_list();
	static unsigned short get_button(const char* cmd_name);
	static const char* get_name(const unsigned short button);
	static usercmd_s* get();
	
	static void execute(const unsigned short button);
	static void execute(const char* cmd_name);

	static void block(const unsigned short button);
	static void block(const char* cmd_name);

	static bool button_is(const unsigned short button);
	static bool button_is(const char* cmd_name);

	static bool oldbutton_is(const unsigned short button);
	static bool oldbutton_is(const char* cmd_name);

	static void sidemove(const float value);
	static void forwardmove(const float value);
	static void upmove(const float value);

	static void viewangles(const Vector& angles);
	
	static void clear();

	static void release_pointer();

private:
	static std::unique_ptr<usercmd_s> m_cmd;
	static unsigned short oldbuttons;
};