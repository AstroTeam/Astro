class Destructible : public Persistent {
public:
	float maxHp; //max health points
	float hp; //current hit points
	float baseDefense; //damage deflected
	float totalDefense; //damage deflected
	char *corpseName; //the actor's name once dead/destroyed
	int xp; //xp gained when killing this monster, or player xp
	
	float heal(float amount);
	Destructible(float maxHp, float defense, const char *corpseName, int xp);
	~Destructible();
	inline bool isDead() {return hp <= 0;}
	float takeDamage(Actor *owner, float damage);
	virtual void die(Actor *owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	static Destructible *create(TCODZip &zip);
	
protected:
	enum DestructibleType {
		MONSTER, PLAYER
	};
};

class MonsterDestructible : public Destructible {
public:
	MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp);
	void die (Actor *owner);
	void save(TCODZip &zip);
	void vendingMenu(Actor *owner);
};

class PlayerDestructible : public Destructible {
public:
	PlayerDestructible(float maxHp, float defense, const char *corpseName);
	void die(Actor *owner);
	void save(TCODZip &zip);
};