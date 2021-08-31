struct CKnifebotPoint {
	int			hitbox;
	float		fraction;
	Vector		point;
};

class CKnifebot;
class CKnifebot {
public:
	void		Run(usercmd_s* cmd);
};
extern CKnifebot g_Knifebot;