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
	
	float infectedCrewMemMaxHp = 10;
	float infectedCrewMemDef = 0;
	float infectedCrewMemAtk = 5;
	float infectedCrewMemXp = 10;
	
	float sporeCreatureMaxHp = 16;
	float sporeCreatureDef = 1;
	float sporeCreatureAtk = 7;
	float sporeCreatureXp = 20;
	
	//Certain enemies' strength scales up as you go down a dungeon 
	
	infectedCrewMemMaxHp += level/2; //increment infected crew member's MaxHp by 1 every even level
	infectedCrewMemAtk += (level-1)/2; //increment infected crew member's Atk by 1 every odd level
	infectedCrewMemXp += (level-1)/2; //increment infected crew member's Xp by 1 every odd level
		
	sporeCreatureMaxHp += level/3; //increment spore creature's MaxHp by 1 every third level (starting at 3)
	sporeCreatureAtk += (level+2)/3; //increment spore creature's Atk by 1 every third level (starting at 4)
	sporeCreatureXp += level/3; //increment spore creature's Xp by 1 every third level (starting at 3)
	
	//The percent of spore creatures starts at 20% and increases by 5 percent as you go down each level, but going no higher than 50%	
	float percentInfectedCrewMembers =  ( (85 - 5*level) > 50 ? (85 - 5*level) : 50 ); 
	if (rng->getInt(0,100) < percentInfectedCrewMembers) {
		//create an infected crew member
		Actor *infectedCrewMember = new Actor(x,y,164,"Infected Crewmember",TCODColor::white);
		infectedCrewMember->destructible = new MonsterDestructible(infectedCrewMemMaxHp,infectedCrewMemDef,"infected corpse",infectedCrewMemXp);
		infectedCrewMember->attacker = new Attacker(infectedCrewMemAtk);
		infectedCrewMember->container = new Container(2);
		infectedCrewMember->ai = new MonsterAi();
		generateRandom(infectedCrewMember, 164);
		engine.actors.push(infectedCrewMember);
	}
	else {	
		//create a spore creature
		Actor *sporeCreature = new Actor(x,y,165,"Spore Creature",TCODColor::white);
		sporeCreature->destructible = new MonsterDestructible(sporeCreatureMaxHp,sporeCreatureDef,"gross spore remains",sporeCreatureXp);
		sporeCreature->attacker = new Attacker(sporeCreatureAtk);
		sporeCreature->container = new Container(2);
		sporeCreature->ai = new MonsterAi();
		sporeCreature->oozing = true;
		sporeCreature->enviroment = this;
		generateRandom(sporeCreature, 165);
		engine.actors.push(sporeCreature);
	}
}

