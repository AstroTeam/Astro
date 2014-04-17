#include <stdio.h>
#include <math.h>
#include "main.hpp"
#include <string>
Ai *Ai::create(TCODZip &zip) {
	AiType type = (AiType)zip.getInt();
	std::cout << "got AITYPE" << type << std::endl;
	Ai *ai = NULL;
	switch(type) {
		case PLAYER: ai = new PlayerAi(); break;
		case MONSTER: ai = new MonsterAi(); break;
		case CONFUSED_ACTOR: ai = new ConfusedActorAi(0,NULL); break;
	    case EPICENTER: ai = new EpicenterAi(); break;
		case RANGED: ai = new RangedAi(); break;
		case LIGHT: ai = new LightAi(0,0); break;
		case FLARE: ai = new FlareAi(0,0); break;
		case GRENADIER: ai = new GrenadierAi(); break;
		case TURRET: ai = new TurretAi(); break;
		case CLEANER: ai = new CleanerAi(); break;
		case INTERACTIBLE: ai = new InteractibleAi(); break;
		
		case CONSOLE: ai = new ConsoleAi(); break;
		
		case VENDING: ai = new VendingAi(); break;
		case ENGINEER: ai = new EngineerAi(5,5); break;
	    case TRIGGER: ai = new TriggerAi(); break;
		case SECURITY: ai = new SecurityBotAi(); break;
		case TURRETCONTROL: ai = new TurretControlAi(); break;
		case LOCKER: ai = new LockerAi(); break;
		case GARDNER: ai = new GardnerAi(); break;
		case FRUIT: ai = new FruitAi(NULL,0); break;
		case ZED: ai = new ZedAi(); break;
		case COMPANION: std::cout<<"got here"<<std::endl; ai = new CompanionAi(NULL,0); break;
		
	}
	std::cout << "got past switch " << std::endl;
	ai->load(zip);
	return ai;
}
Actor *Ai::choseFromInventory(Actor *owner,int type, bool isVend) {
	static const int INVENTORY_WIDTH = 38;
	static const int INVENTORY_HEIGHT = 28;
	inventoryScreen = new TCODConsole(INVENTORY_WIDTH, INVENTORY_HEIGHT);
	
	//display the inventory frame
	inventoryScreen->setDefaultForeground(TCODColor(100,180,250));
	inventoryScreen->printFrame(0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,true,TCOD_BKGND_DEFAULT);
	
	//display the items with their keyboard shortcut
	inventoryScreen->setDefaultForeground(TCODColor::white);
	int shortcut = 'a';
	int y = 1;
	for (Actor **it = owner->container->inventory.begin();
		it != owner->container->inventory.end(); it++) {
		Actor *actor = *it;
		if(actor->sort == type){
			if((actor->pickable->type == Pickable::EQUIPMENT || actor->pickable->type == Pickable::WEAPON)&& ((Equipment*)(actor->pickable))->equipped){
				inventoryScreen->print(1,y,"(%c) %s(E)",shortcut,actor->name);
			}else{
				inventoryScreen->print(1,y,"(%c) %s",shortcut,actor->name);
			}
			owner->container->select[shortcut] = actor->name;
			if (actor->pickable->stacks) {
				if(isVend){
					inventoryScreen->print(22, y, "Pbc: %d Ink: %d",actor->pickable->value,actor->pickable->inkValue);
				}else{
					inventoryScreen->print(24, y, "(%d)",actor->pickable->stackSize);
				}
			}else if(isVend){
				inventoryScreen->print(22, y, "Pbc:%d Ink:%d",actor->pickable->value,actor->pickable->inkValue);
			}
			y++;
			shortcut++;
		}
	}
	//blit the inventory console on the root console
	if(isVend){
		TCODConsole::blit(inventoryScreen,0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,
			TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2 + 20,
			engine.screenHeight/2 - INVENTORY_HEIGHT/2 - 3);
	}else{
		TCODConsole::blit(inventoryScreen,0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,
			TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
			engine.screenHeight/2 - INVENTORY_HEIGHT/2 + 1);
	}
	TCODConsole::flush();
	
	//wait for a key press
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
	
	if (key.vk == TCODK_CHAR) {
		if(owner->container->select[key.c]){
			int index = 0;
			for(Actor **it = owner->container->inventory.begin(); it != owner->container->inventory.end(); ++it){
				Actor *actor = *it;
				if((index >= key.c - 'a') && strcmp(actor->name,owner->container->select[key.c]) == 0){
					engine.gui->message(TCODColor::grey, "You picked the %s",actor->name);
					owner->container->select.clear();
					return actor;
				}
				index++;
			}
		}
	}
	return NULL;
}

PlayerAi::PlayerAi() : xpLevel(1) {
}

const int LEVEL_UP_BASE = 20;
const int LEVEL_UP_FACTOR = 15;

int PlayerAi::getNextLevelXp() {
	return LEVEL_UP_BASE + xpLevel * LEVEL_UP_FACTOR;
}
	
void PlayerAi::load(TCODZip &zip) {
	xpLevel = zip.getInt();
}

void PlayerAi::save(TCODZip &zip) {
	zip.putInt(PLAYER);
	zip.putInt(xpLevel);
}

void PlayerAi::update(Actor *owner) {

	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);


	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	
	int levelUpXp = getNextLevelXp();
	if (owner->destructible->xp >= levelUpXp) {
		bool choice_made = false, first = true;
		while (!choice_made) {
		if (first) {
			engine.gui->message(TCODColor::yellow,"As the monster falls to the floor, you feel yourself filled with some unknown strength!");
			TCODConsole::flush();
			xpLevel++;
			owner->destructible->xp -= levelUpXp;
		}
		engine.gui->menu.clear();
		engine.gui->menu.addItem(Menu::CONSTITUTION, "Vitality (bonus health)");
		engine.gui->menu.addItem(Menu::STRENGTH, "Strength (melee damage)");
		engine.gui->menu.addItem(Menu::DEXTERITY, "Dexterity (ranged damage)");
		engine.gui->menu.addItem(Menu::INTELLIGENCE, "Intelligence (tech damage)");
		//engine.gui->menu.addItem(Menu::AGILITY, "Agility (+1 defence)");
		Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::LEVELUP);
	
		
		switch (menuItem) {
			case Menu::CONSTITUTION	:
				owner->destructible->maxHp += owner->getHpUp();
				owner->destructible->hp += owner->getHpUp();
				owner->vit += owner->getHpUp();;
				choice_made = true;
				break;
			case Menu::STRENGTH :
				owner->attacker->basePower += 1;
				owner->attacker->totalPower += 1;
				owner->str += 1;
				owner->totalStr += 1;
				choice_made = true;
				break;
			case Menu::DEXTERITY :
				owner->dex += 1;
				owner->totalDex += 1;
				choice_made = true;
				break;
			case Menu::INTELLIGENCE :
				owner->intel += 1;
				owner->totalIntel += 1;
				choice_made = true;
				break;
			case Menu::AGILITY:
				owner->destructible->baseDodge += 1;
				owner->destructible->totalDodge += 1;
			/*case Menu::AGILITY:
				owner->destructible->defense += 1;
				choice_made = true;
				break;*/
			case Menu::NO_CHOICE:
				first = false;
				break;
			default: break;
		}
		}
	}
	engine.player->lastX = engine.player->x;
	engine.player->lastY = engine.player->y;
	int dx =0, dy =0;
	switch (engine.lastKey.vk) {
	case TCODK_UP: case TCODK_KP8: dy = -1; break;
	case TCODK_DOWN: case TCODK_KP2: dy = 1; break;
	case TCODK_LEFT: case TCODK_KP4: dx =-1; break;
	case TCODK_RIGHT: case TCODK_KP6: dx = 1; break;
	case TCODK_KP7: case TCODK_7: dy = dx = -1; break;
	case TCODK_KP9: case TCODK_9: dy = -1; dx = 1; break;
	case TCODK_KP1: case TCODK_1: dx = -1; dy = 1; break;
	case TCODK_KP3: case TCODK_3: dx = dy = 1; break;
	case TCODK_TAB: 
		engine.save();
		engine.gui->message(TCODColor::pink, "saved"); break;
	/* case TCODK_CONTROL: 	//cheat mode teleport to stairs for debug
		engine.player->x = engine.stairs->x;
		engine.playerLight->x = engine.stairs->x;
		engine.player->y = engine.stairs->y;
		engine.playerLight->y = engine.stairs->y; 
		
		engine.map->computeFov(); break;*/
		
	case TCODK_PRINTSCREEN:
		TCODSystem::saveScreenshot(NULL);
		engine.gui->message(TCODColor::orange,"screenshot saved"); break;
	case TCODK_KP5: engine.map->computeFov(); engine.gameStatus = Engine::NEW_TURN; owner->destructible->takeFireDamage(owner, 3.0); /*engine.gui->message(TCODColor::white,"fireDmg");*/ break;
	case TCODK_CHAR: handleActionKey(owner, engine.lastKey.c); break;
	default: break;
	}
	if (dx != 0 || dy != 0) {
		engine.gameStatus = Engine::NEW_TURN;
		if (moveOrAttack(owner, owner->x+dx, owner->y+dy)) {
			engine.map->computeFov();
		}
	}
	
}

bool PlayerAi::moveOrAttack(Actor *owner, int targetx, int targety) {
	if (engine.map->isWall(targetx, targety) )
	{
		//owner->destructible->takeFireDamage(owner, 3.0);
		//engine.gui->message(TCODColor::white,"fireDmg");
		return false;
	}
	if (!owner->attacker)
	{
		//owner->destructible->takeFireDamage(owner, 3.0);
		//engine.gui->message(TCODColor::white,"fireDmg");
		return false;
	}
	
	
	
	for (Actor **iterator = engine.actors.begin();
		iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor->blocks && actor->x == targetx &&actor->y == targety) {
			if (actor->destructible && !actor->destructible->isDead() ) {
				if ((actor->hostile || owner->hostile) && (engine.map->tiles[actor->x+(actor->y)*engine.map->width].decoration != 56 && engine.map->tiles[actor->x+(actor->y)*engine.map->width].decoration != 57 )){
					owner->attacker->attack(owner, actor);
					if(!actor->hostile && actor->ch == 129) //currently this only applies to security bots, if the player attacks a nonhostile enemy, should that actor generally become hostile?
					{
						actor->hostile = true;
						actor->ch = 130; //update to active security bot
					}else if(!actor->hostile && actor->ch == 'G') //gardner become hostile
						actor->hostile = true;
					engine.damageDone += owner->attacker->totalPower - actor->destructible->totalDodge;
				}else if(actor->interact && ((!owner->hostile) || (engine.map->tiles[actor->x+(actor->y)*engine.map->width].decoration == 56 || engine.map->tiles[actor->x+(actor->y)*engine.map->width].decoration == 57 )))
					((InteractibleAi*)actor->ai)->interaction(actor, owner);
				else if(!owner->hostile && !actor->hostile && actor->ch == 129)
					engine.gui->message(TCODColor::grey, "The %s seems to be inactive", actor->name);
				else if(!owner->hostile && !actor->hostile && actor->ch == 'G')
					engine.gui->message(TCODColor::grey, "Welcome to Hydroponics, please do not touch anything!", actor->name);
			}else if(actor->interact){
					((InteractibleAi*)actor->ai)->interaction(actor, owner);
			}
			//attacking something like a generator that doesn't have a destructible or something
			owner->destructible->takeFireDamage(owner, 3.0);
			//engine.gui->message(TCODColor::white,"fireDmg");
			return false;
		}
	}
	/* //display everything you step over
	for (Actor **iterator = engine.actors.begin();
		iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		bool corpseOrItem = (actor->destructible && actor->destructible->isDead())
			|| actor->pickable;
			
		if (corpseOrItem
			&& actor->x == targetx && actor->y == targety) {
			engine.gui->message(TCODColor::white,"There's a %s here\n", actor->name);
		}
		
	} */
	
	
	
	owner->x = targetx;
	engine.playerLight->x = targetx;
	owner->y = targety;
	engine.playerLight->y = targety;
	
	owner->destructible->takeFireDamage(owner, 3.0);

	//engine.gui->message(TCODColor::white,"fireDmg");
	int level = engine.map->infectionState(owner->x, owner->y); 

	if (level > 2) {
		Aura *aura1 = new Aura(2,Aura::HEALTH,Aura::CONTINUOUS,-1);
		aura1->apply(owner);
		owner->auras.push(aura1);

		if (level > 3) {
			Aura *aura2 = new Aura(2,Aura::TOTALINTEL,Aura::CONTINUOUS,-3);
			aura2->apply(owner);
			owner->auras.push(aura2);

			engine.gui->message(TCODColor::green, "You have a headache.");
			if (level > 4) {
				Aura *aura3 = new Aura(2,Aura::TOTALDEX,Aura::CONTINUOUS,-3);
				aura3->apply(owner);
				owner->auras.push(aura3);
				engine.gui->message(TCODColor::green, "It's hard to move.");
			}
		}

		engine.gui->message(TCODColor::green, "The moss saps your health.");
	}
	return true;
}


