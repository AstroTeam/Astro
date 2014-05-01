#include "main.hpp"
#include <string>
#include <iostream>

Container::Container(int size) : size(4*size), sort1_size(size),sort2_size(size),sort3_size(size),sort4_size(size),
	wallet(0),head(NULL),chest(NULL),legs(NULL),feet(NULL),hand1(NULL),hand2(NULL),ranged(NULL){
}

Container::~Container() {
	inventory.clearAndDelete();
}

void Container::load(TCODZip &zip) {
	size = zip.getInt();
	sort1_size = zip.getInt();
	sort2_size = zip.getInt();
	sort3_size = zip.getInt();
	sort4_size = zip.getInt();
	wallet = zip.getInt();
	int nbActors = zip.getInt();
	////std::cout << "got size " << nbActors << std::endl;
	while (nbActors > 0) {
		Actor *actor = new Actor(0,0,0,NULL,TCODColor::white);
		actor->load(zip);
		inventory.push(actor);
		nbActors--;
	}
	bool has_head = zip.getInt();
	bool has_chest = zip.getInt();
	bool has_legs = zip.getInt();
	bool has_feet = zip.getInt();
	bool has_hand1 = zip.getInt();
	bool has_hand2 = zip.getInt();
	bool has_ranged = zip.getInt();
	if(has_head){
		head = new Actor(0,0,0,NULL,TCODColor::white);
		head->load(zip);
	}
	if(has_chest){
		chest = new Actor(0,0,0,NULL,TCODColor::white);
		chest->load(zip);
	}
	if(has_legs){
		legs = new Actor(0,0,0,NULL,TCODColor::white);
		legs->load(zip);
	}
	if(has_feet){
		feet = new Actor(0,0,0,NULL,TCODColor::white);
		feet->load(zip);
	}
	if(has_hand1){
		hand1 = new Actor(0,0,0,NULL,TCODColor::white);
		hand1->load(zip);
	}
	if(has_hand2){
		hand2 = new Actor(0,0,0,NULL,TCODColor::white);
		hand2->load(zip);
	}
	if(has_ranged){
		ranged = new Actor(0,0,0,NULL,TCODColor::white);
		ranged->load(zip);
	}

}

void Container::save(TCODZip &zip) {
	zip.putInt(size);
	zip.putInt(sort1_size);
	zip.putInt(sort2_size);
	zip.putInt(sort3_size);
	zip.putInt(sort4_size);
	zip.putInt(wallet);
	zip.putInt(inventory.size());
	//std::cout << "put size " << inventory.size() << std::endl;
	for (Actor **it = inventory.begin(); it != inventory.end(); it++) {
		(*it)->save(zip);
		//std::cout << "saved " << (*it)->name << std::endl;
	}
	zip.putInt(head != NULL);
	zip.putInt(chest != NULL);
	zip.putInt(legs != NULL);
	zip.putInt(feet != NULL);
	zip.putInt(hand1 != NULL);
	zip.putInt(hand2 != NULL);
	zip.putInt(ranged != NULL);
	if(head != NULL) head->save(zip);
	if(chest != NULL) chest->save(zip);
	if(legs != NULL) legs->save(zip);
	if(feet != NULL) feet->save(zip);
	if(hand1 != NULL) hand1->save(zip);
	if(hand2 != NULL) hand2->save(zip);
	if(ranged != NULL) ranged->save(zip);/////////////////////
}

bool Container::add(Actor *actor) {
	if(actor->pickable->type == Pickable::CURRENCY) {
		wallet += actor->pickable->stackSize;
		return true;
	}
	
	int amount = 0;
	for(Actor **it = inventory.begin(); it != inventory.end(); it++){
		Actor *actor = *it;
		amount += actor->pickable->stackSize;
	}

	bool wasIn = false;
	if (actor->pickable->stacks) {
		for (Actor **it = inventory.begin(); it != inventory.end(); it++) {
			Actor *act2 = *it;
			if(strncmp(act2->name,actor->name,15) == 0) {
				wasIn = true;
				act2->pickable->stackSize += actor->pickable->stackSize;
				delete actor;
			}
		}
	}
	if (wasIn == false) {
		switch(actor->sort){
			case 1:
				if (sort1_size >0){
					sort1_size--;
				}else {
					return false;
				}
				break;
			case 2:
				if (sort2_size >0){
					sort2_size--;
				}else {
					return false;
				}
				break;
			case 3:
				if (sort3_size >0){
					sort3_size--;
				}else {
					return false;
				}
				break;
			case 4:
				if (sort4_size >0){
					sort4_size--;
				}else {
					return false;
				}
				break;
			default: return false; break;
		}
		inventory.push(actor);
	}
	return true;
}

void Container::remove(Actor *actor) {
	inventory.remove(actor);
	switch(actor->sort){
		case 1: sort1_size++; break;
		case 2: sort2_size++; break;
		case 3: sort3_size++; break;
		case 4: sort4_size++; break;
		default: break;
	}
}

void Container::sendToBegin(Actor *actor) {
	inventory.remove(actor);
	inventory.insertBefore(actor,0);
}