void Map::addItem(int x, int y) {

	TCODRandom *rng = TCODRandom::getInstance();
	int dice = rng->getInt(0,140);
	if (dice < 25) {
		//create a health potion
		Actor *healthPotion = new Actor(x,y,184,"Medkit", TCODColor::white);
		healthPotion->type = 1;
		healthPotion->blocks = false;
		healthPotion->pickable = new Healer(20);
		engine.actors.push(healthPotion);
		engine.sendToBack(healthPotion);
	} else if(dice < 25+25) {
		//create a scroll of lightningbolt
		Actor *scrollOfLightningBolt = new Actor(x,y,183, "EMP Pulse",
			TCODColor::white);
		scrollOfLightningBolt->type = 2;
		scrollOfLightningBolt->blocks = false;
		scrollOfLightningBolt->pickable = new LightningBolt(5,20);
		engine.actors.push(scrollOfLightningBolt);
		engine.sendToBack(scrollOfLightningBolt);
	} else if(dice < 25+25+25) {
		//create a scroll of fireball
		Actor *scrollOfFireball = new Actor(x,y,182,"Firebomb",
			TCODColor::white);
		scrollOfFireball->type = 2;
		scrollOfFireball->blocks = false;
		scrollOfFireball->pickable = new Fireball(3,12,8);
		engine.actors.push(scrollOfFireball);
		engine.sendToBack(scrollOfFireball);
	} else if(dice < 25+25+25+50) {
		//create a pair of mylar underpants
		Actor *undies = new Actor(x,y,'[',"Mylar underpants",TCODColor::lightPink);
		undies->blocks = false;
		ItemBonus *bonus = new ItemBonus(ItemBonus::HEALTH,20);
		undies->pickable = new Equipment(0,Equipment::LEGS,bonus);
	
		engine.actors.push(undies);
		engine.sendToBack(undies);
	} else {
		//create a scroll of confusion
		Actor *scrollOfConfusion = new Actor(x,y,181,"Flashbang",
			TCODColor::white);
		scrollOfConfusion->type = 2;
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
					}
					else {
						engine.mapcon->setChar(x, y, 31);
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
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
					}
					else {
						engine.mapcon->setChar(x, y, 30);
						engine.mapcon->setCharBackground(x,y,TCODColor::blue);
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
					healthPotion->type = 1;
					healthPotion->blocks = false;
					healthPotion->pickable = new Healer(20);
					engine.actors.push(healthPotion);
					healthPotion->pickable->pick(healthPotion,owner);
				} else if(rnd < 10+30) {
					//create a scroll of lightningbolt
					Actor *scrollOfLightningBolt = new Actor(0,0,183, "EMP Pulse",
						TCODColor::white);
					scrollOfLightningBolt->type = 2;
					scrollOfLightningBolt->blocks = false;
					scrollOfLightningBolt->pickable = new LightningBolt(5,20);
					engine.actors.push(scrollOfLightningBolt);
					scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
				} else if(rnd < 10+30+20) {
					//create a scroll of fireball
					Actor *scrollOfFireball = new Actor(0,0,182,"Firebomb",
						TCODColor::white);
					scrollOfFireball->type = 2;
					scrollOfFireball->blocks = false;
					scrollOfFireball->pickable = new Fireball(3,12,8);
					engine.actors.push(scrollOfFireball);
					scrollOfFireball->pickable->pick(scrollOfFireball,owner);
				} else{
					//create a scroll of confusion
					Actor *scrollOfConfusion = new Actor(0,0,181,"Flashbang",
						TCODColor::white);
					scrollOfConfusion->type = 2;
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
					healthPotion->type = 1;
					healthPotion->blocks = false;
					healthPotion->pickable = new Healer(20);
					engine.actors.push(healthPotion);
					healthPotion->pickable->pick(healthPotion,owner);
				} else if(rnd2 < 25+20) {
					//create a scroll of lightningbolt
					Actor *scrollOfLightningBolt = new Actor(0,0,183, "EMP Pulse",
						TCODColor::white);
					scrollOfLightningBolt->type = 2;
					scrollOfLightningBolt->blocks = false;
					scrollOfLightningBolt->pickable = new LightningBolt(5,20);
					engine.actors.push(scrollOfLightningBolt);
					scrollOfLightningBolt->pickable->pick(scrollOfLightningBolt,owner);
				} else if(rnd2 < 25+20+25) {
					//create a scroll of fireball
					Actor *scrollOfFireball = new Actor(0,0,182,"Firebomb",
						TCODColor::white);
					scrollOfFireball->type = 2;
					scrollOfFireball->blocks = false;
					scrollOfFireball->pickable = new Fireball(3,12,8);
					engine.actors.push(scrollOfFireball);
					scrollOfFireball->pickable->pick(scrollOfFireball,owner);
				} else{
					//create a scroll of confusion
					Actor *scrollOfConfusion = new Actor(0,0,181,"Flashbang",
						TCODColor::white);
					scrollOfConfusion->type = 2;
					scrollOfConfusion->blocks = false;
					scrollOfConfusion->pickable = new Confuser(10,8);
					engine.actors.push(scrollOfConfusion);
					scrollOfConfusion->pickable->pick(scrollOfConfusion,owner);
				}
		}
		}
	}
}
