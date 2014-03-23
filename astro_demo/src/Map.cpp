
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
	else if (tiles[i].tileType == Param::KITCHEN)
	{return 5;}
	else if (tiles[i].tileType == Param::SERVER)
	{return 6;}
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
	//Create boss, for now it is a simple security bot
	if (withActors) {
		Actor *boss = createSecurityBot(engine.stairs->x+1, engine.stairs->y);
		boss->name = "Infected Security Bot";
		boss->ch = 146;
		boss->destructible->hp = boss->destructible->hp*2;
		boss->destructible->maxHp = boss->destructible->hp;
		boss->totalStr = boss->totalStr*1.25;
		engine.boss = boss;
	}
}

void Map::save(TCODZip &zip) {
	zip.putInt(seed);
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
	}
}

void Map::load(TCODZip &zip) {
	seed = zip.getInt();
	init(false);
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
							a->name = "blarg!";
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
//	float turretChance = 50;
//	float vendorChance = 100;

	int uB = 1000;
	
	if(isHorde) //since engineers and cleaners don't spawn in hordes, adjust uppber bounded accordingly
		uB =- (infectedEngineerChance+cleanerChance);

	
	int dice = rng->getInt(0,uB);
	
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
	float scale = 1 + .1*(level - 1);
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
Actor* Map::createVendor(int x, int y)
{
	int level = engine.level;
	float scale = 1 + .1*(level - 1);
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
	vendor->container = new Container(10);
	generateRandom(vendor, vendorAscii);
	engine.actors.push(vendor);
	
	return vendor;
}
void Map::addItem(int x, int y, RoomType roomType) {

	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0,375);
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
	}else if(dice< 40+40+40+15+15+5+40+40+40){
		//create a scroll of fragging
		Actor *scrollOfFragging = createFrag(x,y);
		engine.actors.push(scrollOfFragging);
		engine.sendToBack(scrollOfFragging);
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
				for (int i = 0; i <= rng->getInt(1,5); i++) {
					roomList->push(OFFICE);
				}	
				//small amount of barracks
				for (int i = 0; i <= rng->getInt(1,3); i++) {
					roomList->push(BARRACKS);
				}	
				roomList->push(KITCHEN);
				roomList->push(KITCHEN);
				//need to see if end list items are less common
				roomList->push(SERVER);
				roomList->push(SERVER);
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
					locker->container = new Container(3);
					generateRandom(locker,243);
					engine.actors.push(locker);
					Actor *locker2 = new Actor(((x1+x2)/2)+1,i,243,"Government Issue Locker", TCODColor::white);
					//engine.mapconDec->setChar(((x1+x2)/2)+1,i, 23);//Locker
					engine.map->tiles[(((x1+x2)/2)+1)+i*engine.map->width].decoration = 23;
					locker2->destructible = new MonsterDestructible(1,0,0,0);
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
						engine.map->tiles[i+j*engine.map->width].decoration = 44;
						engine.actors.push(pcmu);
					}
				}
			}
		}
	}

	if (room->type == SERVER) {
cout << "Server room made";
		//expand the room outwards
		int tx1 = x1-1;
		int tx2 = x2+1;
		int ty1 = y1-1;
		int ty2 = y2+1;
		/*//top and bottom wall with servers
		for (int i = tx1; i <= tx2; i++) {
			map->setProperties(i,ty1,true,true);
			map->setProperties(i,ty2,true,true);
			Actor * server1 = new Actor(i, ty1, 243, "A server", TCODColor::white);
			engine.map->tiles[i+(ty1)*engine.map->width].decoration = rng->getInt(45,47);
			engine.map->tiles[i+(ty1)*engine.map->width].tileType = SERVER;
			server1->smashable = true;
			engine.actors.push(server1);
			Actor * server2 = new Actor(i, ty2, 243, "A server", TCODColor::white);
			engine.map->tiles[i+(ty2)*engine.map->width].decoration = rng->getInt(45,47);
			engine.map->tiles[i+(ty2)*engine.map->width].tileType = SERVER;
			server2->smashable = true;
			engine.actors.push(server2);
		}
		//left and right walls with servers
		for (int i = ty1; i <= ty2; i++) {
			map->setProperties(tx1,i,true,true);
			map->setProperties(tx2,i,true,true);
			Actor * server1 = new Actor(tx1, i, 243, "A server", TCODColor::white);
			engine.map->tiles[tx1+i*engine.map->width].decoration = rng->getInt(45,47);
			engine.map->tiles[tx1+i*engine.map->width].tileType = SERVER;
			server1->smashable = true;
			engine.actors.push(server1);
			Actor * server2 = new Actor(tx2, i, 243, "A server", TCODColor::white);
			engine.map->tiles[tx2+i*engine.map->width].decoration = rng->getInt(45,47);
			engine.map->tiles[tx2+i*engine.map->width].tileType = SERVER;
			server2->smashable = true;
			engine.actors.push(server2);
		}*/
		
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
					Actor * console = new Actor(i, j, 'c', "A console", TCODColor::white);
					engine.actors.push(console);
				}
				else if (j != ((y1+y2)/2)){
					Actor * server1 = new Actor(i, j, 243, "A server", TCODColor::white);
					engine.map->tiles[i+j*engine.map->width].decoration = rng->getInt(45,47);
					engine.actors.push(server1);
				}
			}
		}
	}


	/*
	 *
	 * SETTINGS FOR OTHER ROOMS CAN BE PLACED HERE
	 *
	 */
	
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
	}
	//TCODRandom *rng = TCODRandom::getInstance();

	/* monster section */

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
		
			addMonster(x,y,nbMonsters >= MAX_ROOM_MONSTERS);
			nbMonsters--;
		}
	}
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
					light->name = "An flickering hastily erected Emergency Light";
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
	
	
	
	
	int rand = rng->getInt(0,100);
	//Vending Machines spawn in corners of standard rooms at random
	
	if(rand <= 20 && room->type == STANDARD) //a room has a 30% chance of having a vending machine (provided it has rooms)
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
	//15% chance of spawning turrets in the corners of standard rooms
	if(rand >= 15 && rand <= 30 && room->type == STANDARD)
	{
		int c = rng->getInt(0,3);
		bool x1y1 = canWalk(x1,y1) && engine.getAnyActor(x1,y1)==NULL;
		bool x1y2 = canWalk(x1,y2) && engine.getAnyActor(x1,y2)==NULL;
		bool x2y2 = canWalk(x2,y2) && engine.getAnyActor(x2,y2)==NULL;
		bool x2y1 = canWalk(x2,y1) && engine.getAnyActor(x2,y1)==NULL;
		for(int i = 0; i < 4; i++)
		{
			c = rng->getInt(0,3);
			if(c == 0 && x1y1)
				createTurret(x1,y1);
			if(c == 1 && x1y2)
				createTurret(x1,y2);
			if(c == 2 && x2y2)
				createTurret(x2,y2);
			if(c == 3 && x2y1)
				createTurret(x2, y1);
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
		}else if(ascii == 133) //infected grenadier
		{
			for(int i = 0; i < owner->container->size; i++)
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
		}else if(ascii == 165 || ascii == 166){
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
Actor *Map::createFireBomb(int x, int y){
	Actor *scrollOfFireball = new Actor(x,y,182,"Firebomb",TCODColor::white);
	scrollOfFireball->sort = 2;
	scrollOfFireball->blocks = false;
	scrollOfFireball->pickable = new Fireball(3,12,8);
	scrollOfFireball->pickable->value = 45;
	scrollOfFireball->pickable->inkValue = 10;
	return scrollOfFireball;
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
Actor *Map::createTitanMail(int x, int y){
	Actor *chainMail = new Actor(x,y,185,"Titan-mail",TCODColor::white);
	chainMail->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::DR,3);
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,8);
	chainMail->pickable = new Equipment(0,Equipment::CHEST,bonus,requirement);
	chainMail->sort = 3;
	chainMail->pickable->value = 1600;
	chainMail->pickable->inkValue = 50;
	return chainMail;
}
Actor *Map::createMylarBoots(int x, int y){
	Actor *myBoots = new Actor(x,y,185,"Mylar-Lined Boots",TCODColor::white);
	myBoots->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::HEALTH,20);
	ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,4);
	myBoots->pickable = new Equipment(0,Equipment::FEET,bonus,requirement);
	myBoots->sort = 3;
	myBoots->pickable->value = 150;
	myBoots->pickable->inkValue = 15;
	return myBoots;
}
Actor *Map::createMLR(int x, int y){
	Actor *MLR = new Actor(x,y,169,"MLR",TCODColor::white);
	MLR->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::DEXTERITY,1);
	ItemReq *requirement = new ItemReq(ItemReq::DEXTERITY,4);
	MLR->pickable = new Equipment(0,Equipment::RANGED,bonus,requirement);
	MLR->sort = 4;
	MLR->pickable->value = 200;
	MLR->pickable->inkValue = 30;
	return MLR;
}
Actor *Map::createCombatKnife(int x, int y){
	Actor *combatKnife = new Actor(x,y,169,"Combat Knife",TCODColor::white);
	combatKnife->blocks = false;
	ItemBonus *bonus = new ItemBonus(ItemBonus::STRENGTH,1);
	ItemReq *requirment = new ItemReq(ItemReq::STRENGTH,3);
	combatKnife->pickable = new Equipment(0,Equipment::HAND1,bonus,requirment);
	combatKnife->pickable->value = 100;
	combatKnife->pickable->inkValue = 10;
	combatKnife->sort = 4;
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

