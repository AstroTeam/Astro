#include "main.hpp"

Pickable::Pickable(bool stacks, int stackSize, PickableType type) : 
	stacks(stacks),stackSize(stackSize), type(type) {
}

Pickable *Pickable::create(TCODZip &zip) {
	PickableType type = (PickableType)zip.getInt();
	Pickable *pickable = NULL;
	switch(type) {
		case CURRENCY: pickable = new Coinage(0); break;
		case HEALER: pickable = new Healer(0); break;
		case CHARGER: pickable = new Charger(0); break;
		case LIGHTNING_BOLT: pickable = new LightningBolt(0,0); break;
		case CONFUSER: pickable = new Confuser(0,0); break;
		case FIREBALL: pickable = new Fireball(0,0,0); break;
		case EQUIPMENT: pickable = new Equipment(0); break;
		case FLARE: pickable = new Flare(0,0,0); break;
		case FRAGMENT: pickable = new Fragment(0,0,0); break;
		case WEAPON: pickable = new Weapon(0,0,0); break;
		case NONE: break;
	}
	pickable->load(zip);
	return pickable;
}

bool Pickable::pick(Actor *owner, Actor *wearer) {
	if (wearer->container && wearer->container->add(owner)) {
		engine.actors.remove(owner);
		return true;
	}
	return false;
}

bool Pickable::use(Actor *owner, Actor *wearer) {
	if (wearer->container && owner->pickable->stackSize < 2) {
		wearer->container->remove(owner);
		delete owner;
		return true;
	} else {
		owner->pickable->stackSize -= 1;
		return true;
	}
	return false;
}

Coinage::Coinage(bool stacks, int stackSize, PickableType type)
	: Pickable(stacks, stackSize, type) {
}

