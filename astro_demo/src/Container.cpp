#include "main.hpp"
#include <string>

Container::Container(int size) : size(size),head(false),chest(false),
	legs(false),feet(false),hand1(false),hand2(false),ranged(false){
}

Container::~Container() {
	inventory.clearAndDelete();
}

void Container::load(TCODZip &zip) {
	size = zip.getInt();
	int nbActors = zip.getInt();
	while (nbActors > 0) {
		Actor *actor = new Actor(0,0,0,NULL,TCODColor::white);
		actor->load(zip);
		inventory.push(actor);
		nbActors--;
	}
	head = zip.getInt();
	chest = zip.getInt();
	legs = zip.getInt();
	feet = zip.getInt();
	hand1 = zip.getInt();
	hand2 = zip.getInt();
	ranged = zip.getInt();

}

void Container::save(TCODZip &zip) {
	zip.putInt(size);
	zip.putInt(inventory.size());
	for (Actor **it = inventory.begin(); it != inventory.end(); it++) {
		(*it)->save(zip);
	}
	zip.putInt(head);
	zip.putInt(chest);
	zip.putInt(legs);
	zip.putInt(feet);
	zip.putInt(hand1);
	zip.putInt(hand2);
	zip.putInt(ranged);
}

bool Container::add(Actor *actor) {
	if (size > 0 && inventory.size() >= size) {
		//inventory full
		return false;
	}
	bool wasIn = false;
	if (actor->pickable->stacks) {
		for (Actor **it = inventory.begin(); it != inventory.end(); it++) {
			Actor *act2 = *it;
			if(strcmp(act2->name,actor->name) == 0) {
				wasIn = true;
				act2->pickable->stackSize += actor->pickable->stackSize;
				delete actor;
			}
		}
	}
	if (wasIn == false) {
	inventory.push(actor);
	}
	return true;
}

void Container::remove(Actor *actor) {
	inventory.remove(actor);
}

void Container::sendToBegin(Actor *actor) {
	inventory.remove(actor);
	inventory.insertBefore(actor,0);
}
