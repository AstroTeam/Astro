#include "main.hpp"
#include <iostream>
using namespace Param;
using namespace std;

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;


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

			//will this room be special?
			int index = map.rng->getInt(0, 10);
			if (index < roomList->size()) {
				room->type = roomList->get(index);
				cout << "OFFICE MADE" << endl;
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

Map::Map(int width, int height, short epicenterAmount): width(width),height(height),epicenterAmount(epicenterAmount) {
	seed = TCODRandom::getInstance()->getInt(0,0x7FFFFFFF);
}

Map::~Map() {
	delete [] tiles;
	delete map;
}

int Map::tileType(int x, int y) {
	int i = x+y*width;
	if (tiles[i].tileType == Param::OFFICE)
	{return 2;}
	else
	{return 1;}
	//return tiles[x*y].tileType;
}

void Map::init(bool withActors, LevelType levelType) {
	cout << levelType << endl << endl;

	rng = new TCODRandom(seed,TCOD_RNG_CMWC);
	tiles = new Tile[width*height];
	map = new TCODMap(width, height);
	TCODBsp bsp(0,0,width,height);
	bsp.splitRecursive(rng,8,ROOM_MAX_SIZE,ROOM_MAX_SIZE,1.5f, 1.5f);
	BspListener listener(*this);
	listener.bspActors = withActors;
	listener.roomList = getRoomTypes(levelType);
	bsp.traverseInvertedLevelOrder(&listener, (void *)withActors);
	
}

void Map::save(TCODZip &zip) {
	zip.putInt(seed);
	for (int i = 0; i < width*height; i++) {
		zip.putInt(tiles[i].explored);
		zip.putFloat(tiles[i].infection);
	}
}

void Map::load(TCODZip &zip) {
	seed = zip.getInt();
	init(false);
	for (int i = 0; i <width*height; i++) {
		tiles[i].explored = zip.getInt();
		tiles[i].infection = zip.getFloat();
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
			
			if (a != NULL)
			{
				cout << a->name << endl;
				if (strcmp(a->name,"a filing cabinet") == 0)
				{
					//engine.actors.remove(a);
					//CHANGE THE SPRITE TO BROKEN CABINET
					//CHANGE THE NAME TO BROKEN CABINET
					
					//engine.gui->message(TCODColor::red, "playery  %d",plyy);
					//cout << "breaking cabinet";
					a->blocks = false;
					a->ch = 241;
					a->name = "a destroyed filing cabinet";
					
					int n = rng->getInt(5,8);
					int x = a->x;
					int y = a->y;
					int add = rng->getInt(0,10);
					for (int xxx = -1; xxx <= 1; xxx++)/////////////////////9x9 for loop to add papers
					{
						for (int yyy = -1; yyy <= 1; yyy++)
						{
							if (add > 3 )
							{
								engine.mapconDec->setChar(x+xxx, y+yyy, n);
							}
							n = rng->getInt(5,8);
							add = rng->getInt(0,10);
						}
					}
					
					//engine.mapconDec->setChar(x+1, y, n);
					//n = rng->getInt(5,8);
					//engine.mapconDec->setChar(x-1, y, n);
					//n = rng->getInt(5,8);
					//engine.mapconDec->setChar(x, y+1, n);
					//n = rng->getInt(5,8);
					//engine.mapconDec->setChar(x, y-1, n);
					//delete a;
				}
			}
			//delete a;
		}
	}
}

