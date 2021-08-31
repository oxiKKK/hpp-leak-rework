class CKnifeBot
{
public:
	CKnifeBot();
	~CKnifeBot();

	void					Run(usercmd_s* cmd);

private:
	void					CorrectPhysentSolid(const int& nPlayerID);
};

extern std::unique_ptr<CKnifeBot> g_pKnifeBot;