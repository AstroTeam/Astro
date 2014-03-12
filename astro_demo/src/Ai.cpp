#include <stdio.h>
#include <math.h>
#include "main.hpp"

Ai *Ai::create(TCODZip &zip) {
	AiType type = (AiType)zip.getInt();
	Ai *ai = NULL;
	switch(type) {
		case PLAYER: ai = new PlayerAi(); break;
		case MONSTER: ai = new MonsterAi(); break;
		case CONFUSED_ACTOR: ai = new ConfusedActorAi(0,NULL); break;
	    case EPICENTER: ai = new EpicenterAi(); break;
		case RANGED: ai = new RangedAi(); break;
		case LIGHT: ai = new LightAi(0,0); break;
		case FLARE: ai = new FlareAi(0,0); break;
	}
	ai->load(zip);
	return ai;
}
Actor *Ai::choseFromInventory(Actor *owner,int type) {
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
			if(actor->pickable->type == Pickable::EQUIPMENT && ((Equipment*)(actor->pickable))->equipped){
				inventoryScreen->print(2,y,"(%c) %s(E)",shortcut,actor->name);
			}else{
				inventoryScreen->print(2,y,"(%c) %s",shortcut,actor->name);
			}
			owner->container->select[shortcut] = actor->name;
			if (actor->pickable->stacks) {
				inventoryScreen->print(17, y, "(%d)",actor->pickable->stackSize);
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
		Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::PAUSE);
	
		switch (menuItem) {
			case Menu::CONSTITUTION	:
				owner->destructible->maxHp += 20;
				owner->destructible->hp += 20;
				owner->vit += 1;
				choice_made = true;
				break;
			case Menu::STRENGTH :
				owner->attacker->basePower += 1;
				owner->attacker->totalPower += 1;
				owner->str += 1;
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
				owner->destructible->baseDefense += 1;
				owner->destructible->totalDefense += 1;
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
	int dx =0, dy =0;
	switch (engine.lastKey.vk) {
	case TCODK_UP: case TCODK_KP8: dy = -1; break;
	case TCODK_DOWN: case TCODK_KP2: dy = 1; break;
	case TCODK_LEFT: case TCODK_KP4: dx =-1; break;
	case TCODK_RIGHT: case TCODK_KP6: dx = 1; break;
	case TCODK_KP7: dy = dx = -1; break;
	case TCODK_KP9: dy = -1; dx = 1; break;
	case TCODK_KP1: dx = -1; dy = 1; break;
	case TCODK_KP3: dx = dy = 1; break;
	case TCODK_TAB: 
		engine.save();
		engine.gui->message(TCODColor::pink, "saved"); break;
	case TCODK_CONTROL: 
		engine.player->x = engine.stairs->x;
		engine.playerLight->x = engine.stairs->x;
		engine.player->y = engine.stairs->y;
		engine.playerLight->y = engine.stairs->y;
		
		engine.map->computeFov(); break;
		
	case TCODK_PRINTSCREEN:
		TCODSystem::saveScreenshot(NULL);
		engine.gui->message(TCODColor::orange,"screenshot saved"); break;
	case TCODK_KP5: engine.map->computeFov(); engine.gameStatus = Engine::NEW_TURN; break;
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
	if (engine.map->isWall(targetx, targety) ) return false;
	if (!owner->attacker) return false;
	
	for (Actor **iterator = engine.actors.begin();
		iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor->blocks && actor->x == targetx &&actor->y == targety) {
			if (actor->destructible && !actor->destructible->isDead()) {
				owner->attacker->attack(owner, actor);
				engine.damageDone += owner->attacker->totalPower;
			}
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
	return true;
}

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
	//bool first = true;
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
						engine.gui->message(TCODColor::lightGrey, "You pick up %d %s.", actor->pickable->stackSize, actor->name);
						break;
					} else if (!found ) {
						found = true;
						engine.gui->message(TCODColor::red, "Your inventory is full.");
					}
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
		case 'i': //display inventory
		{ 
			engine.map->computeFov();
			engine.gui->menu.clear();
			//TCODConsole::root->clear();
			engine.invState = 1;
			engine.gui->menu.addItem(Menu::ITEMS, "ITEMS");
			engine.gui->menu.addItem(Menu::TECH, "TECH");
			engine.gui->menu.addItem(Menu::ARMOR, "ARMOR");
			engine.gui->menu.addItem(Menu::WEAPONS, "WEAPONS");
			engine.gui->menu.addItem(Menu::EXIT, "EXIT");
			//Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::INVENTORY);
			Actor *actor;
			bool choice = true;
			while (engine.invState != 4){
				TCODConsole::flush();
			}
			//TCODConsole::root->clear();
			
			while(choice){
			
			Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::INVENTORY);
			//TCODConsole::root->clear();
				switch (menuItem) {
					case Menu::ITEMS :
						actor = choseFromInventory(owner,1);
						if(actor)
							choice = false;
						break;
					case Menu::TECH :
						actor = choseFromInventory(owner,2);
						if(actor)
							choice = false;
						break;
					case Menu::ARMOR:
						actor = choseFromInventory(owner,3);
						if(actor)
							choice = false;
						break;
					case Menu::WEAPONS:
						actor = choseFromInventory(owner,4);
						if(actor)
							choice = false;
						break;
					case Menu::NO_CHOICE:
						break;
					case Menu::EXIT:
						choice = false;
					default: break;
				}
			}
			//TCODConsole::root->clear();
			engine.invState = 0;
			engine.invFrames = 0;
			if (actor) {
				
				bool used;
				used = actor->pickable->use(actor,owner);
				if (used) {
					engine.gameStatus = Engine::NEW_TURN;
				}
			}
		}break;
		case 'd': //drop an item
		{
			engine.map->computeFov();
			engine.gui->menu.clear();
			engine.invState = 1;
			engine.gui->menu.addItem(Menu::ITEMS, "Items");
			engine.gui->menu.addItem(Menu::TECH, "Tech");
			engine.gui->menu.addItem(Menu::ARMOR, "Armor");
			engine.gui->menu.addItem(Menu::WEAPONS, "Weapons");
			engine.gui->menu.addItem(Menu::EXIT, "Exit");
			//Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::INVENTORY);
			Actor *actor;
			bool choice = true;
			while (engine.invState != 4){
				TCODConsole::flush();
			}
			while(choice){
			//inventoryScreen->setDefaultBackground(TCODColor::black);
			//inventoryScreen->clear();
			Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::INVENTORY);
				switch (menuItem) {
					case Menu::ITEMS :
						actor = choseFromInventory(owner,1);
						if(actor)
							choice = false;
						break;
					case Menu::TECH :
						actor = choseFromInventory(owner,2);
						if(actor)
							choice = false;
						break;
					case Menu::ARMOR:
						actor = choseFromInventory(owner,3);
						if(actor)
							choice = false;
						break;
					case Menu::WEAPONS:
						actor = choseFromInventory(owner,4);
						if(actor)
							choice = false;
						break;
					case Menu::NO_CHOICE:
						break;
					case Menu::EXIT:
						choice = false;
					default: break;
				}
			}
			engine.invState = 0;
			engine.invFrames = 0;
			if (actor) {
				actor->pickable->drop(actor,owner);
				engine.gameStatus = Engine::NEW_TURN;
			}
		}break;
		case 'l':
		{
			engine.map->computeFov();
			engine.gui->renderKeyLook();
		}break;
		case '>':
		if (engine.stairs->x == owner->x && engine.stairs->y == owner->y) {
			engine.player->attacker->lastTarget = NULL;
			engine.nextLevel();
		} else {
			engine.gui->message(TCODColor::lightGrey, "There are no stairs here. Perhaps you are disoriented?");
		} break;
		case 'v':
		int w, h;
		if (!TCODConsole::isFullscreen()){
			TCODSystem::getCurrentResolution(&w,&h);
			TCODConsole::initRoot(w/16,h/16,"Astro",true);
		} else {
			engine.gui->message(TCODColor::darkerPink,"minimizing");
			TCODConsole::initRoot(engine.screenWidth,engine.screenHeight, "Astro", false);
		}
		break;
		case 'f':
			//shooty shooty bang bang -Mitchell
			//need to figure out how to check if the user has a gun
			if(owner->container->ranged){
				//engine.gui->message(TCODColor::darkerOrange,"You fire your MLR");
				Actor *closestMonster = engine.getClosestMonster(owner->x, owner->y,3);
				if (!closestMonster) {
					engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to shoot.");
					return;
				}
				//hit the closest monster for <damage> hit points;
				else{
					if(owner->attacker && owner->attacker->battery >= 1){
						owner->attacker->shoot(owner, closestMonster);
						owner->attacker->usePower(owner, 1);
						engine.damageDone += (int)owner->totalDex - closestMonster->destructible->totalDefense;
						engine.gameStatus = Engine::NEW_TURN;
					}
					else{
						engine.gui->message(TCODColor::lightGrey, "Not enough battery to shoot.");
					}
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
				if (!engine.pickATile(&x, &y, 3)) {
					//engine.gui->message(TCODColor::lightGrey, "You can't shoot that far.");
					return;
				}
				Actor *actor = engine.getActor(x,y);
				if (!actor) {
					engine.gui->message(TCODColor::lightGrey, "No enemy at that location.");
					return;
				}
				/*if (!closestMonster) {
					engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to shoot.");
					//return false;
				}*/
				//hit the closest monster for <damage> hit points;
				else{
					if(owner->attacker && owner->attacker->battery >= 1){
						owner->attacker->shoot(owner, actor);
						owner->attacker->usePower(owner, 1);
						engine.damageDone += (int)owner->totalDex - actor->destructible->totalDefense;
						engine.gameStatus = Engine::NEW_TURN;
					}
					else{
						engine.gui->message(TCODColor::lightGrey, "Not enough battery to shoot.");
					}
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
	}
}

Actor *PlayerAi::choseFromInventory(Actor *owner,int type) {
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
			if(actor->pickable->type == Pickable::EQUIPMENT && ((Equipment*)(actor->pickable))->equipped){
				inventoryScreen->print(2,y,"(%c) %s(E)",shortcut,actor->name);
			}else{
				inventoryScreen->print(2,y,"(%c) %s",shortcut,actor->name);
			}
			owner->container->select[shortcut] = actor->name;
			if (actor->pickable->stacks) {
				inventoryScreen->print(17, y, "(%d)",actor->pickable->stackSize);
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
	con.print(14,20,"ARMOR");
	con.print(1,22,"HEAD: ");
	con.print(1,24,"CHEST: ");
	con.print(1,26,"LEGS: ");
	con.print(1,28,"FEET: ");
	con.print(1,30,"HAND1: ");
	con.print(1,32,"HAND2: ");
	con.print(1,34,"RANGED: ");
	for(Actor **it = owner->container->inventory.begin(); it != owner->container->inventory.end(); ++it){
		Actor *actor = *it;
		if(actor->pickable->type == Pickable::EQUIPMENT && ((Equipment*)(actor->pickable))->equipped){
			switch(((Equipment*)(actor->pickable))->slot){
				case Equipment::HEAD:
					con.print(6,22,"%s",actor->name);
				break;
				case Equipment::CHEST:
					con.print(7,24,"%s",actor->name);
				break;
				case Equipment::LEGS:
					con.print(6,26,"%s",actor->name);
				break;
				case Equipment::FEET:
					con.print(6,28,"%s",actor->name);
				break;
				case Equipment::HAND1:
					con.print(7,30,"%s",actor->name);
				break;
				case Equipment::HAND2:
					con.print(7,32,"%s",actor->name);
				break;
				case Equipment::RANGED:
					con.print(8,34,"%s",actor->name);
				break;
				case Equipment::NOSLOT:
				break;
				default: break;
			}
		}
	}
	//Diplay Character Stats
	con.print(2,4,"STATS");
	con.print(1,6,"VIT: %d",owner->vit);
	con.print(1,8,"DEX: %d",owner->dex);
	con.print(1,10,"STR: %d",owner->str);
	con.print(1,12,"INT: %d",owner->intel);
	con.print(1,14,"KILLS: %d",engine.killCount);
	con.print(1,16,"DMG DONE: %g",engine.damageDone);
	con.print(1,18,"DMG TAKEN: %g",engine.damageReceived);
	
	//Display Character Image
	con.print(20,8,"@");
	
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
	TCODConsole::flush();
	
	//Keep info displayed until the player presses any character
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
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
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	if (engine.map->isInFov(owner->x,owner->y)) {
		//can see the palyer, move towards him
		moveCount = TRACKING_TURNS;
	} else {
		moveCount--;
	}
	if (moveCount > 0) {
		moveOrAttack(owner, engine.player->x, engine.player->y);
	} else {
		moveCount = 0;
	}
}

void MonsterAi::moveOrAttack(Actor *owner, int targetx, int targety){
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
		owner->attacker->attack(owner,engine.player);
		engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDefense);
	}
	
}
Actor *MonsterAi::choseFromInventory(Actor *owner,int type) {
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
			if(actor->pickable->type == Pickable::EQUIPMENT && ((Equipment*)(actor->pickable))->equipped){
				inventoryScreen->print(2,y,"(%c) %s(E)",shortcut,actor->name);
			}else{
				inventoryScreen->print(2,y,"(%c) %s",shortcut,actor->name);
			}
			owner->container->select[shortcut] = actor->name;
			if (actor->pickable->stacks) {
				inventoryScreen->print(17, y, "(%d)",actor->pickable->stackSize);
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
		engine.map->tiles[i].infection += 1 / (rng->getDouble(.01,1.0)*owner->getDistance(i%width, i/width));
	}
	engine.gui->message(TCODColor::green,"You feel uneasy as the infection seems to spread.");
}