void Map::addMonster(int x, int y) {
	TCODRandom *rng =TCODRandom::getInstance();
	
	int level = engine.level; //Note first engine.level = 1
	
	//Infected Crew Member Base Stats
	float infectedCrewMemMaxHp = 10;
	float infectedCrewMemDef = 0;
	float infectedCrewMemAtk = 5;
	float infectedCrewMemXp = 10;
	float infectedCrewMemChance = 80;
	int infectedCrewMemAscii = 164;
	
	float infectedMarineMaxHp = 10;
	float infectedMarineDef = 0;
	float infectedMarineAtk = 5;
	float infectedMarineXp = 10;
//	float infectedMarineChance = 80;
	int infectedMarineAscii = 149;
	
	//Infected NCO Base Stats
	float infectedNCOMaxHp = 12;
	float infectedNCODef = 1;
	float infectedNCOAtk = 6;
	float infectedNCOXp = 10;
	float infectedNCOChance = 10;
	int infectedNCOAscii = 148;
	
	//Infected Officer Base Stats
	float infectedOfficerMaxHp = 15;
	float infectedOfficerDef = 1;
	float infectedOfficerAtk = 7;
	float infectedOfficerXp = 20;
	float infectedOfficerChance = 6;
	int infectedOfficerAscii = 132;
	
	//Spore Creature Base Stats
	float sporeCreatureMaxHp = 17;
	float sporeCreatureDef = 1;
	float sporeCreatureAtk = 10;
	float sporeCreatureXp = 25;
	float sporeCreatureChance = 4;
	int sporeCreatureAscii = 165;

	
	
	
	if(infectedCrewMemChance - 10*(level-1) <= 20) //lowerbound for infectedCrewMemChance = 20
	{
		infectedCrewMemChance = 20;
		infectedNCOChance = infectedNCOChance + 30;
		infectedOfficerChance = infectedOfficerChance + 18;
		sporeCreatureChance = sporeCreatureChance + 12;
		
	}
	else
	{
		infectedCrewMemChance -= 10*(level-1); //decrement infectedCrewMemChance by 10% each level
		infectedNCOChance += 5*(level-1); //increment infectedNCOMemChance by 5% each level
		infectedOfficerChance += 3*(level-1); //increment infectedOfficerMemChance by 3% each level
		sporeCreatureChance += 2*(level-1); //increment sporeCreatureChance by 2% each level
	}
	
	int dice = rng->getInt(0,100);
	if (dice < infectedCrewMemChance) 
	{//10% of infectedCrewMembers are infectedMarines
		if(dice <= (infectedCrewMemChance*9)/10) 
		{
			Actor *infectedCrewMember = new Actor(x,y,infectedCrewMemAscii,"Infected Crewmember",TCODColor::white);
			infectedCrewMember->destructible = new MonsterDestructible(infectedCrewMemMaxHp,infectedCrewMemDef,"infected corpse",infectedCrewMemXp);
			infectedCrewMember->attacker = new Attacker(infectedCrewMemAtk);
			infectedCrewMember->container = new Container(2);
			infectedCrewMember->ai = new MonsterAi();
			generateRandom(infectedCrewMember, infectedCrewMemAscii);
			engine.actors.push(infectedCrewMember);
		}
		else 
		{
			Actor *infectedMarine = new Actor(x,y,infectedMarineAscii,"Infected Marine",TCODColor::white);
			infectedMarine->destructible = new MonsterDestructible(infectedMarineMaxHp,infectedMarineDef,"infected corpse",infectedMarineXp);
			infectedMarine->attacker = new Attacker(infectedMarineAtk);
			infectedMarine->container = new Container(2);
			infectedMarine->ai = new RangedAi();
			generateRandom(infectedMarine, infectedMarineAscii);
			engine.actors.push(infectedMarine);
		}
		
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance)	
	{
		//create an infected NCO
		Actor *infectedNCO = new Actor(x,y,infectedNCOAscii,"Infected NCO",TCODColor::white);
		infectedNCO->destructible = new MonsterDestructible(infectedNCOMaxHp,infectedNCODef,"infected corpse",infectedNCOXp);
		infectedNCO->attacker = new Attacker(infectedNCOAtk);
		infectedNCO->container = new Container(2);
		infectedNCO->ai = new MonsterAi();
		generateRandom(infectedNCO, infectedNCOAscii);
		engine.actors.push(infectedNCO);
	
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance)
	{
		//create an infected officer
		Actor *infectedOfficer = new Actor(x,y,infectedOfficerAscii,"Infected Officer",TCODColor::white);
		infectedOfficer->destructible = new MonsterDestructible(infectedOfficerMaxHp,infectedOfficerDef,"infected corpse",infectedOfficerXp);
		infectedOfficer->attacker = new Attacker(infectedOfficerAtk);
		infectedOfficer->container = new Container(2);
		infectedOfficer->ai = new MonsterAi();
		generateRandom(infectedOfficer, infectedOfficerAscii);
		engine.actors.push(infectedOfficer);
	}
	else if(dice < infectedCrewMemChance + infectedNCOChance + infectedOfficerChance + sporeCreatureChance)
	{
		//create a spore creature
		Actor *sporeCreature = new Actor(x,y,sporeCreatureAscii,"Spore Creature",TCODColor::white);
		sporeCreature->destructible = new MonsterDestructible(sporeCreatureMaxHp,sporeCreatureDef,"gross spore remains",sporeCreatureXp);
		sporeCreature->attacker = new Attacker(sporeCreatureAtk);
		sporeCreature->container = new Container(2);
		sporeCreature->ai = new MonsterAi();
		sporeCreature->oozing = true;
		generateRandom(sporeCreature, sporeCreatureAscii);
		engine.actors.push(sporeCreature);
	}
}

