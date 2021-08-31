class CKnifebot;
class CKnifebot
{
private:
	bool Attack(CBasePlayer *pPlayer, struct usercmd_s *cmd);

	void CalculateDot(CBasePlayer *pPlayer, std::deque <Vector> &dDot);
public:
	void Run(struct usercmd_s *cmd);
};
extern CKnifebot g_KnifeBot;