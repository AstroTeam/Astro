#include <map>

class Container : public Persistent {
public:
	int size;
	TCODList<Actor *> inventory;
	//the following bools could be actor pointers instead, really
	
	bool head;
	bool chest;
	bool legs;
	bool feet;
	bool hand1;
	bool hand2;
	bool ranged;
	
	Container(int size);
	~Container();
	bool add(Actor *actor);
	void remove(Actor *actor);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	void sendToBegin(Actor *actor);
};