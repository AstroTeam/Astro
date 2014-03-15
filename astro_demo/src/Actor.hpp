class Actor : public Persistent {
public: 
	int x, y;	//map position
	int ch;		//ascii code for character representation 
	int str, dex, intel, vit, totalStr, totalDex, totalIntel; //strength, dexterity, intelligence, vitality
	TCODColor col; //color for representation
	const char *name;  //the actor's name
	const char *race;  //the actor's race (default human)
	const char *role;  //the actor's role (default Marine)
	const char *job;   //the actor's job (default Infantry)
	bool blocks; //can you walk over this guy?
	bool smashable; //can this be smashed?
	bool oozing; //is this guy infected?
	bool susceptible; //is this guy susceptible to the infection?
	int sort; //Actor type (1 = Item, 2 = Tech, 3 = Armor, 4 = Weapon)
	//bool fovOnly; //only display when in FOV (maybe add this later)
	Attacker *attacker; //something that deals damage
	Destructible *destructible; //something that can take damage
	Ai *ai; //something self-updating
	Pickable *pickable; //something that can be picked and used
	Container *container; //something that can contain items/have an inventory
	

	Actor(int x, int y, int ch, const char *name, const TCODColor &col);
	Actor(int x, int y, int ch, const char *name, const char *race, const char *role, const char *job, const TCODColor &col);
	~Actor();
	void render() const;
	void update();
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	float getDistance(int cx, int cy) const;
	float getHpUp();
};
