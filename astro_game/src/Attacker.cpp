#include <stdio.h>
#include "main.hpp"

Attacker::Attacker(float power) : basePower(power), totalPower(power), 
	maxBattery(0), battery(maxBattery), lastTarget(NULL) {
}

Attacker::Attacker(float power, float maxBattery) : basePower(power), totalPower(power), 
	maxBattery(maxBattery), battery(maxBattery), lastTarget(NULL) {
}

void Attacker::load(TCODZip &zip) {
	basePower = zip.getFloat();
	totalPower = zip.getFloat();
	maxBattery = zip.getFloat();
	battery = zip.getFloat();
}

void Attacker::save(TCODZip &zip) {
	zip.putFloat(basePower);
	zip.putFloat(totalPower);
	zip.putFloat(maxBattery);
	zip.putFloat(battery);
}

void Attacker::attack(Actor *owner, Actor *target) {
	if (target->destructible && !target->destructible->isDead() ) {
		//hit chance calculator
		int roll = TCODRandom::getInstance()->getInt(1,20);
		int attackRoll = roll + owner->totalStr;
		float damageTaken = 0;
		float damageRoll1 = (float)TCODRandom::getInstance()->getInt(1,2);
		float critMult = 2;
		float critRange = 20;
		int hands = 1;
		if(owner->container->hand1){
			int powerUse = ((Weapon*)owner->container->hand1->pickable)->powerUse;
			if(owner != engine.player || owner->attacker->battery >= powerUse){
				owner->attacker->usePower(owner, powerUse);
				int minDmg = ((Weapon*)owner->container->hand1->pickable)->minDmg;
				int maxDmg = ((Weapon*)owner->container->hand1->pickable)->maxDmg;
				critMult = ((Weapon*)owner->container->hand1->pickable)->critMult;
				critRange = ((Weapon*)owner->container->hand1->pickable)->critRange;
				damageRoll1 = (float)TCODRandom::getInstance()->getInt(minDmg,maxDmg);
				if(((Weapon*)owner->container->hand1->pickable)->wType == Weapon::HEAVY){
					hands = 2;
				}
			}
			else{
				engine.gui->message(TCODColor::lightGrey,"Not enough battery, %s uses fists instead.", owner->name);
			}
		}
		if(roll >= critRange){
			if(engine.map->isVisible(owner->x, owner->y))
				engine.gui->message(TCODColor::red,"CRITICAL HIT!");
			damageTaken += (critMult * damageRoll1) + hands * owner->totalStr - target->destructible->totalDR;
		}
		else if(roll <= 1){
			if(engine.map->isVisible(owner->x, owner->y))
				engine.gui->message(TCODColor::lightGrey,"critical miss...");
			damageTaken += 0;
		}
		else if(attackRoll >= target->destructible->totalDodge){
			damageTaken += damageRoll1 + hands * owner->totalStr - target->destructible->totalDR;
		}
		//HAND2 stuff!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if(owner->container->hand2 != NULL){
			int powerUse = ((Weapon*)owner->container->hand2->pickable)->powerUse;
			if(owner != engine.player || owner->attacker->battery >= powerUse){
				owner->attacker->usePower(owner, powerUse);
				int minDmg = ((Weapon*)owner->container->hand2->pickable)->minDmg;
				int maxDmg = ((Weapon*)owner->container->hand2->pickable)->maxDmg;
				critMult = ((Weapon*)owner->container->hand2->pickable)->critMult;
				float damageRoll2 = (float)TCODRandom::getInstance()->getInt(minDmg,maxDmg);
				roll = TCODRandom::getInstance()->getInt(1,20);
				attackRoll = roll + owner->totalStr;
				if(roll >= 20){
					if(engine.map->isVisible(owner->x, owner->y))
						engine.gui->message(TCODColor::red,"OFFHAND CRITICAL HIT!");
					damageTaken += critMult * damageRoll2 + ((int)(owner->totalStr/2)) - target->destructible->totalDR;
				}
				else if(roll <= 1){
					if(engine.map->isVisible(owner->x, owner->y))
						engine.gui->message(TCODColor::lightGrey,"offhand critical miss...");
					damageTaken += 0;
				}
				else if(attackRoll >= target->destructible->totalDodge){
					//engine.gui->message(TCODColor::red,"Offhand Attack");
					damageTaken += damageRoll2 + (int)(owner->totalStr/2) - target->destructible->totalDR;
				}
			}
			else{
				engine.gui->message(TCODColor::lightGrey,"Not enough battery for offhand weapon");
			}
		}//end of HAND2 stuff!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (damageTaken > 0 || (owner->oozing && target->susceptible && damageTaken+1 > 0)) {
			if (owner->oozing && target->susceptible) {
				
				damageTaken = target->destructible->takeDamage(target,owner, damageTaken+1);

				if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
					engine.gui->message(TCODColor::red,"The %s attacks the %s for %g hit points!",owner->name, target->name, damageTaken);
				
			}
			else if(strcmp(target->name,"A Government Issue Locker") == 0)//locker code exception
			{
				if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
					engine.gui->message(TCODColor::lightGrey,"The locker opens with a creak as it spills it's forgotten contents.");
				damageTaken = target->destructible->takeDamage(target,owner,damageTaken);
				
				
			}
			else {
			
				damageTaken = target->destructible->takeDamage(target,owner,damageTaken);
				if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
					engine.gui->message(TCODColor::red,"The %s attacks the %s for %g hit points!",owner->name, target->name, damageTaken);
				if(strcmp(target->name,"player") == 0)
					engine.damageReceived += damageTaken;
			}
		} else {
			if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
				engine.gui->message(TCODColor::lightGrey,"The %s attacks the %s but it has no effect...",owner->name, target->name);
		}
	} else {
		if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
			engine.gui->message(TCODColor::lightGrey,"The %s attacks the %s in vain.", owner->name,target->name);
	}
	lastTarget = target;
}

