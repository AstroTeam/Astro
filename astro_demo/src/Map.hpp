namespace Param {
	enum LevelType {
		GENERIC,
		OFFICE_FLOOR
	};

	enum RoomType {
		STANDARD,
		OFFICE,
		BARRACKS,
		GENERATOR,
		SERVER,
		KITCHEN,
		MESSHALL,
		ARMORY,
		OBSERVATORY
	};
};

struct Tile {
	bool explored;
	float infection;
	bool lit;
	int num;
	Param::RoomType tileType;
	Tile() : explored(false), infection (0), lit(false),num (0), tileType(Param::STANDARD) {}
	const TCODColor * lastColor;//if color changed offscreen
	char lastChar;//if the char changed
};

struct Room {
	int x1;
	int y1;
	int x2;
	int y2;
	Param::RoomType type;
};


using namespace Param;

class Map : public Persistent {
public:
	int width, height;
	Map(int width, int height, short epicenterAmount=1);
	~Map();
	bool isWall(int x, int y) const;
	bool canWalk(int x, int y) const;
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	bool isInfected(int x, int y) const;
	bool infectionState(int x, int y) const;
	bool isLit(int x, int y) const;
	void infectFloor(int x, int y);
	void computeFov();
	void render() const;
	void init(bool withActors, LevelType levelType = GENERIC);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	
	int tileType(int x, int y);
	//int tileInf(int x, int y);
	
	Actor *createHealthPotion(int x, int y);
	Actor *createBatteryPack(int x, int y);
	Actor *createFlashBang(int x, int y);
	Actor *createFlare(int x, int y);
	Actor *createFireBomb(int x, int y);
	Actor *createEMP(int x,int y);
	Actor *createTitanMail(int x, int y);
	Actor *createMylarBoots(int x, int y);
	Actor *createMLR(int x, int y);
	
	Tile *tiles;

	
protected:
	TCODMap *map;
	long seed;
	TCODRandom *rng;
	friend class BspListener;
	short epicenterAmount;
	
	void dig(int x1, int y1, int x2, int y2);
	void createRoom(int roomNum, bool withActors, Room * room);
	void addMonster(int x, int y);
	void addItem(int x, int y, RoomType roomType);
	void generateRandom(Actor *owner, int ascii);
	TCODList<RoomType> * getRoomTypes(LevelType levelType);
};
