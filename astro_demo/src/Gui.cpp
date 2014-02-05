#include <stdio.h>
#include <stdarg.h>
#include "main.hpp"

static const int PANEL_HEIGHT = 12;
static const int BAR_WIDTH = 20;
static const int MSG_X = BAR_WIDTH + 2;
static const int MSG_HEIGHT = PANEL_HEIGHT-1;

const int PAUSE_MENU_WIDTH = 32;
const int PAUSE_MENU_HEIGHT = 23;

Gui::Gui() {
	con = new TCODConsole(engine.screenWidth, PANEL_HEIGHT);
	sidebar = new TCODConsole(MSG_X, engine.screenHeight);
}

Gui::~Gui() {
	delete con;
	delete sidebar;
	clear();
}

void Gui::clear() {
	log.clearAndDelete();
}

void Gui::save(TCODZip &zip) {
	zip.putInt(log.size());
	for (Message **it = log.begin(); it != log.end(); it++) {
		zip.putString((*it)->text);
		zip.putColor(&(*it)->col);
	}
}

void Gui::load(TCODZip &zip) {
	int nbMessages = zip.getInt();
	while (nbMessages > 0) {
		const char *text = zip.getString();
		TCODColor col = zip.getColor();
		message(col,text);
		nbMessages--;
	}
}

void Gui::render() {
	//clear the GUI consoles
	con->setDefaultBackground(TCODColor::black);
	con->clear();
	sidebar->setDefaultBackground(TCODColor::black);
	sidebar->clear();
	
	
	
	//create the sidebar
	sidebar->setDefaultForeground(TCODColor(200,180,50));
	sidebar->printFrame(0,0,MSG_X,
		engine.screenHeight-12,true,TCOD_BKGND_ALPHA(50),"CHARACTER INFO");
	
	//draw the health bar
	renderBar(1,3,BAR_WIDTH, "HP", engine.player->destructible->hp,
		engine.player->destructible->maxHp, TCODColor::lightRed, 
		TCODColor::darkerRed);
	//draw the mana bar
	renderBar(1,5,BAR_WIDTH, "MANA",0,0,TCODColor::lightBlue, TCODColor::darkerBlue);
	//draw the last target's hp bar
	if (engine.player->attacker->lastTarget != NULL) {		renderBar(1,11,BAR_WIDTH, "target's HP",engine.player->attacker->lastTarget->destructible->hp,
			engine.player->attacker->lastTarget->destructible->maxHp,TCODColor::lightRed, TCODColor::darkerRed);
    }
	
	//dungeon level
	//sidebar->setDefaultForeground(TCODColor::white);
	sidebar->print(3,7,"Dungeon level %d", engine.level);
	//con->print(3,9,"FPS : %d",TCODSystem::getFps());
	sidebar->print(3,13,"Turn count: %d",engine.turnCount);
	
	//display the armor slots
	sidebar->print(9,15,"Armor");
	sidebar->print(3,17,"Head: ");
	sidebar->print(3,19,"Chest: ");
	sidebar->print(3,21,"Legs: ");
	sidebar->print(3,23,"Feet: ");
	sidebar->print(3,25,"Hand1: ");
	sidebar->print(3,27,"Hand2: ");
	sidebar->print(3,29,"Rangd: ");
	
	//display player xp bar
	PlayerAi *ai = (PlayerAi *)engine.player->ai;
	char xpTxt[128];
	sprintf(xpTxt, "XP(%d)", ai->xpLevel);
	renderBar(1,9,BAR_WIDTH,xpTxt,engine.player->destructible->xp,
		ai->getNextLevelXp(),TCODColor::lightViolet, TCODColor::darkerViolet);
		
		
	//display an ability cooldown bar
	sidebar->print(1,32,"Ability Cooldown: ");
	renderBar(1,34, BAR_WIDTH, NULL, 6, 10, TCODColor::orange, TCODColor::darkerOrange);
	

	//mouse look
	//renderMouseLook();
	
	//draw the message log
	int y = 1;
	float colorCoef = 0.4f;
	for (Message **it = log.begin(); it != log.end(); it++) {
		Message *message = *it;
		con->setDefaultForeground(message->col * colorCoef);
		con->print(1,y,message->text);
		y++;
		if (colorCoef < 1.0f) {
			colorCoef +=0.3f;
		}
	}
	
	//blit the GUI consoles (sidebar and message log) 
	TCODConsole::blit(sidebar, 0, 0, MSG_X, engine.screenHeight, 
		TCODConsole::root, 0, 0);
	TCODConsole::blit(con, 0, 0, engine.screenWidth, PANEL_HEIGHT, 
		TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);
}

void Gui::renderBar(int x, int y, int width, const char *name,
	float value, float maxValue, const TCODColor &barColor, 
	const TCODColor &backColor) {
	
	//fill the background
	sidebar->setDefaultBackground(backColor);
	sidebar->rect(x,y,width,1,false,TCOD_BKGND_SET);
	int barWidth = (int)(value / maxValue * width);
	if (barWidth > 0) {
		//draw the bar
		sidebar->setDefaultBackground(barColor);
		sidebar->rect(x,y,barWidth,1,false,TCOD_BKGND_SET);
	}
	
	//print the text on top of the bar
	if (name != NULL) {
		sidebar->setDefaultForeground(TCODColor::white);
		sidebar->printEx(x+width/2,y,TCOD_BKGND_NONE,TCOD_CENTER,
			"%s : %g/%g", name, value, maxValue);
	}
}

