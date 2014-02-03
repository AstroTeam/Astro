class Actor : public Persistent {
public: 
	int x, y;	//map position
	int ch;		//ascii code for character representation 
	TCODColor col; //color for representation
	const char *name;  //the actor's name
	bool blocks; //can you walk over this guy?
	//bool fovOnly; //only display when in FOV (maybe add this later)
	Attacker *attacker; //something that deals damage
	Destructible *destructible; //something that can take damage
	Ai *ai; //something self-updating
	Pickable *pickable; //something that can be picked and used
	Container *container; //something that can contain items/have an inventory

	Actor(int x, int y, int ch, const char *name, const TCODColor &col);
	~Actor();
	void render() const;
	void update();
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	float getDistance(int cx, int cy) const;
};