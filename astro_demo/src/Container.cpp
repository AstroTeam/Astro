#include "main.hpp"

Container::Container(int size) : size(size) {
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
}

void Container::save(TCODZip &zip) {
	zip.putInt(size);
	zip.putInt(inventory.size());
	for (Actor **it = inventory.begin(); it != inventory.end(); it++) {
		(*it)->save(zip);
	}
}

bool Container::add(Actor *actor) {
	if (size > 0 && inventory.size() >= size) {
		//inventory full
		return false;
	}
	inventory.push(actor);
	std::map<const char*, float>::iterator ii;
	if(stacks[actor->name]){
		stacks[actor->name] = stacks[actor->name] + 1;
		//ii = stacks.find(actor->name);
		//engine.gui->message(TCODColor::white,"The item is %s and the ammount is %g",(*ii).first,(*ii).second);
	}else{
		stacks[actor->name] = 1;
		//ii = stacks.find(actor->name);
		//engine.gui->message(TCODColor::white,"The item is %s and the ammount is %g",(*ii).first,(*ii).second);
	}
	return true;
}

void Container::remove(Actor *actor) {
	inventory.remove(actor);
	if(stacks[actor->name] == 1){
		stacks.erase(actor->name);
		for(std::map<char,const char*>::iterator ii = select.begin(); ii != select.end(); ++ii){\
			if(strcmp(actor->name, (*ii).second) == 0)
				select.erase(ii);
		}
	}else
		stacks[actor->name] = stacks[actor->name] - 1;
}