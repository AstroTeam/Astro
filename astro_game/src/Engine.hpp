class Engine
{	
public:
	enum GameStatus{
		STARTUP,
		IDLE,
		NEW_TURN,
		VICTORY,
		DEFEAT} gameStatus;

	TCODConsole *mapcon;
	TCODConsole *mapconCpy;
	//TCODConsole *mapconDec;
	TCODList<Actor *> actors;
	TCODList<Actor *> purchases;
	Actor *player;
	Actor *playerLight;
	Actor *stairs;
	Actor *boss;
	Map *map;
	//Map *mapCpy;
	//Renderer *rend;
	bool valTer[13];
	int ctrTer;
	int numTer;           
	bool bonusTer;
	
	int invState;
	int invFrames;
	int menuState;
	int armorState;
	int selX;
	int selY;
	bool printMap;
	bool startTutorial;
	
	int mapx1;
	int mapx2;
	int mapy1;
	int mapy2;
	int fovRadius;
	int screenWidth;
	int screenHeight;
	int mapWidth;
	int mapHeight;
	int level;
	int turnCount;
	int killCount;
	int piratesFound; //is one if you are a pirate
	float damageDone;
	float damageReceived;
	Gui *gui;
	TCOD_key_t lastKey;
	//TCOD_mouse_t mouse; comment this out to remove mouse look
	
	//Engine();
	Engine(int screenWidth, int screenHeight);
	~Engine();
	void update();
	void render();
	void nextLevel();
	void sendToBack(Actor *actor);
	Actor *getClosestMonster(int x, int y, float range) const;
	bool pickATileForInfoScreen(int *x, int *y, float maxRange = 0.0f, float AOE = 0.0f);
	bool pickATile(int *x, int *y, float maxRange = 0.0f, float AOE = 0.0f);
	Actor *getActor(int x, int y) const;
	Actor *getAnyActor(int x, int y) const;
	float distance(int x1,int x2,int y1,int y2);
	void init();
	void load(bool pause);
	void save();
	void term();
	void win();
	void classMenu();
	void classSelectMenu(int cat);
	int getInitVit(int race, int job);
	int getInitStr(int race, int job);
	int getInitDex(int race, int job);
	int getInitIntel(int race, int job);
};

extern Engine engine;