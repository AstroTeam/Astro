#include <stdio.h>
#include "main.hpp"

Attacker::Attacker(float power) : basePower(power), totalPower(power), lastTarget(NULL) {
}

void Attacker::load(TCODZip &zip) {
	basePower = zip.getFloat();
	totalPower = zip.getFloat();
}

void Attacker::save(TCODZip &zip) {
	zip.putFloat(basePower);
	zip.putFloat(totalPower);
}

void Attacker::attack(Actor *owner, Actor *target) {
	if (target->destructible && !target->destructible->isDead() ) {
		float damageTaken = totalPower - target->destructible->totalDefense;
		if (damageTaken > 0 || (owner->oozing && target->susceptible && damageTaken+1 > 0)) {
			if (owner->oozing && target->susceptible) {
				engine.gui->message(TCODColor::red,"The %s attacks the %s for %g hit points!\n",owner->name, target->name,damageTaken + 1);
				target->destructible->takeDamage(target,damageTaken+1);
			}
			else {
				engine.gui->message(TCODColor::red,"The %s attacks the %s for %g hit points!\n",owner->name, target->name,damageTaken);
				target->destructible->takeDamage(target,damageTaken);
			}
		} else {
			engine.gui->message(TCODColor::lightGrey,"The %s attacks the %s but it has no effect...\n",owner->name, target->name);
		}
	} else {
		engine.gui->message(TCODColor::lightGrey,"The %s attacks the %s in vain.\n", owner->name,target->name);
	}
	lastTarget = target;
}

void Attacker::shoot(Actor *owner, Actor *target) {
	if (target->destructible && !target->destructible->isDead() ) {
		float damageTaken = totalPower - target->destructible->totalDefense;
		if (damageTaken > 0 || (owner->oozing && target->susceptible && damageTaken+1 > 0)) {
			if (owner->oozing && target->susceptible) {
				engine.gui->message(TCODColor::darkerOrange,"The %s shoots the %s for %g hit points!\n",owner->name, target->name,damageTaken + 1);
				target->destructible->takeDamage(target,damageTaken+1);
			}
			else {
				engine.gui->message(TCODColor::darkerOrange,"The %s shoots the %s for %g hit points!\n",owner->name, target->name,damageTaken);
				target->destructible->takeDamage(target,damageTaken);
			}
		} else {
			engine.gui->message(TCODColor::lightGrey,"The %s shoots the %s but it has no effect...\n",owner->name, target->name);
		}
	} else {
		engine.gui->message(TCODColor::lightGrey,"The %s shoots the %s in vain.\n", owner->name,target->name);
	}
	lastTarget = target;
}