void EpicenterAi::save(TCODZip &zip) {
	zip.putInt(EPICENTER);
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
	//oldMap = new TCODMap(13,13);
	onOff = true;
	frst = true;
	//frstTurn = true;
	moving = movibility;
}

void LightAi::load(TCODZip &zip){}

void LightAi::save(TCODZip &zip){
	zip.putInt(LIGHT);
}

void LightAi::flicker(Actor * owner, float chance){
	int maxx = owner->x+6;
	int minx = owner->x-6;
	int maxy = owner->y+6;
	int miny = owner->y-6;
	//owner->radius
	//lmap->computeFov(owner->x-minx,owner->y-miny,radius);
	
	for (int x=minx; x <= maxx; x++) {
		for (int y=miny; y <= maxy; y++) {
			if (lmap->isInFov(x-minx,y-miny)) {
				if (flkr < chance)
				{
					// && (engine.player->x != x && engine.player->y != y)) {
					engine.map->tiles[x+y*engine.map->width].lit = false;
				}
				else
				{
					// && (engine.player->x != x && engine.player->y != y)) {
					engine.map->tiles[x+y*engine.map->width].lit = true;
				}
			}
			else
			{
				//if (engine.map->tiles[x+y*engine.map->width].lit = true)
				//engine.map->tiles[x+y*engine.map->width].lit = false;
			}
		}
	}
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
			//engine.gui->message(TCODColor::yellow, "changed light!");
			for (int x=lstX-6; x <= lstX+6; x++) {
				for (int y=lstY-6; y <= lstY+6; y++) {
					
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
		
		for (int x=minx; x <= maxx; x++) {
			for (int y=miny; y <= maxy; y++) {
				//inheriting properties of real map
				
				lmap->setProperties(x-minx,y-miny,engine.map->canWalk(maxx-(maxx-x),maxy-(maxy-y)),engine.map->isWall(maxx-(maxx-x),maxy-(maxy-y)));//engine.map->canWalk(x-owner->x,y-owner->y),engine.map->isWall(x-owner->x,y-owner->y));
			}
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
				if (engine.map->tiles[x+y*engine.map->width].num == 1)
				{
					engine.map->tiles[x+y*engine.map->width].lit = false;
					if (!frst)
					{
						engine.map->tiles[x+y*engine.map->width].num--;
						
					}
					//engine.map->tiles[x+y*engine.map->width].num--;
				}
			}
		}
	if (!frst)
			frst = true;//when set to off turn it back to true
	/*for (int i = x1; i <= x2; i++)
	{
		for (int j = y1; j <= y2; j++)
		{
			if (true)//engine.distance(x,i,y,j) <= 3)
			{
				engine.map->tiles[i+j*engine.map->width].infection = true;
				//engine.mapcon->setChar(i, j, 'L');
			}
			else
			{
				engine.map->tiles[i+j*engine.map->width].infection = false;
			}
		}
	}
	*/
	}
	
}