Gui::Message::Message(const char *text, const TCODColor &col) : 
	text(strdup(text)), col(col) {
}

Gui::Message::~Message() {
	free(text);
}

//keyboard-based look
void Gui::renderKeyLook() {
	
	int x = engine.player->x;
	int y = engine.player->y;
	if (engine.pickATile(&x,&y)){
		char buf[128] = ""; 
		if (engine.map->isInFov(x,y)){
			strcat(buf,"You see: ");
		}else {
			strcat(buf,"You remember seeing: ");
		}
		bool first = true;
		for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
			Actor *actor = *it;
			//find actors under the mouse cursor
			if (actor->x == x && actor->y == y) {
				if (!first) {
					strcat(buf, ", ");
				} else {
					first = false;
				}
					strcat(buf,actor->name);
				if (actor->attacker && !actor->destructible->isDead() && engine.map->isInFov(x,y)) {
					engine.player->attacker->lastTarget = actor;
				}
			}
		}
		//display the list of actors under the mouse cursor
		Actor *actor = engine.getAnyActor(x, y);
		if (!actor || !engine.map->isExplored(x,y)) {
			memset(&buf[0], 0, sizeof(buf));
			strcat(buf,"There is nothing interesting here.");
		} 
		message(TCODColor::lightGrey,buf);
	}
}

/* remove the below function to remove mouse look
void Gui::renderMouseLook() {
	if (!engine.map->isInFov(engine.mouse.cx, engine.mouse.cy)) {
		return; //mouse is out of fov, nothing to return
	}
	char buf[128] = "";
	
	bool first = true;
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		//find actors under the mouse cursor
		if (actor->x == engine.mouse.cx && actor->y == engine.mouse.cy) {
			if (!first) {
				strcat(buf, ", ");
			} else {
				first = false;
			}
			strcat(buf,actor->name);
		}
	}
	//display the list of actors under the mouse cursor
	con->setDefaultForeground(TCODColor::lightGrey);
	con->print(1,0,buf);
} */
	
void Gui::message(const TCODColor &col, const char *text, ...) {
	//build the text
	va_list ap;
	char buf[128];
	va_start(ap,text);
	vsprintf(buf, text, ap);
	va_end(ap);
	
	char *lineBegin = buf;
	char *lineEnd;
	
	do {
		//make room for the new message
		if (log.size() == MSG_HEIGHT) {
			Message *toRemove = log.get(0);
			log.remove(toRemove);
			delete toRemove;
		}
		//detect the EOL
		lineEnd = strchr(lineBegin,'\n');
		if (lineEnd) {
			*lineEnd = '\0';
		}
		
		//add a new message to the log
		Message *msg = new Message(lineBegin,col);
		log.push(msg);
		
		//go to the next line
		lineBegin = lineEnd+1;
	} while (lineEnd);
}
	
Menu::~Menu() {
	clear();
}

void Menu::clear() {
	items.clearAndDelete();
}

void Menu::addItem(MenuItemCode code, const char *label) {
	MenuItem *item = new MenuItem();
	item->code = code;
	item->label = label;
	items.push(item);
}

Menu::MenuItemCode Menu::pick(DisplayMode mode) {
	int selectedItem = 0;
	int menux, menuy;
	
	if (mode == PAUSE) {
		menux = engine.screenWidth / 2 - PAUSE_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - PAUSE_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(200,180,50));
		TCODConsole::root->printFrame(menux,menuy,PAUSE_MENU_WIDTH,
			PAUSE_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(70),"PAUSE MENU");
		
	/* 	//This code is an alternative way to render a pause menu, with a background
		//image, preferably a 60x60 PNG
		static TCODImage bkimg("background_space.png");	
		TCODConsole *offscreen = new TCODConsole(PAUSE_MENU_WIDTH, PAUSE_MENU_HEIGHT);
		offscreen->setDefaultForeground(TCODColor(200,180,0));
		offscreen->printFrame(0,0,PAUSE_MENU_WIDTH,PAUSE_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(70), "Pause Menu");
		bkimg.blit2x(offscreen,1,0);
		TCODConsole::blit(offscreen, 0, 0, PAUSE_MENU_WIDTH, PAUSE_MENU_HEIGHT, TCODConsole::root,menux,menuy,0.7,0.7);  */
			menux+=2;
			menuy+=3;
	} else {
		static TCODImage img("background.png");
		img.blit2x(TCODConsole::root,0,6);
		menux = 35;
		menuy = 20 + TCODConsole::root->getHeight() / 3;
	}
	
	while (!TCODConsole::isWindowClosed()) {
		
		int currentItem = 0;
		for (MenuItem **it = items.begin(); it != items.end(); it++) {
			if (currentItem == selectedItem) {
				TCODConsole::root->setDefaultForeground(TCODColor::orange);
			} else {
				TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
			}
			TCODConsole::root->print(menux,menuy+currentItem*3,(*it)->label);
			currentItem++;
		}
		TCODConsole::flush();
		
		//check key presses
		TCOD_key_t key;
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL);
		switch(key.vk) {
			case TCODK_UP:
				selectedItem--;
				if(selectedItem < 0) {
					selectedItem = items.size()-1;
				}
			break;
			case TCODK_DOWN:
				selectedItem = (selectedItem +1) % items.size();
			break;
			case TCODK_ENTER: return items.get(selectedItem)->code;
			case TCODK_ESCAPE: return NO_CHOICE;
			default: break;
		}
	}
	return NONE;
}