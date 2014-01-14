#include <stdio.h>
#include "main.hpp"

Attacker::Attacker(float power) : power(power), lastTarget(NULL) {
}

void Attacker::load(TCODZip &zip) {
	power = zip.getFloat();
}

void Attacker::save(TCODZip &zip) {
	zip.putFloat(power);
}

void Attacker::attack(Actor *owner, Actor *target) {
	if (target->destructible && !target->destructible->isDead() ) {
		if (power - target->destructible->defense > 0) {
			engine.gui->message(TCODColor::red,"The %s attacks the %s for %g hit points!\n",owner->name, target->name,power - target->destructible->defense);
		} else {
			engine.gui->message(TCODColor::lightGrey,"The %s attacks the %s but it has no effect...\n",owner->name, target->name);
		}
		target->destructible->takeDamage(target,power);
	} else {
		engine.gui->message(TCODColor::lightGrey,"The %s attacks the %s in vain.\n", owner->name,target->name);
	}
	lastTarget = target;
}