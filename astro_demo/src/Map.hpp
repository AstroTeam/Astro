struct Tile {
	bool explored;
	float infection;
	Tile() : explored(false),  infection (0) {}
	const TCODColor * lastColor;//if color changed offscreen
	char lastChar;//if the char changed
	
};


class Map : public Persistent {
public:
	int width, height;
	Map(int width, int height);
	~Map();
	bool isWall(int x, int y) const;
	bool canWalk(int x, int y) const;
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	bool isInfected(int x, int y) const;
	void infectFloor(int x, int y);
	void computeFov();
	void render() const;
	void init(bool withActors);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	Tile *tiles;
	
protected:
	TCODMap *map;
	long seed;
	TCODRandom *rng;
	friend class BspListener;
	
	void dig(int x1, int y1, int x2, int y2);
	void createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors);
	void addMonster(int x, int y);
	void addItem(int x, int y);
	void generateRandom(Actor *owner, int ascii);
};
