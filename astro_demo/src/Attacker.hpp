class Attacker : public Persistent {
public:
	float basePower; //attack power
	float totalPower; //attack power
	float maxBattery; //max Battery capacity
	float battery; //current battery capacity
	Actor *lastTarget;
	
	Attacker(float power);
	Attacker(float power, float maxBattery);
	void attack(Actor *owner, Actor *target);
	void shoot(Actor *owner, Actor *target);
	float recharge(float amount); //recharges your battery
	float usePower(Actor *owner, float cost); //spends battery power
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};