
class Ai : public Persistent {
public:
	TCODConsole *inventoryScreen;
	
	virtual void update(Actor *owner) = 0;
	static Ai *create(TCODZip &zip);
	Actor *choseFromInventory(Actor *owner, int type, bool isVend);
protected:
	enum AiType {
		MONSTER, CONFUSED_ACTOR, PLAYER, EPICENTER, RANGED, LIGHT, FLARE
	};
};

class PlayerAi : public Ai {
public:
	int xpLevel;
	TCODConsole *inventoryScreen;
	
	PlayerAi();
	int getNextLevelXp();
	void update(Actor *owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
protected:
	bool moveOrAttack(Actor *owner, int targetx, int targety);
	void handleActionKey(Actor *owner, int ascii);
	Actor *choseFromInventory(Actor *owner, int type, bool isVend);
	void displayCharacterInfo(Actor *owner);
};

class MonsterAi : public Ai {
public:
	TCODConsole *inventoryScreen;
	
	MonsterAi();
	void update(Actor *owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
protected:
	int moveCount;
	
	void moveOrAttack(Actor *owner, int targetx, int targety);
};

class RangedAi : public Ai
{
	public:
		RangedAi();
		void update(Actor *owner);
		void load(TCODZip &zip);
		void save(TCODZip &zip);
	protected:
		int moveCount;
		int range; //range
		void moveOrAttack(Actor *owner, int targetx, int targety);
};

class EpicenterAi : public Ai {
public:
	EpicenterAi();
	void update(Actor * owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);

protected:
	//int turnCount;
	void infectLevel(Actor * owner);
};

class LightAi : public Ai {
public:
	LightAi(int rad, float f);
	LightAi(int rad, float f,bool move);
    float flkr;
	bool onAgn;
	bool onOff;
	bool frst;//to reset num
	bool moving;//are you static or moving
	void flicker(Actor * owner, float f);
	void update(Actor * owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
protected:
	int radius;
	TCODMap *lmap;
	TCODMap *frstMap;
	bool frstBool;
	int lstX;
	int lstY;
};

class FlareAi : public Ai {
public:
	FlareAi(int lightRange, int turns);
	int lightRange;
	void update(Actor * owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
protected:
	int turns;
	int i;
	Actor *light;
};


class ConfusedActorAi : public Ai {
public:
	ConfusedActorAi(int nbTurns, Ai *oldAi);
	void update(Actor *owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
protected:
	int nbTurns;
	Ai *oldAi;
};

class TechAi : public Ai
{
	public:
		TechAi();
		void update(Actor *owner);
		void load(TCODZip &zip);
		void save(TCODZip &zip);
		bool berserk;
	protected:
		int moveCount;
		int range; //range
		int numEmpGrenades;
		void moveOrAttack(Actor *owner, int targetx, int targety);
};