void PlayerAi::handleActionKey(Actor *owner, int ascii) {
	//bool first = true;
	bool invSkip = false;
	switch(ascii) {
		case 'g': //pickup the item
		{
			engine.map->computeFov();
			bool found = false;
			for (Actor **iterator = engine.actors.begin();
				iterator != engine.actors.end(); iterator++) {
				Actor *actor = *iterator;
				if (actor->pickable && actor->x == owner->x && actor->y == owner->y) {
					if (actor->pickable->pick(actor,owner)) {
						found = true;
						if (actor->pickable->stacks) {
							engine.gui->message(TCODColor::lightGrey, "You pick up %d %s.", actor->pickable->stackSize, actor->name);
						} else {
							engine.gui->message(TCODColor::lightGrey, "You pick up %s.", actor->name);
						}
						break;
					} else if (!found ) {
						found = true;
						engine.gui->message(TCODColor::red, "Your inventory is full.");
					}
				}
				//exception for terminal's to replay their message
				if ((actor->ch == 227 || actor->ch == 228) && actor->x == owner->x && actor->y == owner->y)
				{
					found = true;
					TriggerAi* t = (TriggerAi*)actor->ai;
					t->pressed = false;
					engine.gui->message(TCODColor::lightGrey,"The terminal replayed its message.");
					engine.ctrTer -= 1;
					actor->ai->update(actor);
				}
			}
			if (!found) {
					engine.gui->message(TCODColor::lightGrey,"There's nothing interesting here.");
			}
			if (engine.gameStatus != Engine::VICTORY) {
				engine.gameStatus = Engine::NEW_TURN;
			}
		}
		break;
		//case 'I':
			//engine.invState = 5;
			//invSkip = true;
			
		case 'i': //display inventory
		{ 
			engine.map->computeFov();
			engine.gui->menu.clear();
			//TCODConsole::root->clear();
			if (!invSkip)
			{
				engine.invState = 1;
			}
			engine.gui->menu.addItem(Menu::ITEMS, "ITEMS");
			engine.gui->menu.addItem(Menu::TECH, "TECH");
			engine.gui->menu.addItem(Menu::ARMOR, "ARMOR");
			engine.gui->menu.addItem(Menu::WEAPONS, "WEAPONS");
			engine.gui->menu.addItem(Menu::EXIT, "EXIT");
			//Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::INVENTORY);
			Actor *actor;
			bool choice = true;
			bool itemUsed = true;
			Menu::MenuItemCode menuItem;
			while (engine.invState != 4){
				TCODConsole::flush();
			}
			//TCODConsole::root->clear();
			
			while(choice){
				menuItem = engine.gui->menu.pick(Menu::INVENTORY);
				switch (menuItem) {
					case Menu::ITEMS :
						itemUsed = true;
						while(itemUsed){
							actor = choseFromInventory(owner,1,false);
							if(actor){
								itemUsed = actor->pickable->use(actor,owner);
							}else{
								itemUsed = false;
							}
						}
					break;
					case Menu::TECH :
						actor = choseFromInventory(owner,2,false);
						if(actor)
							choice = false;
						break;
					case Menu::ARMOR:
						actor = choseFromInventory(owner,3,false);
						if(actor)
							choice = false;
						break;
					case Menu::WEAPONS:
						actor = choseFromInventory(owner,4,false);
						if(actor)
							choice = false;
						break;
					case Menu::NO_CHOICE:
						break;
					case Menu::EXIT:
						choice = false;
					break;
					default: break;
				}
			}
			engine.invState = 0;
			engine.invFrames = 0;
			if(menuItem != Menu::EXIT){
				if (actor) {
					bool used = actor->pickable->use(actor,owner);
					if (used) {
						engine.gameStatus = Engine::NEW_TURN;
					}
				}
			}
		}
		break;
		case 'd': //drop an item
		{
			engine.map->computeFov();
			engine.gui->menu.clear();
			engine.invState = 1;
			engine.gui->menu.addItem(Menu::ITEMS, "ITEMS");
			engine.gui->menu.addItem(Menu::TECH, "TECH");
			engine.gui->menu.addItem(Menu::ARMOR, "ARMOR");
			engine.gui->menu.addItem(Menu::WEAPONS, "WEAPONS");
			engine.gui->menu.addItem(Menu::EXIT, "EXIT");
			//Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::INVENTORY);
			Actor *actor;
			bool choice = true;
			Menu::MenuItemCode menuItem;
			while (engine.invState != 4){
				TCODConsole::flush();
			}
			while(choice){
				menuItem = engine.gui->menu.pick(Menu::INVENTORY);
				switch (menuItem) {
					case Menu::ITEMS :
						actor = choseFromInventory(owner,1,false);
						if(actor)
							choice = false;
						break;
					case Menu::TECH :
						actor = choseFromInventory(owner,2,false);
						if(actor)
							choice = false;
						break;
					case Menu::ARMOR:
						actor = choseFromInventory(owner,3,false);
						if(actor)
							choice = false;
						break;
					case Menu::WEAPONS:
						actor = choseFromInventory(owner,4,false);
						if(actor)
							choice = false;
						break;
					case Menu::NO_CHOICE:
						break;
					case Menu::EXIT:
						choice = false;
					break;
					default: break;
				}
			}
			engine.invState = 0;
			engine.invFrames = 0;
			if(menuItem != Menu::EXIT){
				if (actor) {
					actor->pickable->drop(actor,owner);
					engine.gameStatus = Engine::NEW_TURN;
				}
			}
		}break;
		case 'l':
		{
			engine.map->computeFov();
			engine.gui->renderKeyLook();
		}break;
		case '>':
		if (engine.stairs->x == owner->x && engine.stairs->y == owner->y) {
			if(engine.boss == NULL || engine.boss->x == 0 || (engine.boss->destructible && engine.boss->destructible->isDead()))
			{
				engine.player->attacker->lastTarget = NULL;
				engine.nextLevel();
			}
			else
			{
				engine.gui->message(TCODColor::lightGrey, "You must first defeat the %s that is guarding the stairs before leaving!", engine.boss->name);
			}
		} else {
			engine.gui->message(TCODColor::lightGrey, "There are no stairs here. Perhaps you are disoriented?");
		} break;
		/*case 'v':
		int w, h;
		if (!TCODConsole::isFullscreen()){
			TCODSystem::getCurrentResolution(&w,&h);
			TCODConsole::initRoot(w/16,h/16,"Astro",true);
		} else {
			engine.gui->message(TCODColor::darkerPink,"minimizing");
			TCODConsole::initRoot(engine.screenWidth,engine.screenHeight, "Astro", false);
		}*/
		break;
		case 'f':
			//shooty shooty bang bang -Mitchell
			//need to figure out how to check if the user has a gun
			if(owner->container->ranged){
				//engine.gui->message(TCODColor::darkerOrange,"You fire your MLR");
				Actor *closestMonster = engine.getClosestMonster(owner->x, owner->y,20);
				if ( !(owner->hostile||(closestMonster && closestMonster->hostile)) || !closestMonster ) {
					engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to shoot.");
					return;
				}
				//hit the closest monster for <damage> hit points;
				else{
					//if(owner->attacker && owner->attacker->battery >= 1){
						owner->attacker->shoot(owner, closestMonster);
						//owner->attacker->usePower(owner, 1);
						engine.damageDone += (int)owner->totalDex - closestMonster->destructible->totalDodge;
						engine.gameStatus = Engine::NEW_TURN;
					//}
					//else{
					//	engine.gui->message(TCODColor::lightGrey, "Not enough battery to shoot.");
					//}
				}
				
			}
			else{
				engine.gui->message(TCODColor::lightGrey,"You do not have a ranged weapon equipped");
			}
		break;
		case 'F':
			//aimed shooty shooty bang bang -Mitchell
			if(owner->container->ranged){
				//Actor *closestMonster = engine.getClosestMonster(owner->x, owner->y,3);
				engine.gui->message(TCODColor::cyan, "Choose a target to shoot");
				int x = engine.player->x;
				int y = engine.player->y;
				if (!engine.pickATile(&x, &y, 10)) {
					//engine.gui->message(TCODColor::lightGrey, "You can't shoot that far.");
					return;
				}
				Actor *actor = engine.getActor(x,y);
				if (!actor || !(engine.map->isVisible(actor->x, actor->y))) {
					engine.gui->message(TCODColor::lightGrey, "No enemy in sight at that location.");
					return;
				}
				/*if (!closestMonster) {
					engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to shoot.");
					//return false;
				}*/
				//hit the closest monster for <damage> hit points;
				else{
					//if(owner->attacker && owner->attacker->battery >= 1){
						owner->attacker->shoot(owner, actor);
						//owner->attacker->usePower(owner, 1);
						engine.damageDone += (int)owner->totalDex - actor->destructible->totalDodge;
						engine.gameStatus = Engine::NEW_TURN;
					//}
					//else{
					//	engine.gui->message(TCODColor::lightGrey, "Not enough battery to shoot.");
					//}
				}
				
			}
			else{
				engine.gui->message(TCODColor::lightGrey,"You do not have a ranged weapon equipped");
			}
		break;
		case 'c':
			engine.map->computeFov();
			displayCharacterInfo(owner);
		break;
		case '=':
			if (engine.player->hostile){
				engine.player->hostile = false;
				engine.gui->message(TCODColor::lightRed,"You assume a normal stance.");
			}else {
				engine.player->hostile = true;
				engine.gui->message(TCODColor::lightRed,"You assume a more hostile stance, ready to destroy all in your path!");
			}
		break;
		case 'm': //displays the printed map
			if (engine.printMap)
			{
				engine.gameStatus = Engine::NEW_TURN;
				engine.gui->message(TCODColor::yellow,"You put away the map.");
				//engine.menuState = 4;
				engine.menuState = 5;
				while(engine.menuState != 2){
					TCODConsole::flush();
				}
				TCOD_key_t key;
				TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
				engine.menuState = 0;
			}
			else
			{
				engine.gui->message(TCODColor::yellow,"You have found no maps yet.");
			}
		break;
		case 'u':
			if (engine.player->companion){
				if (engine.player->getDistance(engine.player->companion->x,engine.player->companion->y) < 2){
					((CompanionAi*)engine.player->companion->ai)->feedMaster(engine.player->companion,engine.player);
				} else {
					engine.gui->message(TCODColor::grey,"You are too far away to reach your companion.");
				}
			}
		break;
	}
}

Actor *PlayerAi::choseFromInventory(Actor *owner,int type, bool isVend) {
	static const int INVENTORY_WIDTH = 38;
	static const int INVENTORY_HEIGHT = 28;
	inventoryScreen = new TCODConsole(INVENTORY_WIDTH, INVENTORY_HEIGHT);
	
	//display the inventory frame
	inventoryScreen->setDefaultForeground(TCODColor(100,180,250));
	inventoryScreen->printFrame(0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,true,TCOD_BKGND_DEFAULT);
	
	//display the items with their keyboard shortcut
	inventoryScreen->setDefaultForeground(TCODColor::white);
	int shortcut = 'a';
	int y = 1;
	for (Actor **it = owner->container->inventory.begin();
		it != owner->container->inventory.end(); it++) {
		Actor *actor = *it;
		if(actor->sort == type){
			if((actor->pickable->type == Pickable::EQUIPMENT || actor->pickable->type == Pickable::WEAPON)&& ((Equipment*)(actor->pickable))->equipped){
				inventoryScreen->print(1,y,"(%c) %s(E)",shortcut,actor->name);
			}else{
				inventoryScreen->print(1,y,"(%c) %s",shortcut,actor->name);
			}
			owner->container->select[shortcut] = actor->name;
			if (actor->pickable->stacks) {
				if(isVend){
					inventoryScreen->print(17, y, "Pbc: %d Ink: %d",actor->pickable->value,actor->pickable->inkValue);
				}else{
					if(strcmp(actor->name,"Medkit") == 0){
						inventoryScreen->print(17, y, "(%dHp)",(int)owner->getHealValue());
					}
					inventoryScreen->print(34, y, "(%d)",actor->pickable->stackSize);
				}
			}else if(isVend){
				inventoryScreen->print(17, y, "Pbc: %d Ink: %d",actor->pickable->value,actor->pickable->inkValue);
			}
			y++;
			shortcut++;
		}
	}
	//blit the inventory console on the root console
	TCODConsole::blit(inventoryScreen,0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,
		TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
		engine.screenHeight/2 - INVENTORY_HEIGHT/2 + 1);
	TCODConsole::flush();
	
	//wait for a key press
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
	
	if (key.vk == TCODK_CHAR) {
		if(owner->container->select[key.c]){
			int index = 0;
			for(Actor **it = owner->container->inventory.begin(); it != owner->container->inventory.end(); ++it){
				Actor *actor = *it;
				if((index >= key.c - 'a') && strcmp(actor->name,owner->container->select[key.c]) == 0){
					engine.gui->message(TCODColor::grey, "You picked the %s",actor->name);
					owner->container->select.clear();
					return actor;
				}
				index++;
			}
		}
	}
	return NULL;
}
void PlayerAi::displayCharacterInfo(Actor *owner){
//Display screen of Character Information
	static const int CHARACTER_HEIGHT = 38;
	static const int CHARACTER_WIDTH = 33;
	TCODConsole con(CHARACTER_WIDTH,CHARACTER_HEIGHT);
	
	con.setDefaultForeground(TCODColor(100,180,250));
	con.printFrame(0,0,CHARACTER_WIDTH,CHARACTER_HEIGHT,true,TCOD_BKGND_DEFAULT,"CHARACTER");
	
	con.setDefaultForeground(TCODColor::white);
	
	//Print Currently Equipped armor onto the screen
	con.print(14,22,"ARMOR");
	con.print(1,24,"HEAD: ");
	con.print(1,26,"CHEST: ");
	con.print(1,28,"LEGS: ");
	con.print(1,30,"FEET: ");
	con.print(1,32,"HAND1: ");
	con.print(1,34,"HAND2: ");
	con.print(1,36,"RANGED: ");
	for(Actor **it = owner->container->inventory.begin(); it != owner->container->inventory.end(); ++it){
		Actor *actor = *it;
		if((actor->pickable->type == Pickable::EQUIPMENT || actor->pickable->type == Pickable::WEAPON) && ((Equipment*)(actor->pickable))->equipped){
			switch(((Equipment*)(actor->pickable))->slot){
				case Equipment::HEAD:
					con.print(6,24,"%s",actor->name);
				break;
				case Equipment::CHEST:
					con.print(7,26,"%s",actor->name);
				break;
				case Equipment::LEGS:
					con.print(6,28,"%s",actor->name);
				break;
				case Equipment::FEET:
					con.print(6,30,"%s",actor->name);
				break;
				case Equipment::HAND1:
					con.print(7,32,"%s",actor->name);
				break;
				case Equipment::HAND2:
					con.print(7,34,"%s",actor->name);
				break;
				case Equipment::RANGED:
					con.print(8,36,"%s",actor->name);
				break;
				case Equipment::NOSLOT:
				break;
				default: break;
			}
		}
	}
	//Diplay Character Stats
	con.print(2,4,"STATS");
	con.print(1,6,"LVL: %d",xpLevel);
	if(owner->destructible->totalDR - owner->destructible->baseDR >= 0)
		con.print(1,8,"DR: %d(+%d)",owner->destructible->baseDR,owner->destructible->totalDR - owner->destructible->baseDR);
	else
		con.print(1,8,"DR: %d(%d)",owner->destructible->baseDR,owner->destructible->totalDR - owner->destructible->baseDR);
	if(owner->totalStr - owner->str >= 0)
		con.print(1,10,"STR: %d(+%d)",owner->str,owner->totalStr - owner->str);
	else
		con.print(1,10,"STR: %d(%d)",owner->str,owner->totalStr - owner->str);
	if(owner->totalDex - owner->dex >= 0)
		con.print(1,12,"DEX: %d(+%d)",owner->dex,owner->totalDex-owner->dex);
	else
		con.print(1,12,"DEX: %d(%d)",owner->dex,owner->totalDex-owner->dex);
	if(owner->totalIntel - owner->intel >= 0)
		con.print(1,14,"INT: %d(+%d)",owner->intel,owner->totalIntel-owner->intel);
	else
		con.print(1,14,"INT: %d(%d)",owner->intel,owner->totalIntel-owner->intel);
	con.print(1,16,"KILLS: %d",engine.killCount);
	//con.print(1,18,"MEDKIT HEAL: %d",(int)owner->getHealValue());
	//con.print(1,18,"DMG DONE: %g",engine.damageDone);
	//con.print(1,20,"DMG TAKEN: %g",engine.damageReceived);
	
	//Display Character Image
	//con.print(20,8,"@");
	
	//Display different Image based on race
	/*switch(owner->ch){
		case 143:
			con.print(20,8,"Â");
		break;
		case 159:
			con.print(20,8,"ƒ");
		break;
		case 174:
			con.print(20,8,"»");
		break;
		default: break;
	}*/
	
	//blit character info onto the game display
	TCODConsole::blit(&con,0,0,CHARACTER_WIDTH,CHARACTER_HEIGHT,
		TCODConsole::root, engine.screenWidth/2 - CHARACTER_WIDTH/2,
		engine.screenHeight/2 - CHARACTER_HEIGHT/2 - 2);
	engine.menuState = 1;
	while(engine.menuState != 2){
		TCODConsole::flush();
	}
	//Keep info displayed until the player presses any button
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
	engine.menuState = 0;
	
}

static const int TRACKING_TURNS = 3;

MonsterAi::MonsterAi() : moveCount(0) {
}

void MonsterAi::load(TCODZip &zip) {
	moveCount = zip.getInt();
}

void MonsterAi::save(TCODZip &zip) {
	zip.putInt(MONSTER);
	zip.putInt(moveCount);
}

