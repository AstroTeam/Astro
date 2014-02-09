class Attacker : public Persistent {
public:
	float basePower; //attack power
	float totalPower; //attack power
	Actor *lastTarget;
	
	Attacker(float power);
	void attack(Actor *owner, Actor *target);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};