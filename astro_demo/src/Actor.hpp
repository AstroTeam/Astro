class Actor : public Persistent {
public: 
	int x, y;	//map position
	int ch;		//ascii code for character representation 
	TCODColor col; //color for representation
	const char *name;  //the actor's name
	bool blocks; //can you walk over this guy?
	bool oozing; //is this guy infected?
	bool susceptible; //is this guy susceptible to the infection?
	int sort; //Actor type (1 = Item, 2 = Tech, 3 = Armor, 4 = Weapon)
	//bool fovOnly; //only display when in FOV (maybe add this later)
	Attacker *attacker; //something that deals damage
	Destructible *destructible; //something that can take damage
	Ai *ai; //something self-updating
	Pickable *pickable; //something that can be picked and used
	Container *container; //something that can contain items/have an inventory
	const char *race; //1=human, 2=robot, 3=alien
	const char *role; //1=marine, 2=explorer, 3=mercenary
	const char *job;  //1=infantry, 2=medic, 3=quartermaster
			  //4=survivalist, 5=pirate, 6=merchant
			  //7=assassin, 8=brute, 9=hacker

	Actor(int x, int y, int ch, const char *name, const TCODColor &col);
	Actor(int x, int y, int ch, const char *name, const char *race, const char *role, const char *job, const TCODColor &col);
	~Actor();
	void render() const;
	void update();
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	float getDistance(int cx, int cy) const;
};
