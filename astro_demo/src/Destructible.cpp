#include <stdio.h>
#include "main.hpp"

Destructible::Destructible(float maxHp, float dodge, const char *corpseName, int xp) :
	maxHp(maxHp),hp(maxHp),baseDodge(dodge+10),totalDodge(dodge+10), xp(xp) {
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
	corpseName = strdup(zip.getString());
	xp = zip.getInt();
}

void Destructible::save(TCODZip &zip) {
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(baseDodge);
	zip.putFloat(totalDodge);
	zip.putString(corpseName);
	zip.putInt(xp);
}

Destructible *Destructible::create(TCODZip &zip) {
	DestructibleType type = (DestructibleType)zip.getInt();
	Destructible *destructible = NULL;
	switch(type) {
		case MONSTER : destructible = new MonsterDestructible(0,0,NULL,0); break;
		case PLAYER : destructible = new PlayerDestructible(0,0,NULL); break;
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
	else if(owner->ch == 131 || owner->ch == 147 || owner->ch == 'V') //roomba, vendors, and turrets show no corpse currently
	{
		owner->ch = ' ';
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

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp) :
	Destructible(maxHp, defense, corpseName, xp) {
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) : 
	Destructible(maxHp, defense, corpseName,0) {
}

void MonsterDestructible::die(Actor *owner) {
	//transform it into a corpse
	//doesnt block, cant be attacked, doesnt move
	if(owner->ch != 243 && owner->ch != 131 && owner->ch != 147 && owner->ch != 'V'){
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
	}else if(owner->ch == 'V') //Vending machine, change when needed
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