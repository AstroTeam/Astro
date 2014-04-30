#include <stdio.h>
#include "main.hpp"
#include <cstring>
#include <iostream>
using namespace std;
Destructible::Destructible(float maxHp, float dodge, float dr, int xp) :
	maxHp(maxHp),hp(maxHp),baseDodge(dodge+10),totalDodge(dodge+10), baseDR(dr), totalDR(dr), xp(xp) {
	hasDied = false;
}

Destructible::~Destructible() {
	//free(corpseName);
}

void Destructible::load(TCODZip &zip) {
	maxHp = zip.getFloat();
	hp = zip.getFloat();
	baseDodge = zip.getFloat();
	totalDodge = zip.getFloat();
	baseDR = zip.getFloat();
	totalDR = zip.getFloat();
	xp = zip.getInt();
	hasDied = zip.getInt();
}

void Destructible::save(TCODZip &zip) {
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(baseDodge);
	zip.putFloat(totalDodge);
	zip.putFloat(baseDR);
	zip.putFloat(totalDR);
	zip.putInt(xp);
	zip.putInt(hasDied);
}

Destructible *Destructible::create(TCODZip &zip) {
	DestructibleType type = (DestructibleType)zip.getInt();
	Destructible *destructible = NULL;
	switch(type) {
		case MONSTER : destructible = new MonsterDestructible(0,0,0,0); break;
		case PLAYER : destructible = new PlayerDestructible(0,0,0); break;
	}
	destructible->load(zip);
	return destructible;
}

