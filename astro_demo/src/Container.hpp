#include <map>

class Container : public Persistent {
public:
	int size;
	TCODList<Actor *> inventory;
	std::map <char,const char*> select;
	//the following bools could be actor pointers instead, really
	//working on it
	int wallet;
	
	Actor *head;
	Actor *chest;
	Actor *legs;
	Actor *feet;
	Actor *hand1;
	Actor *hand2;
	Actor *ranged;///////////////////////
	
	Container(int size);
	~Container();
	bool add(Actor *actor);
	void remove(Actor *actor);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	void sendToBegin(Actor *actor);
};