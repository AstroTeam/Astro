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

bool Container::add(Actor *actor, int type) {
	if (size > 0 && inventory.size() >= size) {
		//inventory full
		return false;
	}
	inventory.push(actor);
	std::map<const char*, float>::iterator ii;
	if(type == 1){
		if(Itemstacks[actor->name]){
			Itemstacks[actor->name] = Itemstacks[actor->name] + 1;
			//ii = Itemstacks.find(actor->name);
			//engine.gui->message(TCODColor::white,"The item is %s and the ammount is %g",(*ii).first,(*ii).second);
		}else{
			Itemstacks[actor->name] = 1;
			//ii = Itemstacks.find(actor->name);
			//engine.gui->message(TCODColor::white,"The item is %s and the ammount is %g",(*ii).first,(*ii).second);
		}
	}else if(type == 2){
		if(Techstacks[actor->name]){
			Techstacks[actor->name] = Techstacks[actor->name] + 1;
			//ii = Techstacks.find(actor->name);
			//engine.gui->message(TCODColor::white,"The item is %s and the ammount is %g",(*ii).first,(*ii).second);
		}else{
			Techstacks[actor->name] = 1;
			//ii = Techstacks.find(actor->name);
			//engine.gui->message(TCODColor::white,"The item is %s and the ammount is %g",(*ii).first,(*ii).second);
		}
	}else if(type == 3){
	}else if(type == 4){
	}
	
	return true;
}

void Container::remove(Actor *actor) {
	inventory.remove(actor);
	if(Itemstacks[actor->name]){
		if(Itemstacks[actor->name] == 1){
			Itemstacks.erase(actor->name);
			Techstacks.erase(actor->name);
			for(std::map<char,const char*>::iterator ii= select.begin(); ii != select.end(); ++ii){
				if(strcmp(actor->name, (*ii).second) == 0)
					select.erase(ii);
			}
		}else{
			Itemstacks[actor->name] = Itemstacks[actor->name] - 1;
			Techstacks.erase(actor->name);
		}
	}else if(Techstacks[actor->name]){
		if(Techstacks[actor->name] == 1){
			Techstacks.erase(actor->name);
			Itemstacks.erase(actor->name);
			for(std::map<char,const char*>::iterator ij= select.begin(); ij != select.end(); ++ij){
				if(strcmp(actor->name, (*ij).second) == 0)
					//engine.gui->message(TCODColor::white,"The item is %s and the ammount is %g",(*ij).first,(*ij).second);
					select.erase(ij);
			}
		}else{
			Techstacks[actor->name] = Techstacks[actor->name] - 1;
			Itemstacks.erase(actor->name);
		}
	}
	
}
