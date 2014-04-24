#include "main.hpp"
#include <iostream>

Pickable::Pickable(bool stacks, int stackSize, PickableType type) : 
	stacks(stacks),stackSize(stackSize), type(type) {
}

Pickable *Pickable::create(TCODZip &zip) {
	PickableType type = (PickableType)zip.getInt();
	std::cout << "pickabletype " << std::endl;
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
		case FOOD: pickable = new Food(0); break;
		case KEY: pickable = new Key(0); break;
		case ALCOHOL: pickable = new Alcohol(0,0);break;
		case TELEPORTER: pickable = new Teleporter(0);break;
		case FLAMETHROWER: pickable = new Flamethrower(0,0); break;
		case NONE: break;
	}
	std::cout << "chose a module type " << std::endl;
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
		float amountHealed = wearer->getHealValue();
		wearer->destructible->heal(amountHealed);
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
	if (!closestMonster || (closestMonster && !engine.map->isVisible(closestMonster->x, closestMonster->y))) {
		engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to strike.");
		return false;
	}
	//hit the closest monster for <damage> hit points;
	float damageTaken = -3 + 3 * wearer->totalIntel;
	damageTaken = closestMonster->destructible->takeDamage(closestMonster,wearer,damageTaken );
	engine.damageDone += 3 * wearer->totalIntel - 3;
	
	if (!closestMonster->destructible->isDead()) 
	{
		if(engine.map->isVisible(closestMonster->x, closestMonster->y) || engine.map->isVisible(wearer->x, wearer->y))
			engine.gui->message(TCODColor::orange,"Taking %g damage, the %s crackles with electricity, crying out in rage.",damageTaken,closestMonster->name);
	} else 
	{
		if(engine.map->isVisible(closestMonster->x, closestMonster->y) || engine.map->isVisible(wearer->x, wearer->y))
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
				damageTaken = actor->destructible->takeDamage(actor, owner, damageTaken);
				
				if (!actor->destructible->isDead()) 
				{
					if(actor == engine.player)
						engine.damageReceived += damageTaken;
					if(engine.map->isVisible(wearer->x, wearer->y) || engine.map->isVisible(actor->x, actor->y))
						engine.gui->message(TCODColor::red,"The %s gets burned for %g hit points.",actor->name, damageTaken);

				} else 
				{
					if(engine.map->isVisible(wearer->x, wearer->y) || engine.map->isVisible(actor->x, actor->y))
						engine.gui->message(TCODColor::red,"The %s is an ashen mound from the %g damage, crumbling under its own weight.",actor->name, damageTaken);

				}
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
		if (actor->destructible && !actor->destructible->isDead() &&actor->getDistance(x,y) <= 1 + (wearer->totalIntel - 1) /3) 
		{
			float damageTaken = 2 * wearer->totalIntel;
			damageTaken = actor->destructible->takeDamage(actor, owner, damageTaken);	
				
			if (!actor->destructible->isDead()) 
			{	
				if(actor == engine.player)
					engine.damageReceived += damageTaken;
				if(engine.map->isVisible(wearer->x, wearer->y) || engine.map->isVisible(actor->x, actor->y))
					engine.gui->message(TCODColor::red,"The %s gets wounded from the blast for %g hit points.",actor->name,damageTaken);

			} else 
			{
				if(engine.map->isVisible(wearer->x, wearer->y) || engine.map->isVisible(actor->x, actor->y))
					engine.gui->message(TCODColor::red,"The %s's guts explode outward after taking %g damage.",actor->name,damageTaken);

			}
						
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
		if(engine.map->isVisible(wearer->x, wearer->y) || engine.map->isVisible(actor->x, actor->y))
			engine.gui->message(TCODColor::lightGreen, "The flash of light confuses the %s, and they start to stumble around!",actor->name);
	}else
	{
		if(engine.map->isVisible(wearer->x, wearer->y) || engine.map->isVisible(actor->x, actor->y))
			engine.gui->message(TCODColor::lightGreen, "The flash bang is not effective against a %s.",actor->name);
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

//owner is the item
//wearer is the locker (thing with the container)
void Pickable::drop(Actor *owner, Actor *wearer, bool isNPC) {
	if (wearer->container) {
		if ((owner->pickable->type == EQUIPMENT || owner->pickable->type == WEAPON ) && ((Equipment*)(owner->pickable))->equipped) {
			((Equipment*)(owner->pickable))->use(owner,wearer);
		}
		int numberDropped = 1;
		if(isNPC && wearer->container){
			wearer->container->remove(owner);
			owner->x = wearer->x;
			if(wearer->ch == 243)
			{ //locker case, note that locker == if(
				if(engine.map->tiles[wearer->x+wearer->y*engine.map->width].decoration != 56 && engine.map->tiles[wearer->x+wearer->y*engine.map->width].decoration != 57 )
					owner->y = wearer->y + 1;
				else //weapon vault case
				{
					owner->x = wearer->x - 1;
					owner->y = wearer->y;
				}
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
				case FOOD: droppy->pickable = new Food(numberDropped); droppy->sort = 1; break;
				case KEY: droppy->pickable = new Key(((Key*)(owner->pickable))->keyType); droppy->sort = 1; break;
				case ALCOHOL: droppy->pickable = new Alcohol(((Alcohol*)(owner->pickable))->strength,((Alcohol*)(owner->pickable))->quality); droppy->sort = 1; break;
				case TELEPORTER: droppy->pickable = new Teleporter(((Teleporter*)(owner->pickable))->range); droppy->sort = 2; break;
				case FLAMETHROWER: break;
				case NONE: break;
			}
			droppy->pickable->stackSize = numberDropped;
			engine.actors.push(droppy);
			engine.sendToBack(droppy);
		}
		if (wearer == engine.player){
			engine.gui->message(TCODColor::lightGrey,"You drop %d %s",numberDropped,owner->name);
		}else {
			if(engine.map->isVisible(wearer->x, wearer->y))
			{
				engine.gui->message(TCODColor::lightGrey,"%s drops %d %s",wearer->name,owner->pickable->stackSize,owner->name);
			}
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

Equipment::Equipment(bool equipped, SlotType slot, TCODList<ItemBonus *> bonus, ItemReq *requirement, bool stacks, int stackSize, PickableType type)
	: Pickable(stacks, stackSize,type), equipped(equipped), slot(slot), 
	bonus(bonus), requirement(requirement) {
	armorArt = 0;
}

void Equipment::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(equipped);
	zip.putInt(slot);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
	zip.putInt(bonus.size());
	for(int i = 0; i < bonus.size(); i++){///////////
		bonus.get(i)->save(zip);
	}
	//bonus->save(zip);
	requirement->save(zip);
}

void Equipment::load(TCODZip &zip) {
	equipped = zip.getInt();
	slot = (SlotType)zip.getInt();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
	int numBonus = zip.getInt();
	for(int i = 0; i < numBonus; i++){//////////
		ItemBonus *bon = new ItemBonus(ItemBonus::NOBONUS,0);
		bon->load(zip);
		bonus.push(bon);
	}
	ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	req->load(zip);
	requirement = req;
}

bool Equipment::use(Actor *owner, Actor *wearer) {
	if (!equipped) {
		switch(slot) {
			case HEAD:
					if(requirementsMet(owner,wearer)){
						if (wearer->container->head) {
							engine.gui->message(TCODColor::orange,"You swap out your head item.");
							wearer->container->head->pickable->use(wearer->container->head,wearer);
							//return false;
						} //else {
						
						wearer->container->head = owner;
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
				break; //} break;
			case CHEST:
					if(requirementsMet(owner,wearer)){
						if (wearer->container->chest) {
							engine.gui->message(TCODColor::orange,"You swap out your chest item.");
							wearer->container->chest->pickable->use(wearer->container->chest,wearer);
							//return false;
						} //else {
						
						wearer->container->chest = owner;
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
				break; //} break;
			case LEGS:
					if(requirementsMet(owner,wearer)){
						if (wearer->container->legs) {
							engine.gui->message(TCODColor::orange,"You swap out your leg item.");
							wearer->container->legs->pickable->use(wearer->container->legs,wearer);
							//return false;
						} //else {
						
						wearer->container->legs = owner;
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
				break; //} break;
			case FEET:
					if(requirementsMet(owner,wearer)){
						if (wearer->container->feet) {
							engine.gui->message(TCODColor::orange,"You swap out your feet item.");
							wearer->container->feet->pickable->use(wearer->container->feet,wearer);
							//return false;
						} //else {
						
						wearer->container->feet = owner;
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
				break; //} break;
			case HAND1:
					if(requirementsMet(owner,wearer)){
						if (wearer->container->hand1) {
							engine.gui->message(TCODColor::orange,"You swap out your main hand item.");
							wearer->container->hand1->pickable->use(wearer->container->hand1,wearer);
							//return false;
						} //else {
						if((((Weapon*)owner->pickable)->wType == Weapon::HEAVY) && (wearer->container->hand2 != NULL)){
							engine.gui->message(TCODColor::orange,"You swap out your off hand item.");
							wearer->container->hand2->pickable->use(wearer->container->hand2,wearer);
						}
						
						wearer->container->hand1 = owner;
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
				break; //} break;
			case HAND2:
					if(requirementsMet(owner,wearer)){
						if (wearer->container->hand2) {
							engine.gui->message(TCODColor::orange,"You swap out your off hand item.");
							wearer->container->hand2->pickable->use(wearer->container->hand2,wearer);
							//return false;
						}
						if((wearer->container->hand1) && ((Weapon*)wearer->container->hand1->pickable)->wType == Weapon::HEAVY){
							engine.gui->message(TCODColor::orange,"You swap out your heavy main hand item.");
							wearer->container->hand1->pickable->use(wearer->container->hand1,wearer);
						}//else {
						
						wearer->container->hand2 = owner;
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
				break; //} break;
			case RANGED:
					if(requirementsMet(owner,wearer)){
						if (wearer->container->ranged) {
							engine.gui->message(TCODColor::orange,"You swap out your ranged item.");
							wearer->container->ranged->pickable->use(wearer->container->ranged,wearer);
							//return false;
						} //else {
						
						wearer->container->ranged = owner;
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
				break; //} break;
			case NOSLOT: break;
			default: break;
		}
		equipped = true;
		for(int i = 0; i < bonus.size(); i++){/////////
			ItemBonus *thisBonus = bonus.get(i);
			switch(thisBonus->type) {
				case ItemBonus::NOBONUS: break;
				case ItemBonus::HEALTH: wearer->destructible->maxHp += thisBonus->bonus; break;
				case ItemBonus::DODGE: wearer->destructible->totalDodge += thisBonus->bonus; break;
				case ItemBonus::DR: wearer->destructible->totalDR += thisBonus->bonus; break;
				case ItemBonus::STRENGTH: wearer->totalStr += thisBonus->bonus; break;
				case ItemBonus::DEXTERITY: wearer->totalDex += thisBonus->bonus; break;
				case ItemBonus::INTELLIGENCE: wearer->totalIntel += thisBonus->bonus; break;
				default: break;
			}
		}
		wearer->container->sendToBegin(owner);
		return true;
	} else {
		equipped = false;
		switch(slot) {
			case HEAD: wearer->container->head = NULL; break;
			case CHEST: wearer->container->chest = NULL; break;
			case LEGS: wearer->container->legs = NULL; break;
			case FEET: wearer->container->feet = NULL; break;
			case HAND1: wearer->container->hand1 = NULL; break;
			case HAND2: wearer->container->hand2 = NULL; break;
			case RANGED: wearer->container->ranged = NULL; break;
			case NOSLOT: break;
			default: break;
		}
		for(int i = 0; i < bonus.size(); i++){///////
			ItemBonus *thisBonus = bonus.get(i);
			switch(thisBonus->type) {
				case ItemBonus::NOBONUS: break;
				case ItemBonus::HEALTH: 
					wearer->destructible->maxHp -= thisBonus->bonus;
					if (wearer->destructible->hp > wearer->destructible->maxHp) {
						wearer->destructible->hp = wearer->destructible->maxHp;
					}
					break;
				case ItemBonus::DODGE: wearer->destructible->totalDodge -= thisBonus->bonus; break;
				case ItemBonus::DR: wearer->destructible->totalDR -= thisBonus->bonus; break;
				case ItemBonus::STRENGTH: wearer->totalStr -= thisBonus->bonus; break;
				case ItemBonus::DEXTERITY: wearer->totalDex -= thisBonus->bonus; break;
				case ItemBonus::INTELLIGENCE: wearer->totalIntel -= thisBonus->bonus; break;
				default: break;
			}
		}
		wearer->container->inventory.remove(owner);
		wearer->container->inventory.push(owner);
		return true;
	}
	return false;
}

Weapon::Weapon(float minDmg, float maxDmg, float critMult, float critRange, float powerUse, WeaponType wType,
		bool equipped, SlotType slot, TCODList<ItemBonus *> bonus, ItemReq *requirement):
	Equipment(equipped, slot, bonus, requirement, false, 1, Pickable::WEAPON), 
		minDmg(minDmg), maxDmg(maxDmg), critMult(critMult),critRange(critRange),powerUse(powerUse),wType(wType){
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
	zip.putInt(bonus.size());
	for(int i = 0; i < bonus.size(); i++){///////////
		bonus.get(i)->save(zip);
	}
	//bonus->save(zip);
	requirement->save(zip);
	zip.putFloat(minDmg);
	zip.putFloat(maxDmg);
	zip.putFloat(critMult);
	zip.putFloat(critRange);
	zip.putFloat(powerUse);
	zip.putInt(wType);
}

void Weapon::load(TCODZip &zip) {
	equipped = zip.getInt();
	slot = (SlotType)zip.getInt();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
	int numBonus = zip.getInt();
	for(int i = 0; i < numBonus; i++){//////////
		ItemBonus *bon = new ItemBonus(ItemBonus::NOBONUS,0);
		bon->load(zip);
		bonus.push(bon);
	}
	ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	req->load(zip);
	requirement = req;
	minDmg = zip.getFloat();
	maxDmg = zip.getFloat();
	critMult = zip.getFloat();
	critRange = zip.getFloat();
	powerUse = zip.getFloat();
	wType = (WeaponType)zip.getInt();
}
Flamethrower::Flamethrower(float range, float powerUse, int width, bool equipped, SlotType slot, TCODList<ItemBonus *> bonus, ItemReq *requirement):
	Equipment(equipped, slot, bonus, requirement, false, 1, Pickable::FLAMETHROWER), range(range), powerUse(powerUse), width(width){
}

bool Flamethrower::use(Actor *owner, Actor *wearer){
	return Equipment::use(owner, wearer);
}

bool Flamethrower::ignite(Actor *owner){
	int x = engine.player->x;
	int y = engine.player->y;
	int width = ((Flamethrower*)(owner->container->ranged->pickable))->width;
	cout << "The Width is " << width << endl;
	//cout << "The Power Use is "<< powerUse << endl;
	if(owner->attacker->battery < powerUse){
		engine.gui->message(TCODColor::red, "You do not have enough battery to use the flamethrower");
		return false;
	}
	engine.gui->message(TCODColor::cyan, "Please choose a tile to ignite, "
		"or hit escape to cancel.");
	if(!engine.pickATile(&x,&y,range,0)){
		return false;
	}
	engine.gui->message(TCODColor::orange, "You ignite all tiles between yourself and your target");
	int xP = engine.player->x;
	int yP = engine.player->y;
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;
	int xT = 0;
	int yT = 0;
	
	if(x > engine.player->x){
		if(y > engine.player->y){
			x1 = x;
			y1 = y - 1;
			x2 = x - 1;
			y2 = y;
		}else if(y < engine.player->y){
			x1 = x;
			y1 = y + 1;
			x2 = x - 1;
			y2 = y;
		}else{
			x1 = x;
			y1 = y + 1;
			x2 = x;
			y2 = y - 1;
		}
	}else if(x < engine.player->x){
		if(y > engine.player->y){
			x1 = x + 1;
			y1 = y;
			x2 = x;
			y2 = y - 1;
		}else if(y < engine.player->y){
			x1 = x + 1;
			y1 = y ;
			x2 = x;
			y2 = y + 1;
		}else{
			x1 = x;
			y1 = y + 1;
			x2 = x;
			y2 = y - 1;
		}
	}else{
		if(y > engine.player->y){
			y1 = y;
			x1 = x+1;
			y2 = y;
			x2 = x-1;
		}else if(y < engine.player->y){
		}
	}
	// Going from point 5,8 to point 13,4
	if(width >= 1){
		TCODLine::init(xP,yP,x,y);
		do {
			// update cell x,y
			engine.map->tiles[x+y*engine.map->width].envSta = 1;
			engine.map->tiles[x+y*engine.map->width].temperature = 6;
			xT = x;
			yT = y;
		} while (!TCODLine::step(&x,&y));
	}
	if(width >= 2){
		TCODLine::init(xT,yT,x1,y1);
		do {
			// update cell x,y
			engine.map->tiles[x1+y1*engine.map->width].envSta = 1;
			engine.map->tiles[x1+y1*engine.map->width].temperature = 6;
		} while (!TCODLine::step(&x1,&y1));
	}
	if(width >= 3){
		TCODLine::init(xT,yT,x2,y2);
		do {
			// update cell x,y
			engine.map->tiles[x2+y2*engine.map->width].envSta = 1;
			engine.map->tiles[x2+y2*engine.map->width].temperature = 6;
		} while (!TCODLine::step(&x2,&y2));
	}
	owner->attacker->usePower(owner, powerUse);
	
	/*if(x == engine.player->x && y == engine.player->y){
		
	}else if(x == engine.player->x){
		int xx = x;
		int yy = y;
		if(y > engine.player->y){
			do{
				engine.map->tiles[xx+yy*engine.map->width].envSta = 1;
				engine.map->tiles[xx+yy*engine.map->width].temperature = 6;
				yy--;
			}while(yy > engine.player->y);
		}else if(y < engine.player->y){
			do{
				engine.map->tiles[xx+yy*engine.map->width].envSta = 1;
				engine.map->tiles[xx+yy*engine.map->width].temperature = 6;
				yy++;
			}while(yy < engine.player->y);
		}
	}else if(y == engine.player->y){
		int xx = x;
		int yy = y;
		if(x > engine.player->x){
			do{
				engine.map->tiles[xx+yy*engine.map->width].envSta = 1;
				engine.map->tiles[xx+yy*engine.map->width].temperature = 6;
				xx--;
			}while(xx > engine.player->x);
		}else if(x < engine.player->x){
			do{
				engine.map->tiles[xx+yy*engine.map->width].envSta = 1;
				engine.map->tiles[xx+yy*engine.map->width].temperature = 6;
				xx++;
			}while(xx < engine.player->x);
		}
	}else{
		//Need to decide in what order the tiles should be set on fire
		//int xx = x;
		//int yy = y;
		if(x > engine.player->x){
			if(y > engine.player->y){
				
			}else if(y < engine.player->y){
			}
		}else if(x < engine.player->x){
			if(y > engine.player->y){
			}else if(y < engine.player->y){
			}
		}
		
	}*/
	return true;
}

void Flamethrower::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(equipped);
	zip.putInt(slot);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
	zip.putInt(bonus.size());
	for(int i = 0; i < bonus.size(); i++){///////////
		bonus.get(i)->save(zip);
	}
	//bonus->save(zip);
	requirement->save(zip);
	zip.putFloat(range);
	zip.putFloat(powerUse);
}

void Flamethrower::load(TCODZip &zip) {
	equipped = zip.getInt();
	slot = (SlotType)zip.getInt();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
	int numBonus = zip.getInt();
	for(int i = 0; i < numBonus; i++){//////////
		ItemBonus *bon = new ItemBonus(ItemBonus::NOBONUS,0);
		bon->load(zip);
		bonus.push(bon);
	}
	ItemReq *req = new ItemReq(ItemReq::NOREQ,0);
	req->load(zip);
	requirement = req;
	range = zip.getFloat();
	powerUse = zip.getFloat();
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

Food::Food(int stackSize) : Pickable(1,stackSize,FOOD){}

void Food::load(TCODZip &zip) {
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Food::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Food::use(Actor *owner, Actor *wearer) {
	float amountFed = owner->hunger;
	wearer->feed(amountFed);
	if (amountFed > 0) {
		return Pickable::use(owner,wearer);
	}
	return false;
}

Key::Key(int keyType, bool stacks, int stackSize, PickableType type) 
: Pickable(stacks, stackSize,type), keyType(keyType) {
	used = false;

}

void Key::load(TCODZip &zip) {
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
	keyType = zip.getInt();
	used = zip.getInt();
}

void Key::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
	zip.putInt(keyType);
	zip.putInt(used);
}

bool Key::use(Actor *owner, Actor *wearer)
{
	if(wearer == engine.player && keyType == 0 && !used)
	{
		engine.gui->message(TCODColor::blue, "This key seems to go to some sort of vault, possibly found in an armory");
		return false;
	}
	else
	{
		return Pickable::use(owner,wearer);
	}
}

Alcohol::Alcohol(int str, int qual)
: Pickable(true, 1, ALCOHOL){
	strength = str;
	quality = qual;
}

void Alcohol::load(TCODZip &zip) {
	stacks = zip.getInt();
	stackSize = zip.getInt();
	strength = zip.getInt();
	quality = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Alcohol::save(TCODZip &zip) {
	zip.putInt(type);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(strength);
	zip.putInt(quality);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Alcohol::use(Actor *owner, Actor *wearer) {
	if (wearer->totalIntel > 0)
	{
		Aura *alcoholSTR = new Aura(quality, Aura::TOTALSTR, Aura::CONTINUOUS, strength);//is this good mitchell?
		Aura *alcoholDR = new Aura(quality, Aura::TOTALDR, Aura::CONTINUOUS, strength/2);//is this good mitchell?
		Aura *alcoholINT = new Aura(quality, Aura::TOTALINTEL, Aura::CONTINUOUS, -strength);//is this good mitchell?
		Aura *alcoholDODGE = new Aura(quality, Aura::TOTALDODGE, Aura::CONTINUOUS, -strength/2);//is this good mitchell?
		engine.player->auras.push(alcoholSTR);
		engine.player->auras.push(alcoholINT);
		engine.player->auras.push(alcoholDR);
		engine.player->auras.push(alcoholDODGE);
		engine.gui->message(TCODColor::blue, "You drink the %s and you begin to feel stronger, but more confused.",owner->name);
		alcoholSTR->apply(engine.player);
		alcoholINT->apply(engine.player);
		alcoholDR->apply(engine.player);
		alcoholDODGE->apply(engine.player);

		return Pickable::use(owner,wearer);
	}
	else
	{
		engine.gui->message(TCODColor::red, "You try to drink from the %s but are so drunk it shatters everywhere since your INT is 0.",owner->name);
		return Pickable::use(owner,wearer);
	}
}

Teleporter::Teleporter(float range, bool stacks, int stackSize, PickableType type) 
	: Pickable(stacks, stackSize, type), range(range) {
}

void Teleporter::load(TCODZip &zip) {
	range = zip.getFloat();
	stacks = zip.getInt();
	stackSize = zip.getInt();
	value = zip.getInt();
	inkValue = zip.getInt();
}

void Teleporter::save(TCODZip &zip) {
	zip.putInt(TELEPORTER);
	zip.putFloat(range);
	zip.putInt(stacks);
	zip.putInt(stackSize);
	zip.putInt(value);
	zip.putInt(inkValue);
}

bool Teleporter::use(Actor *owner, Actor *wearer) {
	engine.gui->message(TCODColor::orange, "Choose where to teleport "
		"or hit escape to cancel.");
	int x = engine.player->x;
	int y = engine.player->y;
	if (!engine.pickATile(&x,&y, range+1, 0)) {
		return false;
	}
	//teleport if not blocked
	if(!engine.map->isVisible(x,y) || !engine.map->canWalk(x,y)){
		engine.gui->message(TCODColor::orange,"You cannot teleport there!");
		return false;
	}
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		if (actor->getDistance(x,y) <= 0 && actor->blocks == true) {
			engine.gui->message(TCODColor::orange,"You cannot teleport there!");
			return false;
		}
	}
	if(engine.player->job[0] == 'H'){ //hacker
		engine.gui->message(TCODColor::orange, "As a hacker, you mod your own x and y variables to tactically reposition yourself!");
		
		if (engine.player->companion && engine.player->companion->destructible && !engine.player->companion->destructible->isDead()){
			((CompanionAi*)(engine.player->companion->ai))->teleportMessage(engine.player->companion);
		}
	}else{
		engine.gui->message(TCODColor::orange, "You teleport to the chosen location!");
		if (engine.player->companion && engine.player->companion->destructible && !engine.player->companion->destructible->isDead()){
			((CompanionAi*)(engine.player->companion->ai))->teleportMessage(engine.player->companion);
		}
	}
	
	engine.player->x = x;
	engine.player->y = y;
	engine.playerLight->x = x;
	engine.playerLight->y = y;
	
	if (engine.player->companion && engine.player->companion->destructible && !engine.player->companion->destructible->isDead()){
		engine.player->companion->x = x;
		engine.player->companion->y = y;
	}
	engine.map->computeFov();
	return Pickable::use(owner,wearer);
}
