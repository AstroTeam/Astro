#include "main.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;
class BspListener : public ITCODBspCallback {
private:
	Map &map; //a map to dig
	int roomNum; //room number
	int lastx, lasty; // center of the last room
	
public:
	BspListener(Map &map) : map(map), roomNum(0) {}
	
	bool visitNode(TCODBsp *node, void *userData) {
		if (node->isLeaf()) {
			int x,y,w,h;
			bool withActors = (bool)userData;
			//dig a room
			w=map.rng->getInt(ROOM_MIN_SIZE, node->w-2);
			h=map.rng->getInt(ROOM_MIN_SIZE, node->h-2);
			x=map.rng->getInt(node->x+1, node->x+node->w-w-1);
			y=map.rng->getInt(node->y+1, node->y+node->h-h-1);
			map.createRoom(roomNum == 0, x, y, x+w-1, y+h-1, withActors);
			
			if (roomNum != 0) {
				//dig a corridor from last room
				map.dig(lastx, lasty, x+w/2, lasty);
				map.dig(x+w/2, lasty, x+w/2, y+h/2);
			}
			
			lastx = x+w/2;
			lasty = y+h/2;
			roomNum++;
			
		}
		return true;
	}
};

Map::Map(int width, int height): width(width),height(height) {
	seed = TCODRandom::getInstance()->getInt(0,0x7FFFFFFF);
}

Map::~Map() {
	delete [] tiles;
	delete map;
}

void Map::init(bool withActors) {
	rng = new TCODRandom(seed,TCOD_RNG_CMWC);
	tiles = new Tile[width*height];
	map = new TCODMap(width, height);

	//give this level an epicenter of the infection
	int epiLocation = rng->getInt(0, width*height);
	Actor * epicenter = new Actor(epiLocation/width, epiLocation%width, 3, "Infection Epicenter", TCODColor::green);
	epicenter->enviroment=this;
	epicenter->ai= new EpicenterAi;
	engine.actors.push(epicenter);

	//intial infection, concentrated at the epicenter
	for (int i = 0; i < width*height; i++) {
		tiles[i].infection = 1 / ((rng->getDouble(.01,1.0))*epicenter->getDistance(i/width, i%width));
	}

	TCODBsp bsp(0,0,width,height);
	bsp.splitRecursive(rng,8,ROOM_MAX_SIZE,ROOM_MAX_SIZE,1.5f, 1.5f);
	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener,(void *)withActors);
}

void Map::save(TCODZip &zip) {
	zip.putInt(seed);
	for (int i = 0; i < width*height; i++) {
		zip.putInt(tiles[i].explored);
	}
}