void Attacker::shoot(Actor *owner, Actor *target) {
	if (target->destructible && !target->destructible->isDead() ) {
		int roll = TCODRandom::getInstance()->getInt(1,20);
		int attackRoll = roll + owner->totalDex;
		float damageTaken = 0;
		float damageRoll = 0;
		float critMult = 2;
		bool enoughPower = false;
		if(owner != engine.player)
			enoughPower = true;
		if(owner->container->ranged){
			int powerUse = ((Weapon*)owner->container->ranged->pickable)->powerUse;
			if(owner != engine.player || owner->attacker->battery >= powerUse){
				enoughPower = true;
				owner->attacker->usePower(owner, powerUse);
				int minDmg = ((Weapon*)owner->container->ranged->pickable)->minDmg;
				int maxDmg = ((Weapon*)owner->container->ranged->pickable)->maxDmg;
				critMult = ((Weapon*)owner->container->ranged->pickable)->critMult;
				damageRoll = (float)TCODRandom::getInstance()->getInt(minDmg,maxDmg);
			}
			else{
				engine.gui->message(TCODColor::lightGrey,"Not enough battery for ranged weapon.");
			}
		}
		if(enoughPower){
			if(roll >= 20){
				if(engine.map->isVisible(target->x, target->y))
					engine.gui->message(TCODColor::red,"CRITICAL HIT!");
				damageTaken += critMult * (damageRoll + owner->totalDex) - target->destructible->totalDR; //save for damage roll
			}
			else if(roll <= 1){
				if(engine.map->isVisible(target->x, target->y))
					engine.gui->message(TCODColor::lightGrey,"critical miss...");
				damageTaken += 0;
			}
			else if(attackRoll >= target->destructible->totalDodge){
				damageTaken += damageRoll + owner->totalDex - target->destructible->totalDR; //save for damage roll
			}
			if (damageTaken > 0 || (owner->oozing && target->susceptible && damageTaken+1 > 0)) {
				if (owner->oozing && target->susceptible) {
					
					damageTaken = target->destructible->takeDamage(target,owner,damageTaken+1);
					
					if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
						engine.gui->message(TCODColor::red,"The %s shoots the %s for %g hit points!",owner->name, target->name, damageTaken);
					
					
				}
				else {
					damageTaken = target->destructible->takeDamage(target,owner,damageTaken);
					if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
						engine.gui->message(TCODColor::red,"The %s shoots the %s for %g hit points!",owner->name, target->name, damageTaken);
					
				}
			} else {
				if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
					engine.gui->message(TCODColor::lightGrey,"The %s shoots the %s but it has no effect...",owner->name, target->name);
			}
		}
	} else {
		if(engine.map->isVisible(owner->x, owner->y) || engine.map->isVisible(target->x, target->y))
			engine.gui->message(TCODColor::lightGrey,"The %s shoots the %s in vain.", owner->name,target->name);
	}
	lastTarget = target;
}

float Attacker::recharge(float amount) {
	battery += amount;
	if (battery > maxBattery) { 
		amount -= battery-maxBattery;
		battery = maxBattery;
	}
	return amount;
}

float Attacker::usePower(Actor *owner, float cost) {
	if (cost > 0){
		battery -= cost;
		if (battery <= 0) {
			battery = 0;
		}
	} else {
		cost = 0;
	}
	return cost;
}