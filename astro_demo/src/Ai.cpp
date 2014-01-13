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
	}
	ai->load(zip);
	return ai;
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
		engine.gui->menu.addItem(Menu::CONSTITUTION, "Constitution (+20hp)");
		engine.gui->menu.addItem(Menu::STRENGTH, "Strength (+1 attack)");
		engine.gui->menu.addItem(Menu::AGILITY, "Agility (+1 defence)");
		Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::PAUSE);
	
		switch (menuItem) {
			case Menu::CONSTITUTION	:
				owner->destructible->maxHp += 20;
				owner->destructible->hp += 20;
				choice_made = true;
				break;
			case Menu::STRENGTH :
				owner->attacker->power += 1;
				choice_made = true;
				break;
			case Menu::AGILITY:
				owner->destructible->defense += 1;
				choice_made = true;
				break;
			case Menu::NO_CHOICE:
				first = false;
				break;
			default: break;
		}
		}
	}
	if (owner->destructible && owner->destructible->isDead()) {
		return;
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
		engine.player->y = engine.stairs->y;
	case TCODK_KP5: engine.gameStatus = Engine::NEW_TURN; break;
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
		if (actor->destructible && !actor->destructible->isDead()
			&& actor->x == targetx &&actor->y == targety) {
			owner->attacker->attack(owner, actor);
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
	owner->y = targety;
	return true;
}

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
	switch(ascii) {
		case 'g': //pickup the item
		{
			bool found = false;
			for (Actor **iterator = engine.actors.begin();
				iterator != engine.actors.end(); iterator++) {
				Actor *actor = *iterator;
				if (actor->pickable && actor->x == owner->x && actor->y == owner->y) {
					if (actor->pickable->pick(actor,owner)) {
						found = true;
						engine.gui->message(TCODColor::lightGrey, "You pick up the %s.", actor->name);
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
			Actor *actor = choseFromInventory(owner);
			if (actor) {
				
				actor->pickable->use(actor,owner);
				engine.gameStatus = Engine::NEW_TURN;
			}
		}break;
		case 'd': //drop an item
		{
			Actor *actor = choseFromInventory(owner);
			if (actor) {
				actor->pickable->drop(actor,owner);
				engine.gameStatus = Engine::NEW_TURN;
			}
		}break;
		case 'l':
		{
			engine.gui->renderKeyLook();
		}break;
		case '>':
		if (engine.stairs->x == owner->x && engine.stairs->y == owner->y) {
			engine.nextLevel();
		} else {
			engine.gui->message(TCODColor::lightGrey, "There are no stairs here. Perhaps you are disoriented?");
		} break;
		case 'v':
		int w, h;
		if (!TCODConsole::isFullscreen()){
			TCODSystem::getCurrentResolution(&w,&h);
			TCODConsole::initRoot(w/8,h/8,"Astro",true);
		} else {
			engine.gui->message(TCODColor::darkerPink,"minimizing");
			TCODConsole::initRoot(engine.screenWidth,engine.screenHeight, "Astro", false);
		}
		break;
	}
}

Actor *PlayerAi::choseFromInventory(Actor *owner) {
	static const int INVENTORY_WIDTH = 50;
	static const int INVENTORY_HEIGHT = 28;
	static TCODConsole con(INVENTORY_WIDTH, INVENTORY_HEIGHT);
	
	//display the inventory frame
	con.setDefaultForeground(TCODColor(200,180,50));
	con.printFrame(0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,true,TCOD_BKGND_DEFAULT, "Inventory");
	
	//display the items with their keyboard shortcut
	con.setDefaultForeground(TCODColor::white);
	int shortcut = 'a';
	int y = 1;
	for (Actor **it = owner->container->inventory.begin();
		it != owner->container->inventory.end(); it++) {
		Actor *actor = *it;
		con.print(2,y,"(%c) %s",shortcut,actor->name);
		y++;
		shortcut++;
	}
	//blit the inventory console on the root console
	TCODConsole::blit(&con,0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,
		TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
		engine.screenHeight/2 - INVENTORY_HEIGHT/2);
	TCODConsole::flush();
	
	//wait for a key press
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
	if (key.vk == TCODK_CHAR) {
		int actorIndex = key.c - 'a';
		if (actorIndex >= 0 && actorIndex < owner->container->inventory.size()) {
			return owner->container->inventory.get(actorIndex);
		}
	}
	return NULL;
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
	} else if (owner->attacker) {
		owner->attacker->attack(owner,engine.player);
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
	TCODRandom *rng = TCODRandom::getInstance();
	int dx = rng->getInt(-1,1);
	int dy = rng->getInt(-1,1);
	
	if (dx != 0 || dy!=0) {
		int destx = owner->x + dx;
		int desty = owner->y + dy;
		if (engine.map->canWalk(destx,desty)) {
			owner->x = destx;
			owner->y = desty;
		} else {
			Actor *actor = engine.getActor(destx, desty);
			if (actor) {
				owner->attacker->attack(owner,actor);
			}
		}
	}
	nbTurns--;
	if(nbTurns == 0) {
		owner->ai = oldAi;
		delete this;
	}
}