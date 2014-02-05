#include <map>

class Container : public Persistent {
public:
	int size;
	TCODList<Actor *> inventory;
	std::map<const char*,float> stacks;
	std::map<char,const char*> select;
	
	Container(int size);
	~Container();
	bool add(Actor *actor);
	void remove(Actor *actor);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};