namespace Param {
	enum LevelType {
		GENERIC,
		OFFICE_FLOOR,
		TUTORIAL
	};

	enum RoomType {
		STANDARD,  //done
		OFFICE,    //done - info papers with lore/statistics/mechanics for items, not done yet
		BARRACKS,  //done - lockers drop armor and flares
		GENERATOR, //done - can turn on power, not yet though
		SERVER,    //done - can look at a map of deck, not yet though
		KITCHEN,   //done - drops food from food processors, can go into refrigerators, not yet though, steal items, frozen people come for you
		MESSHALL,  //to-do - add tables and trash cans that drop items maybe?
		ARMORY,    //to-do - add gun racks and security bots and glass cases, locked door?  need to kill security bot to get in?
		OBSERVATORY, //to-do - breakable floor, no oxygen
		HYDROPONICS, //to-do - grass, spawns food in garden
		//lobby - stair room with seating
		//cult rooms - blood everywhere, crazy cult enemies that have high str, low hp
		//defended rooms - friendly marines inside, give you a gun and ammo, have a medic who will help you? scientist give you tech
		//THE BRIDGE - the win room, has a blackbox
		//laboratory - experiment with spores, infected scientists, 
		//infected room - all floor is infected level 6, epicenters in middle
	};
};

struct Tile {
	bool explored;
	float infection;
	int flower;
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
	Tile() : explored(false), infection (0), flower(-1), lit(false),num (0),drty(false), envSta(0),temperature(0),decoration(0), tileType(Param::STANDARD) {}
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
	Map(int width, int height, int artifacts = 0,short epicenterAmount=1);
	int artifacts;
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
	bool isVisible(int x, int y);
	
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
	Actor *createTurretControl(int x, int y);
	Actor *createVendor(int x, int y);
	Actor *createConsole(int x, int y);
	Actor *createSecurityBot(int x, int y);
	Actor *createInfectedEngineer(int x, int y);
	Actor *createGardner(int x, int y);
	
	Actor *createCurrencyStack(int x, int y);
	Actor *createHealthPotion(int x, int y);
	Actor *createKey(int x, int y, int keyType); //0 keytype = vaultkey
	Actor *createBatteryPack(int x, int y);
	Actor *createFlashBang(int x, int y);
	Actor *createFlare(int x, int y);
	Actor *createAlcohol(int x, int y);
	Actor *createFireBomb(int x, int y);
	Actor *createEMP(int x,int y);
	Actor *createTitanMail(int x, int y);
	Actor *createMylarBoots(int x, int y);
	Actor *createMLR(int x, int y);
	Actor *createCombatKnife(int x, int y);
	Actor *createFrag(int x, int y);
	Actor *createFood(int x, int y);
	Actor *createArtifact(int x, int y);
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
	void spawnTutorial();
};
