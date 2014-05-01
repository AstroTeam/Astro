#include "main.hpp"

#include <cstring>
using namespace Param;
using namespace std;


static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;
static const int MAX_ARTIFACTS = 2;


class BspListener : public ITCODBspCallback {
public:
	bool bspActors;
	TCODList<RoomType> * roomList;

	
private:
	Map &map; //a map to dig
	int roomNum; //room number
	int lastx, lasty; // center of the last room
	

public:
	BspListener(Map &map) : map(map), roomNum(0) {}
	
	bool visitNode(TCODBsp *node, void *userData) {
		if (node->isLeaf()) {

			int x,y,w,h;
			bool withActors = bspActors;

			//dig a room
			w=map.rng->getInt(ROOM_MIN_SIZE, node->w-2);
			h=map.rng->getInt(ROOM_MIN_SIZE, node->h-2);
			x=map.rng->getInt(node->x+1, node->x+node->w-w-1);
			y=map.rng->getInt(node->y+1, node->y+node->h-h-1);
		
			//save info in a Room struct
			Room * room = new Room();
			room->x1 = x;
			room->y1 = y;
			room->x2 = x+w-1;
			room->y2 = y+h-1;
			
			std::cout << "room " << room->x1 << " " << room->y1 << " " << room->x2 << " " << room->y2 << std::endl;
			
			//will this room be special?
			int index = map.rng->getInt(0, roomList->size()-1);
			if (index < roomList->size()) {
				room->type = roomList->get(index);
				roomList->remove(room->type);
			}
			else {
				room->type = STANDARD;
			}

			map.createRoom(roomNum, withActors, room);
			
			if (roomNum != 0) {
				//dig a corridor from last room
				map.dig(lastx, lasty, x+w/2, lasty);
				map.dig(x+w/2, lasty, x+w/2, y+h/2);
			}
			
			lastx = x+w/2;
			lasty = y+h/2;
			cout << roomList->size() << endl;
			roomNum++;
			
		}
		return true;
	}
};

Map::Map(int width, int height,int artifacts, short epicenterAmount): width(width),height(height),artifacts(artifacts),epicenterAmount(epicenterAmount) {
	seed = TCODRandom::getInstance()->getInt(0,0x7FFFFFFF);
	cout<< "seed " << seed << endl;
}

Map::~Map() {
	delete [] tiles;
	delete map;
}

int Map::tileType(int x, int y) {
	int i = x+y*width;
	if (tiles[i].tileType == Param::OFFICE)//
	{return 2;}
	else if (tiles[i].tileType == Param::BARRACKS)//
	{return 3;}
	else if (tiles[i].tileType == Param::GENERATOR)//
	{return 4;}
	else if (tiles[i].tileType == Param::KITCHEN)//
	{return 5;}
	else if (tiles[i].tileType == Param::SERVER)//
	{return 6;}
	else if (tiles[i].tileType == Param::MESSHALL)//
	{return 7;}
	else if (tiles[i].tileType == Param::ARMORY)
	{return 8;}
	else if (tiles[i].tileType == Param::OBSERVATORY)//
	{return 9;}
	else if (tiles[i].tileType == Param::HYDROPONICS)//
	{return 10;}
	else if (tiles[i].tileType == Param::DEFENDED_ROOM)//
	{return 11;}
	else if (tiles[i].tileType == Param::BAR)//
	{return 12;}
	else if (tiles[i].tileType == Param::INFECTED_ROOM)//
	{return 13;}
	else if (tiles[i].tileType == Param::DISPLAY)//
	{return 14;}
	else
	{return 1;}
	//return tiles[x*y].tileType;
}

void Map::init(bool withActors, LevelType levelType) {
	cout << levelType << endl << endl;

	cout << "used seed " << seed << endl;
	rng = new TCODRandom(seed,TCOD_RNG_CMWC);
	tiles = new Tile[width*height];
	map = new TCODMap(width, height);
	type = levelType;
	cout << width << " , " << height << endl;
	if (levelType != TUTORIAL) {
		TCODBsp bsp(0,0,width,height);
		bsp.splitRecursive(rng,8,ROOM_MAX_SIZE,ROOM_MAX_SIZE,1.5f, 1.5f);
		BspListener listener(*this);
		listener.bspActors = withActors;
		listener.roomList = getRoomTypes(type); bsp.traverseInvertedLevelOrder(&listener, (void *)withActors); 

		//Create boss, for now it is a simple security bot
		if (withActors) {
			if (engine.level == 1) {
				//creating the final boss
				int level = engine.level;
				float scale = 1 + .1*(level - 1);
				float zHp = 60*scale;
				float zDodge = 1*scale;
				float zDR = .5*scale;
				float zStr = 20*scale;
				float zDex = 20*scale;
				float zXp = 100*scale;

				Actor *z = new Actor(engine.stairs->x+1, engine.stairs->y,136,"Zed Umber",TCODColor::white);
				Actor *boss = z;
				z->destructible = new MonsterDestructible(zHp,zDodge,zDR,zXp);
				z->totalStr = zStr;
				z->totalDex = zDex;
				z->attacker = new Attacker(zStr);
				z->container = new Container(2);
				z->ai = new ZedAi();
				//generateRandom(z, zAscii);

				boss->name = "Zed Umber";
				boss->ch = 136;
				boss->destructible->hp = boss->destructible->hp*2;
				boss->destructible->maxHp = boss->destructible->hp;
				boss->totalStr = boss->totalStr*.75;
				boss->totalDex = boss->totalStr*.75;

				generateRandom(z, 'Z');

				engine.actors.push(z);
				engine.boss = boss;
			}
			else {
				Actor *boss = createSecurityBot(engine.stairs->x+1, engine.stairs->y);
				boss->name = "Infected Security Bot";
				boss->ch = 146;
				boss->destructible->hp = boss->destructible->hp*2;
				boss->destructible->maxHp = boss->destructible->hp;
				boss->totalStr = boss->totalStr*1.25;
				engine.boss = boss;
			}
		}
		if (engine.level > 1 && artifacts > 0) {
			engine.gui->message(TCODColor::red,"The air hums with unknown energy... Perhaps there is an artifact of great power here!");
		}
	}
	else {
		engine.boss = NULL;
		spawnTutorial();
	}
}

void Map::save(TCODZip &zip) {
	zip.putInt(seed);
	zip.putInt(type);
	cout << "saved seed " << seed << endl;
	for (int i = 0; i < width*height; i++) {
		zip.putInt(tiles[i].explored);
		zip.putFloat(tiles[i].infection);
		zip.putInt(tiles[i].tileType);
		zip.putInt(tiles[i].decoration);
		zip.putInt(tiles[i].num);
		zip.putInt(tiles[i].lit);
		zip.putInt(tiles[i].drty);
		zip.putInt(tiles[i].envSta);
		zip.putInt(tiles[i].temperature);
		zip.putInt(tiles[i].flower);
	}
}

void Map::load(TCODZip &zip) {
	seed = zip.getInt();
	type = (LevelType)zip.getInt();
	cout << "loaded seed " << seed << endl;
	init(false,type);
	for (int i = 0; i <width*height; i++) {
		tiles[i].explored = zip.getInt();
		tiles[i].infection = zip.getFloat();
		tiles[i].tileType = (RoomType)zip.getInt();
		tiles[i].decoration = zip.getInt();
		tiles[i].num = zip.getInt();
		tiles[i].lit = zip.getInt();
		tiles[i].drty = zip.getInt();
		tiles[i].envSta = zip.getInt();
		tiles[i].temperature = zip.getInt();
		tiles[i].flower = zip.getInt();
	}
}
	