void MonsterAi::update(Actor *owner) {
	
		if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);

	
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	
	
	
	if(engine.map->infectionState(owner->x,owner->y) >= 4 && owner->ch == 166) //change miniSporeCreatures into regular spore creatures if tile becomes infected enough
	{
		owner->name = "Spore Creature";
		owner->destructible->hp = 17*(1 + .1*(engine.level - 1));
		owner->destructible->maxHp = 17*(1 + .1*(engine.level - 1));
		owner->destructible->totalDodge = 1*(1 + .1*(engine.level - 1));
		owner->totalStr = 10*(1 + .1*(engine.level - 1));
		owner->destructible->xp = 25*(1 + .1*(engine.level - 1));
		owner->ch = 165;
	}
		Actor *comp = engine.player->companion;
		int compFov = 2;
		bool compTest =  comp && comp->destructible && !comp->destructible->isDead() && comp->getDistance(owner->x, owner->y) <= compFov;
		
	if (engine.map->isInFov(owner->x,owner->y) || compTest) {
		//can see the player//companion, move towards him
		moveCount = TRACKING_TURNS;
	} else {
		moveCount--;
	}
	
	if (moveCount > 0) 
	{
		float d1 = 0;
		float d2 = 100;
		
		if(compTest)
		{
			d1 = engine.player->getDistance(owner->x,owner->y);
			d2 = engine.player->companion->getDistance(owner->x, owner->y);
		}
		
		if(d1 <= d2)
		{
			moveOrAttack(owner, engine.player, engine.player->x, engine.player->y);
		}
		else
		{
			moveOrAttack(owner, comp,comp->x, comp->y);
		}
	
		
	} else{
		moveCount = 0;
	}
	owner->destructible->takeFireDamage(owner, 3.0);
}
void MonsterAi::moveOrAttack(Actor *owner, int targetx, int targety){
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	
	int dxL = engine.player->lastX - owner->x;
	int dyL = engine.player->lastY - owner->y;
	int stepdxL = (dxL > 0 ? 1:-1);
	int stepdyL = (dyL > 0 ? 1:-1);
	stepdxL = (dxL == 0 ? 0:stepdxL);
	stepdyL = (dyL == 0 ? 0:stepdyL);
	float distance = sqrtf(dx*dx+dy*dy);
	
	if(owner->ch == 150 && distance >= 2 && engine.turnCount % 2 == 0)
	{
		//crawlers can only move every other turn
		return;
	}
	
	if (distance >= 2) {
		dx = (int) (round(dx / distance));
		dy = (int) (round(dy / distance));
		if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
			owner->x+=dx;
			owner->y+=dy;
		} else if (engine.map->canWalk(owner->x+stepdxL,owner->y+stepdyL)) {
			owner->x+=stepdxL;
			owner->y+=stepdyL;
		} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
			owner->y += stepdy;
		}
		if (owner->oozing) {
			engine.map->infectFloor(owner->x, owner->y);
		}
	} else if (owner->attacker) {
			owner->attacker->attack(owner,engine.player);
			engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDodge);
	}
	
}

void MonsterAi::moveOrAttack(Actor *owner, Actor *target, int targetx, int targety){
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	
	int dxL = target->lastX - owner->x;
	int dyL = target->lastY - owner->y;
	int stepdxL = (dxL > 0 ? 1:-1);
	int stepdyL = (dyL > 0 ? 1:-1);
	stepdxL = (dxL == 0 ? 0:stepdxL);
	stepdyL = (dyL == 0 ? 0:stepdyL);
	float distance = sqrtf(dx*dx+dy*dy);
	
	if(owner->ch == 150 && distance >= 2 && engine.turnCount % 2 == 0)
	{
		//crawlers can only move every other turn
		return;
	}
	
	if (distance >= 2) {
		dx = (int) (round(dx / distance));
		dy = (int) (round(dy / distance));
		if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
			owner->x+=dx;
			owner->y+=dy;
		} else if (engine.map->canWalk(owner->x+stepdxL,owner->y+stepdyL)) {
			owner->x+=stepdxL;
			owner->y+=stepdyL;
		} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
			owner->y += stepdy;
		}
		if (owner->oozing) {
			engine.map->infectFloor(owner->x, owner->y);
		}
	} else if (owner->attacker) 
	{
		if(target)
			owner->attacker->attack(owner,target);

		if(target == engine.player)
			engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDodge);
	}
	
}

SecurityBotAi::SecurityBotAi() : moveCount(0) {
vendingX = -1;
vendingY = -1;
}

void SecurityBotAi::load(TCODZip &zip) {

	moveCount = zip.getInt();
	vendingX = zip.getInt();
	vendingY = zip.getInt();
}

void SecurityBotAi::save(TCODZip &zip) {
	zip.putInt(SECURITY);
	zip.putInt(moveCount);
	zip.putInt(vendingX);
	zip.putInt(vendingY);
}

void SecurityBotAi::update(Actor *owner) {
		if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);


	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	Actor *comp = engine.player->companion;
		int compFov = 2;
		bool compTest =  comp && comp->destructible && !comp->destructible->isDead() && comp->getDistance(owner->x, owner->y) <= compFov;
	if (engine.map->isInFov(owner->x,owner->y) || compTest) {
		//can see the palyer, move towards him
		moveCount = TRACKING_TURNS;
	} else {
		moveCount--;
	}
	if (moveCount > 0) 
	{
		float d1 = 0;
		float d2 = 100;
		
		if(compTest)
		{
			d1 = engine.player->getDistance(owner->x,owner->y);
			d2 = engine.player->companion->getDistance(owner->x, owner->y);
		}
		
		if(d1 <= d2)
		{
			moveOrAttack(owner, engine.player, engine.player->x, engine.player->y);
		}
		else
		{
			moveOrAttack(owner, comp,comp->x, comp->y);
		}
	} else 
	{
		moveCount = 0;
	}
}

void SecurityBotAi::moveOrAttack(Actor *owner, Actor *target, int targetx, int targety){
	//Cases
	//1. Security Bot does not have any vending machine (isHostile = false ending machine x = -1, vendingmachine y = -1), function as a monster Ai normally
	//2. If vending machinex != -1 and vendingmachiney != -1, then make an instance of vendingmachine ai using x and y. and check if it security deployed
		//2a. If isdeployed, then change vending machine state to hostile
		//3a else nothing
	
	if((vendingX == -1 && vendingY == -1) || owner->hostile)
		MonsterAi::moveOrAttack(owner,target,targetx,targety);
	else
	{
		Actor* vending = engine.getAnyActor(vendingX, vendingY);
		VendingAi* vendingAi = (VendingAi*) vending->ai;
		
		if(vendingAi != NULL && (vendingAi->deployedSecurity || vending->destructible->isDead()))
		{
			owner->ch = 130;
			owner->hostile = true;
			engine.gui->message(TCODColor::red, "Vending Machine Vandalism Detected: %s Activated!", owner->name);
		}
		
	}
}

EpicenterAi::EpicenterAi() {
}

void EpicenterAi::load(TCODZip &zip) {
}

void EpicenterAi::update(Actor *owner) {
	if (engine.turnCount % 250 == 0) {
		infectLevel(owner);
	}
	//set growing flowers boolean to true to have the infected tiles grow some flowers upon infection spread
}

void EpicenterAi::infectLevel(Actor *owner) {
	int width = engine.map->width;
	int height = engine.map->height;
	TCODRandom *rng = TCODRandom::getInstance();

	for (int i = 0; i < width*height; i++) {
		int before = engine.map->tiles[i].infection;
		engine.map->tiles[i].infection += 1 / (rng->getDouble(.01,1.0)*owner->getDistance(i%width, i/width));
		int after = engine.map->tiles[i].infection;
		//update flowers if level has changed and is less than 6
		if (before != after && before <= 6)
		{
			//0-3 = level 3, 4-7 = level 4,8-11 = level 5,12-15 = level 6
			if (engine.map->tiles[i].infection >= 3 && engine.map->tiles[i].infection < 4)
			{
				engine.map->tiles[i].flower = rng->getInt(0,3);
			}
			else if (engine.map->tiles[i].infection >= 4 && engine.map->tiles[i].infection < 5)
			{
				engine.map->tiles[i].flower = rng->getInt(4,7);
			}
			else if (engine.map->tiles[i].infection >= 5 && engine.map->tiles[i].infection < 6)
			{
				engine.map->tiles[i].flower = rng->getInt(8,11);
			}
			else if (engine.map->tiles[i].infection >= 6)
			{
				engine.map->tiles[i].flower = rng->getInt(12,15);
			}
		}
		
	}
	engine.gui->message(TCODColor::green,"You feel uneasy as the infection seems to spread.");
}

void EpicenterAi::save(TCODZip &zip) {
	zip.putInt(EPICENTER);
}

TriggerAi::TriggerAi(const char *text) {
	//cout << "trigger made, message: " << text << endl;
	this->text = text;
	pressed = false;
}
TriggerAi::TriggerAi() {
	pressed = false;
}

void TriggerAi::load(TCODZip &zip) {
	text = zip.getString();
	pressed = zip.getInt();
}

void TriggerAi::save(TCODZip &zip) {
	zip.putInt(TRIGGER);
	zip.putString(text);
	zip.putInt(pressed);
}

void TriggerAi::update(Actor *owner) {
	//cout << "updating: " << text << endl;
	if (!pressed && engine.player->x == owner->x && engine.player->y == owner->y) {
		//engine.gui->message(TCODColor::yellow, text);
		pressed = true;
		engine.armorState = 1;
		//TCODConsole::flush();
		int PAUSE_MENU_WIDTH = 32;
		int PAUSE_MENU_HEIGHT = 16;
		
		TCODConsole termwindow(PAUSE_MENU_WIDTH,PAUSE_MENU_HEIGHT);
		
		int menux = engine.screenWidth / 2 - PAUSE_MENU_WIDTH / 2;
		int menuy = engine.screenHeight / 2 - PAUSE_MENU_HEIGHT / 2;
		//make me red
		termwindow.setDefaultForeground(TCODColor(67,199,50));
		termwindow.setDefaultBackground(TCODColor(0,0,0));
		termwindow.printFrame(0,0,32,16,true,TCOD_BKGND_ALPHA(50),"\{ AUTOMATED TERMINAL \{");
		termwindow.printRect(1,1,30,16,text);
		TCODConsole::blit(&termwindow,0,0,32,16,TCODConsole::root,menux,menuy);
		TCODConsole::flush();
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
		engine.armorState = 0;
		engine.ctrTer += 1;
	}
	if (engine.ctrTer == 3 && !engine.bonusTer && engine.level != 0)
	{
		engine.bonusTer = true;
		cout << "ctr == 3" << endl;
		TCODRandom *random = TCODRandom::getInstance();
		int rng = random->getInt(0,4);
		switch (rng) {
			case 0	:
				engine.player->destructible->maxHp += engine.player->getHpUp();
				engine.player->destructible->hp += engine.player->getHpUp();
				engine.player->vit += engine.player->getHpUp();;
				//choice_made = true;
				engine.gui->message(TCODColor::yellow,"Finding all of the recordings in this deck you feel more HEALTHY and ready to advance.(+1 VIT)");
				break;
			case 1 :
				engine.player->attacker->basePower += 1;
				engine.player->attacker->totalPower += 1;
				engine.player->str += 1;
				engine.player->totalStr += 1;
				//choice_made = true;
				engine.gui->message(TCODColor::yellow,"Having found all the recordings on this deck you use the knowledge to become STRONGER.(+1 STR)");
				break;
			case 2 :
				engine.player->dex += 1;
				engine.player->totalDex += 1;
				//choice_made = true;
				engine.gui->message(TCODColor::yellow,"All the recordings have been found in this deck making you quicker and more DEXTEROUS.(+1 DEX)");
				break;
			case 3 :
				engine.player->intel += 1;
				engine.player->totalIntel += 1;
				//choice_made = true;
				engine.gui->message(TCODColor::yellow,"Finding the recordings in this deck have made more SMARTER and more aware of the infection.(+1 INT)");
				break;
			case 4:
				engine.player->destructible->baseDodge += 1;
				engine.player->destructible->totalDodge += 1;
				engine.gui->message(TCODColor::yellow,"All the recordings you have found have made you able to DODGE better.(+1 DODGE)");
				break;
			default: break;
		}
		
		
	}
	
	
}

LightAi::LightAi(int rad, float f)
{
	//TCODRandom *myRandom = new TCODRandom();
	//float rng = myRandom->getFloat(0.0f,1.0f,0.9);
	flkr = f;
	radius = rad;	
	lstX = 0;
	lstY = 0;
	lmap = new TCODMap(13,13);
	frstMap = new TCODMap(13,13);
	frstBool = true;
	onAgn = false;
	//oldMap = new TCODMap(13,13);
	onOff = true;
	frst = true;
	//frstTurn = true;
	moving = false;
}

LightAi::LightAi(int rad, float f, bool movibility)
{
	//TCODRandom *myRandom = new TCODRandom();
	//float rng = myRandom->getFloat(0.0f,1.0f,0.9);
	flkr = f;
	radius = rad;	
	lstX = 0;
	lstY = 0;
	lmap = new TCODMap(13,13);
	frstMap = new TCODMap(13,13);
	frstBool = true;
	onAgn = false;
	//oldMap = new TCODMap(13,13);
	onOff = true;
	frst = true;
	//frstTurn = true;
	moving = movibility;
}

void LightAi::load(TCODZip &zip){
	flkr = zip.getFloat();
	onAgn = zip.getInt();
	onOff = zip.getInt();
	frst = zip.getInt();
	moving = zip.getInt();
	radius = zip.getInt();
	frstBool = zip.getInt();
	lstX = zip.getInt();
	lstY = zip.getInt();
}

void LightAi::save(TCODZip &zip){
	zip.putInt(LIGHT);
	zip.putFloat(flkr);
	zip.putInt(onAgn);
	zip.putInt(onOff);
	zip.putInt(frst);//to reset num
	zip.putInt(moving);//are you static or moving
	zip.putInt(radius);
	zip.putInt(frstBool);
	zip.putInt(lstX);
	zip.putInt(lstY);
	
}

int LightAi::giveRad() {
	return radius;
}

void LightAi::flicker(Actor * owner, float chance){
	if (flkr < chance)
	{
		//LightAi *l = (LightAi*)owner->ai;
		onOff = false;
		update(owner);
		//onOff = !onOff;
		//update(owner);
	}
	//onOff = true;
	//update(owner);
}

