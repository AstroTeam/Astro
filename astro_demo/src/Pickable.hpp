class Pickable : public Persistent {
public:
	enum PickableType {
		NONE, HEALER, CHARGER, LIGHTNING_BOLT, CONFUSER, FIREBALL, EQUIPMENT, FLARE, CURRENCY
	};
	bool stacks;
	int stackSize;
	PickableType type;
	Pickable(bool stacks,int stacksize, PickableType type = NONE);
	bool pick(Actor *owner, Actor *wearer);
	virtual bool use(Actor *owner, Actor *wearer);
	void drop(Actor *owner, Actor *wearer, bool isNPC = false);
	static Pickable *create(TCODZip &zip);
};

class Coinage: public Pickable {
public: 
	Coinage(bool stacks = true, int stackSize = 100, PickableType type = CURRENCY);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Healer: public Pickable {
public: 
	float amount; //how much it heals
	Healer(float amount, bool stacks = true, int stackSize = 1, PickableType type = HEALER);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Charger: public Pickable {
public: 
	float amount; //how much it recharges
	Charger(float amount, bool stacks = true, int stackSize = 1, PickableType type = CHARGER);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class LightningBolt: public Pickable {
public: 
	float range, damage;
	LightningBolt(float range, float damage, bool stacks = true, 
		int stackSize = 1, PickableType type = LIGHTNING_BOLT);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Fireball : public LightningBolt {
public:
	float maxRange;
	Fireball(float range, float damage, float maxRange,
		bool stacks = true, int stackSize = 1, PickableType type = FIREBALL);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Confuser : public Pickable {
public:
	int nbTurns;
	float range;
	
	Confuser(int nbTurns, float range,
		bool stacks = true, int stackSize = 1, PickableType type = CONFUSER);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Flare : public Pickable {
public:
	int nbTurns;
	float range;
	int lightRange;
	
	Flare(int nbTurns, float range, int lightRange, bool stacks = true, int stackSize = 1, PickableType type = FLARE);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class ItemBonus {
public:
	enum BonusType {
		NOBONUS, HEALTH, DODGE, ATTACK
	};	
	BonusType type;
	float bonus;
	
	ItemBonus(BonusType type, float bonus);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Equipment : public Pickable {
public:
	enum SlotType{
		NOSLOT,HEAD, CHEST, LEGS, FEET, HAND1, HAND2, RANGED
	};
	
	bool equipped;
	SlotType slot;
	ItemBonus *bonus;
	Equipment(bool equipped = false, SlotType slot = NOSLOT, ItemBonus *bonus = NULL,
		bool stacks = false, int stackSize = 1, PickableType type = EQUIPMENT);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};




