#include <stdio.h>
#include <math.h>
#include "main.hpp"
#include <iostream>

Actor::Actor(int x, int y, int ch, const char *name, const TCODColor &col):
	x(x),y(y),ch(ch),str(5),dex(3),intel(3),vit(5),totalStr(5),totalDex(3), totalIntel(3),col(col),name(name),race("Human"),role("Marine"),job("Infantry"), blocks(true),hostile(true),interact(false), smashable(false), oozing(false), susceptible(false),flashable(false), sort(0), hunger(20), maxHunger(20), hungerCount(0), companion(NULL), attacker(NULL),destructible(NULL),ai(NULL),
	pickable(NULL), container(NULL) {
}

Actor::Actor(int x, int y, int ch, const char *name, const char *race, const char *role, const char *job, const TCODColor &col):
	x(x),y(y),ch(ch),str(5),dex(2),intel(3),vit(5),totalStr(5),totalDex(3), totalIntel(3),col(col),name(name),race(race),role(role),job(job), blocks(true),hostile(true),interact(false),smashable(false), oozing(false), susceptible(false), flashable(false), sort(0), hunger(20), maxHunger(20), hungerCount(0), companion(NULL), attacker(NULL),destructible(NULL),ai(NULL),
	pickable(NULL), container(NULL) {
}

Actor::~Actor() {
	if (attacker) delete attacker;
	if (destructible) delete destructible;
	if (ai) delete ai;
	if (pickable) delete pickable;
	if (container) delete (container);
}

void Actor::save(TCODZip &zip) {
	zip.putInt(x);
	zip.putInt(y);
	zip.putInt(ch);
	zip.putInt(str);
	zip.putInt(dex);
	zip.putInt(intel);
	zip.putInt(totalStr);
	zip.putInt(totalDex);
	zip.putInt(totalIntel);
	zip.putColor(&col);
	zip.putString(name);
	zip.putString(race);
	zip.putString(role);
	zip.putString(job);
	zip.putInt(blocks);
	zip.putInt(hostile);
	zip.putInt(interact);
	zip.putInt(smashable);
	zip.putInt(flashable);
	zip.putInt(oozing);
	zip.putInt(susceptible);
	zip.putInt(sort);
	zip.putInt(hunger);
	zip.putInt(maxHunger);
	zip.putInt(hungerCount);
	
	
	zip.putInt(auras.size());
	for (Aura **it = auras.begin(); it != auras.end(); it++) {
		(*it)->save(zip);
	}
	
	zip.putInt(attacker != NULL);
	zip.putInt(destructible != NULL);
	zip.putInt(ai != NULL);
	zip.putInt(pickable != NULL);
	zip.putInt(container != NULL);
	zip.putInt(companion != NULL);
	
	if (attacker) attacker->save(zip);
	if (destructible) destructible->save(zip);
	if (ai) ai->save(zip);
	if (pickable) pickable->save(zip);
	if (container) container->save(zip);
	if (companion) companion->save(zip);
	
	
}

void Actor::load(TCODZip &zip) {
	x = zip.getInt();
	y = zip.getInt();
	ch = zip.getInt();
	str = zip.getInt();
	dex = zip.getInt();
	intel = zip.getInt();
	totalStr = zip.getInt();
	totalDex = zip.getInt();
	totalIntel = zip.getInt();
	col= zip.getColor();
	name = strdup(zip.getString());
	std::cout << "got name " << name << std::endl;
	race = strdup(zip.getString());
	role = strdup(zip.getString());
	job = strdup(zip.getString());
	blocks = zip.getInt();
	hostile = zip.getInt();
	interact = zip.getInt();
	smashable = zip.getInt();
	flashable = zip.getInt();
	oozing = zip.getInt();
	susceptible = zip.getInt();
	sort = zip.getInt();
	hunger = zip.getInt();
	maxHunger = zip.getInt();
	hungerCount = zip.getInt();
	
	int nbAuras = zip.getInt();
	while (nbAuras > 0) {
		Aura *aura = new Aura(0,(Aura::StatType)0,(Aura::LifeStyle)0,0);
		aura->load(zip);
		auras.push(aura);
		nbAuras--;
	}
	
	bool hasAttacker = zip.getInt();
	std::cout <<"got hasAttacker " << hasAttacker << std::endl;
	bool hasDestructible = zip.getInt();
	std::cout <<"got hasDestructible "<< hasDestructible << std::endl;
	bool hasAi = zip.getInt();
	std::cout <<"got hasAi "<< hasAi << std::endl;
	bool hasPickable = zip.getInt();
	std::cout <<"got hasPickable " << hasPickable << std::endl;
	bool hasContainer = zip.getInt();
	std::cout <<"got hasContainer " << hasContainer << std::endl;
	bool hasCompanion = zip.getInt();
	std::cout <<"got hasCompanion " << hasCompanion << std::endl;
	
	if (hasAttacker) {
		attacker = new Attacker(0.0f);
		attacker->load(zip);
	}
	std::cout <<"got attacker module" << std::endl;
	
	if (hasDestructible) {
		std::cout << "trying destructible" << std::endl;
		destructible = Destructible::create(zip);
	}
	std::cout <<"got destructible module" << std::endl;
	
	if (hasAi) {
		std::cout << "trying ai" <<std::endl;
		ai = Ai::create(zip);
	}
	std::cout <<"got ai module" << std::endl;
	
	if (hasPickable) {
		std::cout << "trying pickable module" <<std::endl;
		pickable = Pickable::create(zip);
	}
	std::cout <<"got pickable module" << std::endl;
	
	if (hasContainer) {
		std::cout << "trying container module " << std::endl;
		container = new Container(0);
		container->load(zip);
	}
	std::cout <<"got container module" << std::endl;
	
	if (hasCompanion){
		Actor *act = new Actor(0,0,0,NULL,TCODColor::white);
		act->load(zip);
		companion = act;
		std::cout << "got companion" <<std::endl;
	}
}
	
