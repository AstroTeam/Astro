
class Ai : public Persistent {
public:
	TCODConsole *inventoryScreen;
	
	virtual void update(Actor *owner) = 0;
	static Ai *create(TCODZip &zip);
	Actor *choseFromInventory(Actor *owner, int type, bool isVend);
protected:
	enum AiType {
		MONSTER, SECURITY, CONFUSED_ACTOR, PLAYER, EPICENTER, RANGED, LIGHT, 
		FLARE, GRENADIER, TURRET, CLEANER, INTERACTIBLE, VENDING, ENGINEER

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

class SecurityBotAi: public MonsterAi
{
	public:
		SecurityBotAi();
		void update(Actor *owner);
		void load(TCODZip &zip);
		void save(TCODZip &zip);
		int vendingX, vendingY; //the position of security bot's vending machine
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

class GrenadierAi : public Ai
{
	public:
		GrenadierAi();
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

class TurretAi : public Ai
{
	public:
		TurretAi();
		void update(Actor *owner);
		void load(TCODZip &zip);
		void save(TCODZip &zip);
	protected:
		int range;
		void attack(Actor *owner, int targetx, int target);

};


class CleanerAi : public Ai
{
	public:
		CleanerAi();
		void update(Actor *owner);
		void load(TCODZip &zip);
		void save(TCODZip &zip);
		bool active;
	protected:
		int moveCount;
		float cleanPower;
		void moveOrClean(Actor *owner); //should I make cleaners attack sporecreatures?
};
class EngineerAi: public Ai
{
public:
	EngineerAi(float repairPower, int deployRange);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	void update(Actor *owner);
	int turretX;
	int turretY;
	bool turretDeployed;
	float repairPower; //how much the engineer repairs the turret per turn
	int deployRange; //the max distance between the player and the engineer that will allow the engineer to deploy his turret
	int moveCount;
	void moveOrBuild(Actor *owner, int targetx, int targety); 
	

};
class InteractibleAi: public Ai
{
public:
	InteractibleAi();
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	void update(Actor *owner);
	virtual void interaction(Actor *owner, Actor *target);
};

class VendingAi: public InteractibleAi{
public:
	VendingAi();
	bool deployedSecurity;
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	void interaction(Actor *owner, Actor *target);
};



