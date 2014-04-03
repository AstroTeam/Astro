class Aura : public Persistent{
public:
	enum StatType {
		ALL, TOTALSTR, TOTALDEX, TOTALINTEL, TOTALDODGE, TOTALDR, HEALTH, MAXHEALTH
	};
	enum LifeStyle {
		CONTINUOUS, ITERABLE
	};
	
	Actor *target;
	int duration;
	StatType stat;
	LifeStyle life;
	int bonus;
	
	Aura(int duration, StatType stat, int bonus);
	void save(TCODZip &zip);
	void load(TCODZip &zip);
	void update();
};