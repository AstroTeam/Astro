class Menu {
public:
	enum MenuItemCode{ 
	NONE,
	NO_CHOICE,
	NEW_GAME,
	CONTINUE,
	SAVE,
	EXIT,
	QUIT,
	MAIN_MENU,
	CONSTITUTION,
	VITALITY,
	STRENGTH,
	AGILITY,
	DEXTERITY,
	INTELLIGENCE,
	ITEMS,
	TECH,
	ARMOR,
	WEAPONS,
	RACE,
	CLASS,
	SUB_CLASS,
	STATS,
	HUMAN,
	ROBOT,
	ALIEN,
	MARINE,
	EXPLORER,
	MERCENARY,
	INFANTRY,
	MEDIC,
	QUARTERMASTER,
	SURVIVALIST,
	PIRATE,
	MERCHANT,
	ASSASSIN,
	BRUTE,
	HACKER,
	RESET,
	DISABLE_TURRETS,
	DISABLE_IFF,
	IDENTIFY_FRIENDLY
	};
	enum DisplayMode {
		MAIN,
		PAUSE,
		INVENTORY,
		VENDING,
		CLASS_MENU,
		CLASS_SELECT,
		TURRET_CONTROL
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
/*class ClassMenu : public Menu{
public:
	enum MenuItemCode{
		NONE,
		NO_CHOICE,
		HUMAN,
		ALIEN,
		MARINE,
		SOLDIER,
		CONSTITUTION,
		STRENGTH
	};
	enum DisplayMode {
		RACE,
		CLASS,
		SUB_CLASS,
		STATS
	};
	~InventoryMenu(){clear();}
	void clear(){Menu::clear();}
	void addItem(MenuItemCode code, const char *label){Menu::addItem(code,label);}
	MenuItemCode pick(DisplayMode mode){Menu::pick(mode)};

};*/

class Gui : public Persistent {
public: 
	Menu menu;
	Menu classMenu;
	// Race\Class Selection Values
	int jobSelection;
	int roleSelection;
	int raceSelection;
	int statPoints;
	int conValue;
	int strValue;
	int agValue;
	int intelValue;
	//Values that save the last selection made in Race/Class Menus
	/*int raceChosen;
	int classChosen;
	int subClassChosen;
	int statChosen;*/

	Gui();
	~Gui();
	void render();
	void message(const TCODColor &col, const char *text, ...);
	void currentTileInfo(int x, int y);
	void renderKeyLook();
	void load(TCODZip &zip);
	void save (TCODZip &zip);
	void clear();
	void classSidebar();
protected:
	TCODConsole *con;
	TCODConsole *sidebar;
	TCODConsole *tileInfoScreen;
	//void renderMouseLook(); remove me to remove mouse look
	struct Message {
		char *text;
		TCODColor col;
		Message(const char *text, const TCODColor &col);
		~Message();
	};
	TCODList<Message *> log;
	TCODList<Message *> tileInfoLog;
	char* wrapText(char* lineBegin, char* lineEnd, int maxLength);
	void tileInfoMessage(const TCODColor &col, const char *text, ...);
	void renderBar(int x, int y, int width, const char *name, 
		float value, float maxValue, const TCODColor &barColor, 
		const TCODColor &backColor);
	
};