void Map::addItem(int x, int y, RoomType roomType) {

	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0,235);
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
		Actor *myBoots = createMylarBoots(x,y);
		engine.actors.push(myBoots);
		engine.sendToBack(myBoots);
	} else if(dice < 40+40+40+15+15) {
		//create a Modular Laser Rifle (MLR)
		Actor *MLR = createMLR(x,y);
		engine.actors.push(MLR);
		engine.sendToBack(MLR);
	}else if(dice < 40+40+40+15+15+5){
		//create Titanium Micro Chain-mail
		Actor *chainMail = createTitanMail(x,y);
		engine.actors.push(chainMail);
		engine.sendToBack(chainMail);
	}else if(dice < 40+40+40+15+15+5+40){
		//create a battery pack
		Actor *batteryPack = createBatteryPack(x,y);
		engine.actors.push(batteryPack);
		engine.sendToBack(batteryPack);
	}else {
		//create a scroll of confusion
		Actor *scrollOfConfusion = createFlashBang(x,y);
		engine.actors.push(scrollOfConfusion);
		engine.sendToBack(scrollOfConfusion);
	}
}

TCODList<RoomType> * Map::getRoomTypes(LevelType levelType) {
	TCODList<RoomType> * roomList = new TCODList<RoomType>();
		switch (levelType) {
			case GENERIC:
				//small amount of office rooms
				for (int i = 0; i <= rng->getInt(1,5); i++) {
					roomList->push(OFFICE);
				}	
				break;
			case OFFICE_FLOOR:
				for (int i = 0; i <= rng->getInt(3,9); i++) {
					roomList->push(OFFICE);
				}
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
	if (roomNum == 0) {
		//put the player in the first room
		engine.player->x = (x1+x2)/2;
		engine.player->y = (y1+y2)/2;
	}
	TCODRandom *rng = TCODRandom::getInstance();
	//add monsters
	//horde chance
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
			addMonster(x,y);
			nbMonsters--;
		}
	}
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

	//custom room feature
	if (room->type == OFFICE) {
		for (int tilex = x1; tilex <=x2; tilex++) {
			for (int tiley = y1; tiley <= y2; tiley++) {
				tiles[tilex+tiley*width].tileType = OFFICE;
			}
		}
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
					Actor * cabinet = new Actor(filingCabX,filingCabY,240,"a filing cabinet", TCODColor::white);
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
					Actor * cabinet = new Actor(filingCabX,filingCabY,240,"a filing cabinet", TCODColor::white);
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
					Actor * cabinet = new Actor(filingCabX,filingCabY,240,"a filing cabinet", TCODColor::white);
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
					Actor * cabinet = new Actor(filingCabX,filingCabY,240,"a filing cabinet", TCODColor::white);
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
					Actor * desk = new Actor(xX,yY,242,"a desk", TCODColor::white);
					int n = rng->getInt(1,4);
					engine.mapconDec->setChar(xX, yY, n);
					engine.actors.push(desk);
				}
				//add papers
				//replace items with paper description items
				//or make some random floor tiles into papers, if they only spawn on office rooms/decks then could be unique floor tile
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

		//set the stairs position
		engine.stairs->x = (x1+x2)/2;
		engine.stairs->y = (y1+y2)/2;
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

void Map::infectFloor(int x, int y) {
	tiles[x+y*width].infection += rng->getFloat(.1, 2);
}

bool Map::isInFov(int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return false;
	}
	
	if (map->isInFov(x,y)) {
		tiles[x+y*width].explored = true;
		return true;
	}
	return false;
}