void LightAi::update(Actor * owner)
{
	if (onOff)
	{
		int maxx = owner->x+6;
		int minx = owner->x-6;
		int maxy = owner->y+6;
		int miny = owner->y-6;
		//lmap = new TCODMap(maxx-minx,maxy-miny);
		
		if (moving)
		{
			//cout << "moving light updating" << endl;
			//engine.gui->message(TCODColor::yellow, "changed light!");
				if (lstX != 0 && lstY != 0)
				{
					for (int x=lstX-6; x <= lstX+6; x++) {
						for (int y=lstY-6; y <= lstY+6; y++) {
							//cout << "checking tile (" << x << "," << y << ")" << endl;
							if (x > 0 && y > 0)
							{
								if (engine.map->tiles[x+y*engine.map->width].drty)
								{
									if (engine.map->tiles[x+y*engine.map->width].num == 1)//player's FOV
									{
										engine.map->tiles[x+y*engine.map->width].lit = false;//problem-> the player's flashlight is the only one that needs to move
										engine.map->tiles[x+y*engine.map->width].num--;      //all else never comes here, just add one and be done
										frst = true;				                         //everytime you move, decrement by 1, then add the new shit back
										engine.map->tiles[x+y*engine.map->width].drty = false;
									}
									else if (engine.map->tiles[x+y*engine.map->width].num > 1)
									{
										engine.map->tiles[x+y*engine.map->width].num--;
										frst = true;
										engine.map->tiles[x+y*engine.map->width].drty = false;
									}
									//lmap->setProperties(x-minx,y-miny,engine.map->canWalk(maxx-(maxx-x),maxy-(maxy-y)),engine.map->isWall(maxx-(maxx-x),maxy-(maxy-y)));//engine.map->canWalk(x-owner->x,y-owner->y),engine.map->isWall(x-owner->x,y-owner->y));
								}
							}
						}
					}
				}
		}
		//only do this once for flares!
		for (int x=minx; x <= maxx; x++) {
			for (int y=miny; y <= maxy; y++) {
				//inheriting properties of real map
				
				lmap->setProperties(x-minx,y-miny,engine.map->canWalk(maxx-(maxx-x),maxy-(maxy-y)),engine.map->isWall(maxx-(maxx-x),maxy-(maxy-y)));//engine.map->canWalk(x-owner->x,y-owner->y),engine.map->isWall(x-owner->x,y-owner->y));
				if (frstBool){
					frstMap->setProperties(x-minx,y-miny,engine.map->canWalk(maxx-(maxx-x),maxy-(maxy-y)),engine.map->isWall(maxx-(maxx-x),maxy-(maxy-y)));//engine.map->canWalk(x-owner->x,y-owner->y),engine.map->isWall(x-owner->x,y-owner->y));
				}
			}
		}
		if (frstBool){
			frstBool = false;
		}
		//owner->radius
		lmap->computeFov(owner->x-minx,owner->y-miny,radius);
		for (int x=minx; x <= maxx; x++) {
			for (int y=miny; y <= maxy; y++) {
				//if (engine.distance(owner->x,x,owner->y,y) <= radius)
				//{
					if (lmap->isInFov(x-minx,y-miny)){ //&& !(engine.player->x == x && engine.player->y == y)) {
						
						engine.map->tiles[x+y*engine.map->width].lit = true;
						if (moving)
								engine.map->tiles[x+y*engine.map->width].drty = true;
						if (frst)
						{
							
							engine.map->tiles[x+y*engine.map->width].num++;
							
							//engine.gui->message(TCODColor::green, "calculating lights.");
							//frst = false;//when set to off turn it back to true
						}
					}
					//if ((engine.player->x == x && engine.player->y == y))
					//{
					//	engine.map->tiles[x+y*engine.map->width].lit = false;
					//}
					//else
					//{
					//	engine.map->tiles[x+y*engine.map->width].lit = false;
					//	if (frst)
					//	{
					//		engine.map->tiles[x+y*engine.map->width].num--;
							
					//	}
					//}
				//}
			}
		}
		if (moving){
		lstX = owner->x;
		lstY = owner->y;}
		
		if (frst)
			frst = false;
		
		
	}
	else
	{
		int maxx = owner->x+6;
		int minx = owner->x-6;
		int maxy = owner->y+6;
		int miny = owner->y-6;
		for (int x=minx; x <= maxx; x++) {
			for (int y=miny; y <= maxy; y++) {
				//if there is only one light source on the tile
				frstMap->computeFov(owner->x-minx,owner->y-miny,radius);
				lmap->computeFov(owner->x-minx,owner->y-miny,radius);
				if (frstMap->isInFov(x-minx,y-miny) || lmap->isInFov(x-minx,y-miny)){ //&& !(engine.player->x == x && engine.player->y == y)) {
				//ERROR WHEN SOMETHING BLOCKS THE NEW FOV AND NOT THE OLD ONE IT DOESNT UPDATE
				//RECORD OLD FOV, NOT NEW ONE!
					if (engine.map->tiles[x+y*engine.map->width].num <= 1)//adding lights that have variable FOV because of movement, like flares
					{//may have "0's" in their FOV that are lit, because we only increase num once ever, so if there are 0's, make sure they are not lit
					 //but also make sure they are not decrementing it past 0 -> line 902!	
						if (!frst)
						{
							engine.map->tiles[x+y*engine.map->width].lit = false;
							if (engine.map->tiles[x+y*engine.map->width].num == 1)
								{engine.map->tiles[x+y*engine.map->width].num--;}
							
						}
						//engine.map->tiles[x+y*engine.map->width].num--;
					}
					else if (engine.map->tiles[x+y*engine.map->width].num > 1)
					{
						if (!frst)
						{
							engine.map->tiles[x+y*engine.map->width].num--;
							
						}
					}
						
				}
			}
		}
		
	if (!frst)
			frst = true;//when set to off turn it back to true

	}
	
}

FlareAi::FlareAi(int lightRange, int turns) : lightRange(lightRange),turns(turns)
{
	//light = new Actor(x, y, ' ', "Flare Light", TCODColor::white);
	//light->ai = new LightAi(lightRange,1);
	//engine.actors.push(light);
	i = 0;
}

void FlareAi::load(TCODZip &zip){
	lightRange = zip.getInt();
	turns = zip.getInt();
	i = zip.getInt();
	light = new Actor(0,0,0,NULL,TCODColor::white);
	light->load(zip);
	
}

void FlareAi::save(TCODZip &zip){
	zip.putInt(FLARE);
	zip.putInt(lightRange);
	zip.putInt(turns);
	zip.putInt(i);
	light->save(zip);
}

void FlareAi::update(Actor * owner)
{
	
	if (i == 0)
	{
		light = new Actor(owner->x,owner->y, 171, "Flare Light", TCODColor::white);
		light->ai = new LightAi(lightRange,1);//,true);
		light->blocks = false;
		engine.actors.push(light);
		i++;
		//engine.gui->message(TCODColor::orange, "Flare is burning %d/%d of it's phosphorus remains.",turns-i,turns);
		
	}
	if (i < turns)
	{
		i++;
		//engine.gui->message(TCODColor::orange, "Flare is burning %d/%d of its phosphorus remains.",turns-i+1,turns);
	}
	else
	{
		static bool burnOut = true;
		if (burnOut)
		{
			engine.gui->message(TCODColor::orange, "Flare has burnt out, leaving a pile of ash.");
			burnOut = false;
		}
		//light->(LightAi)ai->onOff = false;
		//LightAi l = (LightAi)light->ai;
		light->ch = ' ';
		owner->name = "a pile of ash";
		owner->ch = 170;
		LightAi *l = (LightAi*)light->ai;
		l->onOff = false;
		l->update(light);
		//l->update(owner);
		//engine.actors.remove(light);
		//light = NULL;
		//SHOULD DELETE THE LIGHT PERMANENTLY 
	}
}



ConfusedActorAi::ConfusedActorAi(int nbTurns, Ai *oldAi)
	:nbTurns(nbTurns),oldAi(oldAi) {
}

void ConfusedActorAi::load(TCODZip &zip) {
	nbTurns = zip.getInt();
	oldAi = Ai::create(zip);
}

void ConfusedActorAi::save(TCODZip &zip) {
	zip.putInt(CONFUSED_ACTOR);
	zip.putInt(nbTurns);
	oldAi->save(zip);
}

void ConfusedActorAi::update(Actor *owner) {
	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);


	if (owner->destructible && !owner->destructible->isDead() ) {
		TCODRandom *rng = TCODRandom::getInstance();
		int dx = rng->getInt(-1,1);
		int dy = rng->getInt(-1,1);
	
		if (dx != 0 || dy!=0) {
			int destx = owner->x + dx;
			int desty = owner->y + dy;
			if (engine.map->canWalk(destx,desty)) {
				owner->x = destx;
				owner->y = desty;
				engine.map->computeFov();
			} else {
				Actor *actor = engine.getActor(destx, desty);
				if (actor && owner->attacker) {
					owner->attacker->attack(owner,actor);
					engine.map->computeFov();
				}
			}
		}
	}
	nbTurns--;
	owner->destructible->takeFireDamage(owner, 3.0);
	if(nbTurns == 0) {
		owner->ai = oldAi;
		delete this;
	}
		if (owner == engine.player) {
		engine.gameStatus = Engine::NEW_TURN;
		return;
	}
	
	
	
}

RangedAi::RangedAi() : moveCount(0), range(3){
}

void RangedAi::load(TCODZip &zip) {
	moveCount = zip.getInt();
	range = zip.getInt();
}

void RangedAi::save(TCODZip &zip) {
	zip.putInt(RANGED);
	zip.putInt(moveCount);
	zip.putInt(range);
}

void RangedAi::update(Actor *owner) {
	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);

	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	Actor *comp = engine.player->companion;
	int compFov = 2;
	bool compTest =  comp && comp->destructible && !comp->destructible->isDead() && comp->getDistance(owner->x, owner->y) <= compFov;	
	if (engine.map->isInFov(owner->x,owner->y) || compTest) {
		//can see the player/companion, move towards him
		moveCount = TRACKING_TURNS + 2; //give ranged characters longer tracking
	} else {
		moveCount--;
	}
	if (moveCount > 0) 
	{
		float d1 = 0;
		float d2 = 100;
		
		if(compTest)
		{
			d1 = engine.player->getDistance(owner->x,owner->y);
			d2 = engine.player->companion->getDistance(owner->x, owner->y);
		}
		
		if(d1 <= d2)
		{
			moveOrAttack(owner, engine.player, engine.player->x, engine.player->y);
		}
		else
		{
			moveOrAttack(owner, comp,comp->x, comp->y);
		}
	} else {
		moveCount = 0;
	}
	owner->destructible->takeFireDamage(owner, 3.0);
}
void RangedAi::moveOrAttack(Actor *owner, Actor *target, int targetx, int targety)
{
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	
	int dxL = target->lastX - owner->x;
	int dyL = target->lastY - owner->y;
	int stepdxL = (dxL > 0 ? 1:-1);
	int stepdyL = (dyL > 0 ? 1:-1);
	stepdxL = (dxL == 0 ? 0:stepdxL);
	stepdyL = (dyL == 0 ? 0:stepdyL);
	float distance = sqrtf(dx*dx+dy*dy);	
	//If the distance > range, then the rangedAi will move towards the player
	//If the distance <= range, then the rangedAi will shoot the player unless the player is right next the rangedAi

	if (distance > range) {

		dx = (int) (round(dx / distance));
		dy = (int)(round(dy / distance));
		if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
			owner->x+=dx;
			owner->y+=dy;
		} else if (engine.map->canWalk(owner->x+stepdxL,owner->y+stepdyL)) {
			owner->x+=stepdxL;
			owner->y+=stepdyL;
		} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
			owner->y += stepdy;
		}
		if (owner->oozing) {
			engine.map->infectFloor(owner->x, owner->y);
		}
	} else if (distance !=1 && owner->attacker) 
	{
		owner->attacker->shoot(owner, target);
		if(target == engine.player)
			engine.damageReceived += (owner->totalDex- engine.player->destructible->totalDodge);

	}
	else if (owner->attacker) 
	{
		owner->attacker->attack(owner,target);
		if(target == engine.player)
			engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDodge);
	}
}


GrenadierAi::GrenadierAi() : moveCount(0), range(3){
	numGrenades = 5;
	berserk = false;
}

void GrenadierAi::load(TCODZip &zip) {
	berserk = zip.getInt();
	moveCount = zip.getInt();
	range = zip.getInt();
	numGrenades = zip.getInt();
}

void GrenadierAi::save(TCODZip &zip) {
	zip.putInt(GRENADIER);
	zip.putInt(berserk);
	zip.putInt(moveCount);
	zip.putInt(range);
	zip.putInt(numGrenades);
}