void Coinage::load(TCODZip &zip) {
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Coinage::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

Healer::Healer(float amount, bool stacks, int stackSize, PickableType type)
	: Pickable(stacks,stackSize,type),amount(amount) {
}

void Healer::load(TCODZip &zip) {
	amount = zip.getFloat();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Healer::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putFloat(amount);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Healer::use(Actor *owner, Actor *wearer) {
	if (wearer->destructible) {
		float amountHealed;
		float factor = 1;
		if(wearer->race[0] == 'R')
			factor *= .5;
		if(wearer->job[0] == 'A')
			factor *= .6;
		amountHealed = wearer->destructible->heal((int)(factor * (wearer->totalIntel * 3 + 6))/1);
		if (amountHealed > 0) {
			return Pickable::use(owner,wearer);
		}
	}
	return false;
}

Charger::Charger(float amount, bool stacks, int stackSize, PickableType type)
	: Pickable(stacks,stackSize,type),amount(amount) {
}

void Charger::load(TCODZip &zip) {
	amount = zip.getFloat();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Charger::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putFloat(amount);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Charger::use(Actor *owner, Actor *wearer) {
	if (wearer->attacker) {
		float amountCharged = wearer->attacker->recharge(amount);
		if (amountCharged > 0) {
			return Pickable::use(owner,wearer);
		}
	}
	return false;
}

LightningBolt::LightningBolt(float range, float damage, bool stacks, int stackSize, PickableType type)
	: Pickable(stacks, stackSize,type),range(range),damage(damage) {
}

void LightningBolt::load(TCODZip &zip) {
	range = zip.getFloat();
	damage = zip.getFloat();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void LightningBolt::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putFloat(range);
	zip.putFloat(damage);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool LightningBolt::use(Actor *owner, Actor *wearer) {
	Actor *closestMonster = engine.getClosestMonster(wearer->x, wearer->y,range);
	if (!closestMonster) {
		engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to strike.");
		return false;
	}
	//hit the closest monster for <damage> hit points;
	float damageTaken = closestMonster->destructible->takeDamage(closestMonster, -3 + 3 * wearer->totalIntel);
	engine.damageDone += 3 * wearer->totalIntel - 3;
	if (!closestMonster->destructible->isDead()) {
		engine.gui->message(TCODColor::orange,"Taking %g damage, the %s crackles with electricity, crying out in rage.",damageTaken,closestMonster->name);
	} else {
		engine.gui->message(TCODColor::orange,"Taking %g damage, the %s crackles with electricity, twitching slightly.",damageTaken,closestMonster->name);
	}
	return Pickable::use(owner,wearer);
}

Fireball::Fireball(float range, float damage,float maxRange, bool stacks, int stackSize, PickableType type)
	: LightningBolt(range, damage, stacks, stackSize, type),maxRange(maxRange) {
}

void Fireball::load(TCODZip &zip) {
	range = zip.getFloat();
	damage = zip.getFloat();
	maxRange = zip.getFloat();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Fireball::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putFloat(range);
	zip.putFloat(damage);
	zip.putFloat(maxRange);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Fireball::use(Actor *owner, Actor *wearer) {
	engine.gui->message(TCODColor::cyan, "Please choose a tile for the fireball, "
		"or hit escape to cancel.");
	int x = engine.player->x;
	int y = engine.player->y;
	if (!engine.pickATile(&x,&y,maxRange, (wearer->totalIntel - 1) /3 +1)) {
		return false;
	}
	//burn everything in range, including the player
	engine.gui->message(TCODColor::orange, "The fireball explodes, burning everything within %d tiles!",1 + (wearer->totalIntel - 1) /3);
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		if (actor->destructible && !actor->destructible->isDead()
			&&actor->getDistance(x,y) <= 1 + (wearer->totalIntel - 1) /3) {
			//the initial damage is a little high, i think it should actually be zero, since it immediatlly affects the monsters
			float damageTaken = 1;
			actor->destructible->takeDamage(actor, 1);
			//engine.damageDone +=  2 * wearer->totalIntel;
			if (!actor->destructible->isDead()) {
				engine.gui->message(TCODColor::orange,"The %s gets burned for %g hit points.",actor->name,damageTaken);
			} else {
				engine.gui->message(TCODColor::orange,"The %s is an ashen mound from the %g damage, crumbling under its own weight.",actor->name, damageTaken);
			}
			//engine.map->tiles[x+y*engine.map->width].envSta = 1;	
		}
	}
	
	for (int xxx = x - ((1 + (wearer->totalIntel - 1) /3)) ; xxx <= x+((1 + (wearer->totalIntel - 1) /3));xxx++)
	{
		for (int yyy = y - ((1 + (wearer->totalIntel - 1) /3)); yyy <= y+((1 + (wearer->totalIntel - 1) /3));yyy++)
		{
			if (engine.distance(x,xxx,y,yyy) <= (1 + (wearer->totalIntel - 1) /3))
			{
				engine.map->tiles[xxx+yyy*engine.map->width].envSta = 1;
				engine.map->tiles[xxx+yyy*engine.map->width].temperature = 6;//should be a function of int 
			}
			
		}
	}
	/*for (int xxx = x - 1 ; xxx <= x+1;xxx++)
	{
		for (int yyy = y-1; yyy <= y+1;yyy++)
		{
			engine.map->tiles[xxx+yyy*engine.map->width].envSta = 1;	
		}
	}*/
	
	return Pickable::use(owner,wearer);
}

Fragment::Fragment(float range, float damage,float maxRange, bool stacks, int stackSize, PickableType type)
	: LightningBolt(range, damage, stacks, stackSize, type),maxRange(maxRange) {
}

void Fragment::load(TCODZip &zip) {
	range = zip.getFloat();
	damage = zip.getFloat();
	maxRange = zip.getFloat();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Fragment::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putFloat(range);
	zip.putFloat(damage);
	zip.putFloat(maxRange);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Fragment::use(Actor *owner, Actor *wearer) {
	engine.gui->message(TCODColor::cyan, "Please choose a tile for the grenade, "
		"or hit escape to cancel.");
	int x = engine.player->x;
	int y = engine.player->y;
	if (!engine.pickATile(&x,&y,maxRange, (wearer->totalIntel - 1) /3 +1)) {
		return false;
	}
	//burn everything in range, including the player
	engine.gui->message(TCODColor::orange, "The fragmentation grenade explodes, eviscerating everything within %d tiles!",1 + (wearer->totalIntel - 1) /3);
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		if (actor->destructible && !actor->destructible->isDead()
			&&actor->getDistance(x,y) <= 1 + (wearer->totalIntel - 1) /3) {
			float damageTaken = 2 * wearer->totalIntel;
			actor->destructible->takeDamage(actor, damageTaken);
			//engine.damageDone +=  2 * wearer->totalIntel;
			if (!actor->destructible->isDead()) {
				engine.gui->message(TCODColor::orange,"The %s gets wounded from the blast for %g hit points.",actor->name,damageTaken);
			} else {
				engine.gui->message(TCODColor::orange,"The %s's guts explode outward after taking %g damage.",actor->name, damageTaken);
			}
			//engine.map->tiles[x+y*engine.map->width].envSta = 2;	
		}
	}
	
	for (int xxx = x - ((1 + (wearer->totalIntel - 1) /3)) ; xxx <= x+((1 + (wearer->totalIntel - 1) /3));xxx++)
	{
		for (int yyy = y - ((1 + (wearer->totalIntel - 1) /3)); yyy <= y+((1 + (wearer->totalIntel - 1) /3));yyy++)
		{
			if (engine.distance(x,xxx,y,yyy) <= (1 + (wearer->totalIntel - 1) /3))
			{
				engine.map->tiles[xxx+yyy*engine.map->width].envSta = 2;
				//engine.map->tiles[xxx+yyy*engine.map->width].temperature = 6;//should be a function of int 
			}
			
		}
	}
	
	return Pickable::use(owner,wearer);
}

Confuser::Confuser(int nbTurns, float range, bool stacks, int stackSize, PickableType type) 
	: Pickable(stacks, stackSize, type),nbTurns(nbTurns), range(range) {
}

void Confuser::load(TCODZip &zip) {
	nbTurns = zip.getInt();
	range = zip.getFloat();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Confuser::save(TCODZip &zip) {
	zip.putInt(CONFUSER);
	zip.putInt(nbTurns);
	zip.putFloat(range);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Confuser::use(Actor *owner, Actor *wearer) {
	engine.gui->message(TCODColor::cyan, "Choose a target to confuse");
	int x = engine.player->x;
	int y = engine.player->y;
	if (!engine.pickATile(&x, &y, range)) {
		return false;
	}
	Actor *actor = engine.getActor(x,y);
	if (!actor) {
		return false;
	}
	
	/* 
	***UNCOMMENT THIS IF YOU WANT THE PLAYER TO NOT BE ABLE TO CONFUSE HIMSELF***
	if (actor == engine.player) {
		engine.gui->message(TCODColor::lightRed, "YOu begin to read the scroll, but grow too confused to continue!");
		return false;
	} */
	
	//confuse the target for nbTurns turns
	if(actor->flashable)
	{
		Ai *confusedAi = new ConfusedActorAi(wearer->totalIntel + 5, actor->ai);
		actor->ai = confusedAi;
		engine.gui->message(TCODColor::lightGreen, "The flash of light confuses the %s, and they start to stumble around!",
		actor->name);
	}else
	{
		engine.gui->message(TCODColor::lightGreen, "The flash bang is not effective against a %s.",
		actor->name);
	}
	return Pickable::use(owner,wearer);
}

Flare::Flare(int nbTurns, float range, int lightRange, bool stacks, int stackSize, PickableType type) 
	: Pickable(stacks, stackSize, type),nbTurns(nbTurns), range(range), lightRange(lightRange) {
}

void Flare::load(TCODZip &zip) {
	nbTurns = zip.getInt();
	range = zip.getFloat();
	lightRange = zip.getInt();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Flare::save(TCODZip &zip) {
	zip.putInt(FLARE);
	zip.putInt(nbTurns);
	zip.putFloat(range);
	zip.putInt(lightRange);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Flare::use(Actor *owner, Actor *wearer) {
	engine.gui->message(TCODColor::orange, "Choose an area to illuminate");
	int x = engine.player->x;
	int y = engine.player->y;
	if (!engine.pickATile(&x, &y, range)) {
		return false;
	}
	//make new actor as a flare item
	Actor *scrollOfFlaring = new Actor(x,y,' ',"Flare", TCODColor::white);
	scrollOfFlaring->ai = new FlareAi(lightRange,nbTurns);
	//scrollOfFlaring->sort = 2;
	scrollOfFlaring->blocks = false;
	engine.actors.push(scrollOfFlaring);
	//scrollOfFlaring->pickable = new Confuser(10,8);
	//return scrollOfFlaring;
	
	
	engine.gui->message(TCODColor::lightOrange, "You fire off the Flare and see the area around it illuminated.");
	return Pickable::use(owner,wearer);
}


void Pickable::drop(Actor *owner, Actor *wearer, bool isNPC) {
	if (wearer->container) {
		if ((owner->pickable->type == EQUIPMENT || owner->pickable->type == WEAPON ) && ((Equipment*)(owner->pickable))->equipped) {
			((Equipment*)(owner->pickable))->use(owner,wearer);
		}
		int numberDropped = 1;
		if(isNPC && wearer->container){
			wearer->container->remove(owner);
			owner->x = wearer->x;
			if(wearer->ch == 243){
				owner->y = wearer->y + 1;
			}else{
				owner->y = wearer->y;
			}
			engine.actors.push(owner);
			engine.sendToBack(owner);
		}else if (numberDropped >= owner->pickable->stackSize && wearer->container) {
			wearer->container->remove(owner);
			owner->x = wearer->x;
			owner->y = wearer->y;
			engine.actors.push(owner);
			engine.sendToBack(owner);
		}
		else {
			Actor *droppy = new Actor(wearer->x, wearer->y, owner->ch,owner->name,owner->col);
			droppy->blocks = false;
			PickableType type = owner->pickable->type;
			owner->pickable->stackSize -= numberDropped;
			switch(type) {
				case CURRENCY: break;
				case HEALER: droppy->pickable = new Healer(((Healer*)owner->pickable)->amount); droppy->sort = 1; break;
				case CHARGER: droppy->pickable = new Charger(((Charger*)owner->pickable)->amount); droppy->sort = 1; break;
				case LIGHTNING_BOLT: droppy->pickable = new LightningBolt(((LightningBolt*)(owner->pickable))->range,((LightningBolt*)(owner->pickable))->damage); droppy->sort = 2; break;
				case CONFUSER: droppy->pickable = new Confuser(((Confuser*)(owner->pickable))->nbTurns,((Confuser*)(owner->pickable))->range); droppy->sort = 2; break;
				case FIREBALL: droppy->pickable = new Fireball(((Fireball*)(owner->pickable))->range,((Fireball*)(owner->pickable))->damage,((Fireball*)(owner->pickable))->maxRange); droppy->sort = 2; break;
				case FLARE: droppy->pickable = new Flare(((Flare*)(owner->pickable))->nbTurns, ((Flare*)(owner->pickable))->range, ((Flare*)(owner->pickable))->lightRange); droppy->sort = 2; break;
				case EQUIPMENT: break;
				case WEAPON: break;
				case FRAGMENT: droppy->pickable = new Fragment(((Fragment*)(owner->pickable))->range,((Fragment*)(owner->pickable))->damage,((Fragment*)(owner->pickable))->maxRange); droppy->sort = 2; break;
				case NONE: break;
			}
			droppy->pickable->stackSize = numberDropped;
			engine.actors.push(droppy);
			engine.sendToBack(droppy);
		}
		if (wearer == engine.player){
			engine.gui->message(TCODColor::lightGrey,"You drop %d %s",numberDropped,owner->name);
		}else {
			engine.gui->message(TCODColor::lightGrey,"%s drops %d %s",wearer->name,owner->pickable->stackSize,owner->name);
		}
	}
}

ItemBonus::ItemBonus(BonusType type, float bonus) : 
	type(type), bonus(bonus) {
}

void ItemBonus::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putFloat(bonus);
}

void ItemBonus::load(TCODZip &zip) {
	type = (BonusType)zip.getInt();
	bonus = zip.getFloat();	
}

ItemReq::ItemReq(ReqType type, float requirement) : type(type), requirement(requirement){
}

void ItemReq::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putFloat(requirement);
}

void ItemReq::load(TCODZip &zip) {
	type = (ReqType)zip.getInt();
	requirement = zip.getFloat();	
}

Equipment::Equipment(bool equipped, SlotType slot, ItemBonus *bonus, ItemReq *requirement, bool stacks, int stackSize, PickableType type)
	: Pickable(stacks, stackSize,type), equipped(equipped), slot(slot), 
	bonus(bonus), requirement(requirement) {
}

void Equipment::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(equipped);
	zip.putInt(slot);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
	bonus->save(zip);
	requirement->save(zip);
}

void Equipment::load(TCODZip &zip) {
	equipped = zip.getInt();
	slot = (SlotType)zip.getInt();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
	ItemBonus *bon = new ItemBonus(ItemBonus::NOBONUS,0);
	bon->load(zip);
	bonus = bon;
	ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	req->load(zip);
	requirement = req;
}

bool Equipment::use(Actor *owner, Actor *wearer) {
	if (!equipped) {
		switch(slot) {
			case HEAD: 
				if (wearer->container->head) {
					engine.gui->message(TCODColor::orange,"You already have a head item equipped!");
					return false;
				} else {
					if(requirementsMet(owner,wearer)){
						wearer->container->head = true;
					}else{
						switch(requirement->type){
							case ItemReq::STRENGTH :
								engine.gui->message(TCODColor::orange,"You need %g strength to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::DEXTERITY :
								engine.gui->message(TCODColor::orange,"You need %g dexterity to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::INTELLIGENCE :
								engine.gui->message(TCODColor::orange,"You need %g intelligence to equip this item!",requirement->requirement);
								return false;
							break;
							default: break;
						}
						
					}
					
				} break;
			case CHEST:
				if (wearer->container->chest) {
					engine.gui->message(TCODColor::orange,"You already have a chest item equipped!");
					return false;
				} else {
					if(requirementsMet(owner,wearer)){
						wearer->container->chest = true;
					}else{
						switch(requirement->type){
							case ItemReq::STRENGTH :
								engine.gui->message(TCODColor::orange,"You need %g strength to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::DEXTERITY :
								engine.gui->message(TCODColor::orange,"You need %g dexterity to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::INTELLIGENCE :
								engine.gui->message(TCODColor::orange,"You need %g intelligence to equip this item!",requirement->requirement);
								return false;
							break;
							default: break;
						}
					}
				} break;
			case LEGS:
				if (wearer->container->legs) {
					engine.gui->message(TCODColor::orange,"You already have a leg item equipped!");
					return false;
				} else {
					if(requirementsMet(owner,wearer)){
						wearer->container->legs = true;
					}else{
						switch(requirement->type){
							case ItemReq::STRENGTH :
								engine.gui->message(TCODColor::orange,"You need %g strength to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::DEXTERITY :
								engine.gui->message(TCODColor::orange,"You need %g dexterity to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::INTELLIGENCE :
								engine.gui->message(TCODColor::orange,"You need %g intelligence to equip this item!",requirement->requirement);
								return false;
							break;
							default: break;
						}
					}
				} break;
			case FEET:
				if (wearer->container->feet) {
					engine.gui->message(TCODColor::orange,"You already have a foot item equipped!");
					return false;
				} else {
					if(requirementsMet(owner,wearer)){
						wearer->container->feet = true;
					}else{
						switch(requirement->type){
							case ItemReq::STRENGTH :
								engine.gui->message(TCODColor::orange,"You need %g strength to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::DEXTERITY :
								engine.gui->message(TCODColor::orange,"You need %g dexterity to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::INTELLIGENCE :
								engine.gui->message(TCODColor::orange,"You need %g intelligence to equip this item!",requirement->requirement);
								return false;
							break;
							default: break;
						}
					}
				} break;
			case HAND1:
				if (wearer->container->hand1) {
					engine.gui->message(TCODColor::orange,"You already have a melee weapon equipped!");
					return false;
				} else {
					if(requirementsMet(owner,wearer)){
						wearer->container->hand1 = true;
					}else{
						switch(requirement->type){
							case ItemReq::STRENGTH :
								engine.gui->message(TCODColor::orange,"You need %g strength to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::DEXTERITY :
								engine.gui->message(TCODColor::orange,"You need %g dexterity to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::INTELLIGENCE :
								engine.gui->message(TCODColor::orange,"You need %g intelligence to equip this item!",requirement->requirement);
								return false;
							break;
							default: break;
						}
					}
				} break;
			case HAND2:
				if (wearer->container->hand2) {
					engine.gui->message(TCODColor::orange,"You already have an off-hand item equipped!");
					return false;
				} else {
					if(requirementsMet(owner,wearer)){
						wearer->container->hand2 = true;
					}else{
						switch(requirement->type){
							case ItemReq::STRENGTH :
								engine.gui->message(TCODColor::orange,"You need %g strength to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::DEXTERITY :
								engine.gui->message(TCODColor::orange,"You need %g dexterity to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::INTELLIGENCE :
								engine.gui->message(TCODColor::orange,"You need %g intelligence to equip this item!",requirement->requirement);
								return false;
							break;
							default: break;
						}
					}
				} break;
			case RANGED:
				if (wearer->container->ranged) {
					engine.gui->message(TCODColor::orange,"You already have a ranged weapon equipped!");
					return false;
				} else {
					if(requirementsMet(owner,wearer)){
						wearer->container->ranged = true;
					}else{
						switch(requirement->type){
							case ItemReq::STRENGTH :
								engine.gui->message(TCODColor::orange,"You need %g strength to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::DEXTERITY :
								engine.gui->message(TCODColor::orange,"You need %g dexterity to equip this item!",requirement->requirement);
								return false;
							break;
							case ItemReq::INTELLIGENCE :
								engine.gui->message(TCODColor::orange,"You need %g intelligence to equip this item!",requirement->requirement);
								return false;
							break;
							default: break;
						}
					}
				} break;
			case NOSLOT: break;
			default: break;
		}
		equipped = true;
		switch(bonus->type) {
			case ItemBonus::NOBONUS: break;
			case ItemBonus::HEALTH: wearer->destructible->maxHp += bonus->bonus; break;
			case ItemBonus::DODGE: wearer->destructible->totalDodge += bonus->bonus; break;
			case ItemBonus::DR: wearer->destructible->totalDR += bonus->bonus; break;
			case ItemBonus::STRENGTH: wearer->totalStr += bonus->bonus; break;
			case ItemBonus::DEXTERITY: wearer->totalDex += bonus->bonus; break;
			case ItemBonus::INTELLIGENCE: wearer->totalIntel += bonus->bonus; break;
			default: break;
		}
		wearer->container->sendToBegin(owner);
		return true;
	} else {
		equipped = false;
		switch(slot) {
			case HEAD: wearer->container->head = false; break;
			case CHEST: wearer->container->chest = false; break;
			case LEGS: wearer->container->legs = false; break;
			case FEET: wearer->container->feet = false; break;
			case HAND1: wearer->container->hand1 = false; break;
			case HAND2: wearer->container->hand2 = false; break;
			case RANGED: wearer->container->ranged = false; break;
			case NOSLOT: break;
			default: break;
		}
		switch(bonus->type) {
			case ItemBonus::NOBONUS: break;
			case ItemBonus::HEALTH: 
				wearer->destructible->maxHp -= bonus->bonus;
				if (wearer->destructible->hp > wearer->destructible->maxHp) {
					wearer->destructible->hp = wearer->destructible->maxHp;
				}
				break;
			case ItemBonus::DODGE: wearer->destructible->totalDodge -= bonus->bonus; break;
			case ItemBonus::DR: wearer->destructible->totalDR -= bonus->bonus; break;
			case ItemBonus::STRENGTH: wearer->totalStr -= bonus->bonus; break;
			case ItemBonus::DEXTERITY: wearer->totalDex -= bonus->bonus; break;
			case ItemBonus::INTELLIGENCE: wearer->totalIntel -= bonus->bonus; break;
			default: break;
		}
		wearer->container->inventory.remove(owner);
		wearer->container->inventory.push(owner);
		return true;
	}
	return false;
}

Weapon::Weapon(float minDmg, float maxDmg, float critMult, WeaponType wType,
		bool equipped, SlotType slot, ItemBonus *bonus, ItemReq *requirement):
	Equipment(equipped, slot, bonus, requirement, false, 1, Pickable::WEAPON), 
		minDmg(minDmg), maxDmg(maxDmg), critMult(critMult), wType(wType){
	//need to make sure no funky combos are done
}

bool Weapon::use(Actor *owner, Actor *wearer){
	return Equipment::use(owner, wearer);
}

void Weapon::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(equipped);
	zip.putInt(slot);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
	bonus->save(zip);
	requirement->save(zip);
	zip.putFloat(minDmg);
	zip.putFloat(maxDmg);
	zip.putFloat(critMult);
	zip.putInt(wType);
}

void Weapon::load(TCODZip &zip) {
	equipped = zip.getInt();
	slot = (SlotType)zip.getInt();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
	ItemBonus *bon = new ItemBonus(ItemBonus::NOBONUS,0);
	bon->load(zip);
	bonus = bon;
	ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	req->load(zip);
	requirement = req;
	minDmg = zip.getFloat();
	maxDmg = zip.getFloat();
	critMult = zip.getFloat();
	wType = (WeaponType)zip.getInt();
}

bool Equipment::requirementsMet(Actor *owner, Actor *wearer){
	//int i = wearer->str;
	//ItemReq::ReqType req = requirement->type;
	switch(requirement->type){
		case ItemReq::NOREQ:
			return true;
		break;
		case ItemReq::STRENGTH:
			if(wearer->str >= requirement->requirement){
				return true;
			}else{
				return false;
			}
		break;
		case ItemReq::DEXTERITY:
			if(wearer->dex >= requirement->requirement){
				return true;
			}else{
				return false;
			}
		break;
		case ItemReq::INTELLIGENCE:
			if(wearer->dex >= requirement->requirement){
				return true;
			}else{
				return false;
			}
		break;
		default: break;
	}
	return false;
}