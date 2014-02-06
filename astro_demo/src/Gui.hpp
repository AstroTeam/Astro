class Menu {
public:
	enum MenuItemCode{ 
	NONE,
	NO_CHOICE,
	NEW_GAME,
	CONTINUE,
	SAVE,
	EXIT,
	MAIN_MENU,
	CONSTITUTION,
	STRENGTH,
	AGILITY,
	ITEMS,
	TECH,
	ARMOR,
	WEAPONS
	};
	enum DisplayMode {
		MAIN,
		PAUSE,
		INVENTORY
	};
	~Menu();
	void clear();
	void addItem(MenuItemCode code, const char *label);
	MenuItemCode pick(DisplayMode mode = MAIN);
protected: 
	struct MenuItem {
		MenuItemCode code;
		const char *label;
	};
	TCODList<MenuItem *> items;
};
/*class InventoryMenu : public Menu{
public:
	enum MenuItemCode{
		NONE,
		NO_CHOICE,
		ITEMS,
		TECH,
		WEAPONS,
		ARMOR
	};
	~InventoryMenu(){clear();}
	void clear(){Menu::clear();}
	void addItem(MenuItemCode code, const char *label){Menu::addItem(code,label);}
	MenuItemCode pick();

};*/

class Gui : public Persistent {
public: 
	Menu menu;

	Gui();
	~Gui();
	void render();
	void message(const TCODColor &col, const char *text, ...);
	void renderKeyLook();
	void load(TCODZip &zip);
	void save (TCODZip &zip);
	void clear();
	
protected:
	TCODConsole *con;
	TCODConsole *sidebar;
	//void renderMouseLook(); remove me to remove mouse look
	struct Message {
		char *text;
		TCODColor col;
		Message(const char *text, const TCODColor &col);
		~Message();
	};
	TCODList<Message *> log;
	void renderBar(int x, int y, int width, const char *name, 
		float value, float maxValue, const TCODColor &barColor, 
		const TCODColor &backColor);
	
};

