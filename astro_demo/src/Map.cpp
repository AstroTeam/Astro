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
	else if (tiles[i].tileType == Param::BARRACKS)
	{return 3;}
	else if (tiles[i].tileType == Param::GENERATOR)
	{return 4;}
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
					engine.sendToBack(a);
					
					int n = rng->getInt(5,8);
					int x = a->x;
					int y = a->y;
					int add = rng->getInt(0,10);
					for (int xxx = -1; xxx <= 1; xxx++)/////////////////////9x9 for loop to add papers, lol xxx,yyy
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
	int dice = rng->getInt(0,335);
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
	}else if(dice< 40+40+40+15+15+5+40+40){
		//create a scroll of confusion
		Actor *scrollOfConfusion = createFlashBang(x,y);
		engine.actors.push(scrollOfConfusion);
		engine.sendToBack(scrollOfConfusion);
	}
	else {
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
				for (int i = 0; i <= rng->getInt(1,5); i++) {
					roomList->push(OFFICE);
				}	
				//small amount of barracks
				for (int i = 0; i <= rng->getInt(1,3); i++) {
					roomList->push(BARRACKS);
				}	
				//need to see if end list items are less common
				//roomList->push(SERVER);
				//roomList->push(ARMORY);
				//roomList->push(MESSHALL);
				//roomList->push(OBSERVATORY);
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
		engine.playerLight = new Actor(engine.player->x, engine.player->y, 'l', "Your Flashlight", TCODColor::white);
		engine.playerLight->ai = new LightAi(2,1,true); //could adjust second '1' to less if the flashlight should flicker
		engine.actors.push(engine.playerLight);
		engine.playerLight->blocks = false;
		//playerLight->ai->moving = true;
		engine.sendToBack(engine.playerLight);
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
	
	
	//record the room type to the tile of the room
	for (int tilex = x1; tilex <=x2; tilex++) {
		for (int tiley = y1; tiley <= y2; tiley++) {
			tiles[tilex+tiley*width].tileType = room->type;
		}
	}
	
	//custom room feature
	//OFFICE ROOMS
	if (room->type == OFFICE) {
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
					}					
					engine.mapconDec->setChar(xX, yY, n);
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
			engine.mapconDec->setChar(x1+1,i, rng->getInt(9,11));//Bed Headboard (9,10,11, add random)
			engine.actors.push(bed);
			Actor * bedf = new Actor(x1+2,i,243,"Bed foot", TCODColor::white);
			engine.mapconDec->setChar(x1+2,i, rng->getInt(15,17));//Bed foot (12,13,14, add random)
			engine.actors.push(bedf);
			//send to back
			Actor *endtable = new Actor(x1+1,i+1,243,"A bare-bones endtable", TCODColor::white);
			engine.mapconDec->setChar(x1+1,i+1, 21);//endtable
			engine.actors.push(endtable);
			endtable->blocks = false;
			engine.sendToBack(endtable);
			//need to check if there is enough space
			Actor * bed2 = new Actor(x2-1,i,243,"Bed Headboard", TCODColor::white);
			engine.mapconDec->setChar(x2-1,i, rng->getInt(12,14));//Bed Headboard (9,10,11, add random)
			engine.actors.push(bed2);
			Actor * bedf2 = new Actor(x2-2,i,243,"Bed foot", TCODColor::white);
			engine.mapconDec->setChar(x2-2,i, rng->getInt(18,20));//Bed Headboard (12,13,14, add random)
			engine.actors.push(bedf2);
			Actor *endtable2 = new Actor(x2-1,i+1,243,"A bare-bones endtable", TCODColor::white);
			engine.mapconDec->setChar(x2-1,i+1, 22);//endtable
			engine.actors.push(endtable2);
			endtable2->blocks = false;
			engine.sendToBack(endtable2);
			if (delta >= 4)
			{
				//have locker, be attackable, drops loot, have one blank ascii & mapcondec number, when you attack it it switches to 
				//another mapcondec number to look opened/looted
				if (!mod2)
				{
					Actor *locker = new Actor((x1+x2)/2,i,243,"A Government Issue Locker", TCODColor::white);
					engine.mapconDec->setChar((x1+x2)/2,i, 23);//Locker
					locker->destructible = new MonsterDestructible(1,0,"Opened Locker",0);
					locker->container = new Container(3);
					generateRandom(locker,243);
					engine.actors.push(locker);
					Actor *locker2 = new Actor(((x1+x2)/2)+1,i,243,"A Government Issue Locker", TCODColor::white);
					engine.mapconDec->setChar(((x1+x2)/2)+1,i, 23);//Locker
					locker2->destructible = new MonsterDestructible(1,0,"Opened Locker",0);
					locker2->container = new Container(3);
					generateRandom(locker2,243);
					engine.actors.push(locker2);
				}
				else
				{
					Actor *locker = new Actor((x1+x2)/2,i,243,"A Government Issue Locker", TCODColor::white);
					engine.mapconDec->setChar((x1+x2)/2,i, 23);//Locker
					locker->destructible = new MonsterDestructible(1,0,"Opened Locker",0);
					locker->container = new Container(3);
					generateRandom(locker,243);
					engine.actors.push(locker);
				}
			}
			i += 2;
			
		}
	}
	if (room->type == GENERATOR) {
		cout << "Gen room Made" << endl;
		Actor * generator = new Actor(x1+1,y1+1,243,"A floor tile that has been jerry rigged to accept a generator.", TCODColor::white);
		engine.mapconDec->setChar(x1+1,y1+1, 25);//
		engine.actors.push(generator);
		generator->blocks = false;
		engine.sendToBack(generator);
		Actor * generator1 = new Actor(x1+2,y1+1,243,"A danger sign and a small toolbox.", TCODColor::white);
		engine.mapconDec->setChar(x1+2,y1+1, 26);//
		engine.actors.push(generator1);
		generator1->blocks = false;
		engine.sendToBack(generator1);
		Actor * generator2 = new Actor(x1+1,y1+2,243,"A bundle of cables.", TCODColor::white);
		engine.mapconDec->setChar(x1+1,y1+2, 27);//
		engine.actors.push(generator2);
		generator2->blocks = false;
		engine.sendToBack(generator2);
		//Actor * generator3 = new Actor(x1+2,y1+2,'G',"a generator", TCODColor::white);
		//engine.actors.push(generator3);
		Actor * generator4 = new Actor(x1+1,y1+3,243,"A portable generator.", TCODColor::white);
		engine.mapconDec->setChar(x1+1,y1+3, 29);//
		engine.actors.push(generator4);
		Actor * generator5 = new Actor(x1+2,y1+3,243,"A generator control console.", TCODColor::white);
		engine.mapconDec->setChar(x1+2,y1+3, 30);//
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
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.mapconDec->getChar(x,y) == ' ') {
				Actor *Drum = new Actor(x, y, 243, "A gasoline drum.", TCODColor::white);
				engine.mapconDec->setChar(x,y, 28);//
				engine.actors.push(Drum);
				i++;
			}
			
		}
		
		int torch = rng->getInt(1,3,3);
		for (int i = 0; i < torch;)
		{	
			int x = rng->getInt(x1+1,x2-1);
			int y = rng->getInt(y1+1,y2-1);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.mapconDec->getChar(x,y) == ' ') {
				Actor *torch = new Actor(x, y, 243, "A blowtorch.", TCODColor::white);
				engine.mapconDec->setChar(x,y, 31);//
				engine.actors.push(torch);
				i++;
			}
			
		}
		
		int pall = rng->getInt(1,4,2);
		for (int i = 0; i < pall;)
		{	
			int x = rng->getInt(x1+1,x2-1);
			int y = rng->getInt(y1+1,y2-1);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.mapconDec->getChar(x,y) == ' ') {
				Actor *pallet = new Actor(x, y, 243, "An empty pallet.", TCODColor::white);
				engine.mapconDec->setChar(x,y, 32);//
				engine.actors.push(pallet);
				pallet->blocks = false;
				engine.sendToBack(pallet);
				i++;
			}
			
		}
	}

	/*
	 *
	 * SETTINGS FOR OTHER ROOMS CAN BE PLACED HERE
	 *
	 */
	
	//TCODRandom *rnd = TCODRandom::getInstance();
	//add lights to all rooms, make test later
	if (rng->getInt(0,10) > 4)
	{
		//42 is star 
		int numLights = 0;
		int rmSze = (x2 - x1) * (y2 - y1);
		numLights = rmSze/30;
		if (numLights <= 0)
			numLights = 1;
		for (int i = 0; i < numLights;)
		{
			//bool valid = false;
			//int x = (x1+x2)/2;
			//int y = (y1+y2)/2;
			int x = rng->getInt(x1+1,x2-1);
			int y = rng->getInt(y1+1,y2-1);
			if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y) && engine.mapconDec->getChar(x,y) == ' ') {
				Actor *light = new Actor(x, y, 224, "An hastily erected Emergency Light", TCODColor::white);
				//4,1 = standard light, radius, flkr
				TCODRandom *myRandom = new TCODRandom();
				//0.8 is lower limit, put closer to 1 for less flicker
				int chance = myRandom->getInt(0,10);
				float rng2;
				if (chance > 1)
				{
					rng2 = myRandom->getFloat(0.5000f,0.9000f,0.8500f);
				}
				else
				{
					rng2 = 1;
				}
				light->ai = new LightAi(rng->getInt(3,6),rng2);
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
	if(dice <= 40){
			return;
	}else{
		if(ascii == 243){//locker, this might be a problem if we want multiple decors to drop different things
			int random = rng->getInt(0,100);
			if(random < 30){
				Actor *flare = createFlare(0,0);
				engine.actors.push(flare);
				flare->pickable->pick(flare,owner);
			}else if(random < 30+10){
				Actor *chainMail = createTitanMail(0,0);
				engine.actors.push(chainMail);
				chainMail->pickable->pick(chainMail,owner);
			}else if(random < 30+10+20){
				Actor *myBoots = createMylarBoots(0,0);
				engine.actors.push(myBoots);
				myBoots->pickable->pick(myBoots,owner);
			}else{
				Actor *batt = createBatteryPack(0,0);
				engine.actors.push(batt);
				batt->pickable->pick(batt,owner);
			}
		}else if(ascii == 149) //infectedMarines have 60% chance of dropping an item with 50% chance of it being a MLR, and the other 50% chance being a battery pack
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
Actor *Map::createCurrencyStack(int x, int y){
	Actor *currencyStack = new Actor(x,y,'B',"PetaBitcoins",TCODColor::yellow);
	currencyStack->sort = 0;
	currencyStack->blocks = false;
	currencyStack->pickable = new Coinage(1,100+75*(engine.level-1));
	return currencyStack;
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
Actor *Map::createFlare(int x, int y){
	Actor *scrollOfFlaring = new Actor(x,y,187,"Flare", TCODColor::white);
	scrollOfFlaring->sort = 2;
	scrollOfFlaring->blocks = false;
	scrollOfFlaring->pickable = new Flare(10,5,5);//10 is turns, can be random, 5 is range of throwability (constant), 5 is range of flare
	return scrollOfFlaring;
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
	Actor *chainMail = new Actor(x,y,185,"Titan-mail",TCODColor::white);
	chainMail->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::DEFENSE,3);
	chainMail->pickable = new Equipment(0,Equipment::CHEST,bonus);
	chainMail->sort = 3;
	return chainMail;
}
Actor *Map::createMylarBoots(int x, int y){
	Actor *myBoots = new Actor(x,y,185,"Mylar-Lined Boots",TCODColor::white);
	myBoots->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::HEALTH,20);
	myBoots->pickable = new Equipment(0,Equipment::FEET,bonus);
	myBoots->sort = 3;
	return myBoots;
}
Actor *Map::createMLR(int x, int y){
	Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	MLR->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::ATTACK,1);
	MLR->pickable = new Equipment(0,Equipment::RANGED,bonus);
	MLR->sort = 4;
	return MLR;
}
Actor *Map::createBatteryPack(int x,int y){
	Actor *batteryPack = new Actor(x,y,186,"Battery Pack", TCODColor::white);
	batteryPack->sort = 1;
	batteryPack->blocks = false;
	batteryPack->pickable = new Charger(5);
	return batteryPack;
}

