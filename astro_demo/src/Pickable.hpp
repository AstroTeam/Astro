class Pickable : public Persistent {
public:
	bool pick(Actor *owner, Actor *wearer);
	virtual bool use(Actor *owner, Actor *wearer);
	void drop(Actor *owner, Actor *wearer);
	static Pickable *create(TCODZip &zip);
protected:
	enum PickableType {
		HEALER, LIGHTNING_BOLT, CONFUSER, FIREBALL
	};
};

class Healer: public Pickable {
public: 
	float amount; //how much it heals
	
	Healer(float amount);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class LightningBolt: public Pickable {
public: 
	float range, damage;
	LightningBolt(float range, float damage);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Fireball : public LightningBolt {
public:
	float maxRange;
	Fireball(float range, float damage, float maxRange);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};

class Confuser : public Pickable {
public:
	int nbTurns;
	float range;
	
	Confuser(int nbTurns, float range);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};
