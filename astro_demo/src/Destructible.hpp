class Destructible : public Persistent {
public:
	float maxHp; //max health points
	float hp; //current hit points
	float baseDodge; //must be overcome by the attacker
	float totalDodge; //must be overcome by the attacker
	float baseDR; //base damage mitigated
	float totalDR; //base damage mitigated
	int xp; //xp gained when killing this monster, or player xp
	
	float heal(float amount);
	Destructible(float maxHp, float dodge, float dr, int xp);
	~Destructible();
	inline bool isDead() {return hp <= 0;}
	float takeDamage(Actor *owner, Actor *attacker, float damage);
	float takeFireDamage(Actor *owner, float damage);
	virtual void die(Actor *owner, Actor *killer);
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
	MonsterDestructible(float maxHp, float dodge, float dr, int xp);
	void suicide(Actor *owner);
	void die (Actor *owner, Actor *killer);
	void save(TCODZip &zip);
};

class PlayerDestructible : public Destructible {
public:
	PlayerDestructible(float maxHp, float dodge, float dr);
	void die(Actor *owner, Actor *killer);
	void save(TCODZip &zip);
};