void GrenadierAi::update(Actor *owner) {
	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);


	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	Actor *comp = engine.player->companion;
	int compFov = 2;
	bool compTest =  comp && comp->destructible && !comp->destructible->isDead() &&  comp->getDistance(owner->x, owner->y) <= compFov;
	
	if (engine.map->isInFov(owner->x,owner->y) || compTest) {
		//can see the palyer, move towards him
		moveCount = TRACKING_TURNS + 2; //give tech characters longer tracking
	} else {
		moveCount--;
	}
	if (moveCount > 0) 
	{	
	
		if(!berserk)
		{
			float d1 = 0;
			float d2 = 100;
			
			if(compTest)
			{
				d1 = engine.player->getDistance(owner->x,owner->y);
				d2 = engine.player->companion->getDistance(owner->x, owner->y);
			}
			
			if(d1 <= d2)
			{
				moveOrAttack(owner, engine.player, engine.player->x, engine.player->y);
			}
			else
			{
				moveOrAttack(owner, comp,comp->x, comp->y);
			}
		}
		else //berserk case, so you need to get the closest monster/player
		{
			Actor *closest = NULL;
			float bestDistance = 1E6f;
			for (Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++) 
			{
				Actor *actor = *iterator;
				const char* name = actor->name;
				if (actor->destructible && !actor->destructible->isDead() && strcmp(name, "infected corpse") != 0 && actor != owner && actor->ch != 163 && actor->ch != 243 && actor->ch != 24) //243 = locker
				{
					float distance = actor->getDistance(owner->x,owner->y);
					if (distance < bestDistance && (distance <= range || range ==0.0f)) 
					{
						bestDistance = distance;
						closest = actor;
					}
				}
			}
			
			if(closest)
				moveOrAttack(owner, closest,closest->x, closest->y);
		}
			
	} 
	else 
		moveCount = 0;
		
		owner->destructible->takeFireDamage(owner, 3.0);
}
void GrenadierAi::useEmpGrenade(Actor *owner, Actor *target, int targetx, int targety)
{
	if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
	engine.gui->message(TCODColor::red,"The %s uses an EMP Grenade on the %s!",owner->name, target->name);
	float damageTaken = -3 + 3 * owner->totalIntel;
	if(target->destructible)
		damageTaken = target->destructible->takeDamage(target, owner, damageTaken);
	numGrenades--;
	if(engine.player == target)
		engine.damageReceived += (3 * owner->totalIntel - 3 - engine.player->destructible->totalDodge);
	
}
void GrenadierAi::useFirebomb(Actor *owner, Actor *target, int targetx, int targety)
{
	int x = targetx;
	int y = targety;

	if(engine.map->isVisible(owner->x, owner->y))
	{
		engine.gui->message(TCODColor::red, "The %s throws a firebomb and it explodes, burning everything within %d tiles!",owner->name, 1 + (owner->totalIntel - 1) /3);
	}
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		if (actor && actor->destructible && !actor->destructible->isDead() && actor->getDistance(x,y) <= 1 + (owner->totalIntel - 1) /3) 
		{
			//the initial damage is a little high, i think it should actually be zero, since it immediatlly affects the monsters
			float damageTaken = 1;
			damageTaken = actor->destructible->takeDamage(actor, owner, 1); //problematic since the actor will die before the following "flavor" text is printed
			if (!actor->destructible->isDead()) 
			{
				if(actor == engine.player)
					engine.damageReceived += damageTaken;
				if(engine.map->isVisible(actor->x, actor->y))
					engine.gui->message(TCODColor::red,"The %s gets burned for %g hit points.",actor->name, damageTaken);

			} else 
			{
				if(engine.map->isVisible(actor->x, actor->y))
					engine.gui->message(TCODColor::red,"The %s is an ashen mound from the %g damage, crumbling under its own weight.",actor->name, damageTaken);

 			}
			
		}
	}
	
	for (int xxx = x - ((1 + (owner->totalIntel - 1) /3)) ; xxx <= x+((1 + (owner->totalIntel - 1) /3));xxx++)
	{
		for (int yyy = y - ((1 + (owner->totalIntel - 1) /3)); yyy <= y+((1 + (owner->totalIntel - 1) /3));yyy++)
		{
			if (engine.distance(x,xxx,y,yyy) <= (1 + (owner->totalIntel - 1) /3))
			{
				engine.map->tiles[xxx+yyy*engine.map->width].envSta = 1;
				engine.map->tiles[xxx+yyy*engine.map->width].temperature = 6;//should be a function of int 
			}
			
		}
	}
	numGrenades--;
}
void GrenadierAi::useFrag(Actor *owner, Actor *target, int targetx, int targety)
{
	int x = targetx;
	int y = targety;
	if(engine.map->isVisible(owner->x, owner->y))
		engine.gui->message(TCODColor::red, "The %s throws a fragmentation grenade and it explodes, eviscerating everything within %d tiles!",owner->name, 1 + (owner->totalIntel - 1) /3);
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		if (actor->destructible && !actor->destructible->isDead()
			&&actor->getDistance(x,y) <= 1 + (owner->totalIntel - 1) /3) 
		{
			float damageTaken = 2 * owner->totalIntel;
			damageTaken = actor->destructible->takeDamage(actor, owner, damageTaken);
			if (!actor->destructible->isDead()) 
			{	
				if(actor == engine.player)
					engine.damageReceived += damageTaken;
				if(engine.map->isVisible(actor->x, actor->y))
					engine.gui->message(TCODColor::red,"The %s gets wounded from the blast for %g hit points.",actor->name,damageTaken);

			} else 
			{
				if(engine.map->isVisible(actor->x, actor->y))
					engine.gui->message(TCODColor::red,"The %s's guts explode outward after taking %g damage.",actor->name,damageTaken);

			}
		}
	}
	
	numGrenades--;
}
void GrenadierAi::kamikaze(Actor *owner, Actor *target)
{
	TCODRandom *rng = TCODRandom::getInstance();
	const char* name = target->name;
	//three cases, randomly determine which grenade to use
	int dice = rng->getInt(0,30);
	if(dice <= 15)
	{
		//emp grenade
		if(engine.map->isVisible(target->x, target->y) || engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::red, "The %s kamikazes with an EMP Grenade on the %s!",owner->name,name);
		float damageTaken = -1*numGrenades*(-3 + 3 * owner->totalIntel);
		damageTaken = target->destructible->takeDamage(target, owner,damageTaken );
		if(target == engine.player)
			engine.damageReceived += -1*numGrenades*(3 * owner->totalIntel - 3 - engine.player->destructible->totalDodge);
	}
	else if(dice <= 25)
	{
		//frag
		int x = owner->x;
		int y = owner->y;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::red, "The %s kamikazes with a fragmentation grenade and it explodes, eviscerating everything within %d tiles!",owner->name, 1 + (owner->totalIntel - 1) /3);
		for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) 
		{
			Actor *actor = *it;
			if (actor->destructible && !actor->destructible->isDead()
				&&actor->getDistance(x,y) <= 1 + (owner->totalIntel - 1) /3) 
			{
				float damageTaken = 2 * owner->totalIntel;
				damageTaken = actor->destructible->takeDamage(actor, owner, damageTaken);
				if (!actor->destructible->isDead()) 
				{	
					if(actor == engine.player)
						engine.damageReceived += damageTaken;
					if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(actor->x, actor->y))
						engine.gui->message(TCODColor::red,"The %s gets wounded from the blast for %g hit points.",actor->name,damageTaken);

				} else 
				{
					if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(actor->x, actor->y))
						engine.gui->message(TCODColor::red,"The %s's guts explode outward after taking %g damage.",actor->name,damageTaken);

				}
			}
		}
	
		
		
	}
	else
	{
		//firebomb
		int x =	owner->x;
		int y = owner->y;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::red, "The %s kamikazes with a firebomb and it explodes, burning everything within %d tiles!",owner->name, 1 + (owner->totalIntel - 1) /3);
		for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
			Actor *actor = *it;
			if (actor->destructible && !actor->destructible->isDead()
				&&actor->getDistance(x,y) <= 1 + (owner->totalIntel - 1) /3) 
			{
				//the initial damage is a little high, i think it should actually be zero, since it immediatlly affects the monsters
				float damageTaken = 1;
				damageTaken = actor->destructible->takeDamage(actor, owner, damageTaken);
				
				if (!actor->destructible->isDead()) 
				{
					if(actor == engine.player)
						engine.damageReceived += damageTaken;
					if(engine.map->isVisible(actor->x, actor->y))
						engine.gui->message(TCODColor::red,"The %s gets burned for %g hit points.",actor->name, damageTaken);

				} else 
				{
					if(engine.map->isVisible(actor->x, actor->y))
						engine.gui->message(TCODColor::red,"The %s is an ashen mound from the %g damage, crumbling under its own weight.",actor->name, damageTaken);

				}	
			}
		}

		for (int xxx = x - ((1 + (owner->totalIntel - 1) /3)) ; xxx <= x+((1 + (owner->totalIntel - 1) /3));xxx++)
		{
			for (int yyy = y - ((1 + (owner->totalIntel - 1) /3)); yyy <= y+((1 + (owner->totalIntel - 1) /3));yyy++)
			{
				if (engine.distance(x,xxx,y,yyy) <= (1 + (owner->totalIntel - 1) /3))
				{
					engine.map->tiles[xxx+yyy*engine.map->width].envSta = 1;
					engine.map->tiles[xxx+yyy*engine.map->width].temperature = 6;//should be a function of int 
				}
				
			}
		}
	}
	
	MonsterDestructible* md = (MonsterDestructible*) owner->destructible;
	md->suicide(owner);

}
void GrenadierAi::moveOrAttack(Actor *owner, Actor *target, int targetx, int targety)
{
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	
	int dxL = target->lastX - owner->x;
	int dyL = target->lastY - owner->y;
	int stepdxL = (dxL > 0 ? 1:-1);
	int stepdyL = (dyL > 0 ? 1:-1);
	stepdxL = (dxL == 0 ? 0:stepdxL);
	stepdyL = (dyL == 0 ? 0:stepdyL);
	float distance = sqrtf(dx*dx+dy*dy);
	
	//I want the grenadier to move towards if it is out of range or it is out of grenades but not right
	if (distance > range || (distance > 1 && numGrenades <= 0 )) {
		dx = (int) (round(dx / distance));
		dy = (int)(round(dy / distance));
			if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
			owner->x+=dx;
			owner->y+=dy;
		} else if (engine.map->canWalk(owner->x+stepdxL,owner->y+stepdyL)) {
			owner->x+=stepdxL;
			owner->y+=stepdyL;
		} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
			owner->y += stepdy;
		}
		if (owner->oozing) {
			engine.map->infectFloor(owner->x, owner->y);
		}
	} else if (distance > 1 && distance <= range && owner->attacker && numGrenades > 0 && !berserk) 
	{
		TCODRandom *rng = TCODRandom::getInstance();
		//Grenadiers will go berserk and attack the nearest monster or the player 50%/(numGrenades+1) left of the time or whenever they have one grenade left
		//The damage done is proportion to the number of grenades left
		
		if(rng->getInt(0,100) < 50/(numGrenades+1) || numGrenades == 1)
		{
			berserk = true;
			numGrenades = -1*numGrenades;
			if(engine.map->isVisible(owner->x,owner->y))
			engine.gui->message(TCODColor::red,"The %s is going berserk!",owner->name);
		}
		else
		{
			int dice = rng->getInt(0,30);
			if(dice <= 15)
				useEmpGrenade(owner,target, target->x, target->y);
			else if(dice <= 25)
				useFrag(owner, target, target->x, target->y);
			else
				useFirebomb(owner, target, target->x, target->y);
		}

		
	}else if (owner->attacker && !berserk) 
	{ //grenadier will melee attack if up close
		owner->attacker->attack(owner,target);
		if(target == engine.player)
			engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDodge);
	}else if(owner->attacker && berserk)
	{
		//kamkaze on the actor closetest to you, given by targetx, targety
		Actor *actor = engine.getActor(targetx,targety);
		
		if(owner->destructible->isDead() || actor == NULL || actor->destructible == NULL || (actor && actor->destructible && actor->destructible->isDead()))
			return;
		
		kamikaze(owner, actor);
		
		
	}	
}

TurretAi::TurretAi()
{
	range = 5;
	controlX = -1;
	controlY = -1;
}

void TurretAi::load(TCODZip &zip) {

	range = zip.getInt();
	controlX = zip.getInt();
	controlY = zip.getInt();
	
}

void TurretAi::save(TCODZip &zip) {
	zip.putInt(TURRET);
	zip.putInt(range);
	zip.putInt(controlX);
	zip.putInt(controlY);
}
void TurretAi::update(Actor *owner)
{
	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
		owner->destructible->die(owner, NULL);

	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	
	
	
	if (engine.map->isVisible(owner->x,owner->y)) 
	{
		if(controlX != -1 && controlY != -1)
		{
			Actor *tc = engine.getAnyActor(controlX, controlY);
			TurretControlAi *ai = NULL;
			if(tc)
				ai = (TurretControlAi*)tc->ai;
			if(tc && ai && ai->attackMode == 0)
			{
			}
			else if (tc && ai && ai->attackMode == 1) //attack only the player mode
			{
				//compute the distance between the player and their companion then determine who to attack
				if(engine.player->companion && !engine.player->companion->destructible->isDead())
				{
					float d1 = engine.player->getDistance(owner->x,owner->y);
					float d2 = engine.player->companion->getDistance(owner->x, owner->y);
					
					if(d1 <= d2)
						attack(owner, engine.player);
					else
						attack(owner, engine.player->companion);
				}
				else
					attack(owner, engine.player);
			}
			else if(tc && ai && ai->attackMode == 2) //frenzy mode, turrets attack any nearby NPC, including the player
			{
				Actor *closest = NULL;
				float bestDistance = 1E6f;
				for (Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++) 
				{
					Actor *actor = *iterator;
					const char* name = actor->name;
					if (actor->destructible && actor != tc && !actor->destructible->isDead() && strcmp(name, "infected corpse") != 0 && actor != owner && actor->ch != 163 && actor->ch != 243 && actor->ch != 24 && actor->ch != 225 && actor->ch != 226) //243 = locker
					{
						float distance = actor->getDistance(owner->x,owner->y);
						if (distance < bestDistance && (distance <= range || range ==0.0f)) 
						{
							bestDistance = distance;
							closest = actor;
						}
					}
				}
				
				if(closest)
					attack(owner, closest);
				
			}
			else if(tc && ai && ai->attackMode == 3) //turrets only attack the non-enemy players
			{
				Actor *closest = NULL;
				float bestDistance = 1E6f;
				for (Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++) 
				{
					Actor *actor = *iterator;
					const char* name = actor->name;
					if (actor->destructible && actor != engine.player && actor != tc && !actor->destructible->isDead() && strcmp(name, "infected corpse") != 0 && actor != owner && actor->ch != 163 && actor->ch != 243 && actor->ch != 24 && actor->ch != 147 && actor->ch != 225 && actor->ch != 226 && actor != engine.player->companion) //243 = locker
					{
						float distance = actor->getDistance(owner->x,owner->y);
						if (distance < bestDistance && (distance <= range || range ==0.0f)) 
						{
							bestDistance = distance;
							closest = actor;
						}
					}
				}
				
				if(closest)
					attack(owner, closest);
			}		
		}
		else
		{
			if(engine.player->companion && !engine.player->companion->destructible->isDead())
			{
				float d1 = engine.player->getDistance(owner->x,owner->y);
				float d2 = engine.player->companion->getDistance(owner->x, owner->y);
				
				if(d1 <= d2)
					attack(owner, engine.player);
				else
					attack(owner, engine.player->companion);
			}
			else
				attack(owner, engine.player);
		}
	}
}

void TurretAi::attack(Actor *owner, Actor *target)
{
	int targetx = target->x;
	int targety = target->y;
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	float distance = sqrtf(dx*dx+dy*dy);
	
	if(distance <= range && owner->attacker) //turrets can only attack the player if they are in the range
	{
		owner->attacker->shoot(owner,target);
		if(target == engine.player)
			engine.damageReceived += (owner->totalDex - target->destructible->totalDodge);
	}
}

CleanerAi::CleanerAi() : moveCount(0){
	active = false;
	cleanPower = 1.1;
}

void CleanerAi::load(TCODZip &zip) {

	moveCount = zip.getInt();
	cleanPower = zip.getFloat();
	active = zip.getInt();
}

void CleanerAi::save(TCODZip &zip) {
	zip.putInt(CLEANER);
	zip.putInt(moveCount);
	zip.putFloat(cleanPower);
	zip.putInt(active);
}

void CleanerAi::update(Actor *owner) {

	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	
	for(int j = -2; j <= 2 && !active; j++)
		for(int i = -2; i <= 2; i++)
		{
			if((owner->x + i >= 0) && (owner->y + j >= 0) && engine.map->infectionState(owner->x + i, owner->y + j) >= 1 && engine.map->canWalk(owner->x + i,owner->y + j))
			{
				active = true;
				break;
			}
		}
	
	moveOrClean(owner);
	
}
void CleanerAi::moveOrClean(Actor *owner)
{
	if(!active) //only moveOrClean when active
		return;
		
	if(engine.map->tiles[owner->x+owner->y*(engine.map->width)].infection >0)
	{
		//clean tile
		engine.map->tiles[owner->x+owner->y*(engine.map->width)].infection =- cleanPower;
	}
	else //once a tile is clean, find a dirty tile to move towards or move randomly and clean next tile
	{

		int targetx = -1, targety = -1;
		for(int j = -1; j <= 1; j++)
			for(int i = -1; i <= 1; i++)
			{
				if((owner->x + i >= 0) && (owner->y + j >= 0) && engine.map->infectionState(owner->x + i, owner->y + j) >= 1 && engine.map->canWalk(owner->x + i,owner->y + j))
				{
					targetx = owner->x + i;
					targety = owner->y + j;
					break;
				}
			}
		
		if(targetx == -1) //meaning you can't find an nearby area with infection, so randomly move
		{
			TCODRandom *rng = TCODRandom::getInstance();
			int dx = rng->getInt(-1,1);
			int dy = rng->getInt(-1,1);
			if (dx != 0 || dy!=0) 
			{
				int destx = owner->x + dx;
				int desty = owner->y + dy;
				if (engine.map->canWalk(destx,desty)) {
					owner->x = destx;
					owner->y = desty;
				} 
				/*else { //for now, no attacking
					Actor *actor = engine.getActor(destx, desty);
					if (actor) {
						owner->attacker->attack(owner,actor);
						engine.map->computeFov();
					}
				}
					
				*/
			}
		}
		else //move towards area with infection
		{
			int dx = targetx - owner->x;
			int dy = targety - owner->y;
			int stepdx = (dx > 0 ? 1:-1);
			int stepdy = (dy > 0 ? 1:-1);
			float distance = sqrtf(dx*dx+dy*dy);
			dx = (int) (round(dx / distance));
			dy = (int)(round(dy / distance));
			if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
				owner->x+=dx;
				owner->y+=dy;
			} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
				owner->x += stepdx;
			} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
				owner->y += stepdy;
			}
		}
	}
}	

