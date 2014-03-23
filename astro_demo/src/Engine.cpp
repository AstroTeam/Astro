#include "main.hpp"
#include "SDL/SDL.h"
#include <math.h>

/* Engine::Engine() : gameStatus(STARTUP), fovRadius(3)
{
	TCODConsole::initRoot(80,50,"Astro", false);
	player = new Actor(40,25,'@', "player",TCODColor::white);
	actors.push(player);
	map = new Map(80,45);

} */

Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP),
	player(NULL),playerLight(NULL),boss(NULL),map(NULL), fovRadius(3),
	screenWidth(screenWidth),screenHeight(screenHeight),level(1),turnCount(0), piratesFound(0) {
	mapWidth = 100;
	mapHeight = 100;
	TCODConsole::initRoot(screenWidth,screenHeight,"Astro", false,TCOD_RENDERER_SDL);
	//TCODSystem::registerSDLRenderer(new Renderer());
	//TCODSystem::registerSDLRenderer(new blengine());
	
	mapcon = new TCODConsole(mapWidth,mapHeight);
	engine.mapcon->setDefaultBackground(TCODColor::blue);
	mapconCpy = new TCODConsole(mapWidth, mapHeight);
	//mapconDec = new TCODConsole(mapWidth, mapHeight);
	gui = new Gui();
	invState = 0;
	menuState = 0;
	damageDone = 0;
	damageReceived = 0;
	selX = 0;
	selY = 0;
	mapx1 = 0;
	mapx2 = 0;
	mapy1 = 0;
	mapy2 = 0;
//	rend = new Renderer();
}

Engine::~Engine() {
	term();
	delete gui;
}

void Engine::term() {
	actors.clearAndDelete();
	if (map) delete map;
	gui->clear();
	engine.turnCount = 0;
}

