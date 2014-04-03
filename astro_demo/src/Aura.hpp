class Aura : public Persistent{
public:
	enum StatType {
		ALL, TOTALSTR, TOTALDEX, TOTALINTEL, TOTALDODGE, TOTALDR, HEALTH, MAXHEALTH
	};
	enum LifeStyle {
		CONTINUOUS, ITERABLE, HUNGER    
		//continunous is a one-time buff/debuff, iterable affects the actor every turn, and hunger is a special case that only removes upon eating
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