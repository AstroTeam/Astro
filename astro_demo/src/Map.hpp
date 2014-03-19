namespace Param {
	enum LevelType {
		GENERIC,
		OFFICE_FLOOR
	};

	enum RoomType {
		STANDARD,  //done lol
		OFFICE,    //done
		BARRACKS,  //done* -> must add dropping items from lockers
		GENERATOR, //must populate with decor
		SERVER,    //arting
		KITCHEN,   //almost done
		MESSHALL,  //to-do
		ARMORY,    //to-do
		OBSERVATORY//to-do
	};
};

struct Tile {
	bool explored;
	float infection;
	//lighting stuff
	bool lit;
	int num;
	bool drty;
	//environment stuff
	int envSta;
	int temperature;
	//decoration int
	int decoration;
	//add number of flowers and a boolean to grow them when epicenter updates
	//could have flower levels?  easier to make flower tiles to overlay than keep track of x,y's for every flower on a tile
	Param::RoomType tileType;
	Tile() : explored(false), infection (0), lit(false),num (0),drty(false), envSta(0),temperature(0),decoration(0), tileType(Param::STANDARD) {}
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
	int infectionState(int x, int y) const;
	bool isLit(int x, int y) const;
	void infectFloor(int x, int y);
	void computeFov();
	void render() const;
	void init(bool withActors, LevelType levelType = GENERIC);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	
	int tileType(int x, int y);
	//int tileInf(int x, int y);
	Actor *createCleanerBot(int x, int y);
	Actor *createInfectedCrewMember(int x, int y);
	Actor *createInfectedNCO(int x, int y);
	Actor *createInfectedOfficer(int x, int y);
	Actor *createInfectedMarine(int x, int y);
	Actor *createInfectedGrenadier(int x, int y);
	Actor *createSporeCreature(int x, int y);
	Actor *createMiniSporeCreature(int x, int y);
	Actor *createTurret(int x, int y);
	Actor *createVendor(int x, int y);
	Actor *createSecurityBot(int x, int y);
	Actor *createInfectedEngineer(int x, int y);
	
	Actor *createCurrencyStack(int x, int y);
	Actor *createHealthPotion(int x, int y);
	Actor *createBatteryPack(int x, int y);
	Actor *createFlashBang(int x, int y);
	Actor *createFlare(int x, int y);
	Actor *createFireBomb(int x, int y);
	Actor *createEMP(int x,int y);
	Actor *createTitanMail(int x, int y);
	Actor *createMylarBoots(int x, int y);
	Actor *createMLR(int x, int y);
	Actor *createCombatKnife(int x, int y);
	void generateRandom(Actor *owner, int ascii);
	Tile *tiles;

	
protected:
	TCODMap *map;
	long seed;
	TCODRandom *rng;
	friend class BspListener;
	short epicenterAmount;
	
	void dig(int x1, int y1, int x2, int y2);
	void createRoom(int roomNum, bool withActors, Room * room);
	void addMonster(int x, int y, bool isHorde); //is this monster part of a horde?
	void addItem(int x, int y, RoomType roomType);
	TCODList<RoomType> * getRoomTypes(LevelType levelType);
};
