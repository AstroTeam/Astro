#include <stdio.h>
#include "main.hpp"

Destructible::Destructible(float maxHp, float dodge, float dr, const char *corpseName, int xp) :
	maxHp(maxHp),hp(maxHp),baseDodge(dodge+10),totalDodge(dodge+10), baseDR(dr), totalDR(dr), xp(xp) {
	this->corpseName = strdup(corpseName);
}

Destructible::~Destructible() {
	free(corpseName);
}

void Destructible::load(TCODZip &zip) {
	maxHp = zip.getFloat();
	hp = zip.getFloat();
	baseDodge = zip.getFloat();
	totalDodge = zip.getFloat();
	baseDR = zip.getFloat();
	totalDR = zip.getFloat();
	corpseName = strdup(zip.getString());
	xp = zip.getInt();
}

void Destructible::save(TCODZip &zip) {
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(baseDodge);
	zip.putFloat(totalDodge);
	zip.putFloat(baseDR);
	zip.putFloat(totalDR);
	zip.putString(corpseName);
	zip.putInt(xp);
}

Destructible *Destructible::create(TCODZip &zip) {
	DestructibleType type = (DestructibleType)zip.getInt();
	Destructible *destructible = NULL;
	switch(type) {
		case MONSTER : destructible = new MonsterDestructible(0,0,0,NULL,0); break;
		case PLAYER : destructible = new PlayerDestructible(0,0,0,NULL); break;
	}
	destructible->load(zip);
	return destructible;
}

float Destructible::takeDamage(Actor *owner, float damage) {
	
	if (damage > 0){
		hp -= damage;
		if (hp <= 0) {
			die(owner);
		}
	} else {
		damage = 0;
	}
	if(owner->ch == 225) //meaning you're attacking a Vending machine
	{
		VendingAi* va = (VendingAi*) owner->ai;
		va->deployedSecurity = true;
	}
	
	return damage;
}

float Destructible::takeFireDamage(Actor *owner, float damage) {
	if(engine.map->tiles[(owner->x)+(owner->y)*engine.map->width].temperature > 0 && !owner->destructible->isDead())
	{
		//SHOULD SCALE WITH DUNGEON LEVEL
		//int dmg = 0;//engine.map->tiles[(targetx)+(targety)*engine.map->width].temperature*0.5;
		//if (engine.map->tiles[(owner->x)+(owner->y)*engine.map->width].temperature > 0)
			//dmg = 3;
		//owner->destructible->takeDamage(owner, (float)(dmg));
		//if (dmg > 0)
		
		
		if (damage > 0){
			hp -= damage;
			engine.gui->message(TCODColor::red, "%s takes %g fire damage.",owner->name,damage);
			if (hp <= 0) {
				die(owner);
			}
		} else {
			damage = 0;
		}
	}
	
	return damage;
}

float Destructible::heal(float amount) {
	hp += amount;
	if (hp > maxHp) { 
		amount -= hp-maxHp;
		hp = maxHp;
	}
	return amount;
}

void Destructible::die(Actor *owner) {
	//transform the actor into a corpse
	//check who owner was to decide what corpse they get
	//if spore creature they get spore body
	
	if (owner->ch == 165 || owner->ch == 166){ 
		owner->ch = 162;
		owner->blocks = false;
	}
	else if(owner->ch == 243){
		//engine.mapconDec->setChar(owner->x,owner->y, 24);//Locker
		engine.map->tiles[owner->x+owner->y*engine.map->width].decoration = 24;
		owner->ch = 243;
	}
	else if(owner->ch == 131 || owner->ch == 147 || owner->ch == 225 || owner->ch == 130) //roomba, vendors, and turrets, and security bots show no corpse currently
	{
		owner->ch = 161;
		owner->blocks = false;
	}//else generic blood whale
	else
	{
		owner->ch = 163;
		owner->blocks = false;
	}
	owner->col = TCODColor::white;
	owner->name = corpseName;
	//owner->blocks = false;
	//make sure corpses are drawn before other important things
	engine.sendToBack(owner);
}

MonsterDestructible::MonsterDestructible(float maxHp, float dodge, float dr, const char *corpseName, int xp) :
	Destructible(maxHp, dodge, dr, corpseName, xp) {
}

PlayerDestructible::PlayerDestructible(float maxHp, float dodge, float dr, const char *corpseName) : 
	Destructible(maxHp, dodge, dr, corpseName,0) {
}

void MonsterDestructible::die(Actor *owner) {
	//transform it into a corpse
	//doesnt block, cant be attacked, doesnt move
	if(owner->ch != 243 && owner->ch != 131 && owner->ch != 147 && owner->ch != 225 && owner->ch != 130){
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is dead! You feel a rush as it sputters its last breath.", owner->name);
	}
	else if(owner->ch == 131) //Ascii for Cleaner bot
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == 147) //Ascii for Sentry Turret
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}else if(owner->ch == 130) //ascii for security bot
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == 225) //Vending machine, change when needed
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	engine.player->destructible->xp += xp;
	
	//Makes Vending UI appear upon monster death (For Testing Purposes Only)
	//engine.gui->vendingMenu(owner);
	
	if(!owner->container->inventory.isEmpty()){
		Actor **iterator=owner->container->inventory.begin();
		for(int i = 0; i < owner->container->size; i++){
			if(owner->container->inventory.isEmpty()){
				break;
			}
			Actor *actor = *iterator;
			if(actor){
				actor->pickable->drop(actor,owner,true);
			}
			
			if(iterator != owner->container->inventory.end())
			{
				++iterator;
			}
		}
	}
	Destructible::die(owner);
}

void MonsterDestructible::suicide(Actor *owner) {
	//transform it into a corpse
	//doesnt block, cant be attacked, doesnt move
	hp = 0;
	if(!owner->container->inventory.isEmpty()){
		Actor **iterator=owner->container->inventory.begin();
		for(int i = 0; i < owner->container->size; i++){
			if(owner->container->inventory.isEmpty()){
				break;
			}
			Actor *actor = *iterator;
			if(actor){
				actor->pickable->drop(actor,owner,true);
			}
			
			if(iterator != owner->container->inventory.end())
			{
				++iterator;
			}
		}
	}
	Destructible::die(owner);
}
void PlayerDestructible::die(Actor *owner) {
	engine.gui->message(TCODColor::darkRed,"You died!\n");
	Destructible::die(owner);
	engine.gameStatus=Engine::DEFEAT;
	engine.save();
}

void PlayerDestructible::save(TCODZip &zip) {
	zip.putInt(PLAYER);
	Destructible::save(zip);
}

void MonsterDestructible::save(TCODZip &zip) {
	zip.putInt(MONSTER);
	Destructible::save(zip);
}