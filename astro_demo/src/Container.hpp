#include <map>

class Container : public Persistent {
public:
	int size;
	TCODList<Actor *> inventory;
	std::map<const char*,float> Itemstacks;
	std::map<char,const char*> select;
	std::map<const char*, float> Techstacks;
	
	Container(int size);
	~Container();
	bool add(Actor *actor, int type);
	void remove(Actor *actor);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};