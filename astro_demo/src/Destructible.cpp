#include <stdio.h>
#include "main.hpp"
#include <cstring>
#include <iostream>
using namespace std;
Destructible::Destructible(float maxHp, float dodge, float dr, int xp) :
	maxHp(maxHp),hp(maxHp),baseDodge(dodge+10),totalDodge(dodge+10), baseDR(dr), totalDR(dr), xp(xp) {
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
}

void Destructible::save(TCODZip &zip) {
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(baseDodge);
	zip.putFloat(totalDodge);
	zip.putFloat(baseDR);
	zip.putFloat(totalDR);
	zip.putInt(xp);
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
	int dummyAscii = 145;
	
	
	if (owner->ch == 165 || owner->ch == 166){ 
		owner->ch = 162;
		owner->blocks = false;
	}
	else if(owner->ch == 243 && engine.map->tiles[owner->x+owner->y*engine.map->width].decoration == 23){
		//engine.mapconDec->setChar(owner->x,owner->y, 24);//Locker
		engine.map->tiles[owner->x+owner->y*engine.map->width].decoration = 24;
		owner->ch = 243;
	}
	else if(owner->ch == 131 || owner->ch == 147 || owner->ch == 225 || owner->ch == 130 || owner->ch == 129 || owner->ch == 146 || owner->ch == 'C' || owner->ch == dummyAscii) //roomba, vendors, and turrets, 
	{
		if(owner->ch == 'C')
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

void MonsterDestructible::die(Actor *owner) {
	//transform it into a corpse
	//doesnt block, cant be attacked, doesnt move
	//cout << "Destrutible::Die beginning" << endl;
	//cout << owner->ch << endl;
	//cout << "the char to test" << endl;
	int dummyAscii = 145;
	if(owner->ch != 243 && owner->ch != 131 && owner->ch != 147 && owner->ch != 225 && owner->ch != 130 && owner->ch != 129 && owner->ch != 146 && owner->ch != dummyAscii){
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is dead! You feel a rush as it sputters its last breath.", owner->name);
	}
	else if(owner->ch == 131) //Ascii for Cleaner bot
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == 147 || owner->ch == 'C') //Ascii for Sentry Turret
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}else if(owner->ch == 130 || owner->ch == 129 || owner->ch == 146) //ascii for security bot
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == 225) //Vending machine, change when needed
	{
		engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s is destroyed!", owner->name);
	}
	else if(owner->ch == dummyAscii) //change to target dummy
	{
		//engine.killCount++;
		engine.gui->message(TCODColor::lightGrey,"The %s crumples into a useless pile of metal!", owner->name);
		//cout << "target dummy killed!" << endl;
	}
	//cout << "done testing" << endl;
	engine.player->destructible->xp += xp;
	
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