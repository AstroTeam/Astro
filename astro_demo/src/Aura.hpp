class Aura : public Persistent{
public:
	enum StatType {
		ALL, TOTALSTR, TOTALDEX, TOTALINTEL, TOTALDODGE, TOTALDR, HEALTH, MAXHEALTH
	};
	enum LifeStyle {
		CONTINUOUS, ITERABLE, HUNGER
	};
	
	int duration;
	int totalDuration;
	StatType stat;
	LifeStyle life;
	int bonus;
	
	Aura(int duration, StatType stat, LifeStyle life, int bonus);
	void save(TCODZip &zip);
	void load(TCODZip &zip);
	void update();
	void apply(Actor *target);
	void unApply(Actor *target);
};