ConsoleAi::ConsoleAi() {
	//deployedSecurity = false;
	TCODRandom *rng = TCODRandom::getInstance();
	coins = rng->getInt(0,10);
	mapMine = true;
	//population = 1;
}

void ConsoleAi::save(TCODZip &zip){
	zip.putInt(CONSOLE);
	zip.putInt(mapMine);
	zip.putInt(coins);
	//zip.putInt(ink);
	//zip.putInt(population);
}

void ConsoleAi::load(TCODZip &zip){
	mapMine = zip.getInt();
	coins = zip.getInt();
	//population = zip.getInt();
}

void ConsoleAi::interaction(Actor *owner, Actor *target){
	engine.gameStatus = Engine::NEW_TURN;
	//if no printout yet
	if (!engine.printMap)
	{
		engine.gui->message(TCODColor::yellow,"You close the console and a map prints out, press 'm' to access it.");
		engine.printMap = true;
	}
	//if already printed
	else
	{
		engine.gui->message(TCODColor::yellow,"The console prints a map out, but you already have one.");
	}
	//engine.menuState = 4;
	engine.menuState = 4;
	while(engine.menuState != 2){
		TCODConsole::flush();
	}
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
	engine.menuState = 0;
}

InteractibleAi::InteractibleAi() {
}

void InteractibleAi::save(TCODZip &zip){
	zip.putInt(INTERACTIBLE);
}

void InteractibleAi::load(TCODZip &zip){
}

void InteractibleAi::update(Actor *owner){
if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
		owner->destructible->die(owner, NULL);
}

void InteractibleAi::interaction(Actor *owner, Actor *target){
}

TurretControlAi::TurretControlAi()
{
	attackMode = 1;
	locked = false;
}

void TurretControlAi::save(TCODZip &zip)
{
	zip.putInt(TURRETCONTROL);
	zip.putInt(attackMode);
	zip.putInt(locked);
}

void TurretControlAi::load(TCODZip &zip)
{
	attackMode = zip.getInt();
	locked = zip.getInt();
}

void TurretControlAi::update(Actor *owner)
{
if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
		owner->destructible->die(owner, NULL);
}

void TurretControlAi::interaction(Actor *owner, Actor *target)
{	
	TCODRandom *rang = TCODRandom::getInstance();
	int dice = rang->getInt(0,100);
	
	bool choice_made = false, first = true;
	if(locked)
	{
		engine.gui->message(TCODColor::orange, "The turret console is locked, and you may no longer access it.");
		return;
	}
	while (!choice_made) 
	{
		if (first) {
			TCODConsole::flush();
		}
		engine.gui->menu.clear();
		engine.gui->menu.addItem(Menu::DISABLE_TURRETS, "Disable all turrets in this room.");
		engine.gui->menu.addItem(Menu::DISABLE_IFF, "Make turrets hostile to all in this room.");
		engine.gui->menu.addItem(Menu::IDENTIFY_FRIENDLY, "Make turrets hostile to all except \n yourself and allies in this room");
		engine.gui->menu.addItem(Menu::EXIT, "Exit");
		Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::TURRET_CONTROL);
		switch (menuItem) {
			case Menu::DISABLE_TURRETS:
				if(dice <= 25 + 5*engine.player->intel || engine.player->job[0] == 'H')
				{
					attackMode = 0;
					engine.gui->message(TCODColor::orange, "Success. Turrets in this room have been disabled.");
				}
				else
				{
					attackMode = 1;
					engine.gui->message(TCODColor::orange, "Unauthorized access detected, the turret console is now locked.");
				}
				locked = true;
				choice_made = true;
				break;
			case Menu::DISABLE_IFF :
				if(dice <= 50 + 5*engine.player->intel || engine.player->job[0] == 'H')
				{
					attackMode = 2;
					engine.gui->message(TCODColor::orange, "Success. Turrets in this room have become hostile to all");
				}
				else
				{
					attackMode = 1;
					engine.gui->message(TCODColor::orange, "Unauthorized access detected, the turret console is now locked.");
				}
				locked = true;
				choice_made = true;
				break;
			case Menu::IDENTIFY_FRIENDLY :
				if(dice <= 15 + 5*engine.player->intel || engine.player->job[0] == 'H')
				{
					attackMode = 3;
					engine.gui->message(TCODColor::orange, "Success. Turrets in this room have become hostile to all except you and allies.");
				}
				else
				{
					attackMode = 1;
					engine.gui->message(TCODColor::orange, "Unauthorized access detected, the turret console is now locked.");
				}
				locked = true;
				choice_made = true;
				break;
			case Menu::EXIT :
				choice_made = true;
				break;
			case Menu::NO_CHOICE:
				first = false;
				break;
			default: break;
		}
	}
}


VendingAi::VendingAi() {
	deployedSecurity = false;
	TCODRandom *rng = TCODRandom::getInstance();
	ink = rng->getInt(10,100,65);
	population = 1;
}

void VendingAi::save(TCODZip &zip){
	zip.putInt(VENDING);
	zip.putInt(deployedSecurity);
	zip.putInt(ink);
	zip.putInt(population);
}

void VendingAi::load(TCODZip &zip){
	deployedSecurity = zip.getInt();
	ink = zip.getInt();
	population = zip.getInt();
}

void VendingAi::interaction(Actor *owner, Actor *target){
	engine.gui->message(TCODColor::yellow,"The vending machine lets out a soft hum.");
	if(population <= 1){
		populate(owner);
		population++;
	}
	vend(owner);
	engine.gameStatus = Engine::NEW_TURN;
	
}
void VendingAi::vendSidebar(){
	//create vending machine sidebar
	TCODConsole vendBar(16,32);
	vendBar.setDefaultBackground(TCODColor::black);
	vendBar.clear();
	vendBar.setDefaultForeground(TCODColor(200,180,50));
	vendBar.printFrame(0,0,16,32,true,TCOD_BKGND_ALPHA(50),"VENDING");
	
	vendBar.print(1,5,"Pbc: %d",engine.player->container->wallet);
	vendBar.print(1,7,"INK: %d",ink);
	TCODConsole::blit(&vendBar,0,0, 16, 32, TCODConsole::root, (engine.screenWidth / 2 - 38 / 2) + 4,(engine.screenHeight / 2 - 4 / 2) - 19);
}
void VendingAi::vend(Actor *owner){
	engine.gui->menu.clear();
	engine.gui->menu.addItem(Menu::ITEMS,"ITEMS");
	engine.gui->menu.addItem(Menu::TECH,"TECH");
	engine.gui->menu.addItem(Menu::ARMOR,"ARMOR");
	engine.gui->menu.addItem(Menu::WEAPONS, "WEAPONS");
	engine.gui->menu.addItem(Menu::EXIT, "EXIT");
	Actor *actor;
	bool select = true;
	bool itemBought = true;
	Menu::MenuItemCode menuItem;
	while(select){
		vendSidebar();
		menuItem = engine.gui->menu.pick(Menu::VENDING);
		
		switch(menuItem){
			case Menu::ITEMS:
				itemBought = true;
				while(itemBought){
					vendSidebar();
					actor = owner->ai->choseFromInventory(owner,1,true);
					if(actor){
						if(actor->pickable->value > engine.player->container->wallet){
							engine.gui->message(TCODColor::red,"You need more PetaBitCoins to make a purchase!");
							itemBought = false;
							select = false;
						}else if(actor->pickable->inkValue > ink){
							engine.gui->message(TCODColor::red,"The vending machine does not have enough ink to print the item!");
							itemBought = false;
							select = false;
						}else{
							Actor *purchase = clone(actor);
							engine.actors.push(purchase);
							purchase->pickable->pick(purchase,engine.player);
							engine.player->container->wallet -= actor->pickable->value;
							ink -= actor->pickable->inkValue;
							engine.gui->message(TCODColor::grey,"You purchased a %s", purchase->name);
						}
					}else{
						itemBought = false;
					}
				}
			break;
			case Menu::TECH:
				itemBought = true;
				while(itemBought){
					vendSidebar();
					actor = owner->ai->choseFromInventory(owner,2,true);
					if(actor){
						if(actor->pickable->value > engine.player->container->wallet){
							engine.gui->message(TCODColor::red,"You need more PetaBitCoins to make a purchase!");
							itemBought = false;
							select = false;
						}else if(actor->pickable->inkValue > ink){
							engine.gui->message(TCODColor::red,"The vending machine does not have enough ink to print the item!");
							itemBought = false;
							select = false;
						}else{
							Actor *purchase = clone(actor);
							engine.actors.push(purchase);
							purchase->pickable->pick(purchase,engine.player);
							engine.player->container->wallet -= actor->pickable->value;
							ink -= actor->pickable->inkValue;
							engine.gui->message(TCODColor::grey,"You purchased a %s", purchase->name);
							itemBought = false;
						}
					}else{
						itemBought = false;
					}
				}
			break;
			case Menu::ARMOR:
				itemBought = true;
				while(itemBought){
					vendSidebar();
					actor = owner->ai->choseFromInventory(owner,3,true);
					if(actor){
						if(actor->pickable->value > engine.player->container->wallet){
							engine.gui->message(TCODColor::red,"You need more PetaBitCoins to make a purchase!");
							itemBought = false;
							select = false;
						}else if(actor->pickable->inkValue > ink){
							engine.gui->message(TCODColor::red,"The vending machine does not have enough ink to print the item!");
							itemBought = false;
							select = false;
						}else{
							Actor *purchase = clone(actor);
							engine.actors.push(purchase);
							purchase->pickable->pick(purchase,engine.player);
							engine.player->container->wallet -= actor->pickable->value;
							ink -= actor->pickable->inkValue;
							engine.gui->message(TCODColor::grey,"You purchased a %s", purchase->name);
							itemBought = false;
						}
					}else{
						itemBought = false;
					}
				}
			break;
			case Menu::WEAPONS:
				itemBought = true;
				while(itemBought){
					vendSidebar();
					actor = owner->ai->choseFromInventory(owner,4,true);
					if(actor){
						if(actor->pickable->value > engine.player->container->wallet){
							engine.gui->message(TCODColor::red,"You need more PetaBitCoins to make a purchase!");
							itemBought = false;
							select = false;
						}else if(actor->pickable->inkValue > ink){
							engine.gui->message(TCODColor::red,"The vending machine does not have enough ink to print the item!");
							itemBought = false;
							select = false;
						}else{
							Actor *purchase = clone(actor);
							engine.actors.push(purchase);
							purchase->pickable->pick(purchase,engine.player);
							engine.player->container->wallet -= actor->pickable->value;
							ink -= actor->pickable->inkValue;
							engine.gui->message(TCODColor::grey,"You purchased a %s", purchase->name);
							itemBought = false;
						}
					}else{
						itemBought = false;
					}
				}
			break;
			case Menu::EXIT:
			select = false;
			break;
			case Menu::NO_CHOICE:
			break;
			default: break; 
		}
	}
}
Actor *VendingAi::clone(Actor *owner){
		Actor *droppy = new Actor(owner->x, owner->y, owner->ch,owner->name,owner->col);
		droppy->blocks = false;
		Pickable::PickableType type = owner->pickable->type;
		switch(type) {
			case Pickable::CURRENCY: break;
			case Pickable::HEALER: droppy->pickable = new Healer(((Healer*)owner->pickable)->amount); droppy->sort = 1; break;
			case Pickable::CHARGER: droppy->pickable = new Charger(((Charger*)owner->pickable)->amount); droppy->sort = 1; break;
			case Pickable::LIGHTNING_BOLT: droppy->pickable = new LightningBolt(((LightningBolt*)(owner->pickable))->range,((LightningBolt*)(owner->pickable))->damage); droppy->sort = 2; break;
			case Pickable::CONFUSER: droppy->pickable = new Confuser(((Confuser*)(owner->pickable))->nbTurns,((Confuser*)(owner->pickable))->range); droppy->sort = 2; break;
			case Pickable::FIREBALL: droppy->pickable = new Fireball(((Fireball*)(owner->pickable))->range,((Fireball*)(owner->pickable))->damage,((Fireball*)(owner->pickable))->maxRange); droppy->sort = 2; break;
			case Pickable::FLARE: droppy->pickable = new Flare(((Flare*)(owner->pickable))->nbTurns, ((Flare*)(owner->pickable))->range, ((Flare*)(owner->pickable))->lightRange); droppy->sort = 2; break;
			case Pickable::EQUIPMENT: droppy->pickable = new Equipment(0,((Equipment*)(owner->pickable))->slot,((Equipment*)(owner->pickable))->bonus,((Equipment*)(owner->pickable))->requirement); droppy->sort = owner->sort; break;
			case Pickable::FRAGMENT: droppy->pickable = new Fragment(((Fragment*)(owner->pickable))->range,((Fragment*)(owner->pickable))->damage,((Fragment*)(owner->pickable))->maxRange); droppy->sort = 2; break;
			case Pickable::WEAPON: droppy->pickable = new Weapon(((Weapon*)(owner->pickable))->minDmg,((Weapon*)(owner->pickable))->maxDmg,((Weapon*)(owner->pickable))->critMult,((Weapon*)(owner->pickable))->critRange,((Weapon*)(owner->pickable))->powerUse,((Weapon*)(owner->pickable))->wType,0,((Equipment*)(owner->pickable))->slot,((Equipment*)(owner->pickable))->bonus,((Equipment*)(owner->pickable))->requirement); droppy->sort = 4; break;
			case Pickable::FOOD: break; //I don't think FOOD should be in vending machines, interestingly enough. There are PCMUs for that
			case Pickable::KEY: break; //Keys probably shouldn't be in vending machines
			case Pickable::ALCOHOL: break; //NO ALCOHOL IN 3D PRINTERS!
			case Pickable::TELEPORTER: break; //no teleporters in 3D printers
			case Pickable::NONE: break;
		}
		return droppy;
}
void VendingAi::populate(Actor *owner){
	//Populates the vending machine with one of each item that can be purchased
	
	Actor *combatKnife = engine.map->createCombatKnife(0,0);
	engine.actors.push(combatKnife);
	combatKnife->pickable->pick(combatKnife,owner);
	
	Actor *mlr = engine.map->createMLR(0,0,true);
	engine.actors.push(mlr);
	mlr->pickable->pick(mlr,owner);
	
	Actor* myCap = engine.map->createMylarCap(0,0,true);
	engine.actors.push(myCap);
	myCap->pickable->pick(myCap,owner);
	
	Actor* myVest = engine.map->createMylarVest(0,0,true);
	engine.actors.push(myVest);
	myVest->pickable->pick(myVest,owner);
	
	Actor *myGreaves = engine.map->createMylarGreaves(0,0,true);
	engine.actors.push(myGreaves);
	myGreaves->pickable->pick(myGreaves,owner);
	
	Actor *myBoots = engine.map->createMylarBoots(0,0,true);
	engine.actors.push(myBoots);
	myBoots->pickable->pick(myBoots,owner);
	
	Actor *kevlarHelm = engine.map->createKevlarHelmet(0,0,true);
	engine.actors.push(kevlarHelm);
	kevlarHelm->pickable->pick(kevlarHelm,owner);
	
	Actor *kevlarVest = engine.map->createKevlarVest(0,0,true);
	engine.actors.push(kevlarVest);
	kevlarVest->pickable->pick(kevlarVest,owner);
	
	Actor *kevlarGreaves = engine.map->createKevlarGreaves(0,0,true);
	engine.actors.push(kevlarGreaves);
	kevlarGreaves->pickable->pick(kevlarGreaves,owner);
	
	Actor *kevlarBoots = engine.map->createKevlarBoots(0,0,true);
	engine.actors.push(kevlarBoots);
	kevlarBoots->pickable->pick(kevlarBoots,owner);
	
	Actor *titanHelm = engine.map->createTitanHelm(0,0,true);
	engine.actors.push(titanHelm);
	titanHelm->pickable->pick(titanHelm,owner);
	
	Actor *titanMail = engine.map->createTitanMail(0,0,true);
	engine.actors.push(titanMail);
	titanMail->pickable->pick(titanMail,owner);
	
	Actor *titanGreaves = engine.map->createTitanGreaves(0,0,true);
	engine.actors.push(titanGreaves);
	titanGreaves->pickable->pick(titanGreaves,owner);
	
	Actor *titanBoots = engine.map->createTitanBoots(0,0,true);
	engine.actors.push(titanBoots);
	titanBoots->pickable->pick(titanBoots,owner);
	
	Actor *medKit = engine.map->createHealthPotion(0,0);
	engine.actors.push(medKit);
	medKit->pickable->pick(medKit,owner);
	
	Actor *battery = engine.map->createBatteryPack(0,0);
	engine.actors.push(battery);
	battery->pickable->pick(battery,owner);
	
	Actor *flashBang = engine.map->createFlashBang(0,0);
	engine.actors.push(flashBang);
	flashBang->pickable->pick(flashBang,owner);
	
	Actor *flare = engine.map->createFlare(0,0);
	engine.actors.push(flare);
	flare->pickable->pick(flare,owner);
	
	Actor *fireBomb = engine.map->createFireBomb(0,0);
	engine.actors.push(fireBomb);
	fireBomb->pickable->pick(fireBomb,owner);
	
	Actor *frag = engine.map->createFrag(0,0);
	engine.actors.push(frag);
	frag->pickable->pick(frag,owner);
	
	Actor *emp = engine.map->createEMP(0,0);
	engine.actors.push(emp);
	emp->pickable->pick(emp,owner);
}