void Map::load(TCODZip &zip) {
	seed = zip.getInt();
	init(false);
	for (int i = 0; i <width*height; i++) {
		tiles[i].explored = zip.getInt();
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

	for (int tilex = x1; tilex <=x2; tilex++) {
		for (int tiley = y1; tiley <= y2; tiley++) {
			map->setProperties(tilex,tiley,true,true);
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
	{
		//create an infected crew member
		Actor *infectedCrewMember = new Actor(x,y,infectedCrewMemAscii,"Infected Crewmember",TCODColor::white);
		infectedCrewMember->destructible = new MonsterDestructible(infectedCrewMemMaxHp,infectedCrewMemDef,"infected corpse",infectedCrewMemXp);
		infectedCrewMember->attacker = new Attacker(infectedCrewMemAtk);
		infectedCrewMember->container = new Container(2);
		infectedCrewMember->ai = new MonsterAi();
		generateRandom(infectedCrewMember, infectedCrewMemAscii);
		engine.actors.push(infectedCrewMember);
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
		sporeCreature->enviroment = this;
		generateRandom(sporeCreature, sporeCreatureAscii);
		engine.actors.push(sporeCreature);
	}
}

void Map::addItem(int x, int y) {

	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0,120);
	if (dice < 25) {
		//create a health potion
		Actor *healthPotion = new Actor(x,y,184,"Medkit", TCODColor::white);
		healthPotion->sort = 1;
		healthPotion->blocks = false;
		healthPotion->pickable = new Healer(20);
		engine.actors.push(healthPotion);
		engine.sendToBack(healthPotion);
	} else if(dice < 25+25) {
		//create a scroll of lightningbolt
		Actor *scrollOfLightningBolt = new Actor(x,y,183, "EMP Pulse",
			TCODColor::white);
		scrollOfLightningBolt->sort = 2;
		scrollOfLightningBolt->blocks = false;
		scrollOfLightningBolt->pickable = new LightningBolt(5,20);
		engine.actors.push(scrollOfLightningBolt);
		engine.sendToBack(scrollOfLightningBolt);
	} else if(dice < 25+25+25) {
		//create a scroll of fireball
		Actor *scrollOfFireball = new Actor(x,y,182,"Firebomb",
			TCODColor::white);
		scrollOfFireball->sort = 2;
		scrollOfFireball->blocks = false;
		scrollOfFireball->pickable = new Fireball(3,12,8);
		engine.actors.push(scrollOfFireball);
		engine.sendToBack(scrollOfFireball);
	} else if(dice < 25+25+25+10) {
		//create a pair of mylar boots
		Actor *myBoots = new Actor(x,y,'[',"Mylar-Lined Boots",TCODColor::lightPink);
		myBoots->blocks = false;
		ItemBonus *bonus = new ItemBonus(ItemBonus::HEALTH,20);
		myBoots->pickable = new Equipment(0,Equipment::FEET,bonus);
		myBoots->sort = 3;
		engine.actors.push(myBoots);
		engine.sendToBack(myBoots);
	}else if(dice < 25+25+25+10+5){
		//create Titanium Micro Chain-mail
		Actor *chainMail = new Actor(x,y,210,"Titanium Micro Chainmail",TCODColor::lightPink);
		chainMail->blocks = false;
		ItemBonus *bonus = new ItemBonus(ItemBonus::DEFENSE,3);
		chainMail->pickable = new Equipment(0,Equipment::CHEST,bonus);
		chainMail->sort = 3;
		engine.actors.push(chainMail);
		engine.sendToBack(chainMail);
	}else {
		//create a scroll of confusion
		Actor *scrollOfConfusion = new Actor(x,y,181,"Flashbang",
			TCODColor::white);
		scrollOfConfusion->sort = 2;
		scrollOfConfusion->blocks = false;
		scrollOfConfusion->pickable = new Confuser(10,8);
		engine.actors.push(scrollOfConfusion);
		engine.sendToBack(scrollOfConfusion);
	}
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors) {
	dig(x1,y1,x2,y2);
	
	if (!withActors) {
		return;
	}
	if (first) {
		//put the player in the first room
		engine.player->x = (x1+x2)/2;
		engine.player->y = (y1+y2)/2;
	}
	TCODRandom *rng = TCODRandom::getInstance();
	//add monsters
	//horde chance
	int nbMonsters;
	if (!first && rng->getInt(0,19) == 0) {
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
	//add items
	int nbItems = rng->getInt(0, MAX_ROOM_ITEMS);
	while (nbItems > 0) {
		int x = rng->getInt(x1,x2);
		int y = rng->getInt(y1,y2);
		if (canWalk(x,y)&& (x != engine.player->x && y!= engine.player->y)) {
			addItem(x,y);
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
			if (isInFov(x,y)) {
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
						engine.mapcon->setChar(x, y, 29);
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
						engine.mapcon->setChar(x, y, 28);
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
		if(ascii == 164){
			for(int i = 0; i < owner->container->size; i++){
				int rnd = rng->getInt(0,100);
				if (rnd < 30) {
					//create a health potion
					Actor *healthPotion = new Actor(0,0,184,"Medkit", TCODColor::white);
					healthPotion->sort = 1;
					healthPotion->blocks = false;
					healthPotion->pickable = new Healer(20);
					engine.actors.push(healthPotion);
					healthPotion->pickable->pick(healthPotion,owner);
				} else if(rnd < 10+30) {
					//create a scroll of lightningbolt
					Actor *scrollOfLightningBolt = new Actor(0,0,183, "EMP Pulse",
						TCODColor::white);
					scrollOfLightningBolt->sort = 2;
					scrollOfLightningBolt->blocks = false;
					scrollOfLightningBolt->pickable = new LightningBolt(5,20);
					engine.actors.push(scrollOfLightningBolt);
					scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
				} else if(rnd < 10+30+20) {
					//create a scroll of fireball
					Actor *scrollOfFireball = new Actor(0,0,182,"Firebomb",
						TCODColor::white);
					scrollOfFireball->sort = 2;
					scrollOfFireball->blocks = false;
					scrollOfFireball->pickable = new Fireball(3,12,8);
					engine.actors.push(scrollOfFireball);
					scrollOfFireball->pickable->pick(scrollOfFireball,owner);
				} else{
					//create a scroll of confusion
					Actor *scrollOfConfusion = new Actor(0,0,181,"Flashbang",
						TCODColor::white);
					scrollOfConfusion->sort = 2;
					scrollOfConfusion->blocks = false;
					scrollOfConfusion->pickable = new Confuser(10,8);
					engine.actors.push(scrollOfConfusion);
					scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
				}
			}
		}else if(ascii == 165){
			for(int i = 0; i < owner->container->size; i++){
				int rnd2 = rng->getInt(0,100);
				if (rnd2 < 25) {
					//create a health potion
					Actor *healthPotion = new Actor(0,0,184,"Medkit", TCODColor::white);
					healthPotion->sort = 1;
					healthPotion->blocks = false;
					healthPotion->pickable = new Healer(20);
					engine.actors.push(healthPotion);
					healthPotion->pickable->pick(healthPotion,owner);
				} else if(rnd2 < 25+20) {
					//create a scroll of lightningbolt
					Actor *scrollOfLightningBolt = new Actor(0,0,183, "EMP Pulse",
						TCODColor::white);
					scrollOfLightningBolt->sort = 2;
					scrollOfLightningBolt->blocks = false;
					scrollOfLightningBolt->pickable = new LightningBolt(5,20);
					engine.actors.push(scrollOfLightningBolt);
					scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
				} else if(rnd2 < 25+20+25) {
					//create a scroll of fireball
					Actor *scrollOfFireball = new Actor(0,0,182,"Firebomb",
						TCODColor::white);
					scrollOfFireball->sort = 2;
					scrollOfFireball->blocks = false;
					scrollOfFireball->pickable = new Fireball(3,12,8);
					engine.actors.push(scrollOfFireball);
					scrollOfFireball->pickable->pick(scrollOfFireball,owner);
				} else{
					//create a scroll of confusion
					Actor *scrollOfConfusion = new Actor(0,0,181,"Flashbang",
						TCODColor::white);
					scrollOfConfusion->sort = 2;
					scrollOfConfusion->blocks = false;
					scrollOfConfusion->pickable = new Confuser(10,8);
					engine.actors.push(scrollOfConfusion);
					scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
				}
		}
		}else if(ascii == 148){
			for(int i = 0; i < owner->container->size; i++){
					int rnd = rng->getInt(0,120);
					if (rnd < 30) {
						//create a health potion
						Actor *healthPotion = new Actor(0,0,184,"Medkit", TCODColor::white);
						healthPotion->sort = 1;
						healthPotion->blocks = false;
						healthPotion->pickable = new Healer(20);
						engine.actors.push(healthPotion);
						healthPotion->pickable->pick(healthPotion,owner);
					} else if(rnd < 10+30) {
						//create a scroll of lightningbolt
						Actor *scrollOfLightningBolt = new Actor(0,0,183, "EMP Pulse",
							TCODColor::white);
						scrollOfLightningBolt->sort = 2;
						scrollOfLightningBolt->blocks = false;
						scrollOfLightningBolt->pickable = new LightningBolt(5,20);
						engine.actors.push(scrollOfLightningBolt);
						scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
					} else if(rnd < 10+30+20) {
						//create a scroll of fireball
						Actor *scrollOfFireball = new Actor(0,0,182,"Firebomb",
							TCODColor::white);
						scrollOfFireball->sort = 2;
						scrollOfFireball->blocks = false;
						scrollOfFireball->pickable = new Fireball(3,12,8);
						engine.actors.push(scrollOfFireball);
						scrollOfFireball->pickable->pick(scrollOfFireball,owner);
					}else if(rnd < 10+30+20+20){
						//create a pair of mylar boots
						Actor *myBoots = new Actor(0,0,'[',"Mylar-Lined Boots",TCODColor::lightPink);
						myBoots->blocks = false;
						ItemBonus *bonus = new ItemBonus(ItemBonus::HEALTH,20);
						myBoots->pickable = new Equipment(0,Equipment::FEET,bonus);
						myBoots->sort = 3;
						engine.actors.push(myBoots);
						myBoots->pickable->pick(myBoots,owner);
					}else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = new Actor(0,0,181,"Flashbang",
							TCODColor::white);
						scrollOfConfusion->sort = 2;
						scrollOfConfusion->blocks = false;
						scrollOfConfusion->pickable = new Confuser(10,8);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
				}
		}else if(ascii == 132){
			for(int i = 0; i < owner->container->size; i++){
					int rnd = rng->getInt(0,100);
					if (rnd < 30) {
						//create a health potion
						Actor *healthPotion = new Actor(0,0,184,"Medkit", TCODColor::white);
						healthPotion->sort = 1;
						healthPotion->blocks = false;
						healthPotion->pickable = new Healer(20);
						engine.actors.push(healthPotion);
						healthPotion->pickable->pick(healthPotion,owner);
					} else if(rnd < 10+30) {
						//create a scroll of lightningbolt
						Actor *scrollOfLightningBolt = new Actor(0,0,183, "EMP Pulse",
							TCODColor::white);
						scrollOfLightningBolt->sort = 2;
						scrollOfLightningBolt->blocks = false;
						scrollOfLightningBolt->pickable = new LightningBolt(5,20);
						engine.actors.push(scrollOfLightningBolt);
						scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
					} else if(rnd < 10+30+20) {
						//create a scroll of fireball
						Actor *scrollOfFireball = new Actor(0,0,182,"Firebomb",
							TCODColor::white);
						scrollOfFireball->sort = 2;
						scrollOfFireball->blocks = false;
						scrollOfFireball->pickable = new Fireball(3,12,8);
						engine.actors.push(scrollOfFireball);
						scrollOfFireball->pickable->pick(scrollOfFireball,owner);
					}else if(rnd < 10+30+20+10){
						//create Titanium Micro Chain-mail
						Actor *chainMail = new Actor(0,0,210,"Titanium Micro Chainmail",TCODColor::lightPink);
						chainMail->blocks = false;
						ItemBonus *bonus = new ItemBonus(ItemBonus::DEFENSE,3);
						chainMail->pickable = new Equipment(0,Equipment::CHEST,bonus);
						chainMail->sort = 3;
						engine.actors.push(chainMail);
						chainMail->pickable->pick(chainMail,owner);
					}else{
						//create a scroll of confusion
						Actor *scrollOfConfusion = new Actor(0,0,181,"Flashbang",
							TCODColor::white);
						scrollOfConfusion->sort = 2;
						scrollOfConfusion->blocks = false;
						scrollOfConfusion->pickable = new Confuser(10,8);
						engine.actors.push(scrollOfConfusion);
						scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
					}
			}
		}
	}
}
