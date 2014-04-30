#include "main.hpp"

Aura::Aura(int duration, StatType stat, LifeStyle life, int bonus) 
	: duration(duration),totalDuration(totalDuration),stat(stat),life(life),bonus(bonus){
}

void Aura::save(TCODZip &zip){
	zip.putInt(duration);
	zip.putInt(stat);
	zip.putInt(life);
	zip.putInt(bonus);
}

void Aura::load(TCODZip &zip){
	duration = zip.getInt();
	stat = (StatType)zip.getInt();
	life = (LifeStyle)zip.getInt();
	bonus = zip.getInt();
}

void Aura::apply(Actor *target){
	switch(stat){
		case ALL: 
			target->totalStr += bonus;
			target->totalDex += bonus;
			target->totalIntel += bonus;
			if (target->destructible){
				target->destructible->totalDR += bonus;
				target->destructible->totalDodge += bonus;
				target->destructible->maxHp += bonus;
				if (target->destructible->maxHp < target->destructible->hp) {
					target->destructible->hp = target->destructible->maxHp;
				}
			}
			break;
		case TOTALSTR: target->totalStr += bonus; break;
		case TOTALDEX: target->totalDex += bonus; break;
		case TOTALINTEL: target->totalIntel += bonus; break;
		case TOTALDODGE: 
			if(target->destructible) {
				target->destructible->totalDodge += bonus;
			}
			break;
		case TOTALDR:
			if(target->destructible) {
				target->destructible->totalDR += bonus;
			}
			break;
		case HEALTH:
			if(target->destructible) {
				target->destructible->heal(bonus);
			}
			break;
		case MAXHEALTH:
			if(target->destructible) {
				target->destructible->maxHp += bonus;
				if (target->destructible->maxHp < target->destructible->hp) {
					target->destructible->hp = target->destructible->maxHp;
				}
			}
			break;
		case FIRE:
			if(target->destructible) {
				target->destructible->heal(bonus);
				if(engine.map->isVisible(target->x, target->y))
					engine.gui->message(TCODColor::orange,"The %s is burning!",target->name);
				//engine.map->tiles[(xM+i)+(yM+j)*engine.map->width].envSta = 1;
			}
			break;
		default: break;
	}
}

void Aura::unApply(Actor *target){

	if (life == CONTINUOUS || life == HUNGER){
		switch(stat){
			case ALL: 
				target->totalStr -= bonus;
				target->totalDex -= bonus;
				target->totalIntel -= bonus;
				if (target->destructible){
					target->destructible->totalDR -= bonus;
					target->destructible->totalDodge -= bonus;
					if (target->destructible->hp > bonus){
						target->destructible->hp -= bonus;
					}
					target->destructible->maxHp -= bonus;
					if (target->destructible->maxHp < target->destructible->hp) {
						target->destructible->hp = target->destructible->maxHp;
					}
				}
				break;
			case TOTALSTR: target->totalStr -= bonus; break;
			case TOTALDEX: target->totalDex -= bonus; break;
			case TOTALINTEL: target->totalIntel -= bonus; break;
			case TOTALDODGE: 
				if(target->destructible) {
					target->destructible->totalDodge -= bonus;
				}
				break;
			case TOTALDR:
				if(target->destructible) {
					target->destructible->totalDR -= bonus;
				}
				break;
			case HEALTH: break; //this is either heal/damage; doesn't go away
			case MAXHEALTH:
				if(target->destructible) {
					target->destructible->maxHp -= bonus;
					if (target->destructible->maxHp < target->destructible->hp) {
						target->destructible->hp = target->destructible->maxHp;
					}
				}
				break;
			default: break;
		}
		
	} else if(life == ITERABLE){
		switch(stat){
			case ALL: 
				target->totalStr -= totalDuration*bonus;
				target->totalDex -= totalDuration*bonus;
				target->totalIntel -= totalDuration*bonus;
				if (target->destructible){
					target->destructible->totalDR -= totalDuration*bonus;
					target->destructible->totalDodge -= totalDuration*bonus;
					if (target->destructible->hp > totalDuration*bonus){
						target->destructible->hp -= totalDuration*bonus;
					}
					target->destructible->maxHp -= totalDuration*bonus;
					if (target->destructible->maxHp < target->destructible->hp) {
						target->destructible->hp = target->destructible->maxHp;
					}
				}
				break;
			case TOTALSTR: target->totalStr -= totalDuration*bonus; break;
			case TOTALDEX: target->totalDex -= totalDuration*bonus; break;
			case TOTALINTEL: target->totalIntel -= totalDuration*bonus; break;
			case TOTALDODGE: 
				if(target->destructible) {
					target->destructible->totalDodge -= totalDuration*bonus;
				}
				break;
			case TOTALDR:
				if(target->destructible) {
					target->destructible->totalDR -= totalDuration*bonus;
				}
				break;
			case HEALTH:
				break; //this is either heal/damage over time; doesn't go away
			case MAXHEALTH:
				if(target->destructible) {
					target->destructible->maxHp -= totalDuration*bonus;
					if (target->destructible->maxHp < target->destructible->hp) {
						target->destructible->hp = target->destructible->maxHp;
					}
				}
				break;
			case FIRE:
				if(target->destructible) {
					//target->destructible->heal(bonus);
					engine.gui->message(TCODColor::yellow,"%s has ceased burning.",target->name);
					//engine.map->tiles[(xM+i)+(yM+j)*engine.map->width].envSta = 1;
				}
				break;
			default: break;
		}
	}

}