void Engine::init() {
	engine.killCount = 0;
	engine.level = 1;
	player = new Actor(40,25,'@', "player","Human","Marine","Infantry",TCODColor::white);
	//playerLight = new Actor(40, 25, 'l', "Your Flashlight", TCODColor::white);
	//playerLight->ai = new LightAi(2,1,true); //could adjust second '1' to less if the flashlight should flicker
	//engine.actors.push(playerLight);
	//playerLight->blocks = false;
	//playerLight->ai->moving = true;
	//engine.sendToBack(playerLight);
	player->hostile = false;
	player->destructible = new PlayerDestructible(100, 2, 0);
	player->attacker = new Attacker(5,20);
	player->ai = new PlayerAi();
	player->container = new Container(50);
	actors.push(player);
	player->flashable = true;
	player->str=engine.gui->strValue;
	player->totalStr=engine.gui->strValue;
	player->attacker->basePower=engine.gui->strValue;  //old
	player->attacker->totalPower=engine.gui->strValue; //old
	player->dex=engine.gui->agValue;
	player->totalDex=engine.gui->agValue;
	player->intel=engine.gui->intelValue;
	player->totalIntel=engine.gui->intelValue;
	player->vit=engine.gui->conValue;
	player->destructible->hp=engine.gui->conValue;
	player->destructible->maxHp=engine.gui->conValue;
	
	engine.gui->statPoints = 2;
	engine.gui->conValue = 100;
	engine.gui->strValue = 5;
	engine.gui->agValue = 3;
	engine.gui->intelValue = 3;
	
	int plyrAscii = 64;
	switch(engine.gui->raceSelection){
		case 1:
			player->race="Human";
			plyrAscii = 143;
			break;
		case 2:
			player->race="Robot";
			plyrAscii = 159;
			
			player->str += 2;
			player->totalStr += 2;
			player->attacker->basePower += 2;   //old
			player->attacker->totalPower += 2;  //old
			
			player->vit += 20;
			player->destructible->maxHp += 20;
			player->destructible->hp += 20;
			
			player->dex -= 2;
			player->totalDex -= 2;
			
			player->intel += 2;
			player->totalIntel += 2;
			
			break;
		case 3:
			player->race="Alien";
			plyrAscii = 175;
			
			player->str -= 2;
			player->totalStr -= 2;
			player->attacker->basePower -= 2;   //old
			player->attacker->totalPower -= 2;  //old
			
			player->vit -= 40;
			player->destructible->maxHp -= 40;
			player->destructible->hp -= 40;
			
			player->dex += 2;
			player->totalDex += 2;
			
			player->intel += 2;
			player->totalIntel += 2;
			
			break;
			default:
				player->race="Human";
				plyrAscii = 143;
			break;
	}
	
	/*Actor *pants = new Actor(0,0,185,"Marine Fatigue Pants",TCODColor::white);
	ItemBonus *bonus = new ItemBonus(ItemBonus::HEALTH,0);
	pants->blocks = false;
	pants->pickable = new Equipment(0,Equipment::LEGS,bonus);
	pants->sort = 3;
	engine.actors.push(pants);

	Actor *shirt = new Actor(0,0,185,"Marine Fatigue Jacket",TCODColor::white);
	shirt->blocks = false;
	shirt->pickable = new Equipment(0,Equipment::CHEST,bonus);
	shirt->sort = 3;
	engine.actors.push(shirt);*/
	
	ItemReq *requirement = new ItemReq(ItemReq::STRENGTH,0);
	
	Actor *helmet = NULL;
	ItemBonus *bonusHe;
	
	Actor *chest = NULL;
	ItemBonus *bonusC;
	Actor *legs = NULL;
	ItemBonus *bonusL;
	
	Actor *feet = NULL;
	ItemBonus *bonusF;
	
	Actor *hands = NULL;
	ItemBonus *bonusHa;
	
	Actor *ranged = NULL;
	ItemBonus *bonusR;
	
	Actor *equip1 = NULL;
	
	
	switch(engine.gui->jobSelection){
		

		case 1:
			player->role="Marine";
			player->job="Infantry";
			
			player->dex+=3; //job selection bonus
			player->totalDex+=3; //job selection bonus
			
			ranged = new Actor(0,0,169,"MLR",TCODColor::white);
			bonusR = new ItemBonus(ItemBonus::DEXTERITY,1);
			ranged->blocks = false;
			ranged->pickable = new Equipment(0,Equipment::RANGED,bonusR,requirement);
			ranged->sort = 4;
			engine.actors.push(ranged);
			ranged->pickable->pick(ranged,player);
			((Equipment*)(ranged->pickable))->use(ranged,player);
			
			legs = new Actor(0,0,185,"Marine Fatigue Pants",TCODColor::white);
			bonusL = new ItemBonus(ItemBonus::HEALTH,0);
			legs->blocks = false;
			legs->pickable = new Equipment(0,Equipment::LEGS,bonusL,requirement);
			legs->sort = 3;
			engine.actors.push(legs);
			legs->pickable->pick(legs,player);
			((Equipment*)(legs->pickable))->use(legs,player);
			
			feet = new Actor(0,0,185,"Combat Boots",TCODColor::white);
			bonusF = new ItemBonus(ItemBonus::HEALTH,0);
			feet->blocks = false;
			feet->pickable = new Equipment(0,Equipment::FEET,bonusF,requirement);
			feet->sort = 3;
			engine.actors.push(feet);
			feet->pickable->pick(feet,player);
			((Equipment*)(feet->pickable))->use(feet,player);
			
			chest = new Actor(0,0,185,"Marine Fatigue Jacket",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
			
			helmet = new Actor(0,0,185,"Marine Ballistic Helmet",TCODColor::white);
			bonusHe = new ItemBonus(ItemBonus::HEALTH,0);
			helmet->blocks = false;
			helmet->pickable = new Equipment(0,Equipment::HEAD,bonusHe,requirement);
			helmet->sort = 3;
			engine.actors.push(helmet);
			helmet->pickable->pick(helmet,player);
			((Equipment*)(helmet->pickable))->use(helmet,player);
			
			equip1 = new Actor(0,0,182,"FireBomb", TCODColor::white);
			equip1->sort = 2;
			equip1->blocks = false;
			equip1->pickable = new Fireball(3,12,8);
			engine.actors.push(equip1);
			equip1->pickable->pick(equip1,player);
			
			
		break;
		case 2:
			player->role="Marine";
			player->job="Medic";
			
			player->intel += 1;		//job selection bonus
			player->totalIntel += 1;//job selection bonus
			
			ranged = new Actor(0,0,169,"MLR",TCODColor::white);
			bonusR = new ItemBonus(ItemBonus::DEXTERITY,1);
			ranged->blocks = false;
			ranged->pickable = new Equipment(0,Equipment::RANGED,bonusR,requirement);
			ranged->sort = 4;
			engine.actors.push(ranged);
			ranged->pickable->pick(ranged,player);
			((Equipment*)(ranged->pickable))->use(ranged,player);
			
			legs = new Actor(0,0,185,"Marine Fatigue Pants",TCODColor::white);
			bonusL = new ItemBonus(ItemBonus::HEALTH,0);
			legs->blocks = false;
			legs->pickable = new Equipment(0,Equipment::LEGS,bonusL,requirement);
			legs->sort = 3;
			engine.actors.push(legs);
			legs->pickable->pick(legs,player);
			((Equipment*)(legs->pickable))->use(legs,player);
			
			feet = new Actor(0,0,185,"Combat Boots",TCODColor::white);
			bonusF = new ItemBonus(ItemBonus::HEALTH,0);
			feet->blocks = false;
			feet->pickable = new Equipment(0,Equipment::FEET,bonusF,requirement);
			feet->sort = 3;
			engine.actors.push(feet);
			feet->pickable->pick(feet,player);
			((Equipment*)(feet->pickable))->use(feet,player);
			
			chest = new Actor(0,0,185,"Marine Medical Jacket",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);//makes health kits more useful?  or just a class powerup?
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
			
			//Give some medkits
			for(int i=0; i<4; i++){
				Actor *equip1 = new Actor(0,0,184,"Medkit", TCODColor::white);
				equip1->sort = 1;
				equip1->blocks = false;
				equip1->pickable = new Healer(20);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			break;
		case 3:
			player->role="Marine";
			player->job="Quartermaster";
			
			ranged = new Actor(0,0,169,"MLR",TCODColor::white);
			bonusR = new ItemBonus(ItemBonus::DEXTERITY,1);
			ranged->blocks = false;
			ranged->pickable = new Equipment(0,Equipment::RANGED,bonusR,requirement);
			ranged->sort = 4;
			engine.actors.push(ranged);
			ranged->pickable->pick(ranged,player);
			((Equipment*)(ranged->pickable))->use(ranged,player);
			
			legs = new Actor(0,0,185,"Marine Fatigue Pants",TCODColor::white);
			bonusL = new ItemBonus(ItemBonus::HEALTH,0);
			legs->blocks = false;
			legs->pickable = new Equipment(0,Equipment::LEGS,bonusL,requirement);
			legs->sort = 3;
			engine.actors.push(legs);
			legs->pickable->pick(legs,player);
			((Equipment*)(legs->pickable))->use(legs,player);
			
			feet = new Actor(0,0,185,"Combat Boots",TCODColor::white);
			bonusF = new ItemBonus(ItemBonus::HEALTH,0);
			feet->blocks = false;
			feet->pickable = new Equipment(0,Equipment::FEET,bonusF,requirement);
			feet->sort = 3;
			engine.actors.push(feet);
			feet->pickable->pick(feet,player);
			((Equipment*)(feet->pickable))->use(feet,player);
			
			chest = new Actor(0,0,185,"Marine Quarter-Master Jacket",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
			
			//starts with 4000 quarters        (...of a PBC)
			for(int i=0;i<10;i++){
				Actor *equip1 = new Actor(0,0,188,"PetaBitcoins",TCODColor::yellow);
				equip1->sort = 0;
				equip1->blocks = false;
				equip1->pickable = new Coinage(1,100);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			break;
		case 4:
			player->role="Explorer";
			player->job="Survivalist";
			
			player->vit += 20;
			player->destructible->maxHp += 20;
			player->destructible->hp += 20;
			
			chest = new Actor(0,0,185,"T-Shirt (red)",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
			
			//add flares, for now is generic flare
			for(int i=0; i<20; i++){
				equip1 = new Actor(0,0,' ',"Flare", TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Flare(10,5,5);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			for(int i=0; i<5; i++){
				Actor *equip1 = new Actor(0,0,181,"Flashbang", TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Confuser(10,8);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			//((Equipment*)(equip1->pickable))->use(equip1,player);
			
			break;
		case 5:
			player->role="Explorer";
			player->job="Pirate";
			
			piratesFound = 1; //enables more gold! Arrrrrrrrr!
			
			chest = new Actor(0,0,185,"Boarding Vest",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
			
			feet = new Actor(0,0,185,"Boarding Boots",TCODColor::white);
			bonusF = new ItemBonus(ItemBonus::HEALTH,0);
			feet->blocks = false;
			feet->pickable = new Equipment(0,Equipment::FEET,bonusF,requirement);
			feet->sort = 3;
			engine.actors.push(feet);
			feet->pickable->pick(feet,player);
			((Equipment*)(feet->pickable))->use(feet,player);
			
			//add flares, for now is generic flare
			for(int i=0; i<20; i++){
				equip1 = new Actor(0,0,' ',"Flare", TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Flare(10,5,5);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			break;
		case 6:
			player->role="Explorer";
			player->job="Merchant";
			
			player->vit -= 40;
			player->destructible->maxHp -= 40;
			player->destructible->hp -= 40;
			
			//add flare, for now is generic flare
			for(int i=0; i<20; i++){
				equip1 = new Actor(0,0,' ',"Flare", TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Flare(10,5,5);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			//get a flashbang
			for(int i=0; i<1; i++){
				Actor *equip1 = new Actor(0,0,181,"Flashbang", TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Confuser(10,8);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			//get a firebomb
			for(int i=0; i<1; i++){
				Actor *equip1 = new Actor(0,0,182,"Firebomb",TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Fireball(3,12,8);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			//get an EMP
			for(int i=0; i<1; i++){
				Actor *equip1 = new Actor(0,0,183, "EMP Pulse",TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new LightningBolt(5,20);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			//get a health pack
			for(int i=0; i<1; i++){
				Actor *equip1 = new Actor(0,0,184,"Medkit", TCODColor::white);
				equip1->sort = 1;
				equip1->blocks = false;
				equip1->pickable = new Healer(20);
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			//get a frag grenade
			for(int i=0; i<1; i++){
				Actor *equip1 = new Actor(0,0,'g',"Fragmentation Grenade",TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Fragment(3,12,8);
				equip1->pickable->value = 55;
				equip1->pickable->inkValue = 10;
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			break;
		case 7:
			player->role="Mercenary";
			player->job="Assassin";
			
			player->str += 8;
			player->totalStr += 8;
			player->attacker->basePower += 8;   //old
			player->attacker->totalPower += 8;  //old
			
			//cut HP by 2/3
			player->vit /= 3;
			player->destructible->maxHp = player->vit;
			player->destructible->hp = player->vit;
			
			player->dex += 6;
			player->totalDex += 6;
			
			player->intel += 4;
			player->totalIntel += 4;
			
			//get frag grenades
			for(int i=0; i<3; i++){
				Actor *equip1 = new Actor(0,0,'g',"Fragmentation Grenade",TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Fragment(3,12,8);
				equip1->pickable->value = 55;
				equip1->pickable->inkValue = 10;
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			legs = new Actor(0,0,185,"Skinsuit Leggings",TCODColor::white);
			bonusL = new ItemBonus(ItemBonus::HEALTH,0);
			legs->blocks = false;
			legs->pickable = new Equipment(0,Equipment::LEGS,bonusL,requirement);
			legs->sort = 3;
			engine.actors.push(legs);
			legs->pickable->pick(legs,player);
			((Equipment*)(legs->pickable))->use(legs,player);
			
			chest = new Actor(0,0,185,"Skinsuit Jacket",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
			
			helmet = new Actor(0,0,185,"Balaclava",TCODColor::white);
			bonusHe = new ItemBonus(ItemBonus::HEALTH,5);
			helmet->blocks = false;
			helmet->pickable = new Equipment(0,Equipment::HEAD,bonusHe,requirement);
			helmet->sort = 3;
			engine.actors.push(helmet);
			helmet->pickable->pick(helmet,player);
			((Equipment*)(helmet->pickable))->use(helmet,player);
			
			break;
		case 8:
			player->role="Mercenary";
			player->job="Brute";
			
			player->vit += 60;
			player->destructible->maxHp = player->vit;
			player->destructible->hp = player->vit;
			
			//get frag grenades
			for(int i=0; i<3; i++){
				Actor *equip1 = new Actor(0,0,'g',"Fragmentation Grenade",TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Fragment(3,12,8);
				equip1->pickable->value = 55;
				equip1->pickable->inkValue = 10;
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			hands = new Actor(0,0,185,"Bruiser Gloves",TCODColor::white);
			bonusHa = new ItemBonus(ItemBonus::HEALTH,5);
			hands->blocks = false;
			hands->pickable = new Equipment(0,Equipment::HAND1,bonusHa,requirement);
			hands->sort = 3;
			engine.actors.push(hands);
			hands->pickable->pick(hands,player);
			((Equipment*)(hands->pickable))->use(hands,player); 
			
			break;
		case 9:
			player->role="Mercenary";
			player->job="Hacker";
			
			player->vit -= 40;
			player->destructible->maxHp = player->vit;
			player->destructible->hp = player->vit;
			
			player->str -= 2;
			player->totalStr = player->str;
			player->attacker->basePower = player->str;   //old
			player->attacker->totalPower = player->str;  //old
			
			player->dex += 1;
			player->totalDex += 1;
			
			player->intel += 6;
			player->totalIntel += 6;
			
			//get frag grenades
			for(int i=0; i<3; i++){
				Actor *equip1 = new Actor(0,0,'g',"Fragmentation Grenade",TCODColor::white);
				equip1->sort = 2;
				equip1->blocks = false;
				equip1->pickable = new Fragment(3,12,8);
				equip1->pickable->value = 55;
				equip1->pickable->inkValue = 10;
				engine.actors.push(equip1);
				equip1->pickable->pick(equip1,player);
			}
			
			helmet = new Actor(0,0,185,"Tech Helmet",TCODColor::white);
			bonusHe = new ItemBonus(ItemBonus::HEALTH,5);
			helmet->blocks = false;
			helmet->pickable = new Equipment(0,Equipment::HEAD,bonusHe,requirement);
			helmet->sort = 3;
			engine.actors.push(helmet);
			helmet->pickable->pick(helmet,player);
			((Equipment*)(helmet->pickable))->use(helmet,player);
			
			chest = new Actor(0,0,185,"T-Shirt (grey)",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
			
			break;
		default:
			player->role="Marine";
			player->job="Infantry";
			
			player->dex+=3; //job selection bonus
			player->totalDex+=3; //job selection bonus
			

			ranged = new Actor(0,0,169,"MLR",TCODColor::white);
			bonusR = new ItemBonus(ItemBonus::DEXTERITY,1);
			ranged->blocks = false;
			ranged->pickable = new Equipment(0,Equipment::RANGED,bonusR,requirement);
			ranged->sort = 4;
			engine.actors.push(ranged);
			ranged->pickable->pick(ranged,player);
			((Equipment*)(ranged->pickable))->use(ranged,player);
			
			legs = new Actor(0,0,185,"Marine Fatigue Pants",TCODColor::white);
			bonusL = new ItemBonus(ItemBonus::HEALTH,0);
			legs->blocks = false;
			legs->pickable = new Equipment(0,Equipment::LEGS,bonusL,requirement);
			legs->sort = 3;
			engine.actors.push(legs);
			legs->pickable->pick(legs,player);
			((Equipment*)(legs->pickable))->use(legs,player);
			
			feet = new Actor(0,0,185,"Combat Boots",TCODColor::white);
			bonusF = new ItemBonus(ItemBonus::HEALTH,0);
			feet->blocks = false;
			feet->pickable = new Equipment(0,Equipment::FEET,bonusF,requirement);
			feet->sort = 3;
			engine.actors.push(feet);
			feet->pickable->pick(feet,player);
			((Equipment*)(feet->pickable))->use(feet,player);
			
			chest = new Actor(0,0,185,"Marine Fatigue Jacket",TCODColor::white);
			bonusC = new ItemBonus(ItemBonus::HEALTH,0);
			chest->blocks = false;
			chest->pickable = new Equipment(0,Equipment::CHEST,bonusC,requirement);
			chest->sort = 3;
			engine.actors.push(chest);
			chest->pickable->pick(chest,player);
			((Equipment*)(chest->pickable))->use(chest,player);
				
			helmet = new Actor(0,0,185,"Marine Ballistic Helmet",TCODColor::white);
			bonusHe = new ItemBonus(ItemBonus::HEALTH,0);
			helmet->blocks = false;
			helmet->pickable = new Equipment(0,Equipment::HEAD,bonusHe,requirement);
			helmet->sort = 3;
			engine.actors.push(helmet);
			helmet->pickable->pick(helmet,player);
			((Equipment*)(helmet->pickable))->use(helmet,player);
			
			equip1 = new Actor(0,0,182,"FireBomb", TCODColor::white);
			equip1->sort = 2;
			equip1->blocks = false;
			equip1->pickable = new Fireball(3,12,8);
			engine.actors.push(equip1);
			equip1->pickable->pick(equip1,player);
		break;
	}
	//set race and job back to 0
	engine.gui->raceSelection = 0;
	engine.gui->jobSelection = 0;
	engine.gui->roleSelection = 0;
	
	player->ch = plyrAscii;
	stairs = new Actor(0,0,'>', "stairs", TCODColor::white);
	stairs->blocks = false;
	actors.push(stairs);
	map = new Map(mapWidth, mapHeight);
	map->init(true, Param::GENERIC);
	gui->message(TCODColor::red, "Welcome to Astroverius Station! Warning unknown alien life form detected!");
	gui->message(TCODColor::blue,"You appear to be a %s %s %s. Your experience will be needed to complete this journey.", player->race, player->role, player->job);
	if (map->artifacts > 0) {
		engine.gui->message(TCODColor::orange,"The air hums with unknown energy... Perhaps there is an artifact of great power here!");
	}
	gameStatus = STARTUP;
}

void Engine::save() {
	if (player->destructible->isDead()) {
		TCODSystem::deleteFile("game.sav");
	} else {
		TCODZip zip;
		zip.putInt(level);
		zip.putInt(turnCount);
		zip.putInt(killCount);
		zip.putInt(invState);
		zip.putInt(menuState);
		zip.putInt(invFrames);
		zip.putInt(selX);
		zip.putInt(selY);
		//zip.putFloat(damageDone);
		//zip.putFloat(damageReceived);
		zip.putInt(piratesFound);
		//save the map first
		zip.putInt(map->width);
		zip.putInt(map->height);
		map->save(zip);
		//then the player
		player->save(zip);
		//then the stairs
		stairs->save(zip);
		playerLight->save(zip);
		//save the boss
		boss->save(zip);
		//then all the other actors
		zip.putInt(actors.size() - 4); //minus another one for boss actor?
		for (Actor **it = actors.begin(); it!=actors.end(); it++) {
			if (*it != player && *it != stairs && *it != playerLight && *it != boss) { //!= boss like stairs etc.?
				(*it)->save(zip);
			}
		}
		//finally the message log
		gui->save(zip);
		zip.saveToFile("game.sav");
	}
}

void Engine::load(bool pause) {
	engine.gui->menu.clear();
	if (pause) {
	engine.gui->menu.addItem(Menu::NO_CHOICE, "RESUME GAME");
	}
	if (!pause) {
	engine.gui->menu.addItem(Menu::NEW_GAME, "NEW GAME");
	}
	//  else {
	// engine.gui->menu.addItem(Menu::MAIN_MENU, "MAIN MENU");
	// }
	
	if (pause) {
		engine.gui->menu.addItem(Menu::SAVE, "SAVE");
	}
	if(TCODSystem::fileExists("game.sav")) {
		if (pause) {
		engine.gui->menu.addItem(Menu::CONTINUE, "LOAD");
		} else {
		engine.gui->menu.addItem(Menu::CONTINUE, "CONTINUE");
		}
	}
	engine.gui->menu.addItem(Menu::EXIT,"EXIT");
	
	Menu::MenuItemCode menuItem = engine.gui->menu.pick( 
		pause ? Menu::PAUSE : Menu::MAIN);
	
	if (menuItem == Menu::EXIT || menuItem == Menu::NONE) {
		//exit or window closed
		save();
		exit(0);
	} else if (menuItem == Menu::NEW_GAME) {
		//new game 
		engine.classMenu();
		//engine.term();
		//engine.init();
	} else if (menuItem == Menu::SAVE) {
		save();
	} else if (menuItem == Menu::NO_CHOICE) {
	} else if (menuItem == Menu::MAIN_MENU) {
		save();
		TCODConsole::root->clear();
		//engine.term();
		load(false);
	}else {
		TCODZip zip;
		//continue a saved game
		engine.term();
		zip.loadFromFile("game.sav");
		level = zip.getInt();
		turnCount = zip.getInt();
		killCount = zip.getInt();
		piratesFound = zip.getInt();
		//damageDone = zip.getFloat();
		//damageReceived = zip.getFloat();
		menuState = zip.getInt();
		invState = zip.getInt();
		invFrames = zip.getInt();
		selX = zip.getInt();
		selY = zip.getInt();
		//load the map
		int width = zip.getInt();
		int height = zip.getInt();
		map = new Map(width,height);
		map->load(zip);
		//then the player
		player = new Actor(0,0,0,NULL,TCODColor::white);
		player->load(zip);
		//the stairs
		stairs = new Actor(0,0,0,NULL,TCODColor::white);
		stairs->load(zip);
		//then the player's light
		playerLight = new Actor(0,0,0, NULL, TCODColor::white);
		playerLight->load(zip);
		//load the boss
		boss = new Actor(0,0,0, NULL, TCODColor::white);
		boss->load(zip);
		actors.push(player);
		actors.push(stairs);
		actors.push(playerLight);
		actors.push(boss);
		//actors.push(boss); //I push the boss to actorsList on line in Map.cpp so I don't need to push it here?
		//then all other actors
		int nbActors = zip.getInt();
		while (nbActors > 0) {
			Actor *actor = new Actor(0,0,0,NULL, TCODColor::white);
			actor->load(zip);
			actors.push(actor);
			nbActors--;
		}
		//finally, the message log
		gui->load(zip);
		gui->message(TCODColor::pink,"loaded");
		gameStatus = STARTUP;
	} 
}
	
void Engine::update() {
	if (gameStatus == STARTUP) map->computeFov();
	gameStatus = IDLE;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &lastKey, NULL); //delete the mouse stuff to remove mouse look (change &mouse to NULL)
	if (lastKey.vk == TCODK_ESCAPE) {
		//save();  why save automatically every time escape is called?
		load(true);
	} 
	player->update();
	if (map->isInfected(player->x, player->y)) {
		player->susceptible = true;
	}
	else {
		player->susceptible = false;
	}
	if (gameStatus == NEW_TURN){
		engine.turnCount++;
		for (Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
			Actor *actor = *iterator;
			if ( actor != player) {
				actor->update();
			}
		}
	}
}

float Engine::distance(int x1, int x2, int y1, int y2) {
	int dx = x1 - x2;
	int dy = y1 - y2;
	return sqrt(dx*dx+dy*dy);
}

void Engine::render()
{
	TCODConsole::root->clear();
	mapcon->clear();

	//draw the map
	map->render();
	TCODConsole::blit(mapcon, 0, 0, 0, 0, mapconCpy, 0, 0);
	//^this sets a lot of chars down on mapcon
	//currently renders the floors as ' ' space chars
	
	//render the floors!
	//we need to take the chars off mapcon and render them as the png's from floortile.png
//	rend->renderMap();
	//floors now rendered
	//WE MAY NEED TO SET A LOT OF KEYCOLORS FOR THE BLITS OF ITEMS AND SUCH
	
	//draw the actors
	for (Actor **iterator=actors.begin(); iterator != actors.end(); iterator++) {
		Actor *actor = *iterator;
		if ( actor != player) {
			if (map->isInFov(actor->x,actor->y)) {
				actor->render();
			} else if (map->isExplored(actor->x,actor->y)) {
				TCODColor tmpcol = actor->col;
				actor->col = TCODColor::lightGrey;
				actor->render();
				actor->col = tmpcol;
			}
		}
	}
	//^this blits the actors onto the mapcon as chars
	
	player->render();
	//show the player's stats
	
	gui->render();
	//^blits the sidebars onto the console
	
	
	//int mapx1 = 0, mapy1 = 0, mapy2 = 0, mapx2 = 0;
	
	mapx1 = player->x - ((screenWidth -22)/2);
	mapy1 = player->y - ((screenHeight -12)/2);
	mapx2 = player->x + ((screenWidth -22)/2);
	mapy2 = player->y + ((screenHeight -12)/2);
	//engine.gui->message(TCODColor::red, "player x  %d",player->x);
	//engine.gui->message(TCODColor::red, "player y  %d",player->y);
	//engine.gui->message(TCODColor::red, "mapy1 %d",mapy1);
	//engine.gui->message(TCODColor::red, "mapy2  %d",mapy2);
	
	if (mapx1 <= 0) {// <= lets it catch the time when it needs to stop scrolling
		mapx2 += (0-mapx1);
		mapx1 = 0;
		mapx2 += 1; //allows it to render the whole screen
	}	
	if (mapy1 <= 0) { 
		mapy2 += (0-mapy1);
		mapy1 = 0;
		mapy2 += 1;
	}
	if (mapx2 >= 100) {
		mapx1 += (100-mapx2);
		//gui->message(TCODColor::green, "******************************************");
		mapx2 = 100;
		mapx1 -= 1;
	}
	if (mapy2 >= 101) {
		mapy1 += (101-mapy2);
		mapy2 = 101;
		mapy1 -= 1;
	}
	 /*
	 //non hard coded values
	 //worked with 100 by 100, don't know if this would work besides that
	if (mapx2 > mapWidth) {
		mapx1 += (mapWidth-mapx2);
		mapx2 = mapWidth;
	}
	if (mapy2 > mapHeight) {
		mapy1 += (mapHeight-mapy2);
		mapy2 = mapHeight;
	}
	*/
	
	
	
	
	
	//stops the map from spilling into the console
	int mapy2a = mapy2;
	if (mapy2a > TCODConsole::root->getHeight() - 12) mapy2a = TCODConsole::root->getHeight() - 12;
	//gui->message(TCODColor::red, "y2a is %d",mapy2a);
	//need to make a list of '.' under other chars, that there would be a difference between mapcon and mapconCpy
	//then need to make some sort of flag
	
	
	//blitting of the map onto the screen...maybe blit onto temp root copy, then render and blit back
	TCODConsole::blit(mapcon, mapx1, mapy1, mapx2, mapy2a, 
		TCODConsole::root, 22, 0);
	
	
	//the comment below is the old gui code
	/* TCODConsole::root->print(1, screenHeight-2, "HP: %d/%d", 
	(int)player->destructible->hp,(int)player->destructible->maxHp); */
}

void Engine::nextLevel() {
	level++;
	
	gui->message(TCODColor::lightViolet, "Sitting at the top of the stairs, you take a brief moment to rest...");
	player->destructible->heal(player->destructible->maxHp/2);
	gui->message(TCODColor::red,"Gathering your courage, you rush down the dungeon stairs, mindful that greater dangers may lurk below...");
	
	delete map;
	//delete all actors but player and stairs
	for(Actor **it = actors.begin(); it != actors.end(); it++) {
		if (*it != player && *it != stairs) {
			delete *it;
			it = actors.remove(it);
		}
	}
	//engine.mapconDec->clear();
	//create a new map
	map = new Map(mapWidth,mapHeight);
	map->init(true, Param::GENERIC);
	gameStatus = STARTUP;
	save();
	if (map->artifacts > 0) {
		engine.gui->message(TCODColor::red,"The air hums with unknown energy... Perhaps there is an artifact of great power here!");
	}
}

void Engine::sendToBack(Actor *actor) {
	actors.remove(actor);
	actors.insertBefore(actor,0);
}

Actor *Engine::getClosestMonster(int x, int y, float range) const {
	Actor *closest = NULL;
	float bestDistance = 1E6f;
	
	for (Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor != player && actor->destructible 
			&& !actor->destructible->isDead()) {
			float distance = actor->getDistance(x,y);
			if (distance < bestDistance && (distance <= range || range ==0.0f)) {
				bestDistance = distance;
				closest = actor;
			}
		}
	}
	return closest;
}

bool Engine::pickATile(int *x, int *y, float maxRange, float AOE) {   //need to make middle tile unique 
	while (!TCODConsole::isWindowClosed()) {
		int dx = 0, dy = 0;
		render();
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&lastKey,NULL);
		switch (lastKey.vk) {
		case TCODK_UP: dy = -1; break;
		case TCODK_DOWN: dy = 1; break;
		case TCODK_LEFT: dx =-1; break;
		case TCODK_RIGHT: dx = 1; break;
		case TCODK_ENTER: 
		{
			if ((player->getDistance(*x,*y) > maxRange && maxRange != 0)) {
				gui->message(TCODColor::pink,"this tile is out of range!");
				return false;
			} else {
				return true;
			}
		}
		case TCODK_ESCAPE: return false;
		default: break;
		}
		*x += dx;
		*y += dy;
		
		if (*x > 99) *x = 99;
		if (*x < 0) *x = 0;
		if (*y > 99) *y = 99;
		if (*y < 0) *y = 0;
		
		//things with AOE > 1 need to have a more unique middle color so i can render them
		if (AOE > 1.0 )
		{
			//make center unique
			for (int i = 0; i < map->height; i++) {
				for (int j = 0; j < map->width; j++) {
					if ( distance(*x,j,*y,i) <= AOE ) {
						if (distance(*x,j,*y,i) < 1.0) {//middle
							mapcon->setCharBackground(j,i,TCODColor::darkerPink);
							
						}
						else if ( distance(*x,player->x,*y,player->y) >= maxRange && maxRange != 0) {
							mapcon->setCharBackground(j,i,TCODColor::desaturatedPink);
						
						} else {
							mapcon->setCharBackground(j,i,TCODColor::pink);
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < map->height; i++) {
				for (int j = 0; j < map->width; j++) {
					if ( distance(*x,j,*y,i) <= AOE ) {
						if ( distance(*x,player->x,*y,player->y) >= maxRange && maxRange != 0) {
							mapcon->setCharBackground(j,i,TCODColor::desaturatedPink);
						} else {
							mapcon->setCharBackground(j,i,TCODColor::pink);
						}
					}
				}
			}
		}
	//int mapx1 = 0, mapy1 = 0, mapy2 = 0, mapx2 = 0;
	mapx1 = *x - ((screenWidth -22)/2);
	mapy1 = *y - ((screenHeight -12)/2);
	mapx2 = *x + ((screenWidth -22)/2);
	mapy2 = *y + ((screenHeight -12)/2);
	
	
	if (mapx1 <= 0) {// <= lets it catch the time when it needs to stop scrolling
		mapx2 += (0-mapx1);
		mapx1 = 0;
		mapx2 += 1; //allows it to render the whole screen
	}	
	if (mapy1 <= 0) { 
		mapy2 += (0-mapy1);
		mapy1 = 0;
		mapy2 += 1;
	}
	if (mapx2 >= 100) {
		mapx1 += (100-mapx2);
		//gui->message(TCODColor::green, "******************************************");
		mapx2 = 100;
		mapx1 -= 1;
	}
	if (mapy2 >= 101) {
		mapy1 += (101-mapy2);
		mapy2 = 101;
		mapy1 -= 1;
	}
	//stops the map from spilling into the console
	int mapy2a = mapy2;
	if (mapy2a > TCODConsole::root->getHeight() - 12) mapy2a = TCODConsole::root->getHeight() - 12;
	//gui->message(TCODColor::red, "y2a is %d",mapy2a);
	//need to make a list of '.' under other chars, that there would be a difference between mapcon and mapconCpy
	//then need to make some sort of flag
	
	
	//blitting of the map onto the screen...maybe blit onto temp root copy, then render and blit back
	TCODConsole::blit(mapcon, mapx1, mapy1, mapx2, mapy2a, 
		TCODConsole::root, 22, 0);
		
	TCODConsole::flush();
		
	} 
	return false;
}

bool Engine::pickATileForInfoScreen(int *x, int *y, float maxRange, float AOE) {   //need to make middle tile unique 
		int dx = 0, dy = 0;
		render();
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&lastKey,NULL);
		switch (lastKey.vk) {
		case TCODK_UP: dy = -1; break;
		case TCODK_DOWN: dy = 1; break;
		case TCODK_LEFT: dx =-1; break;
		case TCODK_RIGHT: dx = 1; break;
		case TCODK_ENTER: 
		{
			if ((player->getDistance(*x,*y) > maxRange && maxRange != 0)) {
				gui->message(TCODColor::pink,"this tile is out of range!");
				return false;
			} else {
				return true;
			}
		}
		case TCODK_ESCAPE: return false;
		default: break;
		}
		*x += dx;
		*y += dy;
		
		if (*x > 99) *x = 99;
		if (*x < 0) *x = 0;
		if (*y > 99) *y = 99;
		if (*y < 0) *y = 0;
		
		//things with AOE > 1 need to have a more unique middle color so i can render them
		if (AOE > 1.0 )
		{
			//make center unique
			for (int i = 0; i < map->height; i++) {
				for (int j = 0; j < map->width; j++) {
					if ( distance(*x,j,*y,i) <= AOE ) {
						if (distance(*x,j,*y,i) < 1.0) {//middle
							mapcon->setCharBackground(j,i,TCODColor::darkerPink);
							
						}
						else if ( distance(*x,player->x,*y,player->y) >= maxRange && maxRange != 0) {
							mapcon->setCharBackground(j,i,TCODColor::desaturatedPink);
						
						} else {
							mapcon->setCharBackground(j,i,TCODColor::pink);
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < map->height; i++) {
				for (int j = 0; j < map->width; j++) {
					if ( distance(*x,j,*y,i) <= AOE ) {
						if ( distance(*x,player->x,*y,player->y) >= maxRange && maxRange != 0) {
							mapcon->setCharBackground(j,i,TCODColor::desaturatedPink);
						} else {
							mapcon->setCharBackground(j,i,TCODColor::pink);
						}
					}
				}
			}
		}
	//int mapx1 = 0, mapy1 = 0, mapy2 = 0, mapx2 = 0;
	mapx1 = *x - ((screenWidth -22)/2);
	mapy1 = *y - ((screenHeight -12)/2);
	mapx2 = *x + ((screenWidth -22)/2);
	mapy2 = *y + ((screenHeight -12)/2);
	
	
	if (mapx1 <= 0) {// <= lets it catch the time when it needs to stop scrolling
		mapx2 += (0-mapx1);
		mapx1 = 0;
		mapx2 += 1; //allows it to render the whole screen
	}	
	if (mapy1 <= 0) { 
		mapy2 += (0-mapy1);
		mapy1 = 0;
		mapy2 += 1;
	}
	if (mapx2 >= 100) {
		mapx1 += (100-mapx2);
		//gui->message(TCODColor::green, "******************************************");
		mapx2 = 100;
		mapx1 -= 1;
	}
	if (mapy2 >= 101) {
		mapy1 += (101-mapy2);
		mapy2 = 101;
		mapy1 -= 1;
	}
	//stops the map from spilling into the console
	int mapy2a = mapy2;
	if (mapy2a > TCODConsole::root->getHeight() - 12) mapy2a = TCODConsole::root->getHeight() - 12;
	//gui->message(TCODColor::red, "y2a is %d",mapy2a);
	//need to make a list of '.' under other chars, that there would be a difference between mapcon and mapconCpy
	//then need to make some sort of flag
	
	
	//blitting of the map onto the screen...maybe blit onto temp root copy, then render and blit back
	TCODConsole::blit(mapcon, mapx1, mapy1, mapx2, mapy2a, 
		TCODConsole::root, 22, 0);
		
	TCODConsole::flush();
		
	return false;
}

Actor *Engine::getActor(int x, int y) const {
	for (Actor **it = actors.begin(); it != actors.end(); it++) {
		Actor *actor = *it;
		if (actor->x ==x && actor->y==y && actor->destructible &&
			!actor->destructible->isDead()) {
			return actor;
		}
	}
	return NULL;
}

Actor *Engine::getAnyActor(int x, int y) const {
	for (Actor **it = actors.begin(); it != actors.end(); it++) {
		Actor *actor = *it;
		if (actor->x ==x && actor->y == y) {
			return actor;
		}
	}
	return NULL;
}

void Engine::win() {
	gui->message(TCODColor::darkRed,"You win!");
	gameStatus=Engine::VICTORY;
}
void Engine::classMenu(){
	engine.gui->statPoints = 2;
	engine.gui->conValue = 100;
	engine.gui->strValue = 5;
	engine.gui->agValue = 3;
	engine.gui->intelValue = 3;
	engine.gui->menu.clear();
	engine.gui->menu.addItem(Menu::RACE, "RACE");
	engine.gui->menu.addItem(Menu::CLASS, "CLASS");
	engine.gui->menu.addItem(Menu::SUB_CLASS, "SUBCLASS");
	engine.gui->menu.addItem(Menu::STATS, "STATS");
	engine.gui->menu.addItem(Menu::EXIT, "DONE");
	bool choice = true;
			while(choice){
			engine.gui->classSidebar();
			
			Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::CLASS_MENU);
			//Menu::MenuItemCode selection;
				switch (menuItem) {
					case Menu::RACE :
						classSelectMenu(1);
						break;
					case Menu::CLASS :
						classSelectMenu(2);
						break;
					case Menu::SUB_CLASS:
						classSelectMenu(3);
						break;
					case Menu::STATS:
						classSelectMenu(4);
						break;
					case Menu::NO_CHOICE:
						break;
					case Menu::EXIT:
						choice = false;
						engine.term();
						engine.init();
					default: break;
				}
			}
}
void Engine::classSelectMenu(int cat){
if(cat == 1){
	engine.gui->classMenu.clear();
	engine.gui->classMenu.addItem(Menu::HUMAN, "HUMAN");
	engine.gui->classMenu.addItem(Menu::ROBOT, "ROBOT");
	engine.gui->classMenu.addItem(Menu::ALIEN, "ALIEN");
	bool choice = true;
	while(choice){
				Menu::MenuItemCode menuItem = engine.gui->classMenu.pick(Menu::CLASS_SELECT);
				
					switch (menuItem) {
						case Menu::HUMAN :
							engine.gui->raceSelection = 1;
							choice = false;
							break;
						case Menu::ROBOT :
							engine.gui->raceSelection = 2;
							choice = false;
							break;
						case Menu::ALIEN :
							engine.gui->raceSelection = 3;
							choice = false;
							break;
						case Menu::NO_CHOICE:
							choice = false;
							break;
						default: break;
					}
	}

}else if(cat == 2){
	engine.gui->classMenu.clear();
	engine.gui->classMenu.addItem(Menu::MARINE, "MARINE");
	engine.gui->classMenu.addItem(Menu::EXPLORER, "EXPLORER");
	engine.gui->classMenu.addItem(Menu::MERCENARY, "MERCENARY");
	bool choice = true;
	while(choice){
				Menu::MenuItemCode menuItem = engine.gui->classMenu.pick(Menu::CLASS_SELECT);
				
					switch (menuItem) {
						case Menu::MARINE :
							engine.gui->roleSelection = 1;
							engine.gui->jobSelection = 1;
							choice = false;
							break;
						case Menu::EXPLORER :
							engine.gui->roleSelection = 2;
							engine.gui->jobSelection = 4;
							choice = false;
							break;
						case Menu::MERCENARY :
							engine.gui->roleSelection = 3;
							engine.gui->jobSelection = 7;
							choice = false;
							break;
						case Menu::NO_CHOICE:
							choice = false;
							break;
						default: break;
					}
	}
}else if(cat == 3){
	engine.gui->classMenu.clear();
	if(engine.gui->roleSelection == 1){
		engine.gui->classMenu.addItem(Menu::INFANTRY, "INFANTRY");
		engine.gui->classMenu.addItem(Menu::MEDIC, "MEDIC");
		engine.gui->classMenu.addItem(Menu::QUARTERMASTER, "QUARTERMASTER");
	}else if(engine.gui->roleSelection == 2){
		engine.gui->classMenu.addItem(Menu::SURVIVALIST, "SURVIVALIST");
		engine.gui->classMenu.addItem(Menu::PIRATE, "PIRATE");
		engine.gui->classMenu.addItem(Menu::MERCHANT, "MERCHANT");
	}else if(engine.gui->roleSelection == 3){
		engine.gui->classMenu.addItem(Menu::ASSASSIN, "ASSASSIN");
		engine.gui->classMenu.addItem(Menu::BRUTE, "BRUTE");
		engine.gui->classMenu.addItem(Menu::HACKER, "HACKER");
	}else{
		engine.gui->classMenu.addItem(Menu::EXIT,"CHOOSE A CLASS");
	}
	bool choice = true;
	while(choice){
				Menu::MenuItemCode menuItem = engine.gui->classMenu.pick(Menu::CLASS_SELECT);
				
					switch (menuItem) {
						case Menu::INFANTRY :
							engine.gui->jobSelection = 1;
							choice = false;
							break;
						case Menu::MEDIC :
							engine.gui->jobSelection = 2;
							choice = false;
							break;
						case Menu::QUARTERMASTER :
							engine.gui->jobSelection = 3;
							choice = false;
							break;
						case Menu::SURVIVALIST :
							engine.gui->jobSelection = 4;
							choice = false;
							break;
						case Menu::PIRATE :
							engine.gui->jobSelection = 5;
							choice = false;
							break;
						case Menu::MERCHANT :
							engine.gui->jobSelection = 6;
							choice = false;
							break;
						case Menu::ASSASSIN :
							engine.gui->jobSelection = 7;
							choice = false;
							break;
						case Menu::BRUTE :
							engine.gui->jobSelection = 8;
							choice = false;
							break;
						case Menu::HACKER :
							engine.gui->jobSelection = 9;
							choice = false;
							break;
						case Menu::EXIT :
							choice = false;
							break;
						case Menu::NO_CHOICE:
							choice = false;
							break;
						default: break;
					}
	}
}else{
	engine.gui->classMenu.clear();
	engine.gui->classMenu.addItem(Menu::VITALITY, "VITALITY");
	engine.gui->classMenu.addItem(Menu::STRENGTH, "STRENGTH");
	engine.gui->classMenu.addItem(Menu::DEXTERITY, "DEXTERITY");
	engine.gui->classMenu.addItem(Menu::INTELLIGENCE, "INTELLIGENCE");
	engine.gui->classMenu.addItem(Menu::RESET,"RESET SELECTIONS");
	engine.gui->classMenu.addItem(Menu::EXIT, "DONE");
	bool choice = true;
	while(choice){
				Menu::MenuItemCode menuItem = engine.gui->classMenu.pick(Menu::CLASS_SELECT);
				
					switch (menuItem) {
						case Menu::VITALITY :
							if(engine.gui->statPoints == 0)
								choice = false;
							else{
								engine.gui->statPoints = engine.gui->statPoints - 1;
								engine.gui->conValue += 20;
								engine.gui->classSidebar();
							}
							break;
						case Menu::STRENGTH :
							if(engine.gui->statPoints == 0)
								choice = false;
							else{
								engine.gui->statPoints = engine.gui->statPoints - 1;
								engine.gui->strValue += 1;
								engine.gui->classSidebar();
							}
							break;
						case Menu::DEXTERITY :
							if(engine.gui->statPoints == 0)
								choice = false;
							else{
								engine.gui->statPoints = engine.gui->statPoints - 1;
								engine.gui->agValue += 1;
								engine.gui->classSidebar();
							}
							break;
						case Menu::INTELLIGENCE :
							if(engine.gui->statPoints == 0)
								choice = false;
							else{
								engine.gui->statPoints = engine.gui->statPoints - 1;
								engine.gui->intelValue += 1;
								engine.gui->classSidebar();
							}
							break;
						case Menu::EXIT :
							choice = false;
							break;
						case Menu::RESET:
							engine.gui->statPoints = 2;
							engine.gui->conValue = 100;
							engine.gui->strValue = 5;
							engine.gui->agValue = 3;
							engine.gui->intelValue = 3;
							engine.gui->classSidebar();
							break;
						case Menu::NO_CHOICE:
							choice = false;
							break;
						default: break;
					}
	}
}
}