EngineerAi::EngineerAi(float repairPower, int deployRange) {
moveCount = 0;
turretDeployed = false;
this->repairPower = repairPower;
this->deployRange = deployRange;
}

void EngineerAi::save(TCODZip &zip){
	zip.putInt(ENGINEER);
	zip.putInt(turretDeployed);
	zip.putFloat(repairPower);
	zip.putInt(moveCount);
	zip.putInt(turretX);
	zip.putInt(turretY);
	zip.putInt(deployRange);
	
	
}

void EngineerAi::load(TCODZip &zip){
	turretDeployed = zip.getInt();
	repairPower = zip.getFloat();
	moveCount = zip.getInt();
	turretX = zip.getInt();
	turretY = zip.getInt();
	deployRange = zip.getInt();
}

void EngineerAi::update(Actor *owner)
{
	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);
	
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	Actor *comp = engine.player->companion;
	int compFov = 2;
	bool compTest =  comp && comp->destructible && !comp->destructible->isDead() && comp->getDistance(owner->x, owner->y) <= compFov;
	
	if (engine.map->isInFov(owner->x,owner->y) || compTest) {
		//can see the palyer, move towards him
		moveCount = TRACKING_TURNS;
	} else {
		moveCount--;
	}
	if (moveCount > 0) 
	{
		float d1 = 0;
		float d2 = 100;
		
		if(compTest)
		{
			d1 = engine.player->getDistance(owner->x,owner->y);
			d2 = engine.player->companion->getDistance(owner->x, owner->y);
		}
		
		if(d1 <= d2)
		{
			moveOrBuild(owner, engine.player, engine.player->x, engine.player->y);
		}
		else
		{
			moveOrBuild(owner, comp,comp->x, comp->y);
		}
	} else {
		moveCount = 0;
	}
	owner->destructible->takeFireDamage(owner, 3.0);
}

void EngineerAi::moveOrBuild(Actor *owner, Actor *target, int targetx, int targety)
{
	int x = owner->x, y = owner->y;
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	float distance = sqrtf(dx*dx+dy*dy);
	bool viewTest = false;
	
	if(target == engine.player)
		viewTest = engine.map->isInFov(owner->x,owner->y);
	else if(target && target == engine.player->companion)
	{
		int compFov = 2;
		float dComp = target->getDistance(owner->x, owner->y);
		viewTest =  dComp <= compFov;
	}
	if(viewTest && !turretDeployed && distance <= deployRange) //and deployed range
	{//try to deploy turret
	
	
		bool case1 = engine.map->canWalk(x+1, y) && engine.getAnyActor(x + 1,y) == NULL && dx >= 0 && dy == 0;
		bool case2 =  engine.map->canWalk(x+1, y-1) && engine.getAnyActor(x + 1,y-1) == NULL && dx > 0 && dy < 0;
		bool case3 =  engine.map->canWalk(x, y-1) && engine.getAnyActor(x,y-1) == NULL && dy <= 0 && dx == 0;
		bool case4 =  engine.map->canWalk(x, y+1) && engine.getAnyActor(x,y+1) == NULL && dy >= 0 && dx == 0;
		bool case5 =  engine.map->canWalk(x-1, y+1) && engine.getAnyActor(x-1,y+1) == NULL && dy > 0 && dx < 0;
		bool case6 =  engine.map->canWalk(x-1, y) && engine.getAnyActor(x-1,y) == NULL && dx <= 0 && dy == 0;
		bool case7 =  engine.map->canWalk(x-1, y-1) && engine.getAnyActor(x-1,y-1) == NULL && dy < 0 && dx < 0;
		bool case8 = engine.map->canWalk(x+1, y+1) && engine.getAnyActor(x + 1,y+1) == NULL&& dy > 0 && dx > 0;
		
		if(case1)
		{
			turretX = x+1;
			turretY = y;
			turretDeployed = true;
		}else if(case2)
		{
			turretX = x+1;
			turretY = y-1;
			turretDeployed = true;
		}else if(case3)
		{
			turretX = x;
			turretY = y-1;
			turretDeployed = true;
		}else if(case4)
		{
			turretX = x;
			turretY = y+1;
			turretDeployed = true;
		}else if(case5)
		{
			turretX = x-1;
			turretY = y+1;
			turretDeployed = true;
		}else if(case6)
		{
			turretX = x-1;
			turretY = y;
			turretDeployed = true;
		}
		else if(case7)
		{
			turretX = x-1;
			turretY = y-1;
			turretDeployed = true;
		}
		else if(case8)
		{
			turretX = x+1;
			turretY = y+1;
			turretDeployed = true;
		}
		
		if(turretDeployed)
		{
			if(engine.map->isVisible(owner->x, owner->y))
				engine.gui->message(TCODColor::red, "The %s is deploying a sentry turret!", owner->name);
			engine.map->createTurret(turretX,turretY);
		}
		
		
	}else if(turretDeployed)
	{
		Actor *turret = engine.getAnyActor(turretX, turretY);
		if(!turret->destructible->isDead() && turret->destructible->hp < turret->destructible->maxHp)
		{		//repair turret
			if(engine.map->isVisible(owner->x, owner->y))
				engine.gui->message(TCODColor::red, "The %s is repairing their sentry turret!", owner->name);
			turret->destructible->heal(repairPower);
		}	
		else if(turret->destructible->isDead())
		{//attack player normally
			int dx = targetx - owner->x;
			int dy = targety - owner->y;
			int stepdx = (dx > 0 ? 1:-1);
			int stepdy = (dy > 0 ? 1:-1);
			float distance = sqrtf(dx*dx+dy*dy);

			if (distance >= 2) {
				dx = (int) (round(dx / distance));
				dy = (int)(round(dy / distance));
				if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
					owner->x+=dx;
					owner->y+=dy;
				} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
					owner->x += stepdx;
				} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
					owner->y += stepdy;
				}
				if (owner->oozing) {
					engine.map->infectFloor(owner->x, owner->y);
				}
				
			} else if (owner->attacker) {
				owner->attacker->attack(owner,target);
				if(target == engine.player)
					engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDodge);
			}
		}
	}

}


LockerAi::LockerAi(){
locked = false;
}
void LockerAi::save(TCODZip &zip){
zip.putInt(LOCKER);
zip.putInt(locked);

}
void LockerAi::load(TCODZip &zip){
locked = zip.getInt();
}
void LockerAi::interaction(Actor *owner, Actor *target){

	std::cout << "got to interact" << std::endl;
	//this line of code causes the locker dropping flavor text to never be printed, is that intentional?
	if (engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 23){
		engine.map->tiles[owner->x+owner->y*engine.map->width].decoration = 24;
	}
	//weapon racks to empty racks
	else if (engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 54){
		engine.map->tiles[owner->x+owner->y*engine.map->width].decoration = 85;
	}
	//owner->ch = 243;
	if(owner->container && !owner->container->inventory.isEmpty())
	{
		if (engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 23){
			engine.gui->message(TCODColor::lightGrey,"The locker opens with a creak as it spills its forgotten contents.");
		} else if (engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 44){
			engine.gui->message(TCODColor::lightGrey,"The PCMU beeps and spits out a brick of foodstuffs.");
			owner->col = TCODColor::red;
			owner->name = "Used PCMU Food Processor";
		}
		else if(engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 56)
		{
		
		
			bool choice_made = false, first = true;
			bool hasKey = false;
			Actor *key;
			//check if the player has a key
			for (Actor **it = engine.player->container->inventory.begin(); it != engine.player->container->inventory.end(); it++) 
			{
				Actor *actor = *it;
				if(actor->ch == 190)
				{
					hasKey = true;
					key = actor;
					break;
				}
			}
			if(!hasKey)
				engine.gui->message(TCODColor::blue, "The %s appears to be locked, perhaps a key is needed.", owner->name);
			while (!choice_made && locked && hasKey) 
			{
				if (first) {
					TCODConsole::flush();
				}
				engine.gui->menu.clear();
				engine.gui->menu.addItem(Menu::UNLOCK_VAULT, "Unlock weapon vault with a single use key");
				engine.gui->menu.addItem(Menu::EXIT, "Exit");
				Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::KEY_MENU);
				switch (menuItem) {
					case Menu::UNLOCK_VAULT:
						locked = false;
						((Key*)key->pickable)->used = true;
						if(key->pickable->use(key, engine.player))
						{
							engine.gui->message(TCODColor::blue, "The %s opens!", owner->name);
							locked = false;
						}
						choice_made = true;
						engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration = 57;
						engine.save();
						break;
					case Menu::EXIT :
						choice_made = true;
						break;
					case Menu::NO_CHOICE:
						first = false;
						break;
					default: break;
				}
			}
		}
		if(!locked)
		{
			std::cout << "got here" << std::endl;
			Actor **iterator=owner->container->inventory.begin();
			for(int i = 0; i < owner->container->size; i++)
			{
				if(owner->container->inventory.isEmpty())
				{
					break;
				}
				Actor *actor = *iterator;
				if(actor)
				{
					actor->pickable->drop(actor,owner,true);
				}				
				if(iterator != owner->container->inventory.end())
				{
					++iterator;
				}
			}
			
		}
	}
	else if(engine.map->tiles[owner->x+(owner->y)*engine.map->width].decoration == 57) //open vault
	{
			engine.gui->message(TCODColor::blue,"The %s has been opened and is now empty.", owner->name);
	}
}

GardnerAi::GardnerAi()
{
	initX1 = -1;
	initY1 = -1;
	moveCount = 0;
}

void GardnerAi::load(TCODZip &zip)
{
	moveCount = zip.getInt();
	initX1 = zip.getInt();
	initY1 = zip.getInt();
	initX2 = zip.getInt();
	initY2 = zip.getInt();
}
void GardnerAi::save(TCODZip &zip)
{
	zip.putInt(GARDNER);
	zip.putInt(moveCount);
	zip.putInt(initX1);
	zip.putInt(initY1);
	zip.putInt(initX2);
	zip.putInt(initY2);
}

void GardnerAi::update(Actor *owner)
{
	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
		owner->destructible->die(owner, NULL);
		
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	if(!owner->hostile)
	{
		moveOrAttack(owner, 0,0);
		return;
	}
	Actor *comp = engine.player->companion;
	int compFov = 2;
	bool compTest =  comp && comp->destructible && !comp->destructible->isDead() && comp->getDistance(owner->x, owner->y) <= 	compFov;
	
	if (engine.map->isInFov(owner->x,owner->y) || compTest) {
		//can see the palyer/companion, move towards him
		moveCount = TRACKING_TURNS;
	} else {
		moveCount--;
	}
	float d1 = 0;
	float d2 = 100;
	
	if(compTest)
	{
		d1 = engine.player->getDistance(owner->x,owner->y);
		d2 = engine.player->companion->getDistance(owner->x, owner->y);
	}
	Actor *target = engine.player;
	if(d1 > d2)
	{
		target = comp;
	}
	
	//the Gardner will move towards the player/companion if he is in the garden and is hostile
	if (moveCount > 0 ||(target->x <= initX2 && target->x >= initX1 && target->y >= initY1 && target->y <= initY2)) {
		MonsterAi::moveOrAttack(owner, target, target->x, target->y);
	} else {
		moveCount = 0;
	}
	owner->destructible->takeFireDamage(owner, 3.0);

}

void GardnerAi::moveOrAttack(Actor *owner, int targetx, int targety)
{
	//if the player bumps in the gardner and not hostile, then I need to do flavor text, possibly show in playerAi
	int x = owner->x;
	int y = owner->y;

	
	if(!owner->hostile)
	{
		if(engine.map->canWalk(x+1, y) && y == initY1 && x+1 <= initX2)
			owner->x = x + 1;
		else if(engine.map->canWalk(x, y+1) && x == initX2 && y+1 <= initY2)
		{
			owner->y = y + 1;
		}
		else if(engine.map->canWalk(x-1, y) && y == initY2 && x-1 >= initX1)
		{
			owner->x = x - 1;
		}
		else if(engine.map->canWalk(x, y-1) && x == initX1 && y-1 >= initY1)
		{
			owner->y = y - 1;
		}
			
		
	}
}

FruitAi::FruitAi(Actor *keeper, int limit) : keeper(keeper),limit(limit) {
}

void FruitAi::save(TCODZip &zip){
	zip.putInt(FRUIT);
	zip.putInt(limit);
	keeper->save(zip);
}

void FruitAi::load(TCODZip &zip){
	limit = zip.getInt();
	
	Actor *act = new Actor(0,0,0,NULL,TCODColor::white);
	act->load(zip);
	keeper = act;
}


void FruitAi::interaction(Actor *owner, Actor *target){
	
	if ( !((FruitAi*)owner->ai)->keeper->destructible->isDead() && ((FruitAi*)owner->ai)->keeper->hostile != true){
		((FruitAi*)owner->ai)->keeper->hostile = true;
		engine.gui->message(TCODColor::red,"The %s seems angry that you've picked his fruit!",((FruitAi*)owner->ai)->keeper->name);
	}
	
	TCODRandom *rng = TCODRandom::getInstance();
	int stacksize = rng->getInt(1,3);
	Actor *fruit = new Actor(0,0,14,owner->name,TCODColor::white);
	fruit->sort = 1;
	fruit->blocks = false;
	fruit->pickable = new Food(stacksize);
	fruit->pickable->value = 25;
	fruit->pickable->inkValue = 10;
	fruit->hunger = owner->hunger;
	
	if (((FruitAi*)owner->ai)->limit > 0){
		if (target->container && target->container->add(fruit)) {
			engine.gui->message(TCODColor::green,"You pick some %s",fruit->name);
			((FruitAi*)owner->ai)->limit -= 1;
		} else {
			engine.gui->message(TCODColor::grey,"Inventory is full.");
		}
		if (((FruitAi*)owner->ai)->limit == 0){
			engine.actors.remove(owner);
			delete owner;
		}
		
	}
}

