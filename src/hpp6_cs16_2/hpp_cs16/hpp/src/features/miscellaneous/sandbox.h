class CSandbox
{
public:
	CSandbox();
	~CSandbox();

	void SetupCvar(std::string name, std::string value);
	std::string GetCvar(std::string name);

private:
	std::map<std::string, std::string> cvars;
};

extern std::unique_ptr<CSandbox> g_pSandbox;