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
	TCODList<Actor *> actors;
	Actor *player;
	Actor *stairs;
	Map *map;
	//Map *mapCpy;
	//Renderer *rend;
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
	bool pickATile(int *x, int *y, float maxRange = 0.0f, float AOE = 0.0f);
	Actor *getActor(int x, int y) const;
	Actor *getAnyActor(int x, int y) const;
	float distance(int x1,int x2,int y1,int y2);
	void init();
	void load(bool pause);
	void save();
	void term();
	void win();
};

extern Engine engine;