void Map::dig(int x1, int y1, int x2, int y2) {
	if(x2 < x1) {
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	if(y2 < y1) {
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	TCODRandom *rng = TCODRandom::getInstance();
	for (int tilex = x1; tilex <=x2; tilex++) {
		for (int tiley = y1; tiley <= y2; tiley++) {

			map->setProperties(tilex,tiley,true,true);
			Actor* a = NULL;
			a = engine.getAnyActor(tilex,tiley);
			bool blarg = false;
			if (a != NULL)
			{
				if (a->smashable)
				{
					//engine.actors.remove(a);
					//CHANGE THE SPRITE TO BROKEN CABINET
					//CHANGE THE NAME TO BROKEN CABINET
					
					//engine.gui->message(TCODColor::red, "playery  %d",plyy);
					//cout << "breaking cabinet";
					//filing cabinets, counters, ovens, refrigerators
					//to-do: server
					a->blocks = false;
					if (a->ch == 243)//new decor
					{
						
						if(strcmp(a->name,"Kitchen Counter") == 0 || strcmp(a->name,"oven-stove combo") == 0 )//counter
						{
							//engine.mapconDec->setChar(a->x,a->y,41);//destroyed counter
							engine.map->tiles[a->x+a->y*engine.map->width].decoration = 41;
							a->name = "destroyed countertop";
						}
						else if (strcmp(a->name,"a filing cabinet") == 0)
						{
							engine.map->tiles[a->x+a->y*engine.map->width].decoration = -1;
							a->name = "destroyed filing cabinet";
						}
						else if (strcmp(a->name,"A server") == 0)
						{
							engine.map->tiles[a->x+a->y*engine.map->width].decoration = 100;
							a->name = "Server Room Doorway";
							blarg = true;
							////////////////////////////////////////////////////////////////////////////////IS THIS OKAY?
							engine.actors.remove(a);
						}
					}
					else//just in case error
					{
						a->ch = 241;
						engine.map->tiles[a->x+a->y*engine.map->width].decoration = 0;
						a->name = "Debris";
						
					}
					if (!blarg)
					{
						engine.sendToBack(a);
						
						
						int x = a->x;
						int y = a->y;
						int n =  0;
						if (engine.map->tileType(x,y) == 2)//if it is an office
						{
							n = rng->getInt(5,8);
						}
						else
						{
							n = 0;
						}
						int add = rng->getInt(0,10);
						for (int xxx = -1; xxx <= 1; xxx++)/////////////////////9x9 for loop to add papers, lol xxx,yyy
						{
							for (int yyy = -1; yyy <= 1; yyy++)
							{
								if (add > 3 )
								{
									if (engine.map->tiles[(x+xxx)+(y+yyy)*engine.map->width].decoration == 0) {
										//engine.mapconDec->setChar(x+xxx, y+yyy, n);
										engine.map->tiles[(x+xxx)+(y+yyy)*engine.map->width].decoration = n;
										
									}
								}
								////
								if (engine.map->tileType(x,y) == 2)//if it is an office
								{
									n = rng->getInt(5,8);
								}
								else
								{
									n = 0;
								}
								add = rng->getInt(0,10);
							}
						}
					}
					
					
				}
			}
			//delete a;
		}
	}
}

void Map::spawnTutorial() {
	//this resets the level so it'll be 1 for the real level 1
	engine.level = 0;
	cout << engine.mapWidth << " , " << engine.mapHeight << endl;
	int x1 = engine.mapWidth/2-6;
	int x2 = engine.mapWidth/2+6;
	int y1 = -20 +engine.mapHeight-12;
	int y2 = -20 +engine.mapHeight-7;
	cout << "creating tutorial room"<< endl;
	
	for (int tilex = x1; tilex <=x2; tilex++) {//first room
		for (int tiley = y1; tiley <= y2; tiley++) {

			map->setProperties(tilex,tiley,true,true);
		}
	}
	dig((x1+x2)/2,y1,(x1+x2)/2,y1-10);

	//make interaction terminal
	Actor *triggerTileI = new Actor(x1+1,y1+1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileI->ai = new TriggerAi(  
	"INTERACTION BASICS\n\n"
	"Most items can be interacted with by simply moving into them by pressing the corresponding movement key whilst being adjacent to it. (Like these automated terminals)");
	triggerTileI->blocks = false; 
	engine.actors.push(triggerTileI);
	cout << "got past first terminal" << endl;
	//make FOV terminal
	Actor *triggerTileFov = new Actor(x2-1,y2-1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileFov->ai = new TriggerAi(  
	"FOV BASICS\n\n"
	"The small area you can see is called your \"FOV\" and is visible due to your flashlight.");
	triggerTileFov->blocks = false; 
	engine.actors.push(triggerTileFov);
	
	//make health xp terminal
	Actor *triggerTileX = new Actor(x1+1,y2-1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileX->ai = new TriggerAi(  
	"HEALTH/XP BASICS\n\n"
	"In the top left is your health and XP bar (as well as others). Keep an eye out: if your health reaches 0, you die and cannot continue. All saves are deleted.  Your XP goes up when fighting enemies and when reaches it's max you level up!");
	triggerTileX->blocks = false; 
	engine.actors.push(triggerTileX);
	
	//make character info terminal
	Actor *triggerTileCh = new Actor(x2-1,y1+1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileCh->ai = new TriggerAi(  
	"CHARACTER INFO BASICS\n\n"
	"Pressing the \'c\' key will bring up the character info screen where you can view any and all pertinent stats about your character.");
	triggerTileCh->blocks = false; 
	engine.actors.push(triggerTileCh);

	//make looking terminal
	Actor *triggerTileLo = new Actor((x1+x2)/2,(y1+y2)/2+2, 227, "Intercom Terminal", TCODColor::white);
	triggerTileLo->ai = new TriggerAi(  
	"LOOKING BASICS\n\n"
	"Pressing the \'l\' key will allow you to look around. All info about the tile you are looking at will display in the \"tile info\" screen in the bottom right.  Press \'ENTER\' to escape the look command.");
	triggerTileLo->blocks = false; 
	engine.actors.push(triggerTileLo);
	
	
	//make light terminal
	Actor *triggerTileL = new Actor((x1+x2)/2, y1-5, 227, "Intercom Terminal", TCODColor::white);
	triggerTileL->ai = new TriggerAi(  
	"LIGHTING BASICS.\n\n"
	"Above you is a light.\n"
	"The Astro has gone dark, but the crew has put up some emergency lights to help. The ones on this deck seem to work, but others may flicker or even be broken.");
	triggerTileL->blocks = false;
	engine.actors.push(triggerTileL);
	
	
	//map.dig(lastx, lasty, x+w/2, lasty);
	x1 = engine.mapWidth/2-10;
	x2 = engine.mapWidth/2+10;
	y1 = -20 +engine.mapHeight-29;
	y2 = -20 +engine.mapHeight-23;
	for (int tilex = x1; tilex <=x2; tilex++) {//light room
		for (int tiley = y1; tiley <= y2; tiley++) {

			map->setProperties(tilex,tiley,true,true);
		}
	}

	//make light room terminal
	Actor *triggerTileLR = new Actor((x1+x2)/2, (y1+y2)/2+1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileLR->ai = new TriggerAi(  
	"EXPLORE TO SURVIVE\n\n"
	"To the left are some of the Astro's more varied room types to explore.\n\n"
	"To the right is a range where you can test your mettle against some target dummies.\n\n"
	"To the north is the teleporter: the exit to the deck.");
	triggerTileLR->blocks = false;
	engine.actors.push(triggerTileLR);
	
	cout << "got past light room terminal" << endl;
	
	//add server and console
	tiles[x2+y1*engine.mapWidth].tileType = Param::SERVER;
	tiles[x2-1+y1*engine.mapWidth].tileType = Param::SERVER;
	Actor * server1 = new Actor(x2-1, y1, 243, "A server", TCODColor::white);
	engine.map->tiles[x2-1+y1*engine.map->width].decoration = rng->getInt(45,47);
	engine.actors.push(server1);
	createConsole(x2,y1);

	//make console terminal
	Actor *triggerTileC = new Actor(x2, y1+1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileC->ai = new TriggerAi(  
	"CONSOLE BASICS.\n\n"
	"Above you is a console. Consoles appear in server rooms.  Upon interacting with one, it will show you a map of the current deck. And will print out a copy for you to take.  Press \'m\' to take out your printed map.");
	triggerTileC->blocks = false;
	engine.actors.push(triggerTileC);
	
	//add infection
	tiles[x1+y1*engine.mapWidth].infection = 5.1;
	tiles[x1+1+y1*engine.mapWidth].infection = 3.2;
	tiles[x1+(y1+1)*engine.mapWidth].infection = 2.1;
	tiles[x1+1+(y1+1)*engine.mapWidth].infection = 2.2;
	tiles[x1+(y1+2)*engine.mapWidth].infection = 1.1;
	tiles[x1+1+(y1+2)*engine.mapWidth].infection = 1.1;
	tiles[x1+2+(y1+2)*engine.mapWidth].infection = 1.1;
	tiles[x1+3+(y1)*engine.mapWidth].infection = 2.1;
	tiles[x1+y1*engine.mapWidth].flower = 9;
	tiles[x1+1+y1*engine.mapWidth].flower = 2;

	//make INFECTION terminal
	Actor *triggerTileInf = new Actor(x1+1, y1+1, 228, "Odd Intercom Terminal", TCODColor::white);
	triggerTileInf->ai = new TriggerAi(  
	"FHB#KKWIinfect%#IONOWKKKDALLLS\n\n"
	"JFWEBLBFWOEIFHEIUFHJKFNXSAKJDS\n"
	"FUEBFJBFDVIEHUFE*##EHUWBBFDWW2\n"
	"EUFHEUDDWDJO)($NNFKJFUFEUBBFEA\n"
	"FRGLReverywhereLUHFUEFHWEUIFD6\n"
	"FEWF032NENFKJSNF32JNK%DJFNEEWF\n"
	"HFEWFWKEFEWJFstayKEFBLWEFBS8*9\n"
	"FEFawayFBEFBEBFE%!JDFBEFFEFI8K\n"
	"54UEFFBDJG32UYG&IOAZMUHXEEWPPP\n"
	"FWEBFWEUFU%2LUFELUFBrunEFEFEWQ\n"
	"HBEWFGELUFGWGUI&#UWIUFHPXO240F\n"
	"6439UFGHEUFGEBCDHShelpUFUAOIII\n"
	"UFUEGFUno#hopeUHFLIUEFSAUU72UF\n"
	);
	triggerTileInf->blocks = false;
	engine.actors.push(triggerTileInf);
	
	
	dig((x1+x2)/2,y1,(x1+x2)/2,y1-10);//upper hallway
	dig(x2,(y1+y2)/2,x2+5,(y1+y2)/2);//right hallway
	dig(x1,(y1+y2)/2,x1-5,(y1+y2)/2);//left dogleg hallway
	dig(x1-5,(y1+y2)/2,x1-5,y2+3);
	//make flare
	Actor *flare = createFlare(x1-5,(y1+y2)/2);
	engine.actors.push(flare);

	//make flare terminal
	Actor *triggerTileF = new Actor(x1-4, (y1+y2)/2, 227, "Intercom Terminal", TCODColor::white);
	triggerTileF->ai = new TriggerAi(  
	"FLARE/ITEM BASICS.\n\n"
	"To your left is a flare. Press \'g\' to pick it up and then \'i\' to open the inventory. Select the \'TECH\' tab with \'enter\' to find the flare. Press the letter next to it to use it to explore the rooms to the south.");
	triggerTileF->blocks = false;
	engine.actors.push(triggerTileF);
	
	float flkr = 1.0;
	Actor *light = new Actor(((x1+x2)/2), ((y1+y2)/2), 224, "A hastily erected Emergency Light", TCODColor::white);
	light->ai = new LightAi(4,flkr);                //224, crashes when using 224
	engine.actors.push(light);
	//Actor *light2 = new Actor(((x1+x2)/2+5), ((y1+y2)/2), 224, "A hastily erected Emergency Light-right", TCODColor::white);
	//light2->ai = new LightAi(4,flkr);                //224, crashes when using 224
	//engine.actors.push(light2);
	//Actor *light3 = new Actor(((x1+x2)/2-5), ((y1+y2)/2), 224, "A hastily erected Emergency Light-left", TCODColor::white);
	//light3->ai = new LightAi(4,flkr);                //224, crashes when using 224
	//engine.actors.push(light3);
	
	x1 = engine.mapWidth/2-12-5;
	x2 = engine.mapWidth/2-8-5;
	y1 = -20 +engine.mapHeight-11-8;
	y2 = -20 +engine.mapHeight-7-8;
	for (int tilex = x1; tilex <=x2; tilex++) {//side room upper
		for (int tiley = y1; tiley <= y2; tiley++) {

			map->setProperties(tilex,tiley,true,true);
			tiles[tilex+tiley*engine.mapWidth].tileType = Param::KITCHEN;
		}
	}
	Actor * counter = new Actor(x1, y1,243,"Kitchen Counter", TCODColor::white);
	engine.map->tiles[x1+y1*engine.map->width].decoration = 35;
	engine.actors.push(counter);
	Actor * counter2 = new Actor(x1+1, y1,243,"Kitchen Counter", TCODColor::white);
	engine.map->tiles[x1+1+y1*engine.map->width].decoration = 35;
	engine.actors.push(counter2);
	Actor * counter3 = new Actor(x1+3, y1,243,"Kitchen Counter", TCODColor::white);
	engine.map->tiles[x1+3+y1*engine.map->width].decoration = 35;
	engine.actors.push(counter3);
	Actor * counter4 = new Actor(x1+4, y1,243,"Kitchen Counter", TCODColor::white);
	engine.map->tiles[x1+4+y1*engine.map->width].decoration = 35;
	engine.actors.push(counter4);

	//make kitchen terminal
	Actor *triggerTileK = new Actor(x1+2,y1+1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileK->ai = new TriggerAi(  
	"FOOD BASICS.\n\n"
	"Kitchens have food machines.\n\n"
	"Interact with the one on the right to dispense some food to increase your hunger bar. This averts losing stats and health from being too hungry.");
	triggerTileK->blocks = false; 
	engine.actors.push(triggerTileK);
	
	
	Actor *sink = new Actor(x1+2, y1+2,243,"Industrial Sink", TCODColor::white);
	engine.map->tiles[x1+2+(y1+2)*engine.map->width].decoration = 38;
	engine.actors.push(sink);
	Actor *sink2 = new Actor(x1+2, y1+3,243,"Industrial Sink", TCODColor::white);
	engine.map->tiles[x1+2+(y1+3)*engine.map->width].decoration = 37;
	engine.actors.push(sink2);
	
	
	Actor * pcmu4 = new Actor(x2, y1+3, 243, "PCMU Food Processor", TCODColor::white);
	engine.map->tiles[x2+(y1+3)*engine.map->width].decoration = 44;
	pcmu4->destructible = new MonsterDestructible(1,0,0,0);
	pcmu4->ai = new LockerAi();
	pcmu4->hostile = false;
	pcmu4->interact = true;
	pcmu4->container = new Container(3);
	Actor *food = createFood(0,0);
	food->pickable->pick(food, pcmu4);
	engine.actors.push(pcmu4);
	
	
	dig((x1+x2)/2,y2,(x1+x2)/2,engine.mapHeight-11-20);//side room connector
	
	x1 = engine.mapWidth/2-12-5;
	x2 = engine.mapWidth/2-8-5;
	y1 = -20 +engine.mapHeight-11;
	y2 = -20 +engine.mapHeight-7;
	for (int tilex = x1; tilex <=x2; tilex++) {//side room lower
		for (int tiley = y1; tiley <= y2; tiley++) {

			map->setProperties(tilex,tiley,true,true);
			tiles[tilex+tiley*engine.mapWidth].tileType = Param::OFFICE;
		}
	}
	Actor * cabinet = new Actor(x1,y1,243,"a filing cabinet", TCODColor::white);
	engine.map->tiles[x1+y1*engine.map->width].decoration = -2;
	engine.actors.push(cabinet);
	Actor * cabinet1 = new Actor(x1+1,y1,243,"a filing cabinet", TCODColor::white);
	engine.map->tiles[x1+1+y1*engine.map->width].decoration = -2;
	engine.actors.push(cabinet1);
	Actor * cabinet2 = new Actor(x1+3,y1,243,"a filing cabinet", TCODColor::white);
	engine.map->tiles[x1+3+y1*engine.map->width].decoration = -2;
	engine.actors.push(cabinet2);
	Actor * cabinet3 = new Actor(x1+4,y1,243,"a filing cabinet", TCODColor::white);
	engine.map->tiles[x1+4+y1*engine.map->width].decoration = -2;
	engine.actors.push(cabinet3);
	Actor * desk = new Actor(x1+1+1,y1+2,243,"A desk with an angled computer", TCODColor::white);
	engine.map->tiles[x1+1+1+(y1+2)*engine.map->width].decoration = 3;
	engine.actors.push(desk);

	//make shadow terminal
	Actor *triggerTileSh = new Actor(x1+2,y1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileSh->ai = new TriggerAi(  
	"ADVANCED LIGHTING.\n\n"
	"Notice this room's lighting.\n\n"
	"The light to the right of the desk is shining light on the desk, but the desk has cast a shadow to the left.\n\n"
	"Use this breaking of FOV to your advantage in Astro.");
	triggerTileSh->blocks = false; 
	engine.actors.push(triggerTileSh);

	//make room terminal
	Actor *triggerTileR = new Actor(x1+2,y2, 227, "Intercom Terminal", TCODColor::white);
	triggerTileR->ai = new TriggerAi(  
	"ROOM BASICS.\n\n"
	"The Astro has many rooms. Every room is unique, as is every deck.  So it is a good idea to explore it all and learn what each room has in it and what it all does!\n");
	triggerTileR->blocks = false; 
	engine.actors.push(triggerTileR);
	
	Actor * desk2 = new Actor(x1+1+1+1+1,y1+3,243,"A desk with a ruined computer", TCODColor::white);
	engine.map->tiles[x1+1+1+1+1+(y1+3)*engine.map->width].decoration = 1;
	engine.actors.push(desk2);
	
	Actor *lightOf = new Actor(x1+3, y1+2, 224, "A hastily erected Emergency Light", TCODColor::white);
	lightOf->ai = new LightAi(4,flkr);                //224, crashes when using 224
	engine.actors.push(lightOf);
	
	/////////////////////target range
	x1 = engine.mapWidth/2-10+26;
	x2 = engine.mapWidth/2+10+26-5;
	y1 = -20 +engine.mapHeight-29;
	y2 = -20 +engine.mapHeight-23;
	for (int tilex = x1; tilex <=x2; tilex++) {//side room lower
		for (int tiley = y1; tiley <= y2; tiley++) {

			map->setProperties(tilex,tiley,true,true);
			tiles[tilex+tiley*engine.mapWidth].tileType = Param::ARMORY;
		}
	}

	//gun racks
	for (int tiley = y1; tiley < y2; tiley+=2) {
		Actor * rack = new Actor(x1+1, tiley, 243, "Weapon Rack", TCODColor::white);
		engine.map->tiles[x1+1+tiley*engine.map->width].decoration = 54;
		rack->destructible = new MonsterDestructible(1,0,0,0);
		rack->ai = new LockerAi();
		rack->hostile = false;
		rack->interact = true;
		rack->container = new Container(3);
		generateRandom(rack,243);
		engine.actors.push(rack);

		Actor * bat= new Actor(x1+2, tiley, 243, "Battery Rack", TCODColor::white);
		engine.map->tiles[x1+2+tiley*engine.map->width].decoration = 55;
		engine.actors.push(bat);
		bat->destructible = new MonsterDestructible(1,0,0,0);
		bat->ai = new LockerAi();
		bat->hostile = false;
		bat->interact = true;
		bat->container = new Container(3);
		generateRandom(bat,243);

		Actor * dummy = new Actor(x2-1, tiley, 145, "Target Dummy", TCODColor::white);
		//engine.map->tiles[x1+2+tiley*engine.map->width].decoration = 55;
		//engine.actors.push(dummy);
		engine.map->tiles[x2-1+tiley*engine.map->width].decoration = 23;
		dummy->destructible = new MonsterDestructible(10,0,0,1);
		dummy->ai = new DummyAi();
		engine.actors.push(dummy);
	}
	//melee weapons for testing
	/*Actor *combatKnife = new Actor(x,y,169,"Combat Knife",TCODColor::white);
	combatKnife->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::STRENGTH,1);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,3);
	combatKnife->pickable = new Weapon(1,4,3,Weapon::LIGHT,0,Equipment::HAND1,bonus,requirement);
	combatKnife->sort = 4;*/
	
	
	/*ItemBonus *bonus = new ItemBonus(ItemBonus::STRENGTH,1);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,3);
	Actor *knife1 = new Actor(x1+6,y1+1,169,"Standard Knife",TCODColor::white);
	knife1->blocks = false;
	knife1->pickable = new Weapon(1,4,3,Weapon::LIGHT,0,Equipment::HAND1,bonus,requirement);
	knife1->sort = 4;
	engine.actors.push(knife1);
	Actor *knife2 = new Actor(x1+6,y1+3,169,"Offhand Knife",TCODColor::white);
	knife2->blocks = false;
	knife2->pickable = new Weapon(1,4,3,Weapon::LIGHT,0,Equipment::HAND2,bonus,requirement);
	knife2->sort = 4;
	engine.actors.push(knife2);
	Actor *knife3 = new Actor(x1+6,y1+5,169,"Zweihander",TCODColor::white);
	knife3->blocks = false;
	knife3->pickable = new Weapon(1,8,3,Weapon::HEAVY,0,Equipment::HAND1,bonus,requirement);
	knife3->sort = 4;
	engine.actors.push(knife3);*/
		//Actor *flamer = createFlameThrower(x1+6,y1,false);
		//flamer->blocks = false;
		//MLR->name = nameBuf;
		//MLR->pickable = new Equipment(0,Equipment::RANGED,bonus,requirement);
		//1 = min damage, 6 = max damage, 2 is crit mult, RANGED, 0 = not equipped,RANGED, bonus, req
		//bonus.push(modBonus);
		//ItemBonus *bonus = new ItemBonus(ItemBonus::STRENGTH,1);
		//TCODList<ItemBonus *> bonus;
		//ItemReq *req = new ItemReq(ItemReq::STRENGTH,1);
		//flamer->pickable = new Flamethrower(5,2,2,0,Equipment::RANGED,bonus,req);
		//int range = ((Flamethrower*)(flamer->pickable))->range;
		//cout << "The Range is " << range << endl;
		//flamer->sort = 4;
		//((Equipment*)(flamer->pickable))->armorArt = 14;
		//((Equipment*)(MLR->pickable))->armorArt = 13;
		//flamer->pickable->value = 200;
		//flamer->pickable->inkValue = 30;
		//col = TCODColor::white;
		//MLR->col = col;
		//engine.actors.push(flamer);
	//cout << "got to records creation" << endl;
	//for (int tiley = y1; tiley <= y2; tiley+=1) {
		
		/*Actor *flamer = createFlameThrower(x1+5,tiley,false);
		engine.actors.push(flamer);
		Actor *flamer2 = createFlameThrower(x1+6,tiley,false);
		engine.actors.push(flamer2);
		Actor *flamer3 = createFlameThrower(x1+7,tiley,false);
		engine.actors.push(flamer3);*/
		//engine.sendToBack(MLR2);
		/*Actor *booze = createFood(x1+5,tiley);
		engine.actors.push(booze);
		Actor *booze2 = createFood(x1+6,tiley);
		engine.actors.push(booze2);		
		Actor *booze3 = createFood(x1+7,tiley);
		engine.actors.push(booze3);*/
		/*Actor *MLR = createMLR(x1+4,tiley,false);
		engine.actors.push(MLR);
		engine.sendToBack(MLR);
		
		Actor *MLR2 = createMLR(x1+5,tiley,false);
		engine.actors.push(MLR2);
		engine.sendToBack(MLR2);
		Actor *MLR3 = createMLR(x1+6,tiley,false);
		engine.actors.push(MLR3);
		engine.sendToBack(MLR3);
		Actor *booze = createAlcohol(x1+5,tiley);
		engine.actors.push(booze);
		Actor *booze2 = createAlcohol(x1+6,tiley);
		engine.actors.push(booze2);		
		Actor *booze3 = createAlcohol(x1+7,tiley);
		engine.actors.push(booze3);
		//cout << "creating (" << x1+5 << "," << tiley << ")" << endl;
		Actor *record = createRecord(x1+5,tiley);
		//cout << "about to push " << record << endl;
		engine.actors.push(record);
		//cout << "creating (" << x1+6 << "," << tiley << ")" << endl;
		Actor *record2 = createRecord(x1+6,tiley);
		//cout << "about to push " << record << endl;
		engine.actors.push(record2);
		//cout << "creating (" << x1+7 << "," << tiley << ")" << endl;
		Actor *record3 = createRecord(x1+7,tiley);
		//cout << "about to push " << record << endl;
		engine.actors.push(record3);
		//cout << "done with one row" << endl;*/
		/*Actor *MLR = createCombatKnife(x1+4,tiley);
		engine.actors.push(MLR);
		engine.sendToBack(MLR);

		Actor *MLR2 = createCombatKnife(x1+5,tiley);
		engine.actors.push(MLR2);
		engine.sendToBack(MLR2);
		
		Actor *MLR3 = createCombatKnife(x1+6,tiley);
		engine.actors.push(MLR3);
		engine.sendToBack(MLR3);*/

		/*cout<< "create Titan Helm" << endl;
		Actor *kevlarHelm = createTitanHelm(x1+4,tiley,false);
		engine.actors.push(kevlarHelm);
		cout<< "Titan Helm Made" << endl;
		
		cout<< "create Titan Vest" << endl;
		Actor *kevlarVest = createTitanMail(x1+5,tiley,false);
		engine.actors.push(kevlarVest);
		cout<< "Titan Vest Made" << endl;
		
		cout<< "create Titan Greaves" << endl;
		Actor *kevlarGreaves = createTitanGreaves(x1+6,tiley,false);
		engine.actors.push(kevlarGreaves);
		cout<< "Titan Greaves Made" << endl;
		
		cout<< "create Titan Boots" << endl;
		Actor *kevlarBoots = createTitanBoots(x1+7,tiley,false);
		engine.actors.push(kevlarBoots);
		cout<< "Titan Boots Made" << endl;*/
	//}
	//cout << "got past records creation" << endl;
	
	//Actor *record = createRecord(x1+8,y1);
	//engine.actors.push(record);
	//fence
	int batteries = 0;
	for (int tiley = y1; tiley <= y2; tiley++) {
		int dec = 58;//standard sandbag wall
		if (tiley == y1)
			dec = 59;//upper wall
		else if(tiley == y2)
			dec = 60;//lower wall
		Actor * pcmu = new Actor(x2-3, tiley, 243, "Sandbag Wall", TCODColor::white);
		engine.map->tiles[x2-3+tiley*engine.map->width].decoration = dec;
		engine.actors.push(pcmu);
		pcmu->blocks = false;
		if (batteries%2 == 0 && tiley != y1 && tiley != y2)
		{
			Actor *batt = createBatteryPack(x2-4,tiley);
			engine.actors.push(batt);
			//batt->pickable->pick(batt,owner);
		}
		batteries++;
	}
	//light
	Actor *lightAr = new Actor(x2-5, (y1+y2)/2, 224, "A hastily erected Emergency Light", TCODColor::white);
	lightAr->ai = new LightAi(6,flkr);                //224, crashes when using 224
	engine.actors.push(lightAr);

	//make range terminal
	Actor *triggerTileAr = new Actor(x2-6,(y1+y2)/2, 227, "Intercom Terminal", TCODColor::white);
	triggerTileAr->ai = new TriggerAi(  
	"COMBAT BASICS.\n\n"
	"Welcome to the shooting range!\n\n"
	"Try to shoot the dummies from behind the sandbag wall with \'f\' or aim and shoot with \'F\' to specify a target.\n\n"
	"Or run up and attack the dummies in hand to hand combat by moving into them.");
	triggerTileAr->blocks = false; 
	engine.actors.push(triggerTileAr);

	//make MLR terminal
	Actor *triggerTileMLR = new Actor(x1+3,y1+2, 227, "Intercom Terminal", TCODColor::white);
	triggerTileMLR->ai = new TriggerAi(  
	"MLR BASICS\n\n"
	"All weapons, usually a Modular Laser Rifle, or \"MLR\", cleared for use on the Astro are laser based to prevent hull damage.  As such you need to find batteries to charge them after 20 uses.");
	triggerTileMLR->blocks = false; 
	engine.actors.push(triggerTileMLR);
	
	
	//stair room
	x1 = engine.mapWidth/2-2;
	x2 = engine.mapWidth/2+2;
	y1 = -20 +engine.mapHeight-26-17;
	y2 = -20 +engine.mapHeight-23-17;
	for (int tilex = x1; tilex <=x2; tilex++) {//stair room
		for (int tiley = y1; tiley <= y2; tiley++) {

			map->setProperties(tilex,tiley,true,true);
		}
	}

	//make stair terminal
	Actor *triggerTileSta = new Actor((x1+x2)/2,y1+1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileSta->ai = new TriggerAi(  
	"TELEPORTER BASICS\n\n"
	"Every deck has a teleporter to the next deck.  They haven't been calibrated since the ship went dark so they may land you in the middle  of anything.\n"
	"Find the teleporter pad in each deck to advance.\n"
	"Press \'SHIFT+.\' to use the stairs.");
	triggerTileSta->blocks = false; 
	engine.actors.push(triggerTileSta);

	//make vending terminal
	Actor *triggerTileV = new Actor((x1+x2)/2-1,y1+2, 227, "Intercom Terminal", TCODColor::white);
	triggerTileV->ai = new TriggerAi(  
	"3D PRINTER BASICS\n\n"
	"The 3D printer above you is like every other on the Astro. It can print any number of things you may need. \n"
	"All you need to use it are some PetaBitcoins and the printer to not be out of ink.");
	triggerTileV->blocks = false; 
	engine.actors.push(triggerTileV);

	//make stance terminal
	Actor *triggerTileStance = new Actor((x1+x2)/2-2,y1+1, 227, "Intercom Terminal", TCODColor::white);
	triggerTileStance->ai = new TriggerAi(  
	"STANCE BASICS\n\n"
	"You may assume two different stances with the \'=\' key, hostile, and neutral(default).\n"
	"The security bot above is not hostile to you until you switch to hostile mode and attack it, or the vending machine it protects.\n"
	"Sometimes it may be necessary to attack neutral NPCs.");
	triggerTileStance->blocks = false; 
	engine.actors.push(triggerTileStance);
	//make vending machine
	createVendor(x1+1, y1);
	//make brobot
	Actor *securityBot = createSecurityBot(x1, y1);
	securityBot->hostile = false;
	SecurityBotAi *sbAi = (SecurityBotAi*) securityBot->ai;
	sbAi->vendingX = x1+1;
	sbAi->vendingY = y1;
	//make dat $$$
	Actor *stackOfMoney = createCurrencyStack(x1+1,y1+1);
	engine.actors.push(stackOfMoney);
	//engine.sendToBack(stackOfMoney);
	
	
	//int x1 = engine.mapWidth/2-6;
	//int x2 = engine.mapWidth/2+6;
	//int y1 = -20 +engine.mapHeight-12;
	//int y2 = -20 +engine.mapHeight-7;
	//int startX = (engine.mapWidth/2-6+engine.mapWidth/2+6)/2;
	int startX = (engine.mapWidth/2-6+engine.mapWidth/2+6)/2;
	//int startY = (engine.mapHeight-12+engine.mapHeight-6)/2;
	int startY = (engine.mapHeight-12+engine.mapHeight-6)/2-20;
	engine.stairs->x = (x1+x2)/2;
	engine.stairs->y = y1;
	engine.player->x = startX;
	engine.player->y = startY;
	//make a message tile infront the player

	Actor *triggerTile = new Actor(startX, startY-1, 227, "Intercom Terminal", TCODColor::white);
	//we have 30 characters to use until a \n is needed
	//triggerTile->ai = new TriggerAi("<Intercom>: \"Prepare to starve\n in the tutorial level!\"");
	triggerTile->ai = new TriggerAi(  
	"Welcome to the Astroverious.\n\n"
	"To move press the UP, DOWN, LEFT, and RIGHT keys, or use the NUMPAD; 7,9,1 and 3 can be used to move diagonally.\n\n"
	"Try exploring the entirety of this room to get the hang of it.  Good Luck.\n\n"
	"Press \'g\' when standing over a terminal to replay its message");
	triggerTile->blocks = false;
	engine.actors.push(triggerTile);

	engine.playerLight = new Actor(engine.player->x, engine.player->y, 'l', "Your Flashlight", TCODColor::white);
	engine.playerLight->ai = new LightAi(2,1,true); //could adjust second '1' to less if the flashlight should flicker
	engine.actors.push(engine.playerLight);
	engine.playerLight->blocks = false;
	//playerLight->ai->moving = true;
	engine.sendToBack(engine.playerLight);
	
}

void Map::addMonster(int x, int y, bool isHorde) {
	TCODRandom *rng =TCODRandom::getInstance();
	
	
	/*
	Stats (Actor.hpp): int str, dex, intel, vit, totalStr, totalDex, totalIntel; //strength, dexterity, intelligence, vitality
	(Destructible.hpp) hp, maxHp, baseDodge, totalDodge
	Defaults: str(5),dex(3),intel(3),vit(5),totalStr(5),totalDex(3), totalIntel(3)
	Rough Damage formulae:
	shootingDamage = totalDex
	empGrenades damage = -3 + 3 * wearer->totalIntel
	Firebomb damage = 2 * wearer->totalIntel
	Firebomb range = (wearer->totalIntel - 1) /3 +1)
	Flashbang duration = wearer->totalIntel + 5
	health = hp (done in constructor)
	melee == totalStr
	dodge = def
	base dodge = dodge + 10
	
	Shoot combat works:
	roll = (0,20), ==1 -> miss, ==20, 2* damage
	attackRoll = roll + owner->totalDex
	else if attackRoll >= target->destructible->totalDodge + 10, damage = totalDex
	2
	Melee combat works:
	roll = (0,20), ==1 -> miss, ==20, 2* damage
	attackRoll = roll + owner->totalStr
	else if attackRoll >= target->destructible->totalDodge, damage = totalStr
	*/
	
	float cleanerChance = 80;
	float infectedCrewMemChance = 350;
	float infectedMarineChance = 160;
	float infectedGrenadierChance = 70;
	float infectedNCOChance = 100;
	float infectedOfficerChance = 70;
	float miniSporeCreatureChance = 100; 
	float sporeCreatureChance = 10;
	float infectedEngineerChance = 60; //infected engineers have a 5% chance of spawning in rooms other than generators rooms
	float crawlerChance = 100;
//	float turretChance = 50;
//	float vendorChance = 100;

	int uB = 1200;
	
	if(isHorde) //since engineers and cleaners don't spawn in hordes, adjust uppber bounded accordingly
		uB =- (infectedEngineerChance+cleanerChance);

	
	int dice = rng->getInt(0,uB);
	if(engine.map->tiles[x+y*width].tileType == HYDROPONICS) //no enemies here
		return;
	
	if(engine.map->tiles[x+y*width].tileType == GENERATOR && !isHorde) //only engineers spawn in generator rooms (unless it's a horde)
	{
		createInfectedEngineer(x,y);
		return;
	}
	else if(engine.map->tiles[x+y*width].tileType == BARRACKS && !isHorde) //only infected marines and infected grenadiers in barracks (unless it's a horde)
	{
		if(dice < 600)
			createInfectedMarine(x,y);
		else
			createInfectedGrenadier(x,y);
		return;
	}
	
	if (dice < infectedCrewMemChance) 
	{
		createInfectedCrewMember(x,y);	
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance)	
	{
		createInfectedNCO(x,y);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance)
	{	
		createInfectedOfficer(x,y);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + miniSporeCreatureChance)
	{
		createMiniSporeCreature(x,y);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + miniSporeCreatureChance)
	{
		createSporeCreature(x,y);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + infectedMarineChance+ miniSporeCreatureChance)
	{
		createInfectedMarine(x,y);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + infectedMarineChance + infectedGrenadierChance+ miniSporeCreatureChance)
	{
		createInfectedGrenadier(x,y);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + infectedMarineChance + infectedGrenadierChance + cleanerChance + miniSporeCreatureChance && !isHorde)
	{
		createCleanerBot(x,y);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + infectedMarineChance + infectedGrenadierChance + cleanerChance + infectedEngineerChance +  miniSporeCreatureChance && !isHorde)
	{
	
		createInfectedEngineer(x,y);
		//createTurret(x,y);
		//create turrets during room creation
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + infectedMarineChance + infectedGrenadierChance + cleanerChance + infectedEngineerChance +  miniSporeCreatureChance + crawlerChance)
	{
		createCrawler(x,y);
	}
	/*
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + infectedMarineChance + infectedGrenadierChance + cleanerChance + turretChance + miniSporeCreatureChance && !isHorde)
	{
		//createTurret(x,y);
		//create turrets during room creation
	}
	
	else if (dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance + infectedMarineChance + infectedGrenadierChance + cleanerChance + turretChance + miniSporeCreatureChance + vendorChance && !isHorde) 
	{
		//createVendor(x,y);
		//create vending machines during room creation
	}
	*/
}

Actor* Map::createCleanerBot(int x, int y)
{
	int level = engine.level;
	float scale = 1 + .1*(level - 1);
	float cleanerHp = 10*scale;
	float cleanerDodge = 0*scale;
	float cleanerDR = 0*scale;
	float cleanerStr = 0*scale;
	float cleanerXp = 0*scale;
	int cleanerAscii = 131;
	
	Actor *cleaner = new Actor(x,y,cleanerAscii,"Fungal Cleaning Bot",TCODColor::white);
	cleaner->hostile = false;
	cleaner->destructible = new MonsterDestructible(cleanerHp,cleanerDodge,cleanerDR,cleanerXp);
	cleaner->totalStr = cleanerStr;
	cleaner->ai = new CleanerAi();
	cleaner->container = new Container(2);
	generateRandom(cleaner, cleanerAscii);
	engine.actors.push(cleaner);
	
	return cleaner;

}

Actor* Map::createSecurityBot(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float securityBotHp = 25*scale;
	float securityBotDodge = 0*scale;
	float securityBotDR = 0*scale;
	float securityBotStr = 10*scale;
	float securityBotXp = 25*scale;
	int securityBotAscii = 129; //CHANGED

	Actor *securityBot = new Actor(x,y,securityBotAscii,"Security Bot",TCODColor::white);
	securityBot->destructible = new MonsterDestructible(securityBotHp,securityBotDodge,securityBotDR,securityBotXp);
	securityBot->totalStr = securityBotStr;
	securityBot->attacker = new Attacker(securityBotStr);
	securityBot->container = new Container(2);
	securityBot->ai = new SecurityBotAi();
	generateRandom(securityBot, securityBotAscii);
	engine.actors.push(securityBot);
	
	return securityBot;

}
Actor* Map::createInfectedCrewMember(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float infectedCrewMemHp = 10*scale;
	float infectedCrewMemDodge = 0*scale;
	float infectedCrewMemDR = 0*scale;
	float infectedCrewMemStr = 5*scale;
	float infectedCrewMemXp = 10*scale;
	int infectedCrewMemAscii = 164;
	
	Actor *infectedCrewMember = new Actor(x,y,infectedCrewMemAscii,"Infected Crewmember",TCODColor::white);
	infectedCrewMember->destructible = new MonsterDestructible(infectedCrewMemHp,infectedCrewMemDodge,infectedCrewMemDR,infectedCrewMemXp);
	infectedCrewMember->flashable = true;
	infectedCrewMember->totalStr = infectedCrewMemStr;
	infectedCrewMember->attacker = new Attacker(infectedCrewMemStr);
	infectedCrewMember->container = new Container(2);
	infectedCrewMember->ai = new MonsterAi();
	generateRandom(infectedCrewMember, infectedCrewMemAscii);
	engine.actors.push(infectedCrewMember);
	
	return infectedCrewMember;

}

Actor* Map::createInfectedNCO(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float infectedNCOHp = 12*scale;
	float infectedNCODodge = 1*scale;
	float infectedNCODR = 0*scale;
	float infectedNCOStr = 6*scale;
	float infectedNCOXp = 10*scale;
	int infectedNCOAscii = 148;
	
	Actor *infectedNCO = new Actor(x,y,infectedNCOAscii,"Infected NCO",TCODColor::white);
	infectedNCO->destructible = new MonsterDestructible(infectedNCOHp,infectedNCODodge,infectedNCODR,infectedNCOXp);
	infectedNCO->totalStr = infectedNCOStr;
	infectedNCO->flashable = true;
	infectedNCO->attacker = new Attacker(infectedNCOStr);
	infectedNCO->container = new Container(2);
	infectedNCO->ai = new MonsterAi();
	generateRandom(infectedNCO, infectedNCOAscii);
	engine.actors.push(infectedNCO);
	
	return infectedNCO;
}
Actor* Map::createInfectedOfficer(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float infectedOfficerHp = 15*scale;
	float infectedOfficerDodge = 1*scale;
	float infectedOfficerDR = 0*scale;
	float infectedOfficerStr = 7*scale;
	float infectedOfficerXp = 20*scale;
	int infectedOfficerAscii = 132;
	
	
	Actor *infectedOfficer = new Actor(x,y,infectedOfficerAscii,"Infected Officer",TCODColor::white);
	infectedOfficer->destructible = new MonsterDestructible(infectedOfficerHp,infectedOfficerDodge,infectedOfficerDR,infectedOfficerXp);
	infectedOfficer->flashable = true;
	infectedOfficer->totalStr = infectedOfficerStr;
	infectedOfficer->attacker = new Attacker(infectedOfficerStr);
	infectedOfficer->container = new Container(2);
	infectedOfficer->ai = new MonsterAi();
	generateRandom(infectedOfficer, infectedOfficerAscii);
	engine.actors.push(infectedOfficer);
	
	return infectedOfficer;
}
Actor* Map::createInfectedMarine(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float infectedMarineHp = 10*scale;
	float infectedMarineDodge = 0*scale;
	float infectedMarineDR = 0*scale;
	float infectedMarineStr = 2*scale;
	float infectedMarineDex = 3*scale;
	float infectedMarineXp = 10*scale;
	int infectedMarineAscii = 149;
	 
	Actor *infectedMarine = new Actor(x,y,infectedMarineAscii,"Infected Marine",TCODColor::white);
	infectedMarine->destructible = new MonsterDestructible(infectedMarineHp,infectedMarineDodge,infectedMarineDR,infectedMarineXp);
	infectedMarine->flashable = true;
	infectedMarine->attacker = new Attacker(infectedMarineStr);
	infectedMarine->totalStr = infectedMarineStr;
	infectedMarine->totalDex = infectedMarineDex;
	infectedMarine->container = new Container(2);
	infectedMarine->ai = new RangedAi();
	generateRandom(infectedMarine, infectedMarineAscii);
	engine.actors.push(infectedMarine);
	
	return infectedMarine;
}
Actor* Map::createInfectedGrenadier(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float infectedGrenadierHp = 10*scale;
	float infectedGrenadierDodge = 0*scale;
	float infectedGrenadierDR = 0*scale;
	float infectedGrenadierStr = 2*scale;
	float infectedGrenadierIntel = 3*scale; 
	float infectedGrenadierXp = 20*scale;
	int infectedGrenadierAscii = 133;
	
	
	Actor *infectedGrenadier = new Actor(x,y,infectedGrenadierAscii,"Infected Grenadier",TCODColor::white);
	infectedGrenadier->destructible = new MonsterDestructible(infectedGrenadierHp,infectedGrenadierDodge,infectedGrenadierDR,infectedGrenadierXp);
	infectedGrenadier->flashable = true;
	infectedGrenadier->totalStr = infectedGrenadierStr;
	infectedGrenadier->totalIntel = infectedGrenadierIntel;
	infectedGrenadier->attacker = new Attacker(infectedGrenadierStr);
	infectedGrenadier->container = new Container(2);
	infectedGrenadier->ai = new GrenadierAi();
	generateRandom(infectedGrenadier , infectedGrenadierAscii);
	engine.actors.push(infectedGrenadier);
	
	return infectedGrenadier;
}

Actor *Map::createInfectedEngineer(int x, int y)
{

	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float infectedEngineerHp = 10*scale;
	float infectedEngineerDodge = 0*scale;
	float infectedEngineerDR = 0*scale;
	float infectedEngineerStr = 2*scale;
	float infectedEngineerIntel = 5*scale; 
	float infectedEngineerXp = 20*scale;
	int infectedEngineerAscii = 134;  //CHANGED

	Actor *infectedEngineer = new Actor(x,y,infectedEngineerAscii,"Infected Engineer",TCODColor::white);
	infectedEngineer->destructible = new MonsterDestructible(infectedEngineerHp,infectedEngineerDodge,infectedEngineerDR,infectedEngineerXp);
	infectedEngineer->flashable = true;
	infectedEngineer->totalStr = infectedEngineerStr;
	infectedEngineer->totalIntel = infectedEngineerIntel;
	infectedEngineer->attacker = new Attacker(infectedEngineerStr);
	infectedEngineer->container = new Container(2);
	infectedEngineer->ai = new EngineerAi(5,5);
	generateRandom(infectedEngineer , infectedEngineerAscii);
	engine.actors.push(infectedEngineer);
	
	return infectedEngineer;



}
Actor* Map::createMiniSporeCreature(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float miniSporeCreatureHp = 10*scale;
	float miniSporeCreatureDodge = 0*scale;
	float miniSporeCreatureDR = 0*scale;
	float miniSporeCreatureStr = 5*scale;
	float miniSporeCreatureXp = 15*scale;
	int miniSporeCreatureAscii = 166;
	
	
	Actor *miniSporeCreature = new Actor(x,y,miniSporeCreatureAscii,"Small Spore Creature",TCODColor::white);
	miniSporeCreature->destructible = new MonsterDestructible(miniSporeCreatureHp,miniSporeCreatureDodge,miniSporeCreatureDR,miniSporeCreatureXp);
	miniSporeCreature->flashable = true;
	miniSporeCreature->totalStr = miniSporeCreatureStr;
	miniSporeCreature->attacker = new Attacker(miniSporeCreatureStr);
	miniSporeCreature->container = new Container(2);
	miniSporeCreature->ai = new MonsterAi();
	miniSporeCreature->oozing = true;
	generateRandom(miniSporeCreature, miniSporeCreatureAscii);
	engine.actors.push(miniSporeCreature);
	
	return miniSporeCreature;
}
Actor* Map::createSporeCreature(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float sporeCreatureHp = 17*scale;
	float sporeCreatureDodge = 1*scale;
	float sporeCreatureDR = 0*scale;
	float sporeCreatureStr = 10*scale;
	float sporeCreatureXp = 25*scale;
	int sporeCreatureAscii = 165;
	
	
	Actor *sporeCreature = new Actor(x,y,sporeCreatureAscii,"Spore Creature",TCODColor::white);
	sporeCreature->destructible = new MonsterDestructible(sporeCreatureHp,sporeCreatureDodge,sporeCreatureDR,sporeCreatureXp);
	sporeCreature->flashable = true;
	sporeCreature->totalStr = sporeCreatureStr;
	sporeCreature->attacker = new Attacker(sporeCreatureStr);
	sporeCreature->container = new Container(2);
	sporeCreature->ai = new MonsterAi();
	sporeCreature->oozing = true;
	generateRandom(sporeCreature, sporeCreatureAscii);
	engine.actors.push(sporeCreature);
	
	return sporeCreature;
	
}
Actor* Map::createTurret(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float turretHp = 10*scale;
	float turretDodge = 0*scale;
	float turretDR = 0*scale;
	float turretStr = 0*scale; //no melee damage
	float turretDex = 3*scale;
	float turretXp = 25*scale;
	int turretAscii = 147;
	
	Actor *turret = new Actor(x,y,turretAscii,"Sentry Turret",TCODColor::white);
	turret->totalDex = turretDex;
	turret->destructible = new MonsterDestructible(turretHp,turretDodge,turretDR,turretXp);
	turret->totalStr = turretStr;
	turret->attacker = new Attacker(turretStr);
	turret->ai = new TurretAi();
	turret->container = new Container(2);
	generateRandom(turret, turretAscii);
	engine.actors.push(turret);
	
	return turret;
	
}

Actor* Map::createTurretControl(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float controlHp = 15*scale;
	float controlDodge = 0;
	float controlDR = 0;
	float controlXp = 0;
	int controlAscii = 189;
	
	Actor *control = new Actor(x,y,controlAscii,"Turret Room Control",TCODColor::white);
	control->destructible = new MonsterDestructible(controlHp,controlDodge,controlDR,controlXp);
	control->ai = new TurretControlAi();
	control->interact = true;
	control->hostile = false;
	control->container = new Container(2);
	generateRandom(control, controlAscii);
	engine.actors.push(control);
	
	return control;
	
	
}

Actor* Map::createGardner(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float gardnerHp = 10*scale;
	float gardnerDodge = 0*scale;
	float gardnerDR = 0*scale;
	float gardnerStr = 3*scale;
	float gardnerDex = 0*scale;
	float gardnerXp = 25*scale;
	int gardnerAscii = 135;
	
	Actor *gardner = new Actor(x,y,gardnerAscii,"Crazed Gardner",TCODColor::white);
	gardner->hostile = false;
	gardner->totalDex = gardnerDex;
	gardner->destructible = new MonsterDestructible(gardnerHp,gardnerDodge,gardnerDR,gardnerXp);
	gardner->totalStr = gardnerStr;
	gardner->attacker = new Attacker(gardnerStr);
	gardner->ai = new GardnerAi();
	gardner->container = new Container(2);
	generateRandom(gardner, gardnerAscii);
	engine.actors.push(gardner);
	
	return gardner;
}

Actor* Map::createCrawler(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float crawlerHp = 15*scale;
	float crawlerDodge = 3*scale;
	float crawlerDR = 0*scale;
	float crawlerStr = 3*scale;
	float crawlerDex = 0*scale;
	float crawlerXp = 25*scale;
	int crawlerAscii = 150;
	
	Actor *crawler = new Actor(x,y,crawlerAscii,"Infected Legless Crewman",TCODColor::white);
	crawler->hostile = true;
	crawler->totalDex = crawlerDex;
	crawler->destructible = new MonsterDestructible(crawlerHp,crawlerDodge,crawlerDR,crawlerXp);
	crawler->totalStr = crawlerStr;
	crawler->attacker = new Attacker(crawlerStr);
	crawler->ai = new MonsterAi();
	crawler->container = new Container(2);
	generateRandom(crawler, crawlerAscii);
	engine.actors.push(crawler);
	
	return crawler;
}
Actor* Map::createVendor(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float vendorHp = 10*scale;
	float vendorDodge = 0*scale;
	float vendorDR = 0*scale;
	float vendorXp = 25*scale;
	int vendorAscii = 225; //change to desired ascii
	
	Actor *vendor = new Actor(x,y,vendorAscii,"Vending Machine",TCODColor::white);
	vendor->hostile = false;
	vendor->interact = true;
	vendor->destructible = new MonsterDestructible(vendorHp, vendorDodge,vendorDR,vendorXp);
	vendor->ai = new VendingAi();
	vendor->container = new Container(50);
	//generateRandom(vendor, vendorAscii); Vending Machines get populated with items when they are interacted with
	engine.actors.push(vendor);
	
	return vendor;
}
Actor* Map::createConsole(int x, int y)
{
	int level = engine.level;
	float scale = 2 + .2*(level - 1);
	float consoleHp = 10*scale;
	float consoleDodge = 0*scale;
	float consoleDR = 0*scale;
	float consoleXp = 25*scale;
	int consoleAscii = 226; //change to desired ascii
	
	Actor *console = new Actor(x,y,consoleAscii,"A humming Console",TCODColor::white);
	console->hostile = false;
	console->interact = true;
	console->destructible = new MonsterDestructible(consoleHp, consoleDodge,consoleDR,consoleXp);
	console->ai = new ConsoleAi();
	//console->container = new Container(10);
	//generateRandom(console, consoleAscii);
	engine.actors.push(console);
	
	return console;
}
void Map::addItem(int x, int y, RoomType roomType) {

	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0,545);
	if (dice < 40) {
		//create a health potion
		Actor *healthPotion = createHealthPotion(x,y);
		engine.actors.push(healthPotion);
		engine.sendToBack(healthPotion);
	} else if(dice < 40+40) {
		//create a scroll of lightningbolt
		Actor *scrollOfLightningBolt = createEMP(x,y);
		engine.actors.push(scrollOfLightningBolt);
		engine.sendToBack(scrollOfLightningBolt);
	} else if(dice < 40+40+40) {
		//create a scroll of fireball
		Actor *scrollOfFireball = createFireBomb(x,y);
		engine.actors.push(scrollOfFireball);
		engine.sendToBack(scrollOfFireball);
	} else if(dice < 40+40+40+15) {
		//create a pair of mylar boots
		Actor *myBoots = createMylarBoots(x,y,false);
		engine.actors.push(myBoots);
		engine.sendToBack(myBoots);
	} else if(dice < 40+40+40+15+15) {
		//create a Modular Laser Rifle (MLR)
		Actor *MLR = createMLR(x,y,false);
		engine.actors.push(MLR);
		engine.sendToBack(MLR);
	}else if(dice < 40+40+40+15+15+5){
		//create Titanium Micro Chain-mail
		Actor *chainMail = createTitanMail(x,y,false);
		engine.actors.push(chainMail);
		engine.sendToBack(chainMail);
	}else if(dice < 40+40+40+15+15+5+10){
		//create Titanium Micro Chain-mail
		Actor *titanBoots = createTitanBoots(x,y,false);
		engine.actors.push(titanBoots);
		engine.sendToBack(titanBoots);
	}else if(dice < 40+40+40+15+15+5+10+40){
		//create a battery pack
		Actor *batteryPack = createBatteryPack(x,y);
		engine.actors.push(batteryPack);
		engine.sendToBack(batteryPack);
	}else if(dice< 40+40+40+15+15+5+10+40+40){
		//create a scroll of confusion
		Actor *scrollOfConfusion = createFlashBang(x,y);
		engine.actors.push(scrollOfConfusion);
		engine.sendToBack(scrollOfConfusion);
	}else if(dice< 40+40+40+15+15+5+10+40+40+40){
		//create a scroll of fragging
		Actor *scrollOfFragging = createFrag(x,y);
		engine.actors.push(scrollOfFragging);
		engine.sendToBack(scrollOfFragging);
	}else if(dice< 40+40+40+15+15+5+10+40+40+40+40){
		//create a scroll of drunkeness
		Actor *scrollOfDrunkeness = createAlcohol(x,y);
		engine.actors.push(scrollOfDrunkeness);
		engine.sendToBack(scrollOfDrunkeness);
	}else if(dice<40+40+40+15+15+5+10+40+40+40+40+100) {
		Actor *stackOfFood = createFood(x,y);
		//engine.actors.push(stackOfFood);
		engine.sendToBack(stackOfFood);
	}else {
		Actor *stackOfMoney = createCurrencyStack(x,y);
		engine.actors.push(stackOfMoney);
		engine.sendToBack(stackOfMoney);
	}
}

TCODList<RoomType> * Map::getRoomTypes(LevelType levelType) {
	TCODList<RoomType> * roomList = new TCODList<RoomType>();
		switch (levelType) {
			case GENERIC:
				//hopefully one generator room is guarenteed
				roomList->push(GENERATOR);
				//small amount of office rooms
				for (int i = 0; i <= rng->getInt(3,7); i++) {
					roomList->push(OFFICE);
				}	
				//small amount of barracks
				for (int i = 0; i <= rng->getInt(3,5); i++) {
					roomList->push(BARRACKS);
				}	
				for (int i = 0; i <= rng->getInt(3,6); i++) {
					roomList->push(DEFENDED_ROOM);
				}	
				for (int i = 0; i <= rng->getInt(2,3); i++) {
					roomList->push(KITCHEN);
				}	
				for (int i = 0; i <= rng->getInt(2,3); i++) {
					roomList->push(MESSHALL);
				}	
				for (int i = 0; i <= rng->getInt(2,3); i++) {
					roomList->push(BAR);
				}	
				for (int i = 0; i <= rng->getInt(2,3); i++) {
					roomList->push(OBSERVATORY);
				}	
				for (int i = 0; i <= rng->getInt(2,3); i++) {
					roomList->push(SERVER);
				}	
				for (int i = 0; i <= rng->getInt(1,2); i++) {
					roomList->push(ARMORY);
				}	
				//need to see if end list items are less common
				roomList->push(HYDROPONICS);
				roomList->push(INFECTED_ROOM);
				break;
			case OFFICE_FLOOR:
				for (int i = 0; i <= 40; i++) {
					roomList->push(OFFICE);
				}
				break;
			case DEFENDED:
				for (int i = 0; i <= 40; i++) {
					roomList->push(DEFENDED_ROOM);
				}
				break;
			case DRUNK:
				for (int i = 0; i <= 40; i++) {
					roomList->push(BAR);
				}
				break;
			case TUTORIAL:
				//TUTORIALS ONLY HAVE GENERIC ROOMS
				break;
		}

		return roomList;
}


void Map::createRoom(int roomNum, bool withActors, Room * room) {
	int x1 = room->x1;
	int y1 = room->y1;
	int x2 = room->x2;
	int y2 = room->y2;

	dig(x1,y1,x2,y2);

	if (!withActors) {
		return;
	}
	
	TCODRandom *rng = TCODRandom::getInstance();

	//try to place an epicenter
	if (epicenterAmount > 0 && roomNum == 5 ) {
		int x = rng->getInt(x1, x2);
		int y = rng->getInt(y1, y2);

		if(canWalk(x,y) && !isWall(x,y)) {
			Actor * epicenter = new Actor(x, y, 7, "Infection Epicenter", TCODColor::white);
			epicenter->ai=new EpicenterAi;
			engine.actors.push(epicenter);
			//intial infection, concentrated at the epicenter
			for (int i = 0; i < width*height; i++) {
				tiles[i].infection = 1 / ((rng->getDouble(.01,1.0))*epicenter->getDistance(i%width, i/width));
			}
		}
		epicenterAmount--;
	}
	
	
	//record the room type to the tile of the room
	for (int tilex = x1; tilex <=x2; tilex++) {
		for (int tiley = y1; tiley <= y2; tiley++) {
			tiles[tilex+tiley*width].tileType = room->type;
		}
	}
	
	//custom room feature
	//OFFICE ROOMS
	if (room->type == OFFICE) {
		cout << "Office made" << endl;
		int files = 0;
		while (files < 10)
		{
			//place a cabient by the wall as a place holder
			//x1 is the left side of room
			//y1 is top of room
			//x2 is right side of room
			//y2 is bottom of room
			int filingCabX = 0, filingCabY = 0;
			//filingCabinetX
			//filingCabinetY
			//choose which wall to put it on, x1, y1, y2, y3
			//new random 1-4
			TCODRandom *rng = TCODRandom::getInstance();
			int wall = rng->getInt(0,3);
			//case 1 = filing cabinet is on side x1 (left), filingCabinetX is set = x1
			//DOUBLES
			if (wall == 0)
			{
				filingCabX = x1;
				filingCabY = rng->getInt(y1,y2);
				if (isWall(filingCabX-1,filingCabY) && engine.getAnyActor(filingCabX,filingCabY) == NULL)
				{
					Actor * cabinet = new Actor(filingCabX,filingCabY,243,"a filing cabinet", TCODColor::white);
					engine.map->tiles[filingCabX+filingCabY*engine.map->width].decoration = -3;
					cabinet->smashable = true;
					engine.actors.push(cabinet);
				}
			}
			//case 2 = filing cabinet is on side y1 (top), filingCabinetY is set = y1
			else if (wall == 1)
			{
				filingCabY = y1;
				filingCabX = rng->getInt(x1,x2);
				if (isWall(filingCabX,filingCabY-1) && engine.getAnyActor(filingCabX,filingCabY) == NULL)
				{
					Actor * cabinet = new Actor(filingCabX,filingCabY,243,"a filing cabinet", TCODColor::white);
					engine.map->tiles[filingCabX+filingCabY*engine.map->width].decoration = -2;
					cabinet->smashable = true;
					engine.actors.push(cabinet);
				}
			}
			//case 3 = filing cabinet is on side x2 (right), filingCabinetX is set = x2
			else if (wall == 2)
			{
				filingCabX = x2;
				filingCabY = rng->getInt(y1,y2);
				if (isWall(filingCabX+1,filingCabY ) && engine.getAnyActor(filingCabX,filingCabY) == NULL)
				{
					Actor * cabinet = new Actor(filingCabX,filingCabY,243,"a filing cabinet", TCODColor::white);
					engine.map->tiles[filingCabX+filingCabY*engine.map->width].decoration = -4;
					cabinet->smashable = true;
					engine.actors.push(cabinet);
				}
			}
			//case 4 = filing cabinet is on side y2 (bottom), filingCabinetY is set = y2
			else //if (wall == 3)
			{
				filingCabY = y2;
				filingCabX = rng->getInt(x1,x2);
				if (isWall(filingCabX,filingCabY+1) && engine.getAnyActor(filingCabX,filingCabY) == NULL)
				{
					Actor * cabinet = new Actor(filingCabX,filingCabY,243,"a filing cabinet", TCODColor::white);
					engine.map->tiles[filingCabX+filingCabY*engine.map->width].decoration = -5;
					cabinet->smashable = true;
					engine.actors.push(cabinet);
				}
			}
			//now we have chosen the wall side
			//new random = NULL
			//if we are on a left/right wall random between y1-y2, because we have the x at this point but need a y
			
			//if we are on a bottom/top wall random between x1-x2, because we have the y at this point but need an x
			
			//now we have chosen a point somewhere in the middle of the wall to place the cabinet
			//we now have values for filingCabinetX and filingCabinetY
			//now check if we are blocking a hallway
			//if we are on a left wall check the left 3 tiles adjacent
			
			//if we are on a top wall check the upper 3 tiles adjacent
			//... expand for all 4 cases
			//checking the tiles:  if any of the tiles to check are floors then stop placing this cabinet.
			//we can try to place another, or just stop, whatever is good
			//probably just make files--?  break?
			
			//check for doubles, check for corners
			//if you're x,y's are the same as the x,y's then you are in a corner
			//check all 4 corner cases
			
			
			
			//Actor * cabinet = new Actor(x1+1,y1+1,240,"A filing cabinet", TCODColor::white);
			//Actor * cabinet = new Actor(filingCabX,filingCabY,240,"A filing cabinet", TCODColor::white);
			//engine.actors.push(cabinet);
			files++;
		}
		//add desks
		TCODRandom *rng = TCODRandom::getInstance();
			int place = 0;
		//random between x1+2 and x2-3(-2 if random 1x1) (so they can fit, leaving a 1 cell lining, if 2x2) 
		//random between y1+2 and y2-3(-2 if random 1x1) (so they can fit, leaving a 1 cell lining, if 2x2)
		//these are the two x,y's
		for (int xX = x1+2; xX <= x2-2;xX+=2)
		{
			for (int yY = y1+2; yY <= y2-2;yY+=2)
			{
				//add a 2x2 of desks?  add random desks?
				// ...D.D...
				// .........  <- 3x3 of desks with spaces in-between?
				// ...D.D...
				//
				place = rng->getInt(1,10);
				if (place > 4)
				{
					Actor * desk = new Actor(xX,yY,243,"a desk", TCODColor::white);
					int n = rng->getInt(1,4);
					if (n == 1)
					{
						desk->name = "A desk with a ruined computer";
					}
					else if (n == 2)
					{
						desk->name = "A desk with strewn papers about";
					}
					else if (n == 3)
					{
						desk->name = "A desk with an angled computer";
					}
					else
					{
						desk->name = "A desk with a nice computer";
						//n = 4;
					}
					engine.map->tiles[xX+yY*engine.map->width].decoration = n;
					//engine.mapconDec->setChar(xX, yY, n);
					engine.actors.push(desk);
				}
				//add papers
				//replace items with paper description items
				//or make some random floor tiles into papers, if they only spawn on office rooms/decks then could be unique floor tile
			}
		}
		
	}		
	if (room->type == BARRACKS) {
		cout << "Barrack Made" << endl;
		//TCODRandom *rng = TCODRandom::getInstance();
		//add a row on the left, then on the right
		//boolean to see the distance between the beds, if it is enough, add some lockers
		int delta = (x2-2)-(x1+2);  // must be equal to 4 or greater
		//check if there is an even number if rows between the beds or not
		//if mod2 is even (equal zero) there are an odd number of spaces between, so it is just one locker
		//if mod2 is odd (not zero) there are an even number between, so add two to center them out
		bool mod2 = false;
		if (delta%2 == 0)
			mod2 = true;
		for (int i = y1+1; i < y2-1;)
		{
			Actor * bed = new Actor(x1+1,i,243,"Bed Headboard", TCODColor::white);
			//engine.mapconDec->setChar(x1+1,i, rng->getInt(9,11));//Bed Headboard (9,10,11, add random)
			engine.map->tiles[x1+1+i*engine.map->width].decoration = rng->getInt(9,11);
			engine.actors.push(bed);
			Actor * bedf = new Actor(x1+2,i,243,"Bed foot", TCODColor::white);
			//engine.mapconDec->setChar(x1+2,i, rng->getInt(15,17));//Bed foot (12,13,14, add random)
			engine.map->tiles[x1+2+i*engine.map->width].decoration = rng->getInt(15,17);
			engine.actors.push(bedf);
			//send to back
			Actor *endtable = new Actor(x1+1,i+1,243,"A bare-bones endtable", TCODColor::white);
			//engine.mapconDec->setChar(x1+1,i+1, 21);//endtable
			engine.map->tiles[x1+1+(i+1)*engine.map->width].decoration = 21;
			engine.actors.push(endtable);
			endtable->blocks = false;
			engine.sendToBack(endtable);
			//need to check if there is enough space
			Actor * bed2 = new Actor(x2-1,i,243,"Bed Headboard", TCODColor::white);
			//engine.mapconDec->setChar(x2-1,i, rng->getInt(12,14));//Bed Headboard (9,10,11, add random)
			engine.map->tiles[x2-1+i*engine.map->width].decoration = rng->getInt(12,14);
			engine.actors.push(bed2);
			Actor * bedf2 = new Actor(x2-2,i,243,"Bed foot", TCODColor::white);
			//engine.mapconDec->setChar(x2-2,i, rng->getInt(18,20));//Bed Headboard (12,13,14, add random)
			engine.map->tiles[x2-2+i*engine.map->width].decoration = rng->getInt(18,20);
			engine.actors.push(bedf2);
			Actor *endtable2 = new Actor(x2-1,i+1,243,"A bare-bones endtable", TCODColor::white);
			//engine.mapconDec->setChar(x2-1,i+1, 22);//endtable
			engine.map->tiles[x2-1+(i+1)*engine.map->width].decoration = 22;
			engine.actors.push(endtable2);
			endtable2->blocks = false;
			engine.sendToBack(endtable2);
			if (delta >= 4)
			{
				//have locker, be attackable, drops loot, have one blank ascii & mapcondec number, when you attack it it switches to 
				//another mapcondec number to look opened/looted
				if (!mod2)
				{
					Actor *locker = new Actor((x1+x2)/2,i,243,"Government Issue Locker", TCODColor::white);
					//engine.mapconDec->setChar((x1+x2)/2,i, 23);//Locker
					engine.map->tiles[(x1+x2)/2+i*engine.map->width].decoration = 23;
					locker->destructible = new MonsterDestructible(1,0,0,0);
					locker->ai = new LockerAi();
					locker->hostile = false;
					locker->interact = true;
					locker->container = new Container(3);
					generateRandom(locker,243);
					engine.actors.push(locker);
					Actor *locker2 = new Actor(((x1+x2)/2)+1,i,243,"Government Issue Locker", TCODColor::white);
					//engine.mapconDec->setChar(((x1+x2)/2)+1,i, 23);//Locker
					engine.map->tiles[(((x1+x2)/2)+1)+i*engine.map->width].decoration = 23;
					locker2->destructible = new MonsterDestructible(1,0,0,0);
					locker2->ai = new LockerAi();
					locker2->hostile = false;
					locker2->interact = true;
					locker2->container = new Container(3);
					generateRandom(locker2,243);
					engine.actors.push(locker2);
				}
				else
				{
					Actor *locker = new Actor((x1+x2)/2,i,243,"Government Issue Locker", TCODColor::white);
					//engine.mapconDec->setChar((x1+x2)/2,i, 23);//Locker
					engine.map->tiles[(x1+x2)/2+i*engine.map->width].decoration = 23;
					locker->destructible = new MonsterDestructible(1,0,0,0);
					locker->ai = new LockerAi();
					locker->hostile = false;
					locker->interact = true;
					locker->container = new Container(3);
					generateRandom(locker,243);
					engine.actors.push(locker);
				}
			}
			i += 2;
			
		}
	}
	if (room->type == GENERATOR) {
		cout << "Generator room Made" << endl;
		Actor * generator = new Actor(x1+1,y1+1,243,"A floor tile that has been jerry rigged to accept a generator.", TCODColor::white);
		//engine.mapconDec->setChar(x1+1,y1+1, 25);//
		engine.map->tiles[(x1+1)+(y1+1)*engine.map->width].decoration = 25;
		engine.actors.push(generator);
		generator->blocks = false;
		engine.sendToBack(generator);
		Actor * generator1 = new Actor(x1+2,y1+1,243,"A danger sign and a small toolbox.", TCODColor::white);
		//engine.mapconDec->setChar(x1+2,y1+1, 26);//
		engine.map->tiles[(x1+2)+(y1+1)*engine.map->width].decoration = 26;
		engine.actors.push(generator1);
		generator1->blocks = false;
		engine.sendToBack(generator1);
		Actor * generator2 = new Actor(x1+1,y1+2,243,"A bundle of cables.", TCODColor::white);
		//engine.mapconDec->setChar(x1+1,y1+2, 27);//
		engine.map->tiles[(x1+1)+(y1+2)*engine.map->width].decoration = 27;
		engine.actors.push(generator2);
		generator2->blocks = false;
		engine.sendToBack(generator2);
		//Actor * generator3 = new Actor(x1+2,y1+2,'G',"a generator", TCODColor::white);
		//engine.actors.push(generator3);
		Actor * generator4 = new Actor(x1+1,y1+3,243,"A portable generator.", TCODColor::white);
		//engine.mapconDec->setChar(x1+1,y1+3, 29);//
		engine.map->tiles[(x1+1)+(y1+3)*engine.map->width].decoration = 29;
		engine.actors.push(generator4);
		Actor * generator5 = new Actor(x1+2,y1+3,243,"A generator control console.", TCODColor::white);
		//engine.mapconDec->setChar(x1+2,y1+3, 30);//
		engine.map->tiles[(x1+2)+(y1+3)*engine.map->width].decoration = 30;
		engine.actors.push(generator5);
		//add large generators, animated - done
		//add workbench                  - done
		//add wrenchs             
		//add oil cans                   - done
		//add danger sign?
		//electric infected crewmember
		int rmSze = (x2 - x1) * (y2 - y1);
		int numDrums = rmSze/20;
		if (numDrums <= 0)
			numDrums = 1;
		for (int i = 0; i < numDrums;)
		{	
			int x = rng->getInt(x1+1,x2-1);
			int y = rng->getInt(y1+1,y2-1);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.map->tiles[x+y*engine.map->width].decoration == 0) {
				Actor *Drum = new Actor(x, y, 243, "A gasoline drum.", TCODColor::white);
				//engine.mapconDec->setChar(x,y, 28);//
				engine.map->tiles[x+y*engine.map->width].decoration = 28;
				engine.actors.push(Drum);
				i++;
			}
			
		}
		
		int torch = rng->getInt(1,3,3);
		for (int i = 0; i < torch;)
		{	
			int x = rng->getInt(x1+1,x2-1);
			int y = rng->getInt(y1+1,y2-1);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.map->tiles[x+y*engine.map->width].decoration == 0) {
				Actor *torch = new Actor(x, y, 243, "A blowtorch.", TCODColor::white);
				//engine.mapconDec->setChar(x,y, 31);//
				engine.map->tiles[x+y*engine.map->width].decoration = 31;
				engine.actors.push(torch);
				i++;
			}
			
		}
		
		int pall = rng->getInt(1,4,2);
		for (int i = 0; i < pall;)
		{	
			int x = rng->getInt(x1+1,x2-1);
			int y = rng->getInt(y1+1,y2-1);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.map->tiles[x+y*engine.map->width].decoration == 0) {
				Actor *pallet = new Actor(x, y, 243, "An empty pallet.", TCODColor::white);
				//engine.mapconDec->setChar(x,y, 32);//
				engine.map->tiles[x+y*engine.map->width].decoration = 32;
				engine.actors.push(pallet);
				pallet->blocks = false;
				engine.sendToBack(pallet);
				i++;
			}
			
		}
	}

	if (room->type == KITCHEN) {
		cout << "KITCHEN Made" << endl;
		//TCODRandom *rng = TCODRandom::getInstance();
		int midX = (x1+x2)/2;
		for (int i = x1; i < x2+1; i++)
		{
			if (i == x2) {
				Actor * refrigerator = new Actor(i, y1,243,"refrigerator", TCODColor::white);
				//engine.mapconDec->setChar(i,y1, 40);//
				engine.map->tiles[i+y1*engine.map->width].decoration = 40;
				refrigerator->smashable = true;
				engine.actors.push(refrigerator);
			}
			else if (0 == rng->getInt(0,5)) {
				Actor * oven = new Actor(i, y1,243,"oven-stove combo", TCODColor::white);
				//engine.mapconDec->setChar(i,y1, 39);//
				engine.map->tiles[i+y1*engine.map->width].decoration = 39;
				oven->smashable = true;
				engine.actors.push(oven);
			}
			else {
				Actor * counter = new Actor(i, y1,243,"Kitchen Counter", TCODColor::white);
				//engine.mapconDec->setChar(i,y1, 35);//
				engine.map->tiles[i+y1*engine.map->width].decoration = 35;
				counter->smashable = true;
				engine.actors.push(counter);
			}
			if (i > x1+1 && i < x2-1) {
				if (i > midX-2 && i < midX+2) {
					Actor *sink = new Actor(i, y1+3,243,"Industrial Sink", TCODColor::white);
					//engine.mapconDec->setChar(i,y1+3, 38);//
					engine.map->tiles[i+(y1+3)*engine.map->width].decoration = 38;
					engine.actors.push(sink);
					Actor *sink2 = new Actor(i, y1+4,243,"Industrial Sink", TCODColor::white);
					//engine.mapconDec->setChar(i,y1+4, 37);//
					engine.map->tiles[i+(y1+4)*engine.map->width].decoration = 37;
					engine.actors.push(sink2);
				}
				else { 

					Actor * midCounter = new Actor(i, y1+3,243,"Kitchen Counter", TCODColor::white);
					//engine.mapconDec->setChar(i,y1+3, 36);//
					engine.map->tiles[i+(y1+3)*engine.map->width].decoration = 36;
					engine.actors.push(midCounter);
					Actor * midCounter2 = new Actor(i, y1+4,243,"Kitchen Counter", TCODColor::white);
					//engine.mapconDec->setChar(i,y1+4, 35);//
					engine.map->tiles[i+(y1+4)*engine.map->width].decoration = 35;
					engine.actors.push(midCounter2);

				}
			}
			//below counter but not blocking walls make food processors
			
			if (i % 2 == 0 && i > x1 && i < x2) {
				for (int j = y1+6; j < y2-1; j+=2) {
					if (0 == rng->getInt(0, 4)) {
						Actor * pcmu = new Actor(i, j, 243, "PCMU Food Processor", TCODColor::white);
						pcmu->destructible = new MonsterDestructible(1,0,0,0);
						pcmu->ai = new LockerAi();
						pcmu->hostile = false;
						pcmu->interact = true;
						pcmu->container = new Container(3);
						Actor *food = createFood(0,0);
						food->pickable->pick(food, pcmu);
						engine.map->tiles[i+j*engine.map->width].decoration = 44;
						engine.actors.push(pcmu);
					}
				}
			}
		}
	}

	if (room->type == SERVER) {
		cout << "Server room made" << endl;
		//expand the room outwards
		int tx1 = x1-1;
		int tx2 = x2+1;
		int ty1 = y1-1;
		int ty2 = y2+1;
		
		for (int i = tx1; i <= tx2; i++) {
			for (int j = ty1; j <= ty2; j++) {
				if (i == tx1 || i == tx2 || j  == ty1 || j == ty2 )//|| (tx1-i)%2 == 0)
				{
					map->setProperties(i,j,true,true);
					Actor * server1 = new Actor(i, j, 243, "A server", TCODColor::white);
					engine.map->tiles[i+j*engine.map->width].decoration = rng->getInt(45,47);
					engine.map->tiles[i+j*engine.map->width].tileType = SERVER;
					server1->smashable = true;
					engine.actors.push(server1);
				}
				
			}
		}
		
		//columns of servers
		for (int i = x1+1; i <= (x2+x1)/2; i+=2) {
			for (int j = y1+1; j <= y2-1; j++) {
				//walkway
				if (j != ((y1+y2)/2))
				{
					Actor * server1 = new Actor(i, j, 243, "A server", TCODColor::white);
					engine.map->tiles[i+j*engine.map->width].decoration = rng->getInt(45,47);
					engine.actors.push(server1);
				}
			}
		}
		for (int i = x2-1; i > (x2+x1)/2; i-=2) {
			for (int j = y1+1; j <= y2-1; j++) {
				//place a console
				//walkways
				if (i == x2-1 && j == y1+1 && j != ((y1+y2)/2)) {
					//createConsole
					//createBitcoinMiner
					//Actor * console = new Actor(i, j, 'c', "A console", TCODColor::white);
					//engine.actors.push(console);
					createConsole(i,j);
				}
				else if (j != ((y1+y2)/2)){
					Actor * server1 = new Actor(i, j, 243, "A server", TCODColor::white);
					engine.map->tiles[i+j*engine.map->width].decoration = rng->getInt(45,47);
					engine.actors.push(server1);
				}
			}
		}
	}
	if (room->type == MESSHALL) {
		cout << "Messhall made" << endl;

		//placeholder, to be replaced
		//Actor * pcmu = new Actor(x1, y1, 'T', "Cafeteria Table", TCODColor::white);
		//engine.actors.push(pcmu);
		//int sqX = ((x2-1)-(x1+1))%4;
		//int sqY = ((y2-1)-(y1+1))%4;
		for (int i = x1+1; i <= x2-1; i++) {
			//sqY = ((y2-1)-(y1+1))%4;
			//if ((i-(x1+1))%4 == 0 && (i-(x1+1)) != 0)
			//		sqX--;
			for (int j = y1+1; j <= y2-1; j++) {
			//if there are 4 spaces below and right (SQUARE IS VALID)
				//if ((j-(y1+1))%4 == 0 && (j-(y1+1)) != 0)
				//	sqY--;
				//if (sqX > 0 && sqY > 0)
				//{
					if (((((i-x1)-2)%4 == 0) && (((j-y1)-1)%4 == 0)) || //top chair
						((((i-x1)-1)%4 == 0) && (((j-y1)-2)%4 == 0)) || //left chair
						((((i-x1)-3)%4 == 0) && (((j-y1)-2)%4 == 0)) || //right chair
						((((i-x1)-2)%4 == 0) && (((j-y1)-3)%4 == 0)) )  //bottom chair
					{
						Actor * pcmu = new Actor(i, j, 243, "A red messhall chair", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = 49;//top chair
						engine.actors.push(pcmu);
					}
					if (((((i-x1)-2)%4 == 0) && (((j-y1)-2)%4 == 0)))
					{
						Actor * pcmu = new Actor(i, j, 243, "A shining metal table", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = rng->getInt(50,52);
						
						engine.actors.push(pcmu);
					}
					if (((((i-x1)-4)%4 == 0) && (((j-y1)-4)%4 == 0)))
					{
						Actor * pcmu = new Actor(i, j, 243, "A green trash-can", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = 53;
						engine.actors.push(pcmu);
					}
				//}
				//if ((j-(y1+1))%4 == 0 )
					//sqY--;
			
			}
			//sqY = ((y2-1)-(y1+1))%4;
			//if ((i-(x1+1))%4 == 0)
			//		sqX--;
		}

	}
	if (room->type == ARMORY) {
		cout << "Armory made" << endl;

		//placeholder, to be replaced
		//Actor * pcmu = new Actor(x1, y1, 'C', "Weapons Case", TCODColor::white);
		//engine.actors.push(pcmu);
		
		for (int i = x1+1; i <= x2-1; i++) {
			for (int j = y1+1; j <= y2-1; j++) {
				int gunBat = rng->getInt(0,1);
				if (i == x1+1 && j%2 == 0)
				{
					if(gunBat == 0)
					{
						Actor * rack = new Actor(i, j, 243, "Weapon Rack", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = 54;
						engine.actors.push(rack);
						rack->destructible = new MonsterDestructible(1,0,0,0);
						rack->ai = new LockerAi();
						rack->hostile = false;
						rack->interact = true;
						rack->container = new Container(3);
						generateRandom(rack,243);
					}
					else if(gunBat == 1)
					{
						Actor * bat = new Actor(i, j, 243, "Battery Rack", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = 55;
						bat->destructible = new MonsterDestructible(1,0,0,0);
						bat->ai = new LockerAi();
						bat->hostile = false;
						bat->interact = true;
						bat->container = new Container(3);
						generateRandom(bat,243);
						engine.actors.push(bat);
					}
				}
				if (i == x1+3 && j%2 != 0)
				{
					if(gunBat == 0)
					{
						Actor * rack = new Actor(i, j, 243, "Weapon Rack", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = 54;
						engine.actors.push(rack);
						rack->ai = new LockerAi();
						rack->hostile = false;
						rack->interact = true;
						rack->container = new Container(3);
						generateRandom(rack,243);
					}
					else if(gunBat == 1)
					{
						Actor * bat = new Actor(i, j, 243, "Battery Rack", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = 55;
						bat->destructible = new MonsterDestructible(1,0,0,0);
						bat->ai = new LockerAi();
						bat->hostile = false;
						bat->interact = true;
						bat->container = new Container(3);
						generateRandom(bat,243);
						engine.actors.push(bat);
					}
				}
				
				if (i == x2-1)
				{
					int mid = (y1+y2)/2;
					if (j == mid+1 || j == mid-1 || j == mid)
					{
						//weapon vaults:
						Actor *vault = new Actor(i,j,243,"Weapon Vault", TCODColor::white);
						engine.map->tiles[i+j*engine.map->width].decoration = 56;
						vault->destructible = new MonsterDestructible(10,0,0,0);
						vault->ai = new LockerAi();
						((LockerAi*)vault->ai)->locked = true;
						vault->hostile = false;
						vault->interact = true;
						vault->container = new Container(3);
						generateRandom(vault,243);
						engine.actors.push(vault);
					}
					
					if(j == mid+2 || j == mid - 2)
					{
						createTurret(i,j);
					}
					
				}
				
				
			}
		}


	}
	if (room->type == OBSERVATORY) {
		cout << "Observatory made" << endl;

		//placeholder, to be replaced
		//Actor * pcmu = new Actor(x1, y1, 'W', "Space Window", TCODColor::white);
		//engine.actors.push(pcmu);
		for (int i = x1+1; i <= x2-1; i++) {
			for (int j = y1+1; j <= y2-1; j+=2) {
				//the floors for observatories will be blank, and will then adjust the envSta to be "glass" and "broken glass"
			}
		}
	}

	if (room->type == HYDROPONICS) {
		cout << "Hydroponics made" << endl;

		//long rows of hydroponic racks
		bool gardnerCreated = false;
		for (int i = x1+1; i <= x2-1; i++) {
			for (int j = y1+1; j <= y2-1; j+=2) {
				if(!gardnerCreated && canWalk(x1,y1) && engine.getAnyActor(x1,y1)==NULL)
				{
						Actor *g = createGardner(x1, y1);
						gardnerCreated = true;
						((GardnerAi*)g->ai)->initX1 = x1;
						((GardnerAi*)g->ai)->initY1 = y1;
						((GardnerAi*) g->ai)->initY2 = y2;
						((GardnerAi*) g->ai)->initX2 = x2;
				}
				int hydroRng = rng->getInt(0,3,1);
				if (hydroRng == 0)
				{
					Actor * plant = new Actor(i, j, 208, "Hydroponic Oranges", TCODColor::white);//low hunger restore
					plant->hunger = 5;
					if (gardnerCreated == true){
						plant->interact = true;
						Actor *g = engine.getAnyActor(x1,y1);
						int count = rng->getInt(1,3);
						plant->ai = new FruitAi(g,count);
					}
					engine.actors.push(plant);
				}
				else if (hydroRng == 1)
				{
					Actor * plant = new Actor(i, j, 209, "Hydroponic Apples", TCODColor::white);//low hunger restore
					plant->hunger = 5;
					if (gardnerCreated == true){
						plant->interact = true;
						Actor *g = engine.getAnyActor(x1,y1);
						int count = rng->getInt(1,3);
						plant->ai = new FruitAi(g,count);
					}
					engine.actors.push(plant);
				}
				else if (hydroRng == 2)
				{
					Actor * plant = new Actor(i, j, 210, "Hydroponic Bananas", TCODColor::white);//moderate hunger restore
					plant->hunger = 10;
					if (gardnerCreated == true){
						plant->interact = true;
						Actor *g = engine.getAnyActor(x1,y1);
						int count = rng->getInt(1,3);
						plant->ai = new FruitAi(g,count);
					}
					engine.actors.push(plant);
				}
				else if (hydroRng == 3)
				{
					Actor * plant = new Actor(i, j, 211, "Hydroponic Starfruit", TCODColor::white);//large hunger restore
					plant->hunger = 15;
					if (gardnerCreated == true){
						plant->interact = true;
						Actor *g = engine.getAnyActor(x1,y1);
						int count = rng->getInt(1,3);
						plant->ai = new FruitAi(g,count);
					}
					engine.actors.push(plant);
				}
				engine.map->tiles[i+j*engine.map->width].decoration = 48;

			}
		}

	}
	if (room->type == DEFENDED_ROOM) {
		//sandbag
		int dx = x2-x1;
		int dy = y2-y1;
		for (int x = x1; x <= x2; x++)
		{
			for (int y = y1; y <= y2; y++)
			{
				if ((x == x1+1 && y == y1+1) || (x == x2-1 && y == y1+1) || (x == x1+1 && y == y2-1) || (x == x2-1 && y == y2-1))
				{
					float flkr = 1.0;
					Actor *light = new Actor(x,y, 224, "A hastily erected Emergency Light", TCODColor::white);
					light->ai = new LightAi(5,flkr);                //224, crashes when using 224
					engine.actors.push(light);				
				}
				//add all sandbags UP DOWN!
				if ((x == x1 || x == x2) && !((x == x1 && y == y1) || (x == x1 && y == y2) || (x == x2 && y == y1) || (x == x2 && y == y2)))
				{
					Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
					engine.map->tiles[x+y*engine.map->width].decoration = 58;
					engine.actors.push(pcmu);
					pcmu->blocks = false;
					engine.sendToBack(pcmu);
				}
				//add all sandbags LEFT RIGHT!
				if ((y == y1 || y == y2) && !((x == x1 && y == y1) || (x == x1 && y == y2) || (x == x2 && y == y1) || (x == x2 && y == y2)))
				{
					Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
					engine.map->tiles[x+y*engine.map->width].decoration = 76;
					engine.actors.push(pcmu);
					pcmu->blocks = false;
					engine.sendToBack(pcmu);
				}
				//add corners
				if (x == x1 && y == y1)//TL
				{
					Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
					engine.map->tiles[x+y*engine.map->width].decoration = 77;
					engine.actors.push(pcmu);
					pcmu->blocks = false;
					engine.sendToBack(pcmu);
				}
				else if (x == x2 && y == y1)//TR
				{
					Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
					engine.map->tiles[x+y*engine.map->width].decoration = 78;
					engine.actors.push(pcmu);
					pcmu->blocks = false;
					engine.sendToBack(pcmu);
				}
				else if (x == x1 && y == y2)//BL
				{
					Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
					engine.map->tiles[x+y*engine.map->width].decoration = 79;
					engine.actors.push(pcmu);
					pcmu->blocks = false;
					engine.sendToBack(pcmu);
				}
				else if (x == x2 && y == y2)//BR
				{
					Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
					engine.map->tiles[x+y*engine.map->width].decoration = 80;
					engine.actors.push(pcmu);
					pcmu->blocks = false;
					engine.sendToBack(pcmu);
				}
				
				//add the top left pallets+foodStuffs
				if ((x == x1+2 && (y == y1+2 || y == y1+3)) || (x == x1+3 && (y == y1+2 || y ==y1+3)))
				{
					Actor *stackOfFood = createFood(x,y);
					//engine.actors.push(stackOfFood);
					engine.sendToBack(stackOfFood);
					Actor *pallet = new Actor(x, y, 243, "A pallet.", TCODColor::white);
					//engine.mapconDec->setChar(x,y, 32);//
					engine.map->tiles[x+y*engine.map->width].decoration = 32;
					engine.actors.push(pallet);
					pallet->blocks = false;
					engine.sendToBack(pallet);
				}
				//add bottom right pallets+foodStuffs
				if (dx > 7 || dy > 7)
				{
					if ((x == x2-2 && (y == y2-2 || y == y2-3)) || (x == x2-3 && (y == y2-2 || y ==y2-3)))
					{
						Actor *stackOfFood = createFood(x,y);
						//engine.actors.push(stackOfFood);
						engine.sendToBack(stackOfFood);
						Actor *pallet = new Actor(x, y, 243, "A pallet.", TCODColor::white);
						//engine.mapconDec->setChar(x,y, 32);//
						engine.map->tiles[x+y*engine.map->width].decoration = 32;
						engine.actors.push(pallet);
						pallet->blocks = false;
						engine.sendToBack(pallet);
					}
				}
				//add inner sandbags if room allows
				if ((dx >= 7 && dy >= 10))// || (dy > 7 && dx > 10))
				{
					//sandbags UP DOWN
					if (((x == x1+2 && (y >= y1+4 && y <= y2-4))
					|| (x == x2-2 && (y >= y1+4 && y <= y2-4))) && !((x == x1+2 && y == y1+4) || (x == x1+2 && y == y2-4) || (x == x2-2 && y == y1+4) || (x == x2-2 && y == y2-4)))
					{
						Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
						engine.map->tiles[x+y*engine.map->width].decoration = 58;
						engine.actors.push(pcmu);
						pcmu->blocks = false;
						engine.sendToBack(pcmu);
					}
					//sandbags LEFT RIGHT
					if (((y == y1+4 && (x >= x1+2 && x <= x2-2)) 
					|| (y == y2-4 && (x >= x1+2 && x <= x2-2))) && !((x == x1+2 && y == y1+4) || (x == x1+2 && y == y2-4) || (x == x2-2 && y == y1+4) || (x == x2-2 && y == y2-4)))
					{
						Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
						engine.map->tiles[x+y*engine.map->width].decoration = 76;
						engine.actors.push(pcmu);
						pcmu->blocks = false;
						engine.sendToBack(pcmu);
					}
					//sandbag corners!
					if (x == x1+2 && y == y1+4)//TL
					{
						Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
						engine.map->tiles[x+y*engine.map->width].decoration = 77;
						engine.actors.push(pcmu);
						pcmu->blocks = false;
						engine.sendToBack(pcmu);
					}
					else if (x == x1+2 && y == y2-4)//BL
					{
						Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
						engine.map->tiles[x+y*engine.map->width].decoration = 79;
						engine.actors.push(pcmu);
						pcmu->blocks = false;
						engine.sendToBack(pcmu);
					}
					else if (x == x2-2 && y == y2-4)//BR
					{
						Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
						engine.map->tiles[x+y*engine.map->width].decoration = 80;
						engine.actors.push(pcmu);
						pcmu->blocks = false;
						engine.sendToBack(pcmu);
					}
					else if (x == x2-2 && y == y1+4)//TR
					{
						Actor * pcmu = new Actor(x, y, 243, "Sandbag Wall", TCODColor::white);
						engine.map->tiles[x+y*engine.map->width].decoration = 78;
						engine.actors.push(pcmu);
						pcmu->blocks = false;
						engine.sendToBack(pcmu);
					}
					
					
					
					if ((x == x2-3 && y == y1+5) || (x == x1+3 && y == y2-5))
					{
						Actor *MLR = createMLR(x,y, false);
						engine.actors.push(MLR);
						//engine.sendToBack(MLR);
						Actor *pallet = new Actor(x, y, 243, "A pallet.", TCODColor::white);
						//engine.mapconDec->setChar(x,y, 32);//
						engine.map->tiles[x+y*engine.map->width].decoration = 32;
						engine.actors.push(pallet);
						pallet->blocks = false;
						engine.sendToBack(pallet);
					}
				}
				
			}
		}
	}
	if (room->type == BAR) 
	{
		bool frst = true;
		int wall = rng->getInt(1,4);
		int disp = 0;
		int bar = 0;
		switch (wall)
		{
			case 1:bar = x1+2;disp = x1;break;//left wall
			case 2:bar = y1+2;disp = y1;break;//top wall
			case 3:bar = x2-2;disp = x2;break;//right wall
			case 4:bar = y2-2;disp = y2;break;//bottom wall
			default:break;
		}
		for (int x = x1; x <= x2; x++)
		{
			for (int y = y1; y <= y2; y++)
			{
				if (((wall == 1 || wall == 3) && x == disp) || ((wall == 2 || wall == 4) && y == disp))
				{
					//Actor *display = new Actor(x, y, 243, "Liquor Display.", TCODColor::white);
					//engine.map->tiles[x+y*engine.map->width].decoration = 81;
					//engine.actors.push(display);
					//display->blocks = false;
					//engine.sendToBack(display);
					Actor *booze = createAlcohol(x,y);
					engine.actors.push(booze);
					//engine.map->tileType(x,y) = 14;
					tiles[x+y*engine.map->width].tileType = Param::DISPLAY;
					
				}
				if (((wall == 1 || wall == 3) && x == bar) || ((wall == 2 || wall == 4) && y == bar))
				{
					Actor *bar = new Actor(x, y, 243, "Bar.", TCODColor::white);
					engine.map->tiles[x+y*engine.map->width].decoration = rng->getInt(82,84);
					engine.actors.push(bar);
					bar->blocks = false;
					engine.sendToBack(bar);
					static bool second = true;
					if (second)
					{
						int xx = x;
						int yy = y;
						switch (wall)
						{
							case 1:xx = xx+1;break;//left wall
							case 2:yy = yy+1;break;//top wall
							case 3:xx = xx-1;break;//right wall
							case 4:yy = yy-1;break;//bottom wall
							default:break;
						}
						Actor *barstool = new Actor(xx, yy, 243, "Barstool.", TCODColor::white);
						engine.map->tiles[xx+yy*engine.map->width].decoration = 49;
						engine.actors.push(barstool);
						barstool->blocks = false;
						engine.sendToBack(barstool);
					}
					second = !second;
				}
				
				
				if (frst)
				{
					int dx1 = 1;
					int dx2 = 1;
					int dy1 = 1;
					int dy2 = 1;
					switch (wall)
					{
						case 1:
							dx1 = 5;
							break;//left wall
						case 2:
							dy1 = 5;
							break;//top wall
						case 3:
							dx2 = 5;
							break;//right wall
						case 4:
							dy2 = 5;
							break;//bottom wall
						default:break;
					}
					for (int xx = x1+dx1; xx <= x2-dx2; xx++)
					{
						for (int yy = y1+dy1; yy <= y2-dy2; yy++)
						{
							if (((xx-(x1+dx1))%3 != 0) && ((yy-(y1+dy1))%4 != 0))
							{
								Actor *barstool = new Actor(xx, yy, 243, "Standing Table.", TCODColor::white);
								//engine.map->tiles[x+y*engine.map->width].decoration = 32;
								engine.map->tiles[xx+yy*engine.map->width].decoration = rng->getInt(82,84);
								engine.actors.push(barstool);
								barstool->blocks = false;
								engine.sendToBack(barstool);
							}
						}
					}
					frst = false;
				}
				
			}	
		}
	}

	if (room->type == INFECTED_ROOM) 
	{
		for (int x = x1; x <= x2; x++)
		{
			for (int y = y1; y <= y2; y++)
			{
				if (x != x1 && x != x2 && y != y1 && y != y2)
				engine.map->tiles[x+y*engine.map->width].infection = 6.1;
				else
				engine.map->tiles[x+y*engine.map->width].infection = 5.1;
				int fl = rng->getInt(8,15);
				engine.map->tiles[x+y*engine.map->width].flower = fl;
			}
		}
	}
	/*
	 *
	 * SETTINGS FOR OTHER ROOMS CAN BE PLACED HERE
	 *
	 */
	int Recx = 0;
	int Recy = 0;
	if (roomNum == 2 || roomNum == 4 || roomNum == 6)
	{
		Recx = rng->getInt(x1,x2);
		Recy = rng->getInt(y1,y2);
		while (!canWalk(Recx,Recy))
		{
			Recx = rng->getInt(x1,x2);
			Recy = rng->getInt(y1,y2);
		}
	}
	if (roomNum == 2)
	{
		Actor *r1 = createRecord(Recx, Recy);
		engine.actors.push(r1);
	}
	else if (roomNum == 4)
	{
		Actor *r2 = createRecord(Recx, Recy);
		engine.actors.push(r2);
	}
	else if (roomNum == 6)
	{
		Actor *r3 = createRecord(Recx, Recy);
		engine.actors.push(r3);
	}
	
	
	//placing starting locations
	if (roomNum == 0) {
		//put the player in the first room
		bool placed = false;
		while (!placed) {
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);

			if(canWalk(x,y)) {
				engine.player->x = x;
				engine.player->y = y;
				placed = true;
			}
		}
		engine.playerLight = new Actor(engine.player->x, engine.player->y, 'l', "Your Flashlight", TCODColor::white);
		engine.playerLight->ai = new LightAi(2,1,true); //could adjust second '1' to less if the flashlight should flicker
		engine.actors.push(engine.playerLight);
		engine.playerLight->blocks = false;
		//playerLight->ai->moving = true;
		engine.sendToBack(engine.playerLight);
		
		if (engine.level == 1){
			Actor *pet = createCompanion(engine.player->x,engine.player->y,true);
			engine.player->companion = pet;
			engine.actors.push(pet);
			
			if(canWalk(engine.player->x,engine.player->y+1)){
				Actor *pet2 = createCompanion(engine.player->x,engine.player->y+1,false);
				engine.actors.push(pet2);
			}
			
		} else {
			if (engine.player->companion && engine.player->companion->destructible && !engine.player->companion->destructible->isDead()){
				engine.player->companion->x = engine.player->x;
				engine.player->companion->y = engine.player->y;
			}
		}
		
		//Actor *r4 = createRecord(engine.player->x, engine.player->y-1);
		//engine.actors.push(r4);
		/*if (true)
		{
			cout << "making level messages" << endl;
			Actor *r1 = createRecord(engine.player->x+1, engine.player->y);
			engine.actors.push(r1);
			Actor *r2 = createRecord(engine.player->x+2, engine.player->y);
			engine.actors.push(r2);
			Actor *r3 = createRecord(engine.player->x+3, engine.player->y);
			engine.actors.push(r3); 
		}*/
		
	}
	//TCODRandom *rng = TCODRandom::getInstance();

	/* monster section */

	//horde chance
	if (room->type != DEFENDED_ROOM)
	{
		int nbMonsters;
		if (roomNum >0 && rng->getInt(0,19) == 0) {
			nbMonsters = rng->getInt(10, 25);
		}
		else {
			nbMonsters = rng->getInt(0, MAX_ROOM_MONSTERS);
		}

		while (nbMonsters > 0) {
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);

			if(canWalk(x,y) && (x != engine.player->x && y!= engine.player->y)) {
			
				addMonster(x,y,nbMonsters >= MAX_ROOM_MONSTERS);
				nbMonsters--;
			}
		}
	}
	//TCODRandom *rnd = TCODRandom::getInstance();
	//add lights to all rooms, make test later
	if (rng->getInt(0,10) > 4 && room->type != DEFENDED_ROOM)
	{
		//42 is star 
		int numLights = 0;
		int rmSze = (x2 - x1) * (y2 - y1);
		numLights = rmSze/30;
		if (numLights <= 0)
			numLights = 1;
		TCODRandom *myRandom = new TCODRandom();
		int chance = 0;
		for (int i = 0; i < numLights;)
		{
			//bool valid = false;
			//int x = (x1+x2)/2;
			//int y = (y1+y2)/2;
			int x = rng->getInt(x1+1,x2-1);
			int y = rng->getInt(y1+1,y2-1);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.map->tiles[x+y*engine.map->width].decoration == 0) {
				Actor *light = new Actor(x, y, 224, "A hastily erected Emergency Light", TCODColor::white);
				//4,1 = standard light, radius, flkr
				
				//0.8 is lower limit, put closer to 1 for less flicker
				chance = myRandom->getInt(0,10,5);
				bool broke = false;
				float rng2;
				if (chance >= 5 && chance < 10)//could make this number and all flickering number change based on level
				{
					rng2 = myRandom->getFloat(0.9000f,0.9900f,0.9500f);
					light->name = "A flickering hastily erected Emergency Light";
					//engine.gui->message(TCODColor::red, "flickering %d",chance);
				}
				else if (chance >= 10)
				{
					rng2 = myRandom->getFloat(0.9000f,0.9900f,0.9500f);
					light->name = "A broken Emergency Light";
					broke = true;
					//engine.gui->message(TCODColor::red, "broken %d",chance);
				}
				else
				{
					rng2 = 1;
					//engine.gui->message(TCODColor::red, "fine %d",chance);
				}
				light->ai = new LightAi(rng->getInt(3,6),rng2);
				if (broke)
				{
					LightAi *l = (LightAi*)light->ai;
					l->onOff = false;
				}
				engine.actors.push(light);
				i++;
			}
		}
	}
	
	
	//add items
	int nbItems = rng->getInt(0, MAX_ROOM_ITEMS);
	while (nbItems > 0) {
		int x = rng->getInt(x1,x2);
		int y = rng->getInt(y1,y2);
		if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y)) {
			addItem(x,y, room->type);
			nbItems--;
		}
	} 
	
	
	
	if (artifacts < MAX_ARTIFACTS){
		int artChance = rng->getInt(1,100);
		if (artChance < 2+(engine.level/2)) {
			int x = rng->getInt(x1,x2);
			int y = rng->getInt(y1,y2);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y)) {
				createArtifact(x,y);
				artifacts++;
			}
		}
	}
	
	int rand = rng->getInt(0,100);
	//Vending Machines spawn in corners of standard rooms at random
	
	if(rand <= 50 && room->type == MESSHALL) //a mess hall has a 50% chance of having a vending machine (provided it has rooms), increased since standard rooms are less likely
	{
		int c = rng->getInt(0,3);
		bool x1y1 = canWalk(x1,y1) && engine.getAnyActor(x1,y1) == NULL && ( (canWalk(x1,y1 + 1) && engine.getAnyActor(x1,y1 + 1) == NULL)  && (canWalk(x1+1,y1 + 1) && engine.getAnyActor(x1+1,y1)==NULL) ) ;
		bool x1y2 = canWalk(x1,y2) && engine.getAnyActor(x1,y2) == NULL && ((canWalk(x1,y2 -1 ) && engine.getAnyActor(x1,y2-1) == NULL) && (canWalk(x1+1,y2) && engine.getAnyActor(x1+1,y2)== NULL));
		bool x2y2 = canWalk(x2,y2) && engine.getAnyActor(x2,y2) == NULL && ((canWalk(x2,y2 -1 ) && engine.getAnyActor(x2,y2 - 1) == NULL) && (canWalk(x2-1,y2) && engine.getAnyActor(x2-1,y2) == NULL) ) ;
		bool x2y1 = canWalk(x2,y1) && engine.getAnyActor(x2,y2) == NULL && ((canWalk(x2,y1 +1 ) && engine.getAnyActor(x2,y1 + 1) == NULL) && (canWalk(x2-1,y1) && engine.getAnyActor(x2-1,y1) == NULL) );
		
		int vendX = -1, vendY = -1;
		if(c == 0 && x1y1)
		{	vendX = x1;
			vendY = y1;
		}
		if(c == 1 && x1y2)
		{
			vendX = x1;
			vendY = y2;
		}
		if(c == 2 && x2y2)
		{
			vendX = x2;
			vendY = y2;
		}
		if(c == 3 && x2y1)
		{
			vendX = x2;
			vendY = y1;
		}
		if(vendX != -1 && vendY != -1)
		{
			createVendor(vendX, vendY);
			
			bool case1 = (canWalk(vendX-1,vendY) && engine.getAnyActor(vendX-1,vendY) == NULL);
			bool case2 = (canWalk(vendX,vendY-1) && engine.getAnyActor(vendX,vendY-1) == NULL);
			bool case3 = (canWalk(vendX,vendY+1) && engine.getAnyActor(vendX,vendY+1) == NULL);
			bool case4 = (canWalk(vendX+1,vendY) && engine.getAnyActor(vendX+1,vendY) == NULL);
			Actor *securityBot = NULL;
			
			if(case1)
				securityBot = createSecurityBot(vendX-1, vendY);
			else if(case2)
				securityBot = createSecurityBot(vendX, vendY-1);
			else if(case3)
				securityBot = createSecurityBot(vendX, vendY+1);
			else if(case4)
				securityBot = createSecurityBot(vendX+1, vendY);
				
			if(securityBot != NULL)
			{
				securityBot->hostile = false;
				SecurityBotAi *sbAi = (SecurityBotAi*) securityBot->ai;
				sbAi->vendingX = vendX;
				sbAi->vendingY = vendY;
			}
		}

	}
	//85 chance of spawning turrets in the corners of standard rooms, increased since there are less STANDARD rooms
	if(((rand >= 0 && rand <= 70) && room->type == STANDARD) || room->type == ARMORY)
	{
		int cx = (x1 + x2)/2.0;
		int cy = (y1 + y2)/2.0;
		int c = rng->getInt(0,3);
		bool console = canWalk(cx,cy) && engine.getAnyActor(cx,cy)==NULL;
		bool x1y1 = canWalk(x1,y1) && engine.getAnyActor(x1,y1)==NULL;
		bool x1y2 = canWalk(x1,y2) && engine.getAnyActor(x1,y2)==NULL;
		bool x2y2 = canWalk(x2,y2) && engine.getAnyActor(x2,y2)==NULL;
		bool x2y1 = canWalk(x2,y1) && engine.getAnyActor(x2,y1)==NULL;
		if(console || room->type == ARMORY)
		{
			Actor * turret, *turretControl = NULL;
			if(room->type == ARMORY)
			{ //place turret contorls in
				cx = x1;
				cy = y1;
				x1y1 = false;
			}
			if(room->type == STANDARD || true)
				turretControl = createTurretControl(cx,cy);
			
			for(int i = 0; i < 4; i++)
			{
				c = rng->getInt(0,3);
				if((c == 0 || room->type == ARMORY) && x1y1)
				{
					turret = createTurret(x1,y1);
					TurretAi *ai = (TurretAi*)turret->ai;
					if(turretControl)
					{
						ai->controlX = cx;
						ai->controlY = cy;
					}
					x1y1 = false;
				}
				if((c == 1 || room->type == ARMORY) && x1y2)
				{
					turret = createTurret(x1,y2);
					TurretAi *ai = (TurretAi*)turret->ai;
					if(turretControl)
					{
						ai->controlX = cx;
						ai->controlY = cy;
					}
					x1y2 = false;
				}
					
				if((c == 2 || room->type == ARMORY) && x2y2)
				{
					turret = createTurret(x2,y2);
					TurretAi *ai = (TurretAi*)turret->ai;
					if(turretControl)
					{
						ai->controlX = cx;
						ai->controlY = cy;
					}
					x2y2 = false;
				}
				if((c == 3 || room->type == ARMORY) && x2y1)
				{
					turret = createTurret(x2, y1);
					TurretAi *ai = (TurretAi*)turret->ai;
					if(turretControl)
					{
						ai->controlX = cx;
						ai->controlY = cy;
					}
					x2y1 = false;
				}
				if(room->type == ARMORY)
				{	
					int mid = (y1+y2)/2;
					turret = engine.getAnyActor(x2-1,mid+2);
					if(turret && turretControl)
					{
						TurretAi *ai = (TurretAi*)turret->ai;
						ai->controlX = cx;
						ai->controlY = cy;
						
					}
					turret = engine.getAnyActor(x2-1,mid-2);
					if(turret && turretControl)
					{
						TurretAi *ai = (TurretAi*)turret->ai;
						ai->controlX = cx;
						ai->controlY = cy;
					}
	
				}
			}
			
				

		}
			
	}
	
	
	//set the stairs position
	while (true) {
		int x = rng->getInt(x1,x2);
		int y = rng->getInt(y1,y2);
		if (canWalk(x,y) && canWalk(x+1,y)){ //boss spawns at (stairs->x+1 stairs->y), hence the additional check)
			engine.stairs->x = x;
			engine.stairs->y = y;
			break;
		}
	}
}

bool Map::isWall(int x, int y) const {
	return !map->isWalkable(x,y);
}

bool Map::canWalk(int x, int y) const {
	if (isWall(x,y)) {
		//this is  a wall
		return false;
	}
	for (Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor->blocks && actor->x == x && actor->y == y) {
			//there is an actor there. cannot walk through him
			return false;
		}
	}
	return true;
}

bool Map::isExplored(int x, int y) const {
	return tiles[x+y*width].explored;
}

bool Map::isInfected(int x, int y) const {
	return (bool)((int)tiles[x+y*width].infection);
}

int Map::infectionState(int x, int y) const {
	return (int)tiles[x+y*width].infection;
}
void Map::infectFloor(int x, int y) {
	tiles[x+y*width].infection += rng->getFloat(.1, 2);
}

bool Map::isInFov(int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return false;
	}
	
	if ((map->isInFov(x,y)) && (engine.distance(engine.player->x,x,engine.player->y,y) <= 1 || isLit(x,y))) {
		tiles[x+y*width].explored = true;
		return true;
	}
	return false;
}

bool Map::isLit(int x, int y) const {
	return tiles[x+y*width].lit;
}


void Map::computeFov() {
	//compute FOV, then make a light to light up the area?
	//or just have them be lit/unlit
	
	//compute FOV, everything in FOV will be lit 
	//if your FOV interacts with another thing's FOV, light up both FOV's
	map->computeFov(engine.player->x,engine.player->y, 10/*engine.fovRadius*/);//@ 6 you cannot run away from mobs
}
void Map::render() const {

	static const TCODColor darkWall(0,0,100);
	static const TCODColor darkGround(50,50,150);
	static const TCODColor lightWall(30,110,50);
	static const TCODColor lightGround(200,180,50);

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if ((isInFov(x,y) && engine.distance(engine.player->x,x,engine.player->y,y) < 1) || (isInFov(x,y) && isLit(x,y))){// || true) {
				//TCODConsole::root->setCharBackground(x,y,isWall(x,y) ? lightWall : lightGround);
				//this line is all that is needed if you want the tiles view. comment out all the other stuff if so

				//this is going to have to be changed if we add more environment tiles


				if (isWall(x,y)) {
					if (isInfected(x,y)) {
						engine.mapcon->setChar(x, y, '^');
						engine.mapcon->setCharForeground(x,y,TCODColor::green);
					}
					else {
						engine.mapcon->setChar(x, y, '^');
						engine.mapcon->setCharForeground(x,y,TCODColor::white);
					}
				}
				else {
					if (isInfected(x,y)) {
						engine.mapcon->setChar(x, y, 31);//29
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
						//engine.map->tiles[x+y*engine.map->width].num++;
						//engine.mapconCpy->setChar(x, y, 29);
						//engine.mapconCpy->setCharBackground(x,y,TCODColor::blue);
					}
					else {
						engine.mapcon->setChar(x, y, 31);
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
						//engine.map->tiles[x+y*engine.map->width].num++;
						//engine.mapconCpy->setChar(x, y, 31);
						//engine.mapconCpy->setCharBackground(x,y,TCODColor::blue);
					}
				}
			}
			else if (isExplored(x,y)) {
				//TCODConsole::root->setCharBackground(x,y,isWall(x,y) ? darkWall : darkGround);
				//this line is all that is needed if you want the tiles view. comment out all the other stuff if so

				if (isWall(x,y)) {
					if (isInfected(x,y)) {
						engine.mapcon->setChar(x, y, '^');
						engine.mapcon->setCharForeground(x,y,TCODColor::darkGreen);
					}
					else {
						engine.mapcon->setChar(x, y, '^');
						engine.mapcon->setCharForeground(x,y,TCODColor::darkGrey);
					}
				}
				else {
					if (isInfected(x,y)) {
						engine.mapcon->setChar(x, y, 30);//28
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
						//engine.map->tiles[x+y*engine.map->width].num = 0;
						//engine.mapconCpy->setChar(x, y, 28);
						//engine.mapconCpy->setCharBackground(x,y,TCODColor::blue);
					}
					else {
						engine.mapcon->setChar(x, y, 30);
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
						//engine.map->tiles[x+y*engine.map->width].num = 0;
						//engine.mapconCpy->setChar(x, y, 30);
						//engine.mapconCpy->setCharBackground(x,y,TCODColor::blue);
					}
				}
			}
			/*if (isInFov(x,y) )//|| isExplored(x,y))
			{
				//if it is false set to true
				tiles[x+y*width].lit = true;
			}
			else
			{
				tiles[x+y*width].lit = false;
			}
			if (isLit(x,y)){
				engine.mapcon->setCharForeground(x,y,TCODColor::yellow);
			}
			else
			{
				engine.mapcon->setCharForeground(x,y,TCODColor::white);
			}*/
		}
	}
}

void Map::generateRandom(Actor *owner, int ascii){
	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0,100);

	//zed umber
	/*if (ascii == 136) {
		}
	*/
	
	//modify as needed:
	if(ascii == 243 && engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 56) //this is the weapon vault, add whatever gubbins that you want
	{
		//for now, they just get artifacts, but change to whatever
		Actor *artifact = createArtifact(0,0);
		artifact->pickable->pick(artifact, owner);
		return;
	}
	else if(ascii == 243 && engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 54 && engine.level == 0){//weapon rack in tutorial
		int random = rng->getInt(0,125);
		if(random < 75){
			Actor *melee = createCombatKnife(0,0);
			engine.actors.push(melee);
			melee->pickable->pick(melee,owner);
		}
		else if (random < 100) {
			Actor *MLR = createMLR(0,0,false);
			engine.actors.push(MLR);
			MLR->pickable->pick(MLR,owner);
		}else{
			Actor *flamer = createFlameThrower(0,0,false);
			engine.actors.push(flamer);
			flamer->pickable->pick(flamer,owner);
		}
	}
	else if(ascii == 243 && engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 54){//weapon rack
		int random = rng->getInt(0,125);
		if(random < 75){
			Actor *melee = createCombatKnife(0,0);
			engine.actors.push(melee);
			melee->pickable->pick(melee,owner);
		}
		else if (random < 100) {
			Actor *MLR = createMLR(0,0,false);
			engine.actors.push(MLR);
			MLR->pickable->pick(MLR,owner);
		}else{
			Actor *flamer = createFlameThrower(0,0,false);
			engine.actors.push(flamer);
			flamer->pickable->pick(flamer,owner);
		}
	}
	else if(ascii == 243 && engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 55){//battery rack
		int random = rng->getInt(0,100);
		if (random < 100) {
			Actor *batt = createBatteryPack(0,0);
			engine.actors.push(batt);
			batt->pickable->pick(batt,owner);
		}
	}
	else if(ascii == 243){//locker, this might be a problem if we want multiple decors to drop different things
			int random = rng->getInt(0,400);
			if(random < 30){
				Actor *flare = createFlare(0,0);
				engine.actors.push(flare);
				flare->pickable->pick(flare,owner);
			}else if(random < 30+20){
				Actor *kevlarVest = createKevlarVest(0,0,false);
				engine.actors.push(kevlarVest);
				kevlarVest->pickable->pick(kevlarVest,owner);
			}else if(random < 30+20+40){
				Actor *myBoots = createMylarBoots(0,0,false);
				engine.actors.push(myBoots);
				myBoots->pickable->pick(myBoots,owner);
			}else if(random < 30+20+40+30){
				Actor *myGreaves = createMylarGreaves(0,0,false);
				engine.actors.push(myGreaves);
				myGreaves->pickable->pick(myGreaves,owner);
			}else if(random < 30+20+40+30+30){
				Actor *myVest = createMylarVest(0,0,false);
				engine.actors.push(myVest);
				myVest->pickable->pick(myVest,owner);
			}else if(random < 30+20+40+30+30+40){
				Actor *myCap = createMylarCap(0,0,false);
				engine.actors.push(myCap);
				myCap->pickable->pick(myCap,owner);
			}else if(random < 30+20+40+30+30+40+10){
				Actor *titanHelm = createTitanHelm(0,0,false);
				engine.actors.push(titanHelm);
				titanHelm->pickable->pick(titanHelm,owner);
			}else if(random < 30+20+40+30+30+40+10){
				Actor *titanMail = createTitanMail(0,0,false);
				engine.actors.push(titanMail);
				titanMail->pickable->pick(titanMail,owner);
			}else if(random < 30+20+40+30+30+40+10){
				Actor *titanGreaves = createTitanGreaves(0,0,false);
				engine.actors.push(titanGreaves);
				titanGreaves->pickable->pick(titanGreaves,owner);
			}else if(random < 30+20+40+30+30+40+10){
				Actor *titanBoots = createTitanBoots(0,0,false);
				engine.actors.push(titanBoots);
				titanBoots->pickable->pick(titanBoots,owner);
			}else if(random < 30+20+40+30+30+40+10+30){
				Actor *kevlarHelm = createKevlarHelmet(0,0,false);
				engine.actors.push(kevlarHelm);
				kevlarHelm->pickable->pick(kevlarHelm,owner);
			}else if(random < 30+20+40+30+30+40+10+30+20){
				Actor *kevlarGreaves = createKevlarGreaves(0,0,false);
				engine.actors.push(kevlarGreaves);
				kevlarGreaves->pickable->pick(kevlarGreaves,owner);
			}else if(random < 30+20+40+30+30+40+10+30+20+30){
				Actor *kevlarBoots = createKevlarBoots(0,0,false);
				engine.actors.push(kevlarBoots);
				kevlarBoots->pickable->pick(kevlarBoots,owner);
			}else{
				Actor *batt = createBatteryPack(0,0);
				engine.actors.push(batt);
				batt->pickable->pick(batt,owner);
			}
		}
	
	if(dice <= 40 && !(ascii == 129 || ascii == 130 || ascii == 146)){ //security bots should always drop keys
			return;
	}else{
		if(ascii == 133) //infected grenadier
		{
			for(int i = 0; i < owner->container->size/4; i++)
			{
				int rand = rng->getInt(0,30);
				if(rand <= 15)
				{
					Actor *emp = createEMP(0,0);
					engine.actors.push(emp);
					emp->pickable->pick(emp,owner);
				} else if(rand <= 25)
				{
					Actor *frag = createFrag(0,0);
					engine.actors.push(frag);
					frag->pickable->pick(frag,owner);
				}
				else{
					Actor *fb = createFireBomb(0,0);
					engine.actors.push(fb);
					fb->pickable->pick(fb,owner);
				}
			}
		}else if(ascii == 149) //infectedMarines have 60% chance of dropping an item with 50% chance of it being a MLR, and the other 50% chance being a battery pack
		{
			if(dice <= 70)
			{
				Actor *MLR = createMLR(0,0,false);
				engine.actors.push(MLR);
				MLR->pickable->pick(MLR,owner);
			}
			else
			{
				Actor *batt = createBatteryPack(0,0);
				engine.actors.push(batt);
				batt->pickable->pick(batt,owner);
			}
			
			
		}else if(ascii == 164){
			for(int i = 0; i < owner->container->size/4; i++){
				int rndA = rng->getInt(0,100);
				if(rndA > 40){
					int rnd = rng->getInt(0,100);
					if (rnd < 30) {
						//create a health potion
						Actor *healthPotion = createHealthPotion(0,0);
						engine.actors.push(healthPotion);
						healthPotion->pickable->pick(healthPotion,owner);
					} else if(rnd < 10+30) {
						//create a scroll of lightningbolt
						Actor *scrollOfLightningBolt = createEMP(0,0);
						engine.actors.push(scrollOfLightningBolt);
						scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
					} else if(rnd < 10+30+20) {
						//create a scroll of fireball
						Actor *scrollOfFireball = createFireBomb(0,0);
						engine.actors.push(scrollOfFireball);
						scrollOfFireball->pickable->pick(scrollOfFireball,owner);
					} else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = createFlashBang(0,0);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
				}
			}
		}else if(ascii == 165 || ascii == 166){ //Spore Creature and Mini Spore Creature
			for(int i = 0; i < owner->container->size/4; i++){
				int rndA2 = rng->getInt(0,100);
				if(rndA2 > 45){
					int rnd2 = rng->getInt(0,100);
					if (rnd2 < 25) {
						//create a health potion
						Actor *healthPotion = createHealthPotion(0,0);
						engine.actors.push(healthPotion);
						healthPotion->pickable->pick(healthPotion,owner);
					} else if(rnd2 < 25+20) {
						//create a scroll of lightningbolt
						Actor *scrollOfLightningBolt = createEMP(0,0);
						engine.actors.push(scrollOfLightningBolt);
						scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
					} else if(rnd2 < 25+20+25) {
						//create a scroll of fireball
						Actor *scrollOfFireball = createFireBomb(0,0);
						engine.actors.push(scrollOfFireball);
						scrollOfFireball->pickable->pick(scrollOfFireball,owner);
					} else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = createFlashBang(0,0);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
				}
			}
		}else if(ascii == 148){
			for(int i = 0; i < owner->container->size/4; i++){
				int rndA2 = rng->getInt(0,100);
				if(rndA2 > 45){
					int rnd = rng->getInt(0,200);
					if (rnd < 30) {
						//create a health potion
						Actor *healthPotion = createHealthPotion(0,0);
						engine.actors.push(healthPotion);
						healthPotion->pickable->pick(healthPotion,owner);
					} else if(rnd < 10+30) {
						//create a scroll of lightningbolt
						Actor *scrollOfLightningBolt = createEMP(0,0);
						engine.actors.push(scrollOfLightningBolt);
						scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
					} else if(rnd < 10+30+20) {
						//create a scroll of fireball
						Actor *scrollOfFireball = createFireBomb(0,0);
						engine.actors.push(scrollOfFireball);
						scrollOfFireball->pickable->pick(scrollOfFireball,owner);
					}else if(rnd < 10+30+20+30){
						//create a pair of mylar boots
						Actor *myBoots = createMylarBoots(0,0,false);
						engine.actors.push(myBoots);
						myBoots->pickable->pick(myBoots,owner);
					}else if(rnd < 10+30+20+30+30){
						//create a pair of mylar greaves
						Actor *myGreaves = createMylarGreaves(0,0,false);
						engine.actors.push(myGreaves);
						myGreaves->pickable->pick(myGreaves,owner);
					}else if(rnd < 10+30+20+30+30+20){
						//create a mylar vest
						Actor *myVest = createMylarVest(0,0,false);
						engine.actors.push(myVest);
						myVest->pickable->pick(myVest,owner);
					}else if(rnd < 10+30+20+30+30+20+30){
						//create a mylar cap
						Actor *myCap = createMylarCap(0,0,false);
						engine.actors.push(myCap);
						myCap->pickable->pick(myCap,owner);
					}else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = createFlashBang(0,0);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
					}
				}
			
		}else if(ascii == 132){
			for(int i = 0; i < owner->container->size/4; i++){
				int rndA2 = rng->getInt(0,170);
				if(rndA2 > 45){
					int rnd = rng->getInt(0,100);
					if (rnd < 30) {
						//create a health potion
						Actor *healthPotion = createHealthPotion(0,0);
						engine.actors.push(healthPotion);
						healthPotion->pickable->pick(healthPotion,owner);
					} else if(rnd < 10+30) {
						//create a scroll of lightningbolt
						Actor *scrollOfLightningBolt = createEMP(0,0);
						engine.actors.push(scrollOfLightningBolt);
						scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
					} else if(rnd < 10+30+20) {
						//create a scroll of fireball
						Actor *scrollOfFireball = createFireBomb(0,0);
						engine.actors.push(scrollOfFireball);
						scrollOfFireball->pickable->pick(scrollOfFireball,owner);
					}else if(rnd < 10+30+20+10){
						//create Kevlar Vest
						Actor *kevlarVest = createKevlarVest(0,0,false);
						engine.actors.push(kevlarVest);
						kevlarVest->pickable->pick(kevlarVest,owner);
					}else if(rnd < 10+30+20+10+10){
						//create Kevlar Greaves
						Actor *kevlarGreaves = createKevlarGreaves(0,0,false);
						engine.actors.push(kevlarGreaves);
						kevlarGreaves->pickable->pick(kevlarGreaves,owner);
					}else if(rnd < 10+30+20+10+10+20){
						//create Kevlar Helmet
						Actor *kevlarHelm = createKevlarHelmet(0,0,false);
						engine.actors.push(kevlarHelm);
						kevlarHelm->pickable->pick(kevlarHelm,owner);
					}else if(rnd < 10+30+20+10+10+20+20){
						//create Kevlar Boots
						Actor *kevlarBoots = createKevlarBoots(0,0,false);
						engine.actors.push(kevlarBoots);
						kevlarBoots->pickable->pick(kevlarBoots,owner);
					}else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = createFlashBang(0,0);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
				}
			}
		}else if(ascii == 134){
			for(int i = 0; i < owner->container->size/4; i++){
				int rndA2 = rng->getInt(0,100);
				if(rndA2 > 30){
					int rnd = rng->getInt(0,100);
					if (rnd < 30) {
						//create a health potion
						Actor *healthPotion = createHealthPotion(0,0);
						engine.actors.push(healthPotion);
						healthPotion->pickable->pick(healthPotion,owner);
					} else if(rnd < 10+30) {
						//create a emp
						Actor *scrollOfLightningBolt = createEMP(0,0);
						engine.actors.push(scrollOfLightningBolt);
						scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
					} else if(rnd < 10+30+20) {
						//create a flare
						Actor *flare = createFlare(0,0);
						engine.actors.push(flare);
						flare->pickable->pick(flare,owner);
					}else{
						//create a Flashbang
						Actor *scrollOfConfusion = createFlashBang(0,0);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
				}
			}
		}else if(ascii == 147){ //Turret
			int rndA2 = rng->getInt(0,100);
			if(rndA2 > 50){
				for(int i = 0; i < owner->container->size/4; i++){
					//Fill Inventory with Batteries
					Actor *battery = createBatteryPack(0,0);
					engine.actors.push(battery);
					battery->pickable->pick(battery,owner);
				}
			}
		}
		else if(ascii == 129 || ascii == 130 || ascii == 146) //Security Bot gets keys
		{
			for(int i = 0; i < owner->container->size/4; i++)
			{
					//Fill Inventory with Batteries and 1 key
					if(i >= 1)
					{
						Actor *battery = createBatteryPack(0,0);
						engine.actors.push(battery);
						battery->pickable->pick(battery,owner);
					}
					else
					{
						Actor *key = createKey(0,0,0);
						engine.actors.push(key);
						key->pickable->pick(key,owner);
					}
				}
		}
		else if(ascii == 131){ //Cleaner Bot (aka DJ ROOMBA)
			int rndA2 = rng->getInt(0,100);
			if(rndA2 > 60){
				//Give A Battery
				Actor *battery = createBatteryPack(0,0);
				engine.actors.push(battery);
				battery->pickable->pick(battery,owner);
			
			}
		}
	}
}
Actor *Map::createCurrencyStack(int x, int y){
	Actor *currencyStack = new Actor(x,y,188,"PetaBitcoins",TCODColor::yellow);
	currencyStack->sort = 0;
	currencyStack->blocks = false;
	if(engine.piratesFound == 0)
		currencyStack->pickable = new Coinage(1,100+75*(engine.level-1));
	else
		currencyStack->pickable = new Coinage(1,150+100*(engine.level-1));
	return currencyStack;
}

Actor *Map::createHealthPotion(int x,int y){
	Actor *healthPotion = new Actor(x,y,184,"Medkit", TCODColor::white);
	healthPotion->sort = 1;
	healthPotion->blocks = false;
	healthPotion->pickable = new Healer(20);
	healthPotion->pickable->value = 40;
	healthPotion->pickable->inkValue = 15;
	return healthPotion;
}
Actor *Map::createFlashBang(int x, int y){
	Actor *scrollOfConfusion = new Actor(x,y,181,"Flashbang", TCODColor::white);
	scrollOfConfusion->sort = 2;
	scrollOfConfusion->blocks = false;
	scrollOfConfusion->pickable = new Confuser(10,8);
	scrollOfConfusion->pickable->value = 25;
	scrollOfConfusion->pickable->inkValue = 10;
	return scrollOfConfusion;
}
Actor *Map::createFlare(int x, int y){
	Actor *scrollOfFlaring = new Actor(x,y,187,"Flare", TCODColor::white);
	scrollOfFlaring->sort = 2;
	scrollOfFlaring->blocks = false;
	scrollOfFlaring->pickable = new Flare(10,5,5);//10 is turns, can be random, 5 is range of throwability (constant), 5 is range of flare
	scrollOfFlaring->pickable->value = 25;
	scrollOfFlaring->pickable->inkValue = 10;
	return scrollOfFlaring;
}
Actor *Map::createAlcohol(int x, int y){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	Actor *scrollOfDrunk = new Actor(x,y,15,"Bottle o' Alcohol", TCODColor::white);
	//TCODColor col = TCODColor::white; 
	//int ascii = random->getInt(11,15);
	int type = random->getInt(1,15);
	int schnapp = random->getInt(1,5);
	int origin = random->getInt(1,10);
	int quality = random->getInt(-2,2,0);
	int base = 0;
	int strength = 0;
	switch (quality)
	{
		case -2:
			strcat(nameBuf,"Stale ");
			break;
		case -1:
			strcat(nameBuf,"Old ");
			break;
		case 0:
			break;
		case 1:
			strcat(nameBuf,"Nice ");
			break;
		case 2:
			strcat(nameBuf,"Top Shelf ");
			break;
		default:break;
	}
	
	
	switch(origin)
	{
		case 1:
			break;
		case 2:
			strcat(nameBuf,"Legal ");
			break;
		case 3:
			strcat(nameBuf,"Smuggled ");
			break;
		case 4:
			strcat(nameBuf,"Sanctioned ");
			break;
		case 5:
			strcat(nameBuf,"Illegal ");
			break;
		case 6:
			strcat(nameBuf,"Approved ");
			break;
		case 7:
			strcat(nameBuf,"Banned ");
			break;
		case 8:
			strcat(nameBuf,"Lawful ");
			break;
		case 9:
			strcat(nameBuf,"Illicit ");
			break;
		case 10:
			strcat(nameBuf,"Stolen ");
			break;
	}
	int dec = 60;
	switch(type) 
	{
		case 1:
			strcat(nameBuf,"Beer");
			//col = TCODColor::desaturatedOrange;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 2;//8 PROOF
			strength = 1;
			break;
		case 2:
			strcat(nameBuf,"Whiskey");
			//col = TCODColor::lightOrange;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 9;//108 PROOF
			strength = 2;
			break;
		case 3:
			strcat(nameBuf,"Brandy");
			//col = TCODColor::darkOrange;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 9;//106 PROOF
			strength = 2;
			break;
		case 4:
			strcat(nameBuf,"Vodka");
			//col = TCODColor::lighterGrey;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 7;//80 PROOF
			strength = 1;
			break;
		case 5:
			strcat(nameBuf,"Absinthe");
			//col = TCODColor::lighterGreen;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 12;//140 PROOF
			strength = 2;
			break;
		case 6:
			strcat(nameBuf,"Moonshine");
			//col = TCODColor::lighterBrown;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 14;//150 PROOF
			strength = 3;
			break;
		case 7:
			strcat(nameBuf,"Wine");
			//col = TCODColor::lighterPurple;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 4;//20 PROOF
			strength = 1;
			break;
		case 8:
			strcat(nameBuf,"Merlot");
			//col = TCODColor::purple;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 5;//22 PROOF
			strength = 1;
			break;
		case 9:
			strcat(nameBuf,"Bourbon");
			//col = TCODColor::darkOrange;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 9;//109 PROOF
			strength = 2;
			break;
		case 10:
			strcat(nameBuf,"Rum");
			//col = TCODColor::darkerOrange;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 9;//107 PROOF
			strength = 2;
			break;
		case 11:
			
			switch(schnapp)
			{
				case 1:
					strcat(nameBuf,"Mint ");
					//col = TCODColor::lighterRed;
					break;
				case 2:
					strcat(nameBuf,"Peach ");
					//col = TCODColor::pink;
					break;	
				case 3:
					strcat(nameBuf,"Berry ");
					//col = TCODColor::red;
					break;	
				case 4:
					strcat(nameBuf,"Orange ");
					//col = TCODColor::lighterOrange;
					break;
				case 5:
					strcat(nameBuf,"Bland ");
					//col = TCODColor::lighterBrown;
					break;	
				default:break;
			}
			strcat(nameBuf,"Schnapps");
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 8;//100 PROOF
			strength = 1;
			break;
		case 12:
			strcat(nameBuf,"Fermented Starfruit");
			//col = TCODColor::lighterYellow;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 10;//125 PROOF
			strength = 2;
			break;
		case 13:
			strcat(nameBuf,"\"Engineer\'s special\"");
			//col = TCODColor::darkerGrey;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 16;//160 PROOF
			strength = 3;
			break;	
		case 14:
			strcat(nameBuf,"Lager");
			//col = TCODColor::lighterOrange;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 3;//10 PROOF
			strength = 1;
			break;	
		case 15:
			strcat(nameBuf,"Tequila");
			//col = TCODColor::lightGreen;
			engine.map->tiles[x+y*engine.map->width].decoration = dec+type;
			base = 6;//75 PROOF
			strength = 1;
			break;	
		default:break;
	}
	
	scrollOfDrunk->name = nameBuf;
	//scrollOfDrunk->ch = ascii;
	scrollOfDrunk->sort = 1;
	scrollOfDrunk->blocks = false;
	scrollOfDrunk->pickable = new Alcohol(strength,base+quality);//1 is the amt of buff/debuff, 10 is the length of buff/debuff
	scrollOfDrunk->pickable->value = 30;
	scrollOfDrunk->pickable->inkValue = 5;
	//scrollOfDrunk->col = col;
	return scrollOfDrunk;
}
Actor *Map::createRecord(int x, int y){
	//cout << "making recorder" << endl;
	Actor *scrollOfRecords = new Actor(x,y,227,"Personal Recorder", TCODColor::white);
	//cout << "making namebuf" << endl;
	char* nameBuf = new char[250]; 
	memset(nameBuf,0,250);//250 is the maximum # of characters
	//cout << "namebuf made" << endl;
	TCODRandom *random = TCODRandom::getInstance();
	//cout << "about to get temp" << endl;
	//FINALchar ** temp = &engine.records[0];
	//char *temp = engine.records[0];
	//cout << "about to get msg, got temp"<< endl;
	//FINALchar  * msg = temp[random->getInt(0,4)];
	//cout << "msg recieved" << endl;
	//scrollOfRecords->ai = new TriggerAi( ">>102:329:32<< This is an example recording, the ship has become madness, I have had headaches every day for a week, the crew have begun attacking each other without cause");
	//cout << "getting file" << endl;
	string line;
	char * file = new char[35];
	memset(file,0,35);
	strcat(file,"terminals/questTerminals.txt");
	ifstream myfile(file);
	//myfile = new ifstream(file);
	//cout << "file gotten" << endl;
	int lvl = engine.level; 
	//if lvl = 0 NONE
	//if lvl = 1 (1-3)
	//if lvl = 2 (4-6)
	//if lvl = 3 (7-9)
	//if lvl = 4 (10-12)
	//if lvl = 5 (13)
	int maxid = 0;
	int minid = 0;
	switch (lvl)
	{
		case 0:
			maxid = 0;
			minid = 0;
			break;
		case 1:
			maxid = 3;
			minid = 1;
			break;
		case 2:
			maxid = 6;
			minid = 4;
			break;
		case 3:
			maxid = 9;
			minid = 7;
			break;
		case 4:
			maxid = 12;
			minid = 10;
			break;
		case 5:
			maxid = 13;
			minid = 13;
			break;
		default:break;
	}
	int id2find = random->getInt(minid,maxid);//1-8 for the numbers avaiable, 1-99 MAX
	bool txt = true;
	//cout << "there are " << engine.ctrTer << " terminals left" << endl;
	//cout << "ctrTer " << engine.ctrTer << endl;
	if (engine.ctrTer > 0)
	{
		while (!engine.valTer[id2find-1])
		{
			//cout << "id2find: " << id2find << endl;
			id2find = random->getInt(minid,maxid);
		}
		cout << engine.ctrTer << endl;
		engine.ctrTer--;
		engine.valTer[id2find-1] = false;
		//cout << "id2find final: " << id2find << endl;
	}
	else if (engine.ctrTer == 0)
	{
		strcat(nameBuf,"You have found all of the Astro's crew's terminal logs.");
		txt = false;
	}
	
	if (txt)
	{
		if (myfile.is_open())
		{
			//while ( getline (myfile,line) )
			//{
			//cout << line << '\n';
				
			//}
			//char c = id2find;
			//cout << "id to find " << id2find << endl;
			for(int i = 0; i < id2find; i++)
			{
				//cout << "line before getline " << line << endl;
				getline(myfile, line,'@');
				//cout << "line after getline " << line << endl;
			}
			
			//cout << "line before last getline " << line << endl;
			getline(myfile, line,'@');
			//cout << "final display " << line << endl;
			//getline(myfile,line,'@');
			//int tens = (line[11]-48)*10;
			//int ones = line[12]-48;
			//int id = tens+ones;
			//cout << "line[10] = " << line[11] << endl;
			//cout << "line[11] = " << line[12] << endl;
			//cout << "ten's " << tens << endl;
			//cout << "one's " << ones << endl;
			//cout << "id found: " << id << endl;
			//cout << id2find << endl;
			//getline(myfile,line,'@');
			//getline(myfile,line,'@');
			//cout << "about to strcat!" << endl;
			strcat(nameBuf,line.c_str());
			cout << "strcat done" << endl;
			//cout << line << '\n';
			myfile.close();
		}
		//string s = line.c_str();
		//std::string str;
		//const char * message = line.c_str();
		//cout << nameBuf << " message text " << '\n';
		//cout << "setting it to scroll" << endl;
	}
	scrollOfRecords->ai = new TriggerAi(nameBuf);
	scrollOfRecords->blocks = false;
	cout << "done with record" << endl;
	return scrollOfRecords;
		//make interaction terminal
	//Actor *triggerTileI = new Actor(x1+1,y1+1, 227, "Intercom Terminal", TCODColor::white);
	 
	//"INTERACTION BASICS\n\n"
	//"Most items can be interacted with by simply moving into them by pressing the corresponding movement key whilst being adjacent to it. (Like these automated terminals)"
	//triggerTileI->blocks = false; 
	//engine.actors.push(triggerTileI);
}
Actor *Map::createFireBomb(int x, int y){
	Actor *scrollOfFireball = new Actor(x,y,182,"Firebomb",TCODColor::white);
	scrollOfFireball->sort = 2;
	scrollOfFireball->blocks = false;
	scrollOfFireball->pickable = new Fireball(3,12,8);
	scrollOfFireball->pickable->value = 45;
	scrollOfFireball->pickable->inkValue = 10;
	return scrollOfFireball;
}
Actor *Map::createTeleporter(int x, int y){
	Actor *scrollOfTeleport = new Actor(x,y,182,"Blink Stone",TCODColor::white);
	scrollOfTeleport->sort = 2;
	scrollOfTeleport->blocks = false;
	scrollOfTeleport->pickable = new Teleporter(20);
	scrollOfTeleport->pickable->value = 95;
	scrollOfTeleport->pickable->inkValue = 20;
	return scrollOfTeleport;
}
Actor *Map::createFrag(int x, int y){
	Actor *scrollOfFragging = new Actor(x,y,198,"Frag Grenade",TCODColor::white);
	scrollOfFragging->sort = 2;
	scrollOfFragging->blocks = false;
	scrollOfFragging->pickable = new Fragment(3,12,8);
	scrollOfFragging->pickable->value = 55;
	scrollOfFragging->pickable->inkValue = 10;
	return scrollOfFragging;
}
Actor *Map::createEMP(int x, int y){
	Actor *scrollOfLightningBolt = new Actor(x,y,183, "EMP Pulse",TCODColor::white);
	scrollOfLightningBolt->sort = 2;
	scrollOfLightningBolt->blocks = false;
	scrollOfLightningBolt->pickable = new LightningBolt(5,20);
	scrollOfLightningBolt->pickable->value = 60;
	scrollOfLightningBolt->pickable->inkValue = 10;
	return scrollOfLightningBolt;
}
Actor *Map::createTitanHelm(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *titanHelm = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult,
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,3);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-2);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,5);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Dented ");
						modBonus = new ItemBonus(ItemBonus::DR,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Rusty ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Corroded ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Quality ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,1);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::STRENGTH,4);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,7);
						break;
					case 2:
						strcat(nameBuf,"Double Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,2);
						requirement = new ItemReq(ItemReq::STRENGTH,7);
						break;
					case 3:
						strcat(nameBuf,"Mechanized ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,5);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,9);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,60);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"TitanHelm");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	titanHelm->blocks = false;
	titanHelm->name = nameBuf;
	bonus.push(modBonus);
	titanHelm->pickable = new Equipment(0,Equipment::HEAD,bonus,requirement);
	titanHelm->sort = 3;
	((Equipment*)(titanHelm->pickable))->armorArt = 5;
	titanHelm->pickable->value = 1000;
	titanHelm->pickable->inkValue = 50;
	titanHelm->col = col;
	return titanHelm;
}
Actor *Map::createTitanMail(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *titanMail = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult,
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,5);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-3);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,7);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Dented ");
						modBonus = new ItemBonus(ItemBonus::DR,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Rusty ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Corroded ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Quality ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,7);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,0);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,3);
						requirement = new ItemReq(ItemReq::STRENGTH,8);
						break;
					case 2:
						strcat(nameBuf,"Double Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,2);
						requirement = new ItemReq(ItemReq::STRENGTH,8);
						break;
					case 3:
						strcat(nameBuf,"Mechanized ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,4);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,8);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,100);
						requirement = new ItemReq(ItemReq::STRENGTH,7);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"TitanMail");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	titanMail->blocks = false;
	titanMail->name = nameBuf;
	bonus.push(modBonus);
	titanMail->pickable = new Equipment(0,Equipment::CHEST,bonus,requirement);
	titanMail->sort = 3;
	((Equipment*)(titanMail->pickable))->armorArt = 6;
	titanMail->pickable->value = 1000;
	titanMail->pickable->inkValue = 50;
	titanMail->col = col;
	return titanMail;
}
Actor *Map::createTitanBoots(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *titanBoots = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,3);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-1);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,4);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Dented ");
						modBonus = new ItemBonus(ItemBonus::DR,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Rusty ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Corroded ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-10);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Quality ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,0);
						requirement = new ItemReq(ItemReq::STRENGTH,3);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,3);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					case 2:
						strcat(nameBuf,"Double Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,3);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					case 3:
						strcat(nameBuf,"Mechanized ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,3);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,6);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,50);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"TitanBoots");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	titanBoots->blocks = false;
	titanBoots->name = nameBuf;
	bonus.push(modBonus);
	titanBoots->pickable = new Equipment(0,Equipment::FEET,bonus,requirement);
	titanBoots->sort = 3;
	((Equipment*)(titanBoots->pickable))->armorArt = 8;
	titanBoots->pickable->value = 400;
	titanBoots->pickable->inkValue = 35;
	titanBoots->col = col;
	return titanBoots;
}

Actor *Map::createTitanGreaves(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *titanGreaves = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,4);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-3);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,5);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Dented ");
						modBonus = new ItemBonus(ItemBonus::DR,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Rusty ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Corroded ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Quality ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::STRENGTH,4);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,4);
						requirement = new ItemReq(ItemReq::STRENGTH,7);
						break;
					case 2:
						strcat(nameBuf,"Double Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,2);
						requirement = new ItemReq(ItemReq::STRENGTH,7);
						break;
					case 3:
						strcat(nameBuf,"Mechanized ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,4);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,7);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,70);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"TitanGreaves");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	titanGreaves->blocks = false;
	titanGreaves->name = nameBuf;
	bonus.push(modBonus);
	titanGreaves->pickable = new Equipment(0,Equipment::LEGS,bonus,requirement);
	titanGreaves->sort = 3;
	((Equipment*)(titanGreaves->pickable))->armorArt = 7;
	titanGreaves->pickable->value = 600;
	titanGreaves->pickable->inkValue = 30;
	titanGreaves->col = col;
	return titanGreaves;
}
Actor *Map::createKevlarHelmet(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *kevlarHelm = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,1);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-1);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,3);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Thin ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Constricting ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Damaged ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Dense ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,1);
						requirement = new ItemReq(ItemReq::STRENGTH,4);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,1);
						requirement = new ItemReq(ItemReq::STRENGTH,4);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					case 2:
						strcat(nameBuf,"Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,2);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					case 3:
						strcat(nameBuf,"Scientific ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,4);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,7);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,40);
						requirement = new ItemReq(ItemReq::STRENGTH,4);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Kevlar Helmet");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	kevlarHelm->blocks = false;
	kevlarHelm->name = nameBuf;
	bonus.push(modBonus);
	kevlarHelm->pickable = new Equipment(0,Equipment::HEAD,bonus,requirement);
	kevlarHelm->sort = 3;
	((Equipment*)(kevlarHelm->pickable))->armorArt = 9;
	kevlarHelm->pickable->value = 400;
	kevlarHelm->pickable->inkValue = 20;
	kevlarHelm->col = col;
	return kevlarHelm;
}
Actor *Map::createKevlarVest(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *kevlarVest = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,3);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-2);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,4);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Thin ");
						modBonus = new ItemBonus(ItemBonus::DR,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Constricting ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Damaged ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Dense ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::STRENGTH,3);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,4);
						requirement = new ItemReq(ItemReq::STRENGTH,8);
						break;
					case 2:
						strcat(nameBuf,"Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,3);
						requirement = new ItemReq(ItemReq::STRENGTH,8);
						break;
					case 3:
						strcat(nameBuf,"Scientific ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,3);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,6);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,60);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Kevlar Vest");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	kevlarVest->blocks = false;
	kevlarVest->name = nameBuf;
	bonus.push(modBonus);
	kevlarVest->pickable = new Equipment(0,Equipment::CHEST,bonus,requirement);
	kevlarVest->sort = 3;
	((Equipment*)(kevlarVest->pickable))->armorArt = 10;
	kevlarVest->pickable->value = 700;
	kevlarVest->pickable->inkValue = 35;
	kevlarVest->col = col;
	return kevlarVest;
}
Actor *Map::createKevlarGreaves(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *kevlarGreaves = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,2);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-1);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,3);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Thin ");
						modBonus = new ItemBonus(ItemBonus::DR,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Constricting ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Damaged ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Dense ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::STRENGTH,3);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,4);
						requirement = new ItemReq(ItemReq::STRENGTH,8);
						break;
					case 2:
						strcat(nameBuf,"Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,3);
						requirement = new ItemReq(ItemReq::STRENGTH,8);
						break;
					case 3:
						strcat(nameBuf,"Scientific ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,3);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,6);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,60);
						requirement = new ItemReq(ItemReq::STRENGTH,6);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Kevlar KneePads");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	kevlarGreaves->blocks = false;
	kevlarGreaves->name = nameBuf;
	bonus.push(modBonus);
	kevlarGreaves->pickable = new Equipment(0,Equipment::LEGS,bonus,requirement);
	kevlarGreaves->sort = 3;
	((Equipment*)(kevlarGreaves->pickable))->armorArt = 11;
	kevlarGreaves->pickable->value = 500;
	kevlarGreaves->pickable->inkValue = 30;
	kevlarGreaves->col = col;
	return kevlarGreaves;
}
Actor *Map::createKevlarBoots(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *kevlarBoots = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *DRBonus = new ItemBonus(ItemBonus::DR,2);
	ItemBonus *dodgeBonus = new ItemBonus(ItemBonus::DODGE,-1);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(DRBonus);
	bonus.push(dodgeBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,3);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Thin ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Constricting ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Damaged ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-10);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Standard ");
						break;
					case 1:
						strcat(nameBuf,"Dense ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,1);
						requirement = new ItemReq(ItemReq::STRENGTH,4);
						break;
					case 2:
						strcat(nameBuf,"Economy ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::STRENGTH,2);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					case 2:
						strcat(nameBuf,"Plated ");
						modBonus = new ItemBonus(ItemBonus::DR,2);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					case 3:
						strcat(nameBuf,"Scientific ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,3);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,6);
						break;
					case 4:
						strcat(nameBuf,"Pristine ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,40);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Kevlar Boots");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	kevlarBoots->blocks = false;
	kevlarBoots->name = nameBuf;
	bonus.push(modBonus);
	kevlarBoots->pickable = new Equipment(0,Equipment::FEET,bonus,requirement);
	kevlarBoots->sort = 3;
	((Equipment*)(kevlarBoots->pickable))->armorArt = 12;
	kevlarBoots->pickable->value = 400;
	kevlarBoots->pickable->inkValue = 25;
	kevlarBoots->col = col;
	return kevlarBoots;
}
Actor *Map::createMylarGreaves(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *myGreaves = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *STDBonus = new ItemBonus(ItemBonus::DR,1);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(STDBonus);
	ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,3);
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Tattered ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-10);
						requirement = new ItemReq(ItemReq::DEXTERITY,1);
						break;
					case 2:
						strcat(nameBuf,"Worn ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,-1);
						requirement = new ItemReq(ItemReq::DEXTERITY,2);
						break;
					case 3:
						strcat(nameBuf,"Ruined ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Durable ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,1);
						requirement = new ItemReq(ItemReq::STRENGTH,3);
						break;
					case 1:
						strcat(nameBuf,"Useful ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,30);
						requirement = new ItemReq(ItemReq::DEXTERITY,3);
						break;
					case 2:
						strcat(nameBuf,"Cheap ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,2);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,4);
						break;
					case 2:
						strcat(nameBuf,"Tough ");
						modBonus = new ItemBonus(ItemBonus::DR,3);
						requirement = new ItemReq(ItemReq::DEXTERITY,5);
						break;
					case 3:
						strcat(nameBuf,"High Tech ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,2);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,4);
						break;
					case 4:
						strcat(nameBuf,"Reliable ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,40);
						requirement = new ItemReq(ItemReq::DEXTERITY,4);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Mylar Greaves");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	myGreaves->blocks = false;
	myGreaves->name = nameBuf;
	bonus.push(modBonus);
	myGreaves->pickable = new Equipment(0,Equipment::LEGS,bonus,requirement);
	myGreaves->sort = 3;
	((Equipment*)(myGreaves->pickable))->armorArt = 3;
	myGreaves->pickable->value = 200;
	myGreaves->pickable->inkValue = 30;
	myGreaves->col = col;
	return myGreaves;
}
Actor *Map::createMylarVest(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *myVest = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *STDBonus = new ItemBonus(ItemBonus::STRENGTH,2);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(STDBonus);
	ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,3);
	
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Tattered ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Worn ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-25);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Ruined ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,-2);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Durable ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,-1);
						requirement = new ItemReq(ItemReq::STRENGTH,3);
						break;
					case 1:
						strcat(nameBuf,"Useful ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,1);
						requirement = new ItemReq(ItemReq::DEXTERITY,3);
						break;
					case 2:
						strcat(nameBuf,"Cheap ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,1);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,3);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,3);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					case 2:
						strcat(nameBuf,"Tough ");
						modBonus = new ItemBonus(ItemBonus::DR,4);
						requirement = new ItemReq(ItemReq::DEXTERITY,5);
						break;
					case 3:
						strcat(nameBuf,"High Tech ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,3);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,5);
						break;
					case 4:
						strcat(nameBuf,"Reliable ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,50);
						requirement = new ItemReq(ItemReq::DEXTERITY,4);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Mylar Vest");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	myVest->blocks = false;
	myVest->name = nameBuf;
	bonus.push(modBonus);
	myVest->pickable = new Equipment(0,Equipment::CHEST,bonus,requirement);
	myVest->sort = 3;
	((Equipment*)(myVest->pickable))->armorArt = 2;
	myVest->pickable->value = 300;
	myVest->pickable->inkValue = 40;
	myVest->col = col;
	return myVest;
}
Actor *Map::createMylarCap(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *myCap = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *STDBonus = new ItemBonus(ItemBonus::INTELLIGENCE,1);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(STDBonus);
	ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,1);
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Tattered ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 2:
						strcat(nameBuf,"Worn ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-15);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					case 3:
						strcat(nameBuf,"Ruined ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Durable ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,3);
						break;
					case 1:
						strcat(nameBuf,"Useful ");
						modBonus = new ItemBonus(ItemBonus::DEXTERITY,1);
						requirement = new ItemReq(ItemReq::DEXTERITY,3);
						break;
					case 2:
						strcat(nameBuf,"Cheap ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,-1);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,3);
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2);
						requirement = new ItemReq(ItemReq::STRENGTH,5);
						break;
					case 2:
						strcat(nameBuf,"Tough ");
						modBonus = new ItemBonus(ItemBonus::DR,3);
						requirement = new ItemReq(ItemReq::DEXTERITY,4);
						break;
					case 3:
						strcat(nameBuf,"High Tech ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,3);
						requirement = new ItemReq(ItemReq::INTELLIGENCE,5);
						break;
					case 4:
						strcat(nameBuf,"Reliable ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,30);
						requirement = new ItemReq(ItemReq::DEXTERITY,3);
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Mylar Cap");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	myCap->blocks = false;
	myCap->name = nameBuf;
	//ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,1);
	bonus.push(modBonus);
	myCap->pickable = new Equipment(0,Equipment::HEAD,bonus,requirement);
	myCap->sort = 3;
	((Equipment*)(myCap->pickable))->armorArt = 1;
	myCap->pickable->value = 100;
	myCap->pickable->inkValue = 20;
	myCap->col = col;
	return myCap;
}
Actor *Map::createMylarBoots(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *myBoots = new Actor(x,y,185,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *STDBonus = new ItemBonus(ItemBonus::HEALTH,18 + (2*engine.level));
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(STDBonus);
	ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,2 + (engine.level - 1));
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Tattered ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20 + 8 + (2*engine.level));
						requirement = new ItemReq(ItemReq::DEXTERITY,1 + (engine.level - 1));
						break;
					case 2:
						strcat(nameBuf,"Worn ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-5);
						requirement = new ItemReq(ItemReq::DEXTERITY,1 + (engine.level - 1));
						break;
					case 3:
						strcat(nameBuf,"Destroyed ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-20);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Durable ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,5 + (5*engine.level));
						requirement = new ItemReq(ItemReq::DEXTERITY,3 + (engine.level - 1));
						break;
					case 1:
						strcat(nameBuf,"Useful ");
						modBonus = new ItemBonus(ItemBonus::DR,1 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::DEXTERITY,2 + (engine.level - 1));
						break;
					case 2:
						strcat(nameBuf,"Cheap ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,1 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::STRENGTH,2 + (engine.level - 1));
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Reinforced ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::STRENGTH,3 + (engine.level - 1));
						break;
					case 2:
						strcat(nameBuf,"Tough ");
						modBonus = new ItemBonus(ItemBonus::DR,2 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::DEXTERITY,4 + (engine.level - 1));
						break;
					case 3:
						strcat(nameBuf,"High Tech ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,2 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::INTELLIGENCE,4 + (engine.level - 1));
						break;
					case 4:
						strcat(nameBuf,"Reliable ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,10 + (10*engine.level));
						requirement = new ItemReq(ItemReq::DEXTERITY,3 + (engine.level - 1));
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Mylar Boots");

	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	myBoots->blocks = false;
	myBoots->name = nameBuf;
	bonus.push(modBonus);
	myBoots->pickable = new Equipment(0,Equipment::FEET,bonus,requirement);
	myBoots->sort = 3;
	((Equipment*)(myBoots->pickable))->armorArt = 4;
	myBoots->pickable->value = 80 + (20*engine.level);
	myBoots->pickable->inkValue = 15 + (5*engine.level);
	myBoots->col = col;
	return myBoots;
}
Actor *Map::createMLR(int x, int y, bool isVend){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	//Actor *MLR = new Actor(x,y,169,"Art",TCODColor::lighterGreen);
	Actor *MLR = new Actor(x,y,169,"Art",TCODColor::white);
	TCODColor col = TCODColor::white;
	//artifact->pickable = new Equipment(0);
	//Equipment::SlotType slot = Equipment::NOSLOT;
	//ItemBonus *bonus = NULL;
	//NOBONUS, HEALTH, DODGE, DR, STRENGTH, DEXTERITY, INTELLIGENCE
	//min damage, max damage, critMult, 
	TCODList<ItemBonus *> bonus;
	ItemBonus *STDBonus = new ItemBonus(ItemBonus::DEXTERITY,1);
	ItemBonus *ADDBonus = new ItemBonus(ItemBonus::DODGE,-1);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(STDBonus);
	bonus.push(ADDBonus);
	ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,4);
	//ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	int minDmg = 1;
	int maxDmg = 6;
	int critMult = 2;
	int critRange = 20;
	int powerUse = 1;
	//random 1-3, 1 is worse, 2 is average, 3 is good
	int choices = random->getInt(1,3);
	int names = random->getInt(1,5);
	int flaw = random->getInt(1,5);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,5);
	if(!isVend){
		switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Heavy ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,-1);
						break;
					case 2:
						strcat(nameBuf,"Overly Complex ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,-1);
						break;
					case 3:
						strcat(nameBuf,"Low Damage ");
						maxDmg -= 2;
						break;
					case 4:
						strcat(nameBuf,"Critically Flawed ");
						critMult = 1;
						critRange = 21;
						break;
					case 5:
						strcat(nameBuf,"Burning ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-5);
						break;
					default:break;
				}
				//bad MLR'S
				col = TCODColor::lighterRed;
				switch(names)
				{
					case 1:
						strcat(nameBuf,"Chinese MLR");
						break;
					case 2:
						strcat(nameBuf,"Cheap Plastic MLR");
						break;
					case 3:
						strcat(nameBuf,"Barely functional MLR");
						break;
					case 4:
						strcat(nameBuf,"Low Capacity MLR");
						break;
					case 5:
						strcat(nameBuf,"Training MLR");
						break;
					default:break;
				}
				
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Lower Damage  ");
						maxDmg -= 1;
						break;
					case 1:
						break;
					case 2:
						strcat(nameBuf,"Higher Damage ");
						maxDmg += 1;
						break;
					default:break;
				}
				//avergae MLR'S
				switch(names)
				{
					case 1:
						strcat(nameBuf,"MLR");
						break;
					case 2:
						strcat(nameBuf,"Military Issue MLR");
						break;
					case 3:
						strcat(nameBuf,"Standard MLR");
						break;
					case 4:
						strcat(nameBuf,"Jet Black MLR");
						break;
					case 5:
						strcat(nameBuf,"Trusty MLR");
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Light ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,1);
						break;
					case 2:
						strcat(nameBuf,"User Friendly ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,1);
						break;
					case 3:
						strcat(nameBuf,"High Power ");
						maxDmg += 3;
						minDmg += 1;
						break;
					case 4:
						strcat(nameBuf,"Critically Good ");
						critMult = 3;
						break;
					case 5:
						strcat(nameBuf,"Reliable ");
						minDmg += 4;
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				//good MLR's
				switch(names)
				{
					case 1:
						strcat(nameBuf,"Overclocked MLR");
						break;
					case 2:
						strcat(nameBuf,"Battle-Tested MLR");
						break;
					case 3:
						strcat(nameBuf,"High-Voltage MLR");
						break;
					case 4:
						strcat(nameBuf,"Spec-Ops MLR");
						break;
					case 5:
						strcat(nameBuf,"Swiss Made MLR");
						break;
					default:break;
				}
				break;
			default:break;
		}
	}else{
		strcat(nameBuf,"MLR");
	}
	
	//Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	MLR->blocks = false;
	MLR->name = nameBuf;
	//MLR->pickable = new Equipment(0,Equipment::RANGED,bonus,requirement);
	//1 = min damage, 6 = max damage, 2 is crit mult, RANGED, 0 = not equipped,RANGED, bonus, req
	bonus.push(modBonus);
	MLR->pickable = new Weapon(minDmg,maxDmg,critMult,critRange,powerUse,Weapon::RANGED,0,Equipment::RANGED,bonus,requirement);
	MLR->sort = 4;
	((Equipment*)(MLR->pickable))->armorArt = 13;
	MLR->pickable->value = 200;
	MLR->pickable->inkValue = 30;
	//col = TCODColor::white;
	MLR->col = col;
	return MLR;
}
Actor *Map::createFlameThrower(int x, int y, bool isVend){
	char* nameBuf = new char[80];
	memset(nameBuf,0,80);
	Actor *flamer = new Actor(x,y,169,"Art",TCODColor::white);
	flamer->blocks = false;
	TCODList<ItemBonus *> bonus;
	ItemBonus *STDBonus = new ItemBonus(ItemBonus::STRENGTH,3);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(STDBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,5);
	TCODRandom *random = TCODRandom::getInstance();
	TCODColor col = TCODColor::white;
	int range = 5;
	int powerUse = 2;
	int width = random->getInt(1,3);
	int choices = random->getInt(1,3);
	int flaw = random->getInt(1,3);
	int max = random->getInt(0,2);
	int gain = random->getInt(1,4);
	if(!isVend){
	switch(choices) 
		{
			case 1:
				//random flaws
				
				switch(flaw)
				{
					case 1:
						strcat(nameBuf,"Malfunctioning ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-(20 + 8 + (2*engine.level)));
						requirement = new ItemReq(ItemReq::STRENGTH,1 + (engine.level - 1));
						range = 3;
						powerUse = 2;
						break;
					case 2:
						strcat(nameBuf,"Dangerous ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,-(5+(5*engine.level)));
						requirement = new ItemReq(ItemReq::DEXTERITY,2 + (engine.level - 1));
						range = 4;
						powerUse = 3;
						break;
					case 3:
						strcat(nameBuf,"Inefficient ");
						modBonus = new ItemBonus(ItemBonus::DR,-1);
						requirement = new ItemReq(ItemReq::NOREQ,0);
						range = 5;
						powerUse = 5;
						break;
					default:break;
				}
				//bad Mylar Boots
				col = TCODColor::lighterRed;
				break;
			case 2:
				//random damage slightly
				
				//int min = random->getInt(1,2);
				switch(max)
				{
					case 0:
						strcat(nameBuf,"Improved ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,5 + (5*engine.level));
						requirement = new ItemReq(ItemReq::DEXTERITY,3 + (engine.level - 1));
						range = 6;
						powerUse = 2;
						break;
					case 1:
						strcat(nameBuf,"Efficient ");
						modBonus = new ItemBonus(ItemBonus::DR,1 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::DEXTERITY,2 + (engine.level - 1));
						range = 5;
						powerUse = 1;
						break;
					case 2:
						strcat(nameBuf,"Useful ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,1 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::STRENGTH,2 + (engine.level - 1));
						range = 6;
						powerUse = 3;
						break;
					default:break;
				}
				break;
			case 3:
				//random gains
				
				switch(gain)
				{
					case 1:
						strcat(nameBuf,"Long Range ");
						modBonus = new ItemBonus(ItemBonus::STRENGTH,2 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::STRENGTH,3 + (engine.level - 1));
						range = 8;
						powerUse = 3;
						break;
					case 2:
						strcat(nameBuf,"Shielded ");
						modBonus = new ItemBonus(ItemBonus::DR,2 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::DEXTERITY,4 + (engine.level - 1));
						range = 4;
						powerUse = 1;
						break;
					case 3:
						strcat(nameBuf,"Highly Efficient ");
						modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,2 + (engine.level - 1));
						requirement = new ItemReq(ItemReq::INTELLIGENCE,4 + (engine.level - 1));
						range = 7;
						powerUse = 1;
						break;
					case 4:
						strcat(nameBuf,"Reliable ");
						modBonus = new ItemBonus(ItemBonus::HEALTH,10 + (10*engine.level));
						requirement = new ItemReq(ItemReq::DEXTERITY,3 + (engine.level - 1));
						range = 6;
						powerUse = 2;
						break;
					default:break;
				}
				col = TCODColor::lighterGreen;
				break;
			default:break;
		}
	}
	strcat(nameBuf,"Flamethrower");
	flamer->name = nameBuf;
	bonus.push(modBonus);
	flamer->pickable = new Flamethrower(range,powerUse,width,false,Equipment::RANGED,bonus,requirement);
	flamer->col = col;
	flamer->sort = 4;
	((Equipment*)(flamer->pickable))->armorArt = 14;
	flamer->pickable->value = 400;
	flamer->pickable->inkValue = 25;
	return flamer;
	
	
}
Actor *Map::createCombatKnife(int x, int y){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	Actor *combatKnife = new Actor(x,y,169,"Combat Knife",TCODColor::white);
	combatKnife->blocks = false;
	TCODList<ItemBonus *> bonus;
	ItemBonus *STDBonus = new ItemBonus(ItemBonus::STRENGTH,2);
	ItemBonus *modBonus = new ItemBonus(ItemBonus::DR,0);
	bonus.push(STDBonus);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,3);
	TCODRandom *random = TCODRandom::getInstance();
	TCODColor col = TCODColor::white;
	Equipment::SlotType *slot = new Equipment::SlotType(Equipment::HAND1);
	Weapon::WeaponType *wpn = new Weapon::WeaponType(Weapon::LIGHT);
	int goodbad = random->getInt(1,3);
	int hand = random->getInt(1,3);
	int weight = random->getInt(1,3);
	int name = random->getInt(1,5);
	int flawName = random->getInt(1,5);
	int strBUF = 1;
	int reqBUF = 3;
	int minDmg = 1;
	int maxDmg = 4;
	int critMult = 2;
	int critRange = 20;
	int powerUse = 0;
	switch (goodbad)
	{
		case 1://bad
			strBUF -= 1;
			switch (flawName)
			{
				case 1:
					strcat(nameBuf,"Weak ");
					break;
				case 2:
					strcat(nameBuf,"Bent ");
					break;
				case 3:
					strcat(nameBuf,"Dull ");
					break;
				case 4:
					strcat(nameBuf,"Old ");
					break;
				case 5:
					strcat(nameBuf,"Rusty ");
					break;
				default:break;
			}
			col = TCODColor::lighterRed;
			break;
		case 2://average
			switch (flawName)
			{
				case 1:
					strcat(nameBuf,"Average ");
					break;
				case 2:
					strcat(nameBuf,"Normal ");
					break;
				case 3:
					strcat(nameBuf,"Fine ");
					break;
				case 4:
				case 5:
					break;
				default:break;
			}
			break;
		case 3://good
			strBUF += 1;
			switch (flawName)
			{
				case 1:
					strcat(nameBuf,"New ");
					break;
				case 2:
					strcat(nameBuf,"Strong ");
					break;
				case 3:
					strcat(nameBuf,"Gleaming ");
					break;
				case 4:
					strcat(nameBuf,"Sharp ");
					break;
				case 5:
					strcat(nameBuf,"Perfect ");
					break;
				default:break;
			}
			col = TCODColor::lighterGreen;
			break;
		default:break;
	}
	switch(weight)
	{
		case 1://light
			reqBUF -= 1;
			minDmg += 1;//faster, so more reliable
			break;
		case 2://average
			break;
		case 3://heavy
			reqBUF += 1;
			maxDmg += 1;//heavier, so more max
			break;
		default:break;
		
	}
	switch (hand)
	{
		case 1://right hand
			minDmg += 1;//faster, so more reliable
			modBonus = new ItemBonus(ItemBonus::STRENGTH,strBUF);//2 is "average"
			requirement = new ItemReq(ItemReq::STRENGTH,reqBUF);//3 is "average"
			slot = new Equipment::SlotType(Equipment::HAND1);
			wpn = new Weapon::WeaponType(Weapon::LIGHT);
			switch (name)
			{
				case 1:
					strcat(nameBuf,"Knife(H1)");
					break;
				case 2:
					strcat(nameBuf,"Dagger(H1)");
					break;
				case 3:
					strcat(nameBuf,"Shank(H1)");
					break;
				case 4:
					strcat(nameBuf,"Pipe(H1)");
					break;
				case 5:
					strcat(nameBuf,"Crowbar(H1)");
					break;
				default:break;
			}
			
			break;
		case 2://offhand
			minDmg -= 5;//offhand OP
			strBUF-=5;
			modBonus = new ItemBonus(ItemBonus::STRENGTH,strBUF);//2 is "average"
			requirement = new ItemReq(ItemReq::STRENGTH,reqBUF);//3 is "average"
			slot = new Equipment::SlotType(Equipment::HAND2);
			wpn = new Weapon::WeaponType(Weapon::LIGHT);
			switch (name)
			{
				case 1:
					strcat(nameBuf,"Wrench(H2)");
					break;
				case 2:
					strcat(nameBuf,"Knife(H2)");
					break;
				case 3:
					strcat(nameBuf,"Saw(H2)");
					break;
				case 4:
					strcat(nameBuf,"Machete(H2)");
					break;
				case 5:
					strcat(nameBuf,"Ka-Bar(H2)");
					break;
				default:break;
			}
			break;
		case 3://TWO HANDED
			maxDmg += 1;//heavier, so more max
			strBUF+=4;
			modBonus = new ItemBonus(ItemBonus::STRENGTH,strBUF);//5 is "average" (1 more than 2 average 1 handers)
			requirement = new ItemReq(ItemReq::STRENGTH,reqBUF+4);//7 is "average"
			slot = new Equipment::SlotType(Equipment::HAND1);
			wpn = new Weapon::WeaponType(Weapon::HEAVY);
			switch (name)
			{
				case 1:
					strcat(nameBuf,"Sword(2 HANDS)");
					break;
				case 2:
					strcat(nameBuf,"Fire-Axe(2 HANDS)");
					break;
				case 3:
					strcat(nameBuf,"Large Pipe(2 HANDS)");
					break;
				case 4:
					strcat(nameBuf,"Auto-Saw(2 HANDS)");
					break;
				case 5:
					strcat(nameBuf,"Makeshift Morningstar(2 HANDS)");
					break;
				default:break;
			}
			break;
		default:break;
	}
	char* knifeNamePrefix = new char[50];
	memset(knifeNamePrefix,0,50);
	if(strBUF >= 0){
		strcat(knifeNamePrefix, "+");
	}
	strcat(knifeNamePrefix, itoa(strBUF, new char[1],10));
	strcat(knifeNamePrefix, "S ");
	strcat(knifeNamePrefix, nameBuf);
	combatKnife->name = knifeNamePrefix;
	//combatKnife->pickable = new Equipment(0,Equipment::HAND1,bonus,requirement);
	bonus.push(modBonus);
	combatKnife->pickable = new Weapon(minDmg,maxDmg,critMult,critRange,powerUse,*wpn,0,*slot,bonus,requirement);
	combatKnife->pickable->value = 100;
	combatKnife->pickable->inkValue = 10;
	combatKnife->sort = 4;
	combatKnife->col = col;
	return combatKnife;
}
Actor *Map::createBatteryPack(int x,int y){
	Actor *batteryPack = new Actor(x,y,186,"Battery Pack", TCODColor::white);
	batteryPack->sort = 1;
	batteryPack->blocks = false;
	batteryPack->pickable = new Charger(5);
	batteryPack->pickable->value = 50;
	batteryPack->pickable->inkValue = 10;
	return batteryPack;
}

Actor* Map::createKey(int x, int y, int keyType)
{
	Actor *key;
	if(keyType == 0)
		key = new Actor(x,y, 190, "Cardkey", TCODColor::white);
	else
		key = new Actor(x,y, 190, "Key", TCODColor::white);
	key->sort = 1;
	key->blocks = false;
	key->pickable = new Key(keyType);
	key->pickable->value = 0;
	key->pickable->inkValue = 0;
	return key;
}

Actor *Map::createFood(int x, int y){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODColor col = TCODColor::white;
	TCODColor sat = TCODColor::white;
	Actor *scrollOfFeeding = new Actor(x,y,14,"Brick of Foodstuffs", TCODColor::green);
	strcat(nameBuf,"Brick of ");
	TCODRandom *random = TCODRandom::getInstance();
	int quality = random->getInt(1,5);
	switch(quality)
	{
		case 1:
			strcat(nameBuf,"rotten ");
			scrollOfFeeding->hunger = 30;
			sat = TCODColor::darkGrey;
			break;
		case 2:
			strcat(nameBuf,"stale ");
			scrollOfFeeding->hunger = 40;
			sat = TCODColor::grey;
			break;
		case 3:
			scrollOfFeeding->hunger = 50;
			sat = TCODColor::lightGrey;
			break;
		case 4:
			strcat(nameBuf,"new ");
			scrollOfFeeding->hunger = 60;
			sat = TCODColor::lighterGrey;
			break;
		case 5:
			strcat(nameBuf,"quality ");
			scrollOfFeeding->hunger = 70;
			sat = TCODColor::lightestGrey;
			break;
		default:break;
	}
	int color = random->getInt(1,10,3);
	switch(color)
	{
		case 1:
			col = TCODColor::green;//
			//strcat(nameBuf,"green ");
			break;
		case 2:
			col = TCODColor::sea;//
			//strcat(nameBuf,"turquoise ");
			break;
		case 3:
			col = TCODColor::lime;//
			//strcat(nameBuf,"lime ");
			break;
		case 4:
			col = TCODColor::azure;
			//strcat(nameBuf,"blue ");
			break;
		case 5:
			col = TCODColor::red;
			//strcat(nameBuf,"red ");
			break;
		case 6:
			col = TCODColor::yellow;
			//strcat(nameBuf,"yellow ");
			break;
		case 7:
			col = TCODColor::violet;
			//strcat(nameBuf,"purple ");
			break;
		case 8:
			col = TCODColor::magenta;
			//strcat(nameBuf,"pink ");
			break;
		case 9:
			col = TCODColor::chartreuse;//
			//strcat(nameBuf,"chartreuse ");
			break;
		case 10:
			col = TCODColor::sepia;
			//strcat(nameBuf,"brown ");
			break;
		default:break;
	}
	//col = TCODColor::green;
	col = col*sat;
	//col.r = col.r * sat.r / 255
	//col.g = col.g * sat.g / 255
	//col.b = col.b * sat.b / 255
	strcat(nameBuf,"Foodstuffs");
	scrollOfFeeding->sort = 1;
	scrollOfFeeding->name = nameBuf;
	scrollOfFeeding->blocks = false;
	scrollOfFeeding->pickable = new Food(1);//this is the stack size. Food should feed for a static amount
	scrollOfFeeding->pickable->value = 25;
	//scrollOfFeeding->hunger = 60;
	scrollOfFeeding->pickable->inkValue = 10;
	scrollOfFeeding->col = col;
	engine.actors.push(scrollOfFeeding);
	return scrollOfFeeding;
}

Actor *Map::createArtifact(int x, int y){
	char* nameBuf = new char[80]; 
	memset(nameBuf,0,80);
	TCODRandom *random = TCODRandom::getInstance();
	Actor *artifact = new Actor(x,y,153,"Art",TCODColor::gold);
	artifact->pickable = new Equipment(0);
	Equipment::SlotType slot = Equipment::NOSLOT;
	ItemBonus *modBonus = NULL;
	ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	
	int choices = random->getInt(1,13);
	switch(choices) {
		//Garrett's Fantasy
		case 1: strcat(nameBuf,"Rodgort's "); break;
		case 2: strcat(nameBuf,"Adanimus' "); break;
		case 3: strcat(nameBuf,"Diogenes' "); break;
		case 4: strcat(nameBuf,"Umber's "); break;
		//shane's custom
		case 5: strcat(nameBuf,"Computerized "); break;
		case 6: strcat(nameBuf,"Armored "); break;
		case 7: strcat(nameBuf,"Battle-Tested "); break;
		case 8: strcat(nameBuf,"Modern "); break;
		case 9: strcat(nameBuf,"Cutting Edge "); break;
		case 10: strcat(nameBuf,"Lucky "); break;
		case 11: strcat(nameBuf,"Experimental "); break;
		case 12: strcat(nameBuf,"Grizzled "); break;
		case 13: strcat(nameBuf,"Augmented "); break;
		default: break;
	}
	choices = random->getInt(1,6);
	switch(choices) {
		case 1: strcat(nameBuf, "Helmet ");
				slot = Equipment::HEAD;
				artifact->sort = 3;
				break;
		case 2: strcat(nameBuf, "Chestplate "); 
				slot = Equipment::CHEST;
				artifact->sort = 3;
		case 3: strcat(nameBuf, "Greaves "); 
				slot = Equipment::LEGS;
				artifact->sort = 3;
				break;
		case 4: strcat(nameBuf, "Boots "); 
				slot = Equipment::FEET;
				artifact->sort = 3;
				break;
		case 5: strcat(nameBuf, "Dagger "); 
				slot = Equipment::HAND1;
				artifact->sort = 4;
				break;
		case 6: strcat(nameBuf, "Laser Pointer "); 
				slot = Equipment::RANGED;
				artifact->sort = 4;
				break;
		default: break;
	}
	choices = random->getInt(1,6);
	switch(choices) {
		case 1: strcat(nameBuf, "of Adamantium"); 
				modBonus = new ItemBonus(ItemBonus::HEALTH,10+(2*engine.level));
				break;
		case 2: strcat(nameBuf, "of Swiftness"); 
				modBonus = new ItemBonus(ItemBonus::DODGE,3+(2*engine.level));
				break;
		case 3: strcat(nameBuf, "of Defense"); 
				modBonus = new ItemBonus(ItemBonus::DR,3+(2*engine.level));
				break;
		case 4: strcat(nameBuf, "of the Stalwart Fighter");
				modBonus = new ItemBonus(ItemBonus::STRENGTH,3+(2*engine.level));
				break;
		case 5: strcat(nameBuf, "of the Bounding Lynx"); 
				modBonus = new ItemBonus(ItemBonus::DEXTERITY,3+(2*engine.level));
				break;
		case 6: strcat(nameBuf, "of Vast Intellect");
				modBonus = new ItemBonus(ItemBonus::INTELLIGENCE,3+(2*engine.level));
				break;
		default: break;
	}
	TCODList<ItemBonus *> bonus = new TCODList<ItemBonus *>();
	bonus.push(modBonus);
	((Equipment*)(artifact->pickable))->slot = slot;
	((Equipment*)(artifact->pickable))->bonus = bonus;
	((Equipment*)(artifact->pickable))->requirement = req;
	artifact->name = nameBuf;
	artifact->blocks = false;
	engine.actors.push(artifact);
	engine.sendToBack(artifact);
	return artifact;
}

Actor *Map::createCompanion(int x, int y, bool racial){
	Actor *pet = new Actor(x,y,141,"Mr. Bubble-Yum",TCODColor::white);
	pet->hostile = false;
	pet->destructible = new MonsterDestructible(100,0,0,10);
	pet->blocks = false;
	pet->container = new Container(2);
	pet->flashable = true;
	pet->tameable = true;
	if (racial){
		pet->ai = new CompanionAi(engine.player,2,CompanionAi::FOLLOW);
	} else {
		pet->ai = new CompanionAi(NULL,2,CompanionAi::FOLLOW);
	}
	
	TCODRandom *tutu = TCODRandom::getInstance();
	
	char* nameBuf = new char[250]; 
	memset(nameBuf,0,250);
	
	int switc = tutu->getInt(1,10);
	int switcher = tutu->getInt(1,3);
	if (racial)
		switcher = 4;
	switch (switcher){
		case 1://toaster
			switch (switc){
				case 1: strcat(nameBuf,"Shane "); break;
				case 2: strcat(nameBuf,"Wesley "); break;
				case 3: strcat(nameBuf,"Aaron "); break;
				case 4: strcat(nameBuf,"Ryan "); break;
				case 5: strcat(nameBuf,"Garrett "); break;
				case 6: strcat(nameBuf,"Meghan "); break;
				case 7: strcat(nameBuf,"Mitchell "); break;
				case 8: strcat(nameBuf,"Diana "); break;
				case 9: strcat(nameBuf,"Leona "); break;
				case 10: strcat(nameBuf,"Lee "); break;
			}
		case 2://marine (human names)
			switch (switc){
				case 1: strcat(nameBuf,"Jim "); break;
				case 2: strcat(nameBuf,"Johnson "); break;
				case 3: strcat(nameBuf,"Kevin "); break;
				case 4: strcat(nameBuf,"Smith "); break;
				case 5: strcat(nameBuf,"Drake "); break;
				case 6: strcat(nameBuf,"Sam "); break;
				case 7: strcat(nameBuf,"Frank "); break;
				case 8: strcat(nameBuf,"Dan "); break;
				case 9: strcat(nameBuf,"Tom "); break;
				case 10: strcat(nameBuf,"Bruce "); break;
			}
			break;
		case 3://foodstuffs (alien-ish names)
			switch (switc){
				case 1: strcat(nameBuf,"Flynnis "); break;
				case 2: strcat(nameBuf,"Jergenous "); break;
				case 3: strcat(nameBuf,"Jinuss "); break;
				case 4: strcat(nameBuf,"Scrous "); break;
				case 5: strcat(nameBuf,"Druffe "); break;
				case 6: strcat(nameBuf,"Scazuluk "); break;
				case 7: strcat(nameBuf,"Frum "); break;
				case 8: strcat(nameBuf,"Quzzleq "); break;
				case 9: strcat(nameBuf,"Jifflo "); break;
				case 10: strcat(nameBuf,"Lifforq "); break;
			}
			break;
		case 4://generic/any & racial
			switch (switc){
				case 1: strcat(nameBuf,"Mykyl "); break;
				case 2: strcat(nameBuf,"Bob "); break;
				case 3: strcat(nameBuf,"Frigth "); break;
				case 4: strcat(nameBuf,"Lydia "); break;
				case 5: strcat(nameBuf,"Dweezil "); break;
				case 6: strcat(nameBuf,"Springleaf "); break;
				case 7: strcat(nameBuf,"EX-279 "); break;
				case 8: strcat(nameBuf,"Sally "); break;
				case 9: strcat(nameBuf,"TOM "); break;
				case 10: strcat(nameBuf,"Bryndynn "); break;
			}
			break;
	}
	
	switc = tutu->getInt(1,120);
	
	pet->str += 3;
	pet->totalStr +=3;
	//add new ones
	if (switc < 40){
		((CompanionAi*)(pet->ai))->att = CompanionAi::STANDARD;
		((CompanionAi*)(pet->ai))->period = 10 * tutu->getInt(2,6);
		strcat(nameBuf,"the ");
		pet->destructible->baseDR += 3;
		pet->destructible->totalDR += 3;
		
	} 
	else if (switc < 40+20) {
		((CompanionAi*)(pet->ai))->att = CompanionAi::BRUTISH;
		((CompanionAi*)(pet->ai))->period = 10 * tutu->getInt(2,6);
		strcat(nameBuf,"the Brutish ");
		pet->str += 2;
		pet->totalStr +=2;
	}
	else if (switc < 40+20+20) {
		((CompanionAi*)(pet->ai))->att = CompanionAi::SPASTIC;
		((CompanionAi*)(pet->ai))->period = 10 * tutu->getInt(2,6);
		strcat(nameBuf,"the Spastic ");
		pet->destructible->baseDodge += 5;
		pet->destructible->totalDodge += 5;
	}
	else if (switc <= 40+20+20+20){
		((CompanionAi*)(pet->ai))->att = CompanionAi::DEPRESSED;
		((CompanionAi*)(pet->ai))->period = 10 * tutu->getInt(2,6);
		strcat(nameBuf,"the Melancholic ");
		pet->destructible->hp += 30;
		pet->destructible->maxHp += 30;
	}
	else if (switc <= 40+20+20+20+20){
		((CompanionAi*)(pet->ai))->att = CompanionAi::SUICIDAL;
		((CompanionAi*)(pet->ai))->period = 10 * tutu->getInt(2,6);
		strcat(nameBuf,"the Suicidal ");
		pet->destructible->hp += 60;
		pet->destructible->maxHp += 60;
	}
	
	
	if (racial){	
		switch(engine.player->race[0]){
			 
			case 'A':		//Alien
			strcat(nameBuf,"Capybara");
			pet->name = nameBuf;
			pet->ch = 173;
			pet->destructible->maxHp +=50;
			pet->destructible->hp +=50;
			pet->totalStr += 7;
			pet->attacker = new Attacker(pet->totalStr);
			
			if ( ((CompanionAi*)(pet->ai))->att == CompanionAi::STANDARD) {
				((CompanionAi*)(pet->ai))->att = CompanionAi::CAPYBARA;
			}
			break;

			case 'R':		//Robot
			strcat(nameBuf,"Scout Drone");
			pet->name = nameBuf;
			pet->ch = 157;
			pet->destructible->maxHp += 300;
			pet->destructible->hp += 300;
			pet->totalStr += 0;
			pet->attacker = new Attacker(pet->totalStr);
			
			if ( ((CompanionAi*)(pet->ai))->att == CompanionAi::STANDARD) {
				((CompanionAi*)(pet->ai))->att = CompanionAi::DRONE;
			}
			break;
			
			default:		//Human
			strcat(nameBuf,"Donutling");
			pet->name = nameBuf;
			pet->totalStr += -1;
			pet->attacker = new Attacker(pet->totalStr);
			((CompanionAi*)pet->ai)->edible = true;
			((CompanionAi*)(pet->ai))->att = CompanionAi::EDIBLE;
			((CompanionAi*)(pet->ai))->period = 20;
			break;
		}
	} else{
		
		switch(switcher){
			 
			case 1:		//Toaster
			pet->name = "Shane the useless Toaster";
			pet->ch = 158;
			pet->destructible->maxHp += 70;
			pet->destructible->hp += 70;
			pet->totalStr += 0;
			pet->attacker = new Attacker(pet->totalStr);
			((CompanionAi*)(pet->ai))->att = CompanionAi::TOASTER;
			
			break;

			case 2:		//Marine Mook
			strcat(nameBuf,"Marine");
			pet->name = nameBuf;
			pet->ch = 142;
			pet->destructible->maxHp += 50;
			pet->destructible->hp += 50;
			pet->totalStr += 0;
			pet->attacker = new Attacker(pet->totalStr);
			
			break;
			
			case 3:		//Foodstuff Conglomeration
			strcat(nameBuf,"Awakened Foodstuffs");
			pet->name = nameBuf;
			pet->ch = 174;
			pet->destructible->maxHp += 250;
			pet->destructible->hp += 250;
			pet->totalStr += 0;
			((CompanionAi*)pet->ai)->edible = true;
			pet->attacker = new Attacker(pet->totalStr);
			
			break;
			
			default:   //just in case
			strcat(nameBuf,"Something");
			pet->name = nameBuf;
			pet->totalStr = -1;
			pet->attacker = new Attacker(-1);
			
			((CompanionAi*)pet->ai)->edible = true;
			break;
		}
	}
	return pet;
}

bool Map::isVisible(int x, int y){
	return (engine.mapcon->getCharForeground(x,y) == TCODColor::white) && (engine.map->isExplored(x,y));
}