void Map::computeFov() {
	map->computeFov(engine.player->x,engine.player->y, engine.fovRadius);
}
void Map::render() const {

	static const TCODColor darkWall(0,0,100);
	static const TCODColor darkGround(50,50,150);
	static const TCODColor lightWall(30,110,50);
	static const TCODColor lightGround(200,180,50);

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (isInFov(x,y)){// || true) {
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
						//engine.mapconCpy->setChar(x, y, 29);
						//engine.mapconCpy->setCharBackground(x,y,TCODColor::blue);
					}
					else {
						engine.mapcon->setChar(x, y, 31);
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
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
						//engine.mapconCpy->setChar(x, y, 28);
						//engine.mapconCpy->setCharBackground(x,y,TCODColor::blue);
					}
					else {
						engine.mapcon->setChar(x, y, 30);
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
						//engine.mapconCpy->setChar(x, y, 30);
						//engine.mapconCpy->setCharBackground(x,y,TCODColor::blue);
					}
				}
			}
		}
	}



}
void Map::generateRandom(Actor *owner, int ascii){
	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0,100);
	if(dice <= 40){
			return;
	}else{
		if(ascii == 149) //infectedMarines have 60% chance of dropping an item with 50% chance of it being a MLR, and the other 50% chance being a battery pack
		{
			if(dice <= 70)
			{
				Actor *MLR = createMLR(0,0);
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
			for(int i = 0; i < owner->container->size; i++){
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
		}else if(ascii == 165){
			for(int i = 0; i < owner->container->size; i++){
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
			for(int i = 0; i < owner->container->size; i++){
				int rndA2 = rng->getInt(0,100);
				if(rndA2 > 45){
					int rnd = rng->getInt(0,120);
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
					}else if(rnd < 10+30+20+20){
						//create a pair of mylar boots
						Actor *myBoots = createMylarBoots(0,0);
						engine.actors.push(myBoots);
						myBoots->pickable->pick(myBoots,owner);
					}else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = createFlashBang(0,0);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
					}
				}
			
		}else if(ascii == 132){
			for(int i = 0; i < owner->container->size; i++){
				int rndA2 = rng->getInt(0,100);
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
						//create Titanium Micro Chain-mail
						Actor *chainMail = createTitanMail(0,0);
						engine.actors.push(chainMail);
						chainMail->pickable->pick(chainMail,owner);
					}else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = createFlashBang(0,0);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
				}
			}
		}
	}
}
Actor *Map::createHealthPotion(int x,int y){
	Actor *healthPotion = new Actor(x,y,184,"Medkit", TCODColor::white);
	healthPotion->sort = 1;
	healthPotion->blocks = false;
	healthPotion->pickable = new Healer(20);
	return healthPotion;
}
Actor *Map::createFlashBang(int x, int y){
	Actor *scrollOfConfusion = new Actor(x,y,181,"Flashbang", TCODColor::white);
	scrollOfConfusion->sort = 2;
	scrollOfConfusion->blocks = false;
	scrollOfConfusion->pickable = new Confuser(10,8);
	return scrollOfConfusion;
}
Actor *Map::createFireBomb(int x, int y){
	Actor *scrollOfFireball = new Actor(x,y,182,"Firebomb",TCODColor::white);
	scrollOfFireball->sort = 2;
	scrollOfFireball->blocks = false;
	scrollOfFireball->pickable = new Fireball(3,12,8);
	return scrollOfFireball;
}
Actor *Map::createEMP(int x, int y){
	Actor *scrollOfLightningBolt = new Actor(x,y,183, "EMP Pulse",TCODColor::white);
	scrollOfLightningBolt->sort = 2;
	scrollOfLightningBolt->blocks = false;
	scrollOfLightningBolt->pickable = new LightningBolt(5,20);
	return scrollOfLightningBolt;
}
Actor *Map::createTitanMail(int x, int y){
	Actor *chainMail = new Actor(x,y,210,"Titan-mail",TCODColor::lightPink);
	chainMail->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::DEFENSE,3);
	chainMail->pickable = new Equipment(0,Equipment::CHEST,bonus);
	chainMail->sort = 3;
	return chainMail;
}
Actor *Map::createMylarBoots(int x, int y){
	Actor *myBoots = new Actor(x,y,'[',"Mylar-Lined Boots",TCODColor::lightPink);
	myBoots->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::HEALTH,20);
	myBoots->pickable = new Equipment(0,Equipment::FEET,bonus);
	myBoots->sort = 3;
	return myBoots;
}
Actor *Map::createMLR(int x, int y){
	Actor *MLR = new Actor(x,y,'{',"MLR",TCODColor::darkerOrange);
	MLR->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::ATTACK,1);
	MLR->pickable = new Equipment(0,Equipment::RANGED,bonus);
	MLR->sort = 4;
	return MLR;
}
Actor *Map::createBatteryPack(int x,int y){
	Actor *batteryPack = new Actor(x,y,'+',"Battery Pack", TCODColor::yellow);
	batteryPack->sort = 1;
	batteryPack->blocks = false;
	batteryPack->pickable = new Charger(5);
	return batteryPack;
}
