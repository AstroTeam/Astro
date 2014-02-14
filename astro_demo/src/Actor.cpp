#include <stdio.h>
#include <math.h>
#include "main.hpp"

Actor::Actor(int x, int y, int ch, const char *name, const TCODColor &col):
	x(x),y(y),ch(ch),col(col),name(name),  blocks(true), oozing(false), susceptible(false), sort(0), attacker(NULL),destructible(NULL),ai(NULL),
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
	zip.putColor(&col);
	zip.putString(name);
	zip.putInt(blocks);
	zip.putInt(oozing);
	zip.putInt(susceptible);
	zip.putInt(sort);
	zip.putInt(race);
	zip.putInt(role);
	zip.putInt(job);
	zip.putInt(attacker != NULL);
	zip.putInt(destructible != NULL);
	zip.putInt(ai != NULL);
	zip.putInt(pickable != NULL);
	zip.putInt(container != NULL);
	
	if (attacker) attacker->save(zip);
	if (destructible) destructible->save(zip);
	if (ai) ai->save(zip);
	if (pickable) pickable->save(zip);
	if (container) container->save(zip);
}

void Actor::load(TCODZip &zip) {
	x = zip.getInt();
	y = zip.getInt();
	ch = zip.getInt();
	col= zip.getColor();
	name = strdup(zip.getString());
	blocks = zip.getInt();
	oozing = zip.getInt();
	susceptible = zip.getInt();
	sort = zip.getInt();
	race = zip.getInt();
	role = zip.getInt();
	job = zip.getInt();
	bool hasAttacker = zip.getInt();
	bool hasDestructible = zip.getInt();
	bool hasAi = zip.getInt();
	bool hasPickable = zip.getInt();
	bool hasContainer = zip.getInt();
	
	if (hasAttacker) {
		attacker = new Attacker(0.0f);
		attacker->load(zip);
	}
	if (hasDestructible) {
		destructible = Destructible::create(zip);
	}
	if (hasAi) {
		ai = Ai::create(zip);
	}
	if (hasPickable) {
		pickable = Pickable::create(zip);
	}
	if (hasContainer) {
		container = new Container(0);
		container->load(zip);
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