ZedAi::ZedAi() : moveCount(0), range(3), berserk (false), menuPopped(false){
}

void ZedAi::load(TCODZip &zip) {
	moveCount = zip.getInt();
	range = zip.getInt();
	berserk = zip.getInt();
	menuPopped = zip.getInt();

}

void ZedAi::save(TCODZip &zip) {
	zip.putInt(ZED);
	zip.putInt(moveCount);
	zip.putInt(range);
	zip.putInt(berserk);
	zip.putInt(menuPopped);
}

void ZedAi::update(Actor *owner) {
	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
			owner->destructible->die(owner, NULL);


	if (owner->destructible && owner->destructible->isDead()) {
		if (!menuPopped) {
			deathMenu();
			menuPopped = true;
		}
		return;
	}

	//if really hurt go berserk
	if (!berserk && owner->destructible->hp < owner->destructible->maxHp/3) {
		berserk = true;
		engine.gui->message(TCODColor::darkPurple, "<Zed> Muh Ha! Now you'll witness my true power.");
		owner->destructible->maxHp = owner->destructible->maxHp*2;
		owner->destructible->hp = owner->destructible->maxHp;
		engine.gui->message(TCODColor::red, "Zed Umber is going berserk!");
	}
	Actor *comp = engine.player->companion;
	int compFov = 2;
	bool compTest =  comp && comp->destructible && !comp->destructible->isDead() && comp->getDistance(owner->x, owner->y) <= compFov;
	if (engine.map->isInFov(owner->x,owner->y) || compTest) {
		//can see the player, move towards him
		moveCount = TRACKING_TURNS + 10; //give zed much longer tracking
	}
	else {
		moveCount--;
	}
	if (moveCount > 0) 
	{
			float d1 = 0;
			float d2 = 100;
			
			if(compTest)
			{
				d1 = engine.player->getDistance(owner->x,owner->y);
				d2 = engine.player->companion->getDistance(owner->x, owner->y);
			}
			
			if(d1 <= d2)
			{
				moveOrAttack(owner, engine.player, engine.player->x, engine.player->y);
			}
			else
			{
				moveOrAttack(owner, comp,comp->x, comp->y);
			}
	} 
	else {
		moveCount = 0;
	}
	//does a check if the floor is on fire
	owner->destructible->takeFireDamage(owner, 3.0);
}
void ZedAi::moveOrAttack(Actor *owner, Actor *target, int targetx, int targety)
{
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	
	int dxL = target->lastX - owner->x;
	int dyL = target->lastY - owner->y;
	int stepdxL = (dxL > 0 ? 1:-1);
	int stepdyL = (dyL > 0 ? 1:-1);
	stepdxL = (dxL == 0 ? 0:stepdxL);
	stepdyL = (dyL == 0 ? 0:stepdyL);
	float distance = sqrtf(dx*dx+dy*dy);
	
	//If the distance > range, then the rangedAi will move towards the player
	//If the distance <= range, then the rangedAi will shoot the player unless the player is right next the rangedAi
	if (distance > range) {
		dx = (int) (round(dx / distance));
		dy = (int)(round(dy / distance));
		if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
			owner->x+=dx;
			owner->y+=dy;
		} else if (engine.map->canWalk(owner->x+stepdxL,owner->y+stepdyL)) {
			//engine.gui->message(TCODColor::red,"Companion chasing last known location");
			owner->x+=stepdxL;
			owner->y+=stepdyL;
		} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
			owner->y += stepdy;
		}
		if (owner->oozing) {
			engine.map->infectFloor(owner->x, owner->y);
		}
		//not next to the player
	} 

	else if (!berserk && distance !=1 && owner->attacker) {
		TCODRandom *rng = TCODRandom::getInstance();
		int dice = rng->getInt(0,99);
		float dPlayer = engine.player->getDistance(owner->x, owner->y); //distance between zed and the player
		if (dice < 70) 
		{	
			owner->attacker->shoot(owner,target);
			if(target == engine.player)
				engine.damageReceived += (owner->totalDex-engine.player->destructible->totalDodge);
		}
		//taunting
		else if(target == engine.player || dPlayer <= 3) {
			int tauntDice = rng->getInt(0,4);
			switch (tauntDice) {
				case 0:
					engine.gui->message(TCODColor::darkPurple, "<Zed> Prepare for your doom!"); break;
				case 1:
					engine.gui->message(TCODColor::darkPurple, "<Zed> I'm Zed Umber. Muh Ha Ha..."); break;
				case 2:
					engine.gui->message(TCODColor::darkPurple, "<Zed> Muh Ha..."); break;
				case 3:
					engine.gui->message(TCODColor::darkPurple, "<Zed> Cough, cough..."); break;
				case 4:
					engine.gui->message(TCODColor::darkPurple, "<Zed> Hold on... Gotta light this e-cig."); break;
				case 5:
					engine.gui->message(TCODColor::darkPurple, "<Zed> Mu Ha Hah!"); break;
				case 6:
					engine.gui->message(TCODColor::darkPurple, "<Zed> Ha! You are already dead! "); break;
			}
		}
		//standing next to the player
	}
	else if (owner->attacker) {
		if (berserk) {
		dx = (int) (round(dx / distance));
		dy = (int)(round(dy / distance));
		if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
			owner->x+=dx;
			owner->y+=dy;
		} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
			owner->y += stepdy;
		}
		if (owner->oozing) {
			engine.map->infectFloor(owner->x, owner->y);
		}
		}
		if (distance == 1) {
			owner->attacker->attack(owner,target);
			if(target == engine.player)
				engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDodge);
		}
	}
}

void ZedAi::deathMenu() {
	bool choice_made = false;
	while (!choice_made) 
	{
		engine.gui->menu.clear();
		engine.gui->menu.addItem(Menu::END_GAME, "Escape the spacestation.");
		engine.gui->menu.addItem(Menu::CONTINUE_GAME, "Continue to explore.");
		Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::GAME_END);
		switch (menuItem) {
			case Menu::END_GAME:
				engine.gui->message(TCODColor::orange, "Game Over: You win!");
				choice_made = true;
				TCODSystem::deleteFile("game.sav");
				exit(0);
				break;
			case Menu::CONTINUE_GAME:
					engine.gui->message(TCODColor::orange, "The adventure never ends!");
				choice_made = true;
				break;
			case Menu::NO_CHOICE:
				break;
			default: break;
		}
	}
}

CompanionAi::CompanionAi(Actor *tamer, int rangeLimit, Command command):tamer(tamer),edible(false),att(STANDARD),period(40),rangeLimit(rangeLimit),assignedX(0),assignedY(0),command(command){
}

void CompanionAi::save(TCODZip &zip){
	zip.putInt(COMPANION);
	zip.putInt(edible);
	std::cout<<"AI put edible" << edible << std::endl;
	zip.putInt(att);
	zip.putInt(period);
	zip.putInt(rangeLimit);
	std::cout<<"AI put limit" << rangeLimit << std::endl;
	zip.putInt(assignedX);
	std::cout<<"AI put X" << assignedX << std::endl;
	zip.putInt(assignedY);
	std::cout<<"AI put Y" << assignedY << std::endl;
	zip.putInt(command);
	std::cout<<"AI put command" << command << std::endl;
}

void CompanionAi::load(TCODZip &zip){
	edible = zip.getInt();
	att = (Attitude)zip.getInt();
	period = zip.getInt();
	std::cout<<"AI got edible" << edible << std::endl;
	rangeLimit = zip.getInt();
	std::cout<<"AI got limit" << rangeLimit << std::endl;
	assignedX = zip.getInt();
	std::cout<<"AI got X" << assignedX << std::endl;
	assignedY = zip.getInt();
	std::cout<<"AI got Y" << assignedY << std::endl;
	command = (Command)zip.getInt();
	std::cout<<"AI got command" << command << std::endl;

	tamer = engine.player;
}

void CompanionAi::update(Actor *owner){

	if(owner->destructible && !owner->destructible->hasDied && owner->destructible->hp <= 0)
		owner->destructible->die(owner, NULL);


	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	
	if (owner->destructible && !owner->destructible->isDead() && engine.turnCount % (((CompanionAi*)(owner->ai))->period) == 0){
		periodicMessage(owner);
	}
	
	if (command == STAY){
		return;
	}
	else if (command == GUARD_POINT ){
		if(owner->getDistance(assignedX,assignedY) != 0){
			int dx = assignedX - owner->x;
			int dy = assignedY - owner->y;
			int stepdx = (dx > 0 ? 1:-1);
			int stepdy = (dy > 0 ? 1:-1);
			float distance = sqrtf(dx*dx+dy*dy);

			dx = (int) (round(dx / distance));
			dy = (int)(round(dy / distance));
			if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
				owner->x+=dx;
				owner->y+=dy;
			} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
				owner->x += stepdx;
			} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
				owner->y += stepdy;
			}
			if (owner->oozing) {
				engine.map->infectFloor(owner->x, owner->y);
			}
		}else{
			Actor *closestMonster = engine.getClosestMonster(owner->x, owner->y, 1.5);
			if (!closestMonster){
				return;
			}
			if (owner->attacker) {
				owner->attacker->attack(owner,closestMonster);
			}
		}
	
	}
	else if (command == FOLLOW){
		bool targeting = false;
		if (tamer->attacker->lastTarget != NULL && !tamer->attacker->lastTarget->destructible->isDead()){
			if (owner->attacker && tamer->attacker->lastTarget != owner && tamer->attacker->lastTarget != tamer){
				owner->attacker->lastTarget = tamer->attacker->lastTarget;
				//engine.gui->message(TCODColor::violet,"<%s> I will protect you from that %s!",owner->name,owner->attacker->lastTarget->name);
				if (tamer->getDistance(owner->attacker->lastTarget->x,owner->attacker->lastTarget->y) <= rangeLimit){
					targeting = true;
					moveOrAttack(owner,owner->attacker->lastTarget->x,owner->attacker->lastTarget->y);
				}
			}
		} 
		if(targeting==false && owner->getDistance(tamer->x,tamer->y) >= 2){
			moveOrAttack(owner,tamer->x,tamer->y);
		}
	} else if (command == ATTACK) {
		if (owner->attacker->lastTarget && !owner->attacker->lastTarget->destructible->isDead()){
			moveOrAttack(owner,owner->attacker->lastTarget->x,owner->attacker->lastTarget->y);
		}
	}
}

void CompanionAi::moveOrAttack(Actor *owner, int targetx, int targety){
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	
	int dxL = engine.player->lastX - owner->x;
	int dyL = engine.player->lastY - owner->y;
	int stepdxL = (dxL > 0 ? 1:-1);
	int stepdyL = (dyL > 0 ? 1:-1);
	stepdxL = (dxL == 0 ? 0:stepdxL);
	stepdyL = (dyL == 0 ? 0:stepdyL);
	float distance = sqrtf(dx*dx+dy*dy);
	
	if(owner->ch == 150 && distance >= 2 && engine.turnCount % 2 == 0)
	{
		//crawlers can only move every other turn
		return;
	}
	
	if (distance >= 2) {
		dx = (int) (round(dx / distance));
		dy = (int) (round(dy / distance));
		if (engine.map->canWalk(owner->x+dx,owner->y+dy)) {
			//engine.gui->message(TCODColor::red,"Companion chasing known location");
			owner->x+=dx;
			owner->y+=dy;
		} else if (engine.map->canWalk(owner->x+stepdxL,owner->y+stepdyL)) {
			//engine.gui->message(TCODColor::red,"Companion chasing last known location");
			owner->x+=stepdxL;
			owner->y+=stepdyL;
		} else if (engine.map->canWalk(owner->x+stepdx,owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x,owner->y+stepdy)) {
			owner->y += stepdy;
		}
		if (owner->oozing) {
			engine.map->infectFloor(owner->x, owner->y);
		}
	} else if (owner->attacker) {
		owner->attacker->attack(owner,owner->attacker->lastTarget);
	}
}

float CompanionAi::feedMaster(Actor *owner, Actor *master){

	TCODRandom *spagoo = TCODRandom::getInstance();
	int switcher = spagoo->getInt(0,5);
	switch(switcher){
		case 0:	engine.gui->message(TCODColor::violet,"<%s> WWWWAAAAAAAAUUUUGGGGGGHH!!!",owner->name); break;
		case 1:	engine.gui->message(TCODColor::violet,"<%s> AAAAAAAAaaaaaGGGGGGHH!!!",owner->name); break;
		case 2:	engine.gui->message(TCODColor::violet,"<%s> WHYYY!?!?",owner->name); break;
		case 3:	engine.gui->message(TCODColor::violet,"<%s> GEEEYAAAAGGGHH!!!",owner->name); break;
		case 4:	engine.gui->message(TCODColor::violet,"<%s> EEEEYAAAAGH",owner->name); break;
		case 5: engine.gui->message(TCODColor::violet,"<%s> AAAAAAAAUUUUGGGGGGHH!!!",owner->name); break;
	}
	
	if(edible){
		if (owner->destructible == NULL || owner->destructible->isDead()) {
			return 0;
		}
		owner->destructible->takeDamage(owner,master,owner->destructible->maxHp*0.2);
		return master->feed(master->maxHunger);
	} else{
		owner->destructible->takeDamage(owner,master,owner->destructible->maxHp*0.2);
		return 0;
	}
}

void CompanionAi::periodicMessage(Actor *owner){
	switch(((CompanionAi*)(owner->ai))->att){
		case EDIBLE: {
			if (tamer->hunger > 0){
				engine.gui->message(TCODColor::violet,"<%s> on a scale from 1 to 100, your hunger is %d",owner->name,tamer->hunger*100/tamer->maxHunger);
			} else {
				engine.gui->message(TCODColor::violet, "<%s> You look very hungry... You can take a bite out of me with 'u', you know.",owner->name);
			}
			break;
		}
		case CAPYBARA:{
			engine.gui->message(TCODColor::violet, "<%s> I am a cute fluffball. I will try to protect you!",owner->name);
			break;
		}
		case DRONE: {
			TCODRandom *rando = TCODRandom::getInstance();
			int switcher = rando->getInt(1,7);
			switch (switcher)
			{
				case 1:
					engine.gui->message(TCODColor::violet, "<%s> I have detected danger in your area.",owner->name);
					break;
				case 2:
					engine.gui->message(TCODColor::violet, "<%s> You better watch out, this ain't no Care Bear Game.",owner->name);
					break;
				case 3:
					engine.gui->message(TCODColor::violet, "<%s> The Robot Overlords have sent me a message commending your bravery.",owner->name);
					break;
				case 4:
					engine.gui->message(TCODColor::violet, "<%s> Malware detetected.",owner->name);
					break;
				case 5:
					engine.gui->message(TCODColor::violet, "<%s> The Robot Overlords would be pleased with your progress.",owner->name);
					break;
				case 6:
					engine.gui->message(TCODColor::violet, "<%s> My last owner was not this successful.",owner->name);
					break;
				case 7:
					engine.gui->message(TCODColor::violet, "<%s> Just between us, I never liked humans.",owner->name);
					break;
				default:break;
			}
			break;
		}
		default: {
			engine.gui->message(TCODColor::violet,"<%s> I have yet to be accounted for!", owner->name);
			
		}
	}
}