FlareAi::FlareAi(int lightRange, int turns) : lightRange(lightRange),turns(turns)
{
	//light = new Actor(x, y, ' ', "Flare Light", TCODColor::white);
	//light->ai = new LightAi(lightRange,1);
	//engine.actors.push(light);
	i = 0;
}

void FlareAi::load(TCODZip &zip){}

void FlareAi::save(TCODZip &zip){
	zip.putInt(FLARE);
}

void FlareAi::update(Actor * owner)
{
	
	if (i == 0)
	{
		light = new Actor(owner->x,owner->y, 171, "Flare Light", TCODColor::white);
		light->ai = new LightAi(lightRange,1);
		light->blocks = false;
		engine.actors.push(light);
		i++;
		engine.gui->message(TCODColor::orange, "Flare is burning %d/%d of it's phosphorus remains.",turns-i,turns);
		
	}
	if (i < turns)
	{
		i++;
		engine.gui->message(TCODColor::orange, "Flare is burning %d/%d of it's phosphorus remains.",turns-i,turns);
	}
	else
	{
		//engine.gui->message(TCODColor::orange, "Flare has burnt out");
		//light->(LightAi)ai->onOff = false;
		//LightAi l = (LightAi)light->ai;
		light->ch = ' ';
		LightAi *l = (LightAi*)light->ai;
		l->onOff = false;
		l->update(owner);
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
				if (actor) {
					owner->attacker->attack(owner,actor);
					engine.map->computeFov();
				}
			}
		}
	}
	nbTurns--;
	
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
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	if (engine.map->isInFov(owner->x,owner->y)) {
		//can see the palyer, move towards him
		moveCount = TRACKING_TURNS + 2; //give ranged characters longer tracking
	} else {
		moveCount--;
	}
	if (moveCount > 0) {
		moveOrAttack(owner, engine.player->x, engine.player->y);
	} else {
		moveCount = 0;
	}
}
void RangedAi::moveOrAttack(Actor *owner, int targetx, int targety)
{
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	float distance = sqrtf(dx*dx+dy*dy);
	//If the distance > range, then the rangedAi will move towards the player
	//If the distance <= range, then the rangedAi will shoot the player unless the player is right next the rangedAi

	if (distance > range) {

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
	} else if (distance !=1 && owner->attacker) {
		owner->attacker->shoot(owner,engine.player);
		engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDefense);
	}
	else if (owner->attacker) {
		owner->attacker->attack(owner,engine.player);
		engine.damageReceived += (owner->attacker->totalPower - engine.player->destructible->totalDefense);
	}
	
}