float Destructible::takeDamage(Actor *owner, Actor *attacker, float damage) {
	//take a second Actor pointer here, such as attacker, also pass it into the die method
	if (owner->attacker && (owner->attacker->lastTarget == NULL || owner->attacker->lastTarget->destructible->isDead())) {
		owner->attacker->lastTarget = attacker;
	}
	
	if(owner->ch == 225) //meaning you're attacking a Vending machine
	{
		VendingAi* va = (VendingAi*) owner->ai;
		va->deployedSecurity = true;
	}
	if(owner->ch == 243 && (engine.map->tiles[owner->x+owner->y*engine.map->width].decoration == 56 || engine.map->tiles[owner->x+owner->y*engine.map->width].decoration == 57)) //weapon vault
		return 0; //can't damage vaults
	if (damage > 0){
		hp -= (int) damage;
		if (hp <= 0 && !hasDied) {
			//die(owner, attacker);
			if(attacker && (attacker == engine.player || attacker == engine.player->companion)) //only increase XP if the player/companion is the killer
				engine.player->destructible->xp += xp;
		}
	} else {
		damage = 0;	
	}
	
	if (engine.map->tiles[owner->x+owner->y*engine.mapWidth].envSta == 0 && !(owner->ch == 131 || owner->ch == 147 || owner->ch == 225 || owner->ch == 130 || owner->ch == 129 || owner->ch == 146 || owner->ch == 189 || owner->ch == 145 || owner->ch == 157 || owner->ch == 165 || owner->ch == 166 || owner->ch == 243 || owner->ch == 157 || owner->ch == 158 || owner->ch == 174 ))
		engine.map->tiles[owner->x+owner->y*engine.mapWidth].envSta = 3;
	
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
			if(engine.map->isVisible(owner->x, owner->y))
				{
					engine.gui->message(TCODColor::red, "%s takes %g fire damage.",owner->name,damage);
					Aura *fire = new Aura(4, Aura::FIRE, Aura::ITERABLE, -3);
					engine.player->auras.push(fire);
					//fire->apply(engine.player);
					//engine.gui->message(TCODColor::orange,"%s has caught fire!",owner->name);
				}
			if (hp <= 0) {
				//die(owner, NULL);
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

void Destructible::die(Actor *owner, Actor *killer) {
	//transform the actor into a corpse
	//check who owner was to decide what corpse they get
	//if spore creature they get spore body
	int dummyAscii = 145;
	hasDied = true;
	
	if (owner->ch == 165 || owner->ch == 166){ 
		owner->ch = 162;
		owner->blocks = false;
	}
	else if(owner->ch == 243 && engine.map->tiles[owner->x+owner->y*engine.map->width].decoration == 23){
		//engine.mapconDec->setChar(owner->x,owner->y, 24);//Locker
		engine.map->tiles[owner->x+owner->y*engine.map->width].decoration = 24;
		owner->ch = 243;
	}
	else if(owner->ch == 131 || owner->ch == 147 || owner->ch == 225 || owner->ch == 130 || owner->ch == 129 || owner->ch == 146 || owner->ch == 189 || owner->ch == dummyAscii || owner->ch == 157) //roomba, vendors, and turrets, 
	{
		if(owner->ch == 189)
		{
			TurretControlAi *tc = (TurretControlAi*) owner->ai;
			if(tc && tc->attackMode == 1) //only make turrets go into frenzy mode if they were originally in default state
			{
				tc->attackMode = 2;
				engine.gui->message(TCODColor::orange, "Warning: The turrets in this room have been disconnected from their machine intelligence and will attack anything!");
			}
		}
		owner->ch = 161;
		owner->blocks = false;
	}else if(owner->ch == 243){
		owner->col = TCODColor::darkGrey;
	}//else generic blood whale
	else
	{
		owner->ch = 163;
		owner->blocks = false;
	}
	owner->col = TCODColor::white;
	char* newname = new char[100];
	strcpy(newname,owner->name);
	owner->name = strcat(newname,"'s remains");
	//corpseName;
	//owner->blocks = false;
	//make sure corpses are drawn before other important things
	engine.sendToBack(owner);

}

MonsterDestructible::MonsterDestructible(float maxHp, float dodge, float dr, int xp) :
	Destructible(maxHp, dodge, dr, xp) {
}

PlayerDestructible::PlayerDestructible(float maxHp, float dodge, float dr) : 
	Destructible(maxHp, dodge, dr, 0) {
}

void MonsterDestructible::die(Actor *owner, Actor *killer) {
	//transform it into a corpse
	//doesnt block, cant be attacked, doesnt move
	//cout << "Destrutible::Die beginning" << endl;
	//cout << owner->ch << endl;
	//cout << "the char to test" << endl;
	int dummyAscii = 145;
	if((owner->ch != 243 && owner->ch != 131 && owner->ch != 147 && owner->ch != 225 && owner->ch != 130 && owner->ch != 129 && owner->ch != 146 && owner->ch != dummyAscii) && owner->ch != 157){
		engine.killCount++;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::lightGrey,"The %s is dead!", owner->name);
	}
	else if(owner->ch == 131) //Ascii for Cleaner bot
	{
		engine.killCount++;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == 147 || owner->ch == 189) //Ascii for Sentry Turret
	{
		engine.killCount++;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}else if(owner->ch == 130 || owner->ch == 129 || owner->ch == 146) //ascii for security bot
	{
		engine.killCount++;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == 225) //Vending machine, change when needed
	{
		engine.killCount++;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == dummyAscii) //change to target dummy
	{
		//engine.killCount++;
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::lightGrey,"The %s crumples into a useless pile of metal!", owner->name);
		//cout << "target dummy killed!" << endl;
	}
	else if(owner->ch == 157) //scout drone
	{
		if(engine.map->isVisible(owner->x, owner->y))
			engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	//cout << "done testing" << endl;
	
	
	//Makes Vending UI appear upon monster death (For Testing Purposes Only)
	//engine.gui->vendingMenu(owner);
	if(owner->ch != 225 && owner->ch != dummyAscii){
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
	}
	//cout << "destrutible::Die called" << endl;
	Destructible::die(owner, killer);
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
	Destructible::die(owner, NULL);
}
void PlayerDestructible::die(Actor *owner, Actor *killer) {
	engine.gui->message(TCODColor::darkRed,"You died!\n");
	Destructible::die(owner, killer);
	engine.gameStatus=Engine::DEFEAT;
	//engine.save();
}

void PlayerDestructible::save(TCODZip &zip) {
	zip.putInt(PLAYER);
	Destructible::save(zip);
}

void MonsterDestructible::save(TCODZip &zip) {
	zip.putInt(MONSTER);
	Destructible::save(zip);
}