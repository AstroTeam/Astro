struct Tile {
	bool explored;
	float infection;
	Tile() : explored(true),  infection (false) {}
	const TCODColor * lastColor;//if color changed offscreen
	char lastChar;//if the char changed
};

namespace Param {
	enum LevelType {
		GENERIC,
		OFFICE_FLOOR
	};

	enum RoomType {
		STANDARD,
		OFFICE,
	};
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
	void infectFloor(int x, int y);
	void computeFov();
	void render() const;
	void init(bool withActors, LevelType levelType = GENERIC);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
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