void Actor::render() const
{
	TCODConsole *offscrn = new TCODConsole(1,1);
	offscrn->setChar(0, 0, ch);
	offscrn->setCharForeground(0,0,col); 
	//offscrn->setKeyColor(TCODColor::black);
	TCODConsole::blit(offscrn,0,0,0,0,engine.mapcon,x,y,1.0,0.0);
	delete offscrn;
}

void Actor::update()
{
	if (ai) ai->update(this);
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x - cx;
	int dy = y - cy;
	return sqrt(dx*dx+dy*dy);
}

float Actor::getHpUp(){
	if(job[0] == 'A')
		return 7;
	else
		return 20;
}

float Actor::getHealValue(){
	float factor = 1;
	if(race[0] == 'R')
		factor *= .5;
	if(job[0] == 'A')
		factor *= .6;
	return (int)(factor * (this->totalIntel * 3 + 6));
}
	
float Actor::feed(float amount) {
	hunger += amount;
	if (hunger > maxHunger) { 
		amount -= hunger-maxHunger;
		hunger = maxHunger;
	}
	if (hunger > 0 && amount > 0) {
		if(!this->auras.isEmpty()){
			for (Aura **iter = this->auras.begin(); iter!=this->auras.end(); iter++){
				Aura *aura = *iter;
				if (aura->life == Aura::HUNGER){
					aura->unApply(this);
					delete *iter;
					iter = this->auras.remove(iter);
				}
			}
		}
	}
	this->hungerCount = 0;
	engine.gui->message(TCODColor::red,"You feel reinvigorated after eating!");
	return amount;
}	
	
void Actor::getHungry(){
	if (engine.turnCount > 0 && (engine.turnCount)%2 == 0){
		if (this->hunger > 0) {
			this->hunger -= 1;
		} else {
			this->hungerCount += 1;
		}
		if (this->hungerCount > 0 && this->hungerCount % 50 == 0){
			Aura *aura = new Aura(0,Aura::ALL,Aura::HUNGER,-1);
			aura->apply(this);
			this->auras.push(aura);
			engine.gui->message(TCODColor::red,"You feel weak from hunger. Perhaps you should eat.");
		}
		if (this->hungerCount > 0 && this->hungerCount % 75== 0) {
			Aura *aura = new Aura(7,Aura::HEALTH,Aura::ITERABLE,-1);
			//aura->apply(this);   if it is applied here as well, double damage on first turn
			this->auras.push(aura);
			engine.gui->message(TCODColor::darkerRed,"You begin to feel great hunger pains!");
		}
	}
}

void Actor::updateAuras(){
	if(!this->auras.isEmpty()){
		for (Aura **iter = this->auras.begin(); iter!=this->auras.end(); iter++){
			Aura *aura = *iter;
			if (aura->life != Aura::HUNGER){
				aura->duration--;
				if (aura->life == Aura::ITERABLE){
					aura->apply(this);
				}
				if (aura->duration == 0){
					aura->unApply(this);
					delete *iter;
					iter = this->auras.remove(iter);
				}
			}
		}
	}
}
	
/* bool Actor::moveOrAttack(int x, int y)
{
	if (engine.map->isWall(x,y)) return false;
	for (Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++)
	{
		Actor *actor = *iterator;
		if (actor->x ==x && actor->y == y)
		{
			printf("The %s laughs at your puny efforts to attack him!\n", actor->name);
			return false;
		}
	}
	this->x=x;
	this->y=y;
	return true;
} */
