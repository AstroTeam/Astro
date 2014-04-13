#include <stdio.h>
#include <stdarg.h>
#include "main.hpp"

static const int PANEL_HEIGHT = 12;
static const int BAR_WIDTH = 20;
static const int MSG_X = BAR_WIDTH + 2;
static const int MSG_HEIGHT = PANEL_HEIGHT-1;
const int CON_WIDTH = 60; //sets the message console width. height is PANEL_HEIGHT
const int TILE_INFO_WIDTH = 25; //sets the tile info screen width. height is PANEL_HEIGHT

const int PAUSE_MENU_WIDTH = 32;
const int PAUSE_MENU_HEIGHT = 23;

const int INVENTORY_MENU_WIDTH = 38;
const int INVENTORY_MENU_HEIGHT = 4;

const int CLASS_MENU_WIDTH = 65;
const int CLASS_MENU_HEIGHT = 4;

const int TURRET_CONTROL_WIDTH = 45;
const int TURRET_CONTROL_HEIGHT = 23;

const int KEY_MENU_WIDTH = 45;
const int KEY_MENU_HEIGHT = 23;

const int RACE_MENU_HEIGHT = 52;

const int CLASS_SELECT_WIDTH = 65;
const int CLASS_SELECT_HEIGHT = 40;

Gui::Gui() {
	//added the tile info screen, which takes up a part of the bottom panel next to the console
	con = new TCODConsole(CON_WIDTH, PANEL_HEIGHT);
	sidebar = new TCODConsole(MSG_X, engine.screenHeight);
	tileInfoScreen = new TCODConsole(TILE_INFO_WIDTH, PANEL_HEIGHT);

}

Gui::~Gui() {
	delete con;
	delete sidebar;
	delete tileInfoScreen;
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
	tileInfoScreen->setDefaultBackground(TCODColor::black);
	tileInfoScreen->clear();
	
	
	//create the sidebar
	sidebar->setDefaultForeground(TCODColor(200,180,50));
	sidebar->printFrame(0,0,MSG_X,
		engine.screenHeight-PANEL_HEIGHT,true,TCOD_BKGND_ALPHA(50),"CHARACTER INFO");
	
	//draw the health bar
	renderBar(1,3,BAR_WIDTH, "HP", engine.player->destructible->hp,
		engine.player->destructible->maxHp, TCODColor::lightRed, 
		TCODColor::darkerRed);
	//draw the battery bar
	renderBar(1,5,BAR_WIDTH, "Battery",engine.player->attacker->battery,
		engine.player->attacker->maxBattery,TCODColor::blue, TCODColor::darkerBlue);
	//draw the hunger bar
	if (engine.player->hungerCount == 0) {
		renderBar(1,7,BAR_WIDTH, "Hunger", engine.player->hunger, engine.player->maxHunger, TCODColor::orange, TCODColor::darkerOrange);
	} else if (engine.player->hungerCount < 200) {
		sidebar->setDefaultForeground(TCODColor::red);
		sidebar->print(8,7,"HUNGRY");
	} else {
		sidebar->setDefaultForeground(TCODColor::red);
		sidebar->print(6,7,"VERY HUNGRY");
	}
	
	//draw the last target's hp bar
	if ((engine.player->attacker->lastTarget != NULL)&&(engine.player->attacker->lastTarget!=engine.player)&&(!engine.player->attacker->lastTarget->destructible->isDead())) {		renderBar(1,11,BAR_WIDTH, "target's HP",engine.player->attacker->lastTarget->destructible->hp,
			engine.player->attacker->lastTarget->destructible->maxHp,TCODColor::lightRed, TCODColor::darkerRed);
    }
	
	//dungeon level
	sidebar->setDefaultForeground(TCODColor::white);
	sidebar->print(3,27,"Dungeon level %d", engine.level);
	sidebar->print(3,13,"Turn count: %d",engine.turnCount);
	
	
	//sidebar->print(3,15,"Kill Count: %d",engine.killCount);
	
	//display the armor slots
	sidebar->print(9,15,"Armor");
	if (engine.player->container->head)sidebar->print(2,17,"He",engine.player->container->head);
	if (engine.player->container->chest)sidebar->print(5,17,"C",engine.player->container->chest);
	if (engine.player->container->legs)sidebar->print(7,17,"L",engine.player->container->legs);
	if (engine.player->container->feet)sidebar->print(9,17,"F",engine.player->container->feet);
	if (engine.player->container->hand1)sidebar->print(12,17,"H1",engine.player->container->hand1);
	if (engine.player->container->hand2)sidebar->print(16,17,"H2",engine.player->container->hand2);
	if (engine.player->container->ranged)sidebar->print(19,17,"R",engine.player->container->ranged);
	
	//display player xp bar
	PlayerAi *ai = (PlayerAi *)engine.player->ai;
	char xpTxt[128];
	sprintf(xpTxt, "XP(%d)", ai->xpLevel);
	renderBar(1,9,BAR_WIDTH,xpTxt,engine.player->destructible->xp,
		ai->getNextLevelXp(),TCODColor::lightViolet, TCODColor::darkerViolet);
		
		
	//display an ability cooldown bar
	//sidebar->print(1,19,"Ability Cooldown: ");
	//renderBar(1,21, BAR_WIDTH, NULL, 6, 10, TCODColor::orange, TCODColor::darkerOrange);

	
	//display FPS
	sidebar->print(3,23,"FPS : %d",TCODSystem::getFps());
	
	//display wallet amount
	sidebar->print(3,25,"Wallet : %d",engine.player->container->wallet);
	
	
	//mouse look
	//renderMouseLook();
	
	//draw the message log
	int y = 1;
	float colorCoef = 0.4f;
	for (Message **it = log.begin(); it != log.end(); it++) {
		Message *message = *it;
		con->setDefaultForeground(message->col * colorCoef);
		con->print(1,y,message->text,TCOD_BKGND_ALPHA(100));
		y++;
		if (colorCoef < 1.0f) {
			colorCoef +=0.3f;
		}
	}
	
	//draw the tile info screen
	tileInfoScreen->setDefaultForeground(TCODColor(200,180,50));
	tileInfoScreen->printFrame(0, 0, TILE_INFO_WIDTH, 
		PANEL_HEIGHT, true,TCOD_BKGND_ALPHA(50),"TILE INFO");
	//draw the message
	y = 1;
	if(tileInfoLog.size() > 0){
		for (Message **it = tileInfoLog.begin(); it != tileInfoLog.end(); it++) {
			Message *message = *it;
			tileInfoScreen->setDefaultForeground(message->col);
			tileInfoScreen->print(1,y,message->text,TCOD_BKGND_ALPHA(100));
			y++;
		}
		
	}
	
	
	//blit the GUI consoles (sidebar and message log and tile info screen) 
	TCODConsole::blit(con, 0, 0, CON_WIDTH, PANEL_HEIGHT, TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);
	TCODConsole::blit(tileInfoScreen, 0, 0, TILE_INFO_WIDTH, PANEL_HEIGHT, TCODConsole::root, engine.screenWidth - TILE_INFO_WIDTH, engine.screenHeight - PANEL_HEIGHT);	
	TCODConsole::blit(sidebar, 0, 0, MSG_X, engine.screenHeight-PANEL_HEIGHT, TCODConsole::root, 0, 0);	
		
	currentTileInfo(engine.player->x, engine.player->y);	
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


void Gui::currentTileInfo(int x, int y) {

	while(tileInfoLog.size() > 0) {
		Message *toRemove = tileInfoLog.get(0);
		tileInfoLog.remove(toRemove);
		delete toRemove;
	}
	
	
		
		//int c = engine.map->tiles[x+y*engine.map->width].num;

		float i = engine.map->tiles[x+y*engine.map->width].infection;
		
		int f = engine.map->tiles[x+y*engine.map->width].flower;
		
		int r = engine.map->tileType(x,y);
		
		switch (r)
		{
			case 1://generic
				tileInfoMessage(TCODColor::grey, "A grated floor");
				break;
			case 2://office
				tileInfoMessage(TCODColor::lighterPink, "Boring carpet");
				break;
			case 3://barracks
				tileInfoMessage(TCODColor::grey, "A smooth flooring");
				break;
			case 4://generator
				tileInfoMessage(TCODColor::grey, "A floor with exposed pipes");
				break;
			case 5://kitchen and messhall
			case 7:
				tileInfoMessage(TCODColor::lightOrange, "A tan tiled floor");
				break;
			case 6://server
				tileInfoMessage(TCODColor::darkGrey, "A dark mechanical floor");
				break;
			case 8://armory
				tileInfoMessage(TCODColor::grey, "A large grated floor");
				break;
			case 9://obs
				tileInfoMessage(TCODColor::white, "A glass floor looking into space");
				break;
			case 10://hydroponics
				tileInfoMessage(TCODColor::chartreuse, "Flooring with grass in it");
				break;
			default:break;
		
		}

		if (i < 1){}
		else if (i < 2)
			tileInfoMessage(TCODColor::green, "with some green moss on it");
		else if (i < 3)
			tileInfoMessage(TCODColor::green, "with some odd green moss on its surface");
		else if (i < 4)
			tileInfoMessage(TCODColor::green, "which has weird moss covering it");
		else if (i < 5)
			tileInfoMessage(TCODColor::green, "that has a lot of moss on it");
		else if (i < 6)
			tileInfoMessage(TCODColor::green, "almost covered in odd green moss");
		else
			tileInfoMessage(TCODColor::green, "completely covered in weird moss");
		
		if (f != -1)
			tileInfoMessage(TCODColor::purple, "and odd purple flowers");
		
		

		//tileInfoMessage(TCODColor::yellow, "the light level is %d",c);
		if (engine.map->tiles[x+y*engine.map->width].temperature > 0)
		tileInfoMessage(TCODColor::red, "THE TILE IS ON FIRE!");
		if (engine.map->tiles[x+y*engine.map->width].envSta == 2)
		tileInfoMessage(TCODColor::grey, "The tile has been scorched.");
		
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		//find actors under the mouse cursor
		if (actor->x == x && actor->y == y && actor!= engine.player && actor!= engine.playerLight) {
				tileInfoMessage(actor->col, actor->name);
		}
	}
	
	
	if (!engine.map->isExplored(x,y)) {
		//clear all the messages before this
		while(tileInfoLog.size() > 0){
			Message *toRemove = tileInfoLog.get(0);
			tileInfoLog.remove(toRemove);
			delete toRemove;
		}
		
		tileInfoMessage(TCODColor::lightGrey, "\nThere is nothing interesting here.");
	} 
}


//keyboard-based look
void Gui::renderKeyLook() {
	
	//gets rid of everything previously printed to the tileInfoScreen by emptying the tileInfoLog
	/*while(tileInfoLog.size() > 0) {
		Message *toRemove = tileInfoLog.get(0);
		tileInfoLog.remove(toRemove);
		delete toRemove;
	}*/
	
	//gets the info to send to the tileInfoLog
	int x = engine.player->x;
	int y = engine.player->y;
	while(!engine.pickATileForInfoScreen(&x,&y)){
	
		while(tileInfoLog.size() > 0) {
			Message *toRemove = tileInfoLog.get(0);
			tileInfoLog.remove(toRemove);
			delete toRemove;
		}
		
		
		//char buf[128] = ""; 
		//if (engine.map->isInFov(x,y)){
			
			//int c = engine.map->tiles[x+y*engine.map->width].num;

		float i = engine.map->tiles[x+y*engine.map->width].infection;
		
		int f = engine.map->tiles[x+y*engine.map->width].flower;
		
		int r = engine.map->tileType(x,y);
		
		switch (r)
		{
			case 1://generic
				tileInfoMessage(TCODColor::grey, "A grated floor");
				break;
			case 2://office
				tileInfoMessage(TCODColor::lighterPink, "Boring carpet");
				break;
			case 3://barracks
				tileInfoMessage(TCODColor::grey, "A smooth flooring");
				break;
			case 4://generator
				tileInfoMessage(TCODColor::grey, "A floor with exposed pipes");
				break;
			case 5://kitchen and messhall
			case 7:
				tileInfoMessage(TCODColor::lightOrange, "A tan tiled floor");
				break;
			case 6://server
				tileInfoMessage(TCODColor::darkGrey, "A dark mechanical floor");
				break;
			case 8://armory
				tileInfoMessage(TCODColor::grey, "A large grated floor");
				break;
			case 9://obs
				tileInfoMessage(TCODColor::white, "A glass floor looking into space");
				break;
			case 10://hydroponics
				tileInfoMessage(TCODColor::chartreuse, "Flooring with grass in it");
				break;
			default:break;
		
		}

		if (i < 1){}
		else if (i < 2)
			tileInfoMessage(TCODColor::green, "with some green moss on it");
		else if (i < 3)
			tileInfoMessage(TCODColor::green, "with some odd green moss on its surface");
		else if (i < 4)
			tileInfoMessage(TCODColor::green, "which has weird moss covering it");
		else if (i < 5)
			tileInfoMessage(TCODColor::green, "that has a lot of moss on it");
		else if (i < 6)
			tileInfoMessage(TCODColor::green, "almost covered in odd green moss");
		else
			tileInfoMessage(TCODColor::green, "completely covered in weird moss");
		
		if (f != -1)
			tileInfoMessage(TCODColor::purple, "and odd purple flowers");
		

			//tileInfoMessage(TCODColor::yellow, "the light level is %d",c);
			if (engine.map->tiles[x+y*engine.map->width].temperature > 0)
			tileInfoMessage(TCODColor::red, "THE TILE IS ON FIRE!");
			//tileInfoMessage(TCODColor::yellow, "the environment is %d",e);
			if (engine.map->tiles[x+y*engine.map->width].envSta == 2)
			tileInfoMessage(TCODColor::grey, "The tile has been scorched.");
			
		//}else {
			//tileInfoMessage(TCODColor::lightGrey, "You remember seeing:");
		//}
		for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
			Actor *actor = *it;
			//find actors under the mouse cursor
			if (actor->x == x && actor->y == y) {
					tileInfoMessage(actor->col, actor->name);
				if (actor->attacker && !actor->destructible->isDead() && engine.map->isInFov(x,y)) {
					engine.player->attacker->lastTarget = actor;
				}
			}
		}
		
		//uncomment below(s) and replace below-below if you only want the tileInfoLog to show if there is an actor on it. Changes this since we now print out the infection level
		/*Actor *actor = engine.getAnyActor(x, y);
		if (!actor || !engine.map->isExplored(x,y)) {
		*/
		
		if (!engine.map->isExplored(x,y)) {
			//clear all the messages before this
			while(tileInfoLog.size() > 0){
				Message *toRemove = tileInfoLog.get(0);
				tileInfoLog.remove(toRemove);
				delete toRemove;
			}
			
			tileInfoMessage(TCODColor::lightGrey, "\nThere is nothing interesting here.");
		} 
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

void Gui::tileInfoMessage(const TCODColor &col, const char *text, ...) {
	//build the text

	va_list ap;
	char buf[128];
	va_start(ap,text);
	vsprintf(buf, text, ap);
	va_end(ap);
	
	char *lineBegin = buf;
	char *lineEnd;

	do {
			
			//detect the EOL
			lineEnd = strchr(lineBegin,'\n');
			
			lineEnd = wrapText(lineBegin, lineEnd, TILE_INFO_WIDTH - 2);

			Message *msg = new Message(lineBegin, col);
			tileInfoLog.push(msg);
			
			//go to the next line
			lineBegin = lineEnd + 1;
		} while (lineEnd);

}

char* Gui::wrapText(char* lineBegin, char* lineEnd, int maxLength) {
	
	if(lineEnd){
		if(lineEnd - lineBegin > maxLength){
			char temp = *(lineBegin + maxLength);
			if(temp != ' ') {
				*(lineBegin + maxLength) = '\0';
				lineEnd = strrchr(lineBegin, ' ');
				*(lineBegin + maxLength) = temp;
			}
			else{
				lineEnd = lineBegin + maxLength;
			}
		}
		*lineEnd = '\0';
	}
	else{
		if(strlen(lineBegin) > (unsigned)maxLength){
			char temp = *(lineBegin + maxLength);
			if(temp != ' ') {
				*(lineBegin + maxLength) = '\0';
				lineEnd = strrchr(lineBegin, ' ');
				*(lineBegin + maxLength) = temp;
			}
			else{
				lineEnd = lineBegin + maxLength;
			}
			*lineEnd = '\0';
		}
	}
	return lineEnd;

}

void Gui::message(const TCODColor &col, const char *text, ...) {
	//build the text
	va_list ap;
	char buf[128];
	va_start(ap,text);
	vsprintf(buf, text, ap);
	va_end(ap);
	
	char *lineBegin = buf;
	char *lineEnd;
	
	
	/*//uncomment the lines below if you want a space between each message
	Message *space = new Message("\n",col);
	log.push(space);
	*/
	do {
		//make room for the new message
		if (log.size() == MSG_HEIGHT) {
			Message *toRemove = log.get(0);
			log.remove(toRemove);
			delete toRemove;
		}
		
		//detect the EOL	
		lineEnd = strchr(lineBegin,'\n');
		
		//send in lineBegin, return lineEnd. magic will happen
		lineEnd = wrapText(lineBegin, lineEnd, CON_WIDTH - 2);
		
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
	int menux = 0, menuy = 0;
	int menu2x = 0, menu2y = 0;
	//TCODConsole::root->clear();
	if (mode == LEVELUP) {
		menux = engine.screenWidth / 2 - PAUSE_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - PAUSE_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(200,180,50));
		TCODConsole::root->printFrame(menux,menuy - 4,PAUSE_MENU_WIDTH,
			PAUSE_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(0),"LEVEL UP!!!");
		menux+=2;
		menuy+=3;
	}else if (mode == PAUSE) {
		menux = engine.screenWidth / 2 - PAUSE_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - PAUSE_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(200,180,50));
		TCODConsole::root->printFrame(menux,menuy - 4,PAUSE_MENU_WIDTH,
			PAUSE_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(0),"PAUSE MENU");
		
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
	}else if (mode == INVENTORY) {
		
		menux = engine.screenWidth / 2 - INVENTORY_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - INVENTORY_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(100,180,250));
		TCODConsole::root->printFrame(menux,menuy - 15,INVENTORY_MENU_WIDTH,
			INVENTORY_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(0),"INVENTORY MANAGER Pro");
		//THIS LINE REMOVES THE "INVENTORY" HEADER ->
		//TCODConsole::root->clear();
	}else if(mode == VENDING){
		menux = (engine.screenWidth / 2 - INVENTORY_MENU_WIDTH / 2) + 20;
		menuy = (engine.screenHeight / 2 - INVENTORY_MENU_HEIGHT / 2) - 4;
		TCODConsole::root->setDefaultForeground(TCODColor(100,180,250));
		TCODConsole::root->printFrame(menux,menuy - 15,INVENTORY_MENU_WIDTH,
			INVENTORY_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(0),"3D PRINTER");
	} else if(mode == CLASS_MENU){
		menux = engine.screenWidth / 2 - CLASS_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - CLASS_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(50,180,50));
		TCODConsole::root->printFrame(menux + 10,0,CLASS_MENU_WIDTH,
			CLASS_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(100),"CHARACTER");
	}else if(mode == CLASS_SELECT){
		menu2x = engine.screenWidth/2 - CLASS_SELECT_WIDTH / 2;
		menu2y = engine.screenHeight/2 - CLASS_SELECT_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(200,180,50));
		TCODConsole::root->printFrame(menu2x + 10,menu2y + 2,CLASS_SELECT_WIDTH,
			CLASS_SELECT_HEIGHT,true,TCOD_BKGND_ALPHA(100));
	}else if(mode == TURRET_CONTROL)
	{
		
		menux = engine.screenWidth / 2 - TURRET_CONTROL_WIDTH / 2;
		menuy = engine.screenHeight / 2 - TURRET_CONTROL_HEIGHT / 2;
		char c[] = {'\0'};
		TCODConsole termwindow(TURRET_CONTROL_WIDTH,TURRET_CONTROL_HEIGHT);
			//make me red
		termwindow.setDefaultForeground(TCODColor(67,199,50));
		termwindow.setDefaultBackground(TCODColor(0,0,0));
		termwindow.printFrame(0,0,TURRET_CONTROL_WIDTH,TURRET_CONTROL_HEIGHT,true,TCOD_BKGND_ALPHA(50),"TURRET CONTROL");
		termwindow.printRect(1,1,TURRET_CONTROL_WIDTH-2,TURRET_CONTROL_HEIGHT,c);
		TCODConsole::blit(&termwindow,0,0,TURRET_CONTROL_WIDTH,TURRET_CONTROL_HEIGHT,TCODConsole::root,menux+4,menuy-4);
		TCODConsole::flush();
		
		 
	}
	else if(mode == KEY_MENU)
	{
		menux = engine.screenWidth / 2 - KEY_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - KEY_MENU_HEIGHT / 2;
		char c[] = {'\0'};
		TCODConsole termwindow(KEY_MENU_WIDTH,KEY_MENU_HEIGHT);
			//make me red
		termwindow.setDefaultForeground(TCODColor(67,199,50));
		termwindow.setDefaultBackground(TCODColor(0,0,0));
		termwindow.printFrame(0,0,KEY_MENU_WIDTH,KEY_MENU_HEIGHT,true,TCOD_BKGND_ALPHA(50),"Weapon Vault");
		termwindow.printRect(1,1,KEY_MENU_WIDTH-2,KEY_MENU_HEIGHT,c);
		TCODConsole::blit(&termwindow,0,0,KEY_MENU_WIDTH,KEY_MENU_HEIGHT,TCODConsole::root,menux+4,menuy-4);
		TCODConsole::flush();
	}
	else{
		static TCODImage img("wesleyPIXEL.png");
		img.blit2x(TCODConsole::root,0,6);
		menux = 35;
		menuy = 24 + TCODConsole::root->getHeight() / 3;
		
	}
	
	if (mode == INVENTORY || mode == VENDING){
		//clears console when inventory is open and when you select a new dingus, mey need to adjust later if we want to move up/down
		//
		//TCODConsole::flush();
		while (!TCODConsole::isWindowClosed()) {
			//TCODConsole::root->clear();
			int currentItem = 0;
			for (MenuItem **it = items.begin(); it != items.end(); it++) {
				if (currentItem == selectedItem) {
					TCODConsole::root->setDefaultForeground(TCODColor::orange);
				} else {
					TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
				}
				//TCODConsole::flush();
				TCODConsole::root->print(menux+currentItem*8+1,menuy-13,(*it)->label);
				engine.selX = menux+selectedItem*8+1;
				engine.selY = menuy-13;
				currentItem++;
			}
			//TCODConsole::root->clear();
			TCODConsole::flush();
			
			//check key presses
			
			
				TCOD_key_t key;
				TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL);
				switch(key.vk) {
					case TCODK_LEFT:
						//TCODConsole::root->clear();
						//TCODConsole::flush();
						selectedItem--;
						if(selectedItem < 0) {
							selectedItem = items.size()-1;
						}
						//TCODConsole::root->clear();
					break;
					case TCODK_RIGHT:
						//TCODConsole::root->clear();
						//TCODConsole::flush();
						selectedItem = (selectedItem +1) % items.size();
						//TCODConsole::root->clear();
					break;
					case TCODK_ENTER: return items.get(selectedItem)->code;
					case TCODK_ESCAPE: return NO_CHOICE;
					default: break;
				}
			
		}
	}else if(mode == CLASS_MENU){
		while (!TCODConsole::isWindowClosed()) {
		
			int currentItem = 0;
			for (MenuItem **it = items.begin(); it != items.end(); it++) {
				if (currentItem == selectedItem) {
					TCODConsole::root->setDefaultForeground(TCODColor::orange);
				} else {
					TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
				}
				if(currentItem == 2){
					TCODConsole::root->print(menux+currentItem*12+14,menuy-18,(*it)->label);
				}else{
					TCODConsole::root->print(menux+currentItem*12+16,menuy-18,(*it)->label);
				}
				currentItem++;
			}
			TCODConsole::flush();
			
			//check key presses
			
			
				TCOD_key_t key;
				TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL);
				switch(key.vk) {
					case TCODK_LEFT:
						selectedItem--;
						if(selectedItem < 0) {
							selectedItem = items.size()-1;
						}
					break;
					case TCODK_RIGHT:
						selectedItem = (selectedItem +1) % items.size();
					break;
					case TCODK_ENTER: return items.get(selectedItem)->code;
					case TCODK_ESCAPE: if (mode == PAUSE) {
							 	return NO_CHOICE;
							   }
					default: break;
				}
			
		}
	}else if(mode == CLASS_SELECT){
		menu2x = engine.screenWidth - CLASS_SELECT_WIDTH / 2;
		while (!TCODConsole::isWindowClosed()) {
		
			int currentItem = 0;
			for (MenuItem **it = items.begin(); it != items.end(); it++) {
				if (currentItem == selectedItem) {
					TCODConsole::root->setDefaultForeground(TCODColor::orange);
				} else {
					TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
				}
				if(strcmp((*it)->label,"CONSTITUTION") == 0){
					TCODConsole::root->print(menu2x - 7,12+currentItem*3-4,(*it)->label);
				}else{
					TCODConsole::root->print(menu2x - 6,12+currentItem*3-4,(*it)->label);
				}
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
				case TCODK_ESCAPE: if (mode == PAUSE){
							return NO_CHOICE;
						    }
				default: break;
			}
		}
	}
	else if(mode == TURRET_CONTROL)
	{
		while (!TCODConsole::isWindowClosed()) {
		
			int currentItem = 0;
			for (MenuItem **it = items.begin(); it != items.end(); it++) {
				if (currentItem == selectedItem) {
					TCODConsole::root->setDefaultForeground(TCODColor::orange);
				} else {
					TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
				}
				if(currentItem == 3)
					TCODConsole::root->print(menux+5+1,menuy+4+1+currentItem*3-4,(*it)->label);
				else
					TCODConsole::root->print(menux+5+1,menuy+4+currentItem*3-4,(*it)->label);
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
				case TCODK_ESCAPE: if (mode == PAUSE){
							return NO_CHOICE;
						    }
				default: break;
			}
		}
	}
	else if (mode == KEY_MENU)
	{
			while (!TCODConsole::isWindowClosed()) {
		
			int currentItem = 0;
			for (MenuItem **it = items.begin(); it != items.end(); it++) {
				if (currentItem == selectedItem) {
					TCODConsole::root->setDefaultForeground(TCODColor::orange);
				} else {
					TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
				}
				TCODConsole::root->print(menux+5+1,menuy+4+currentItem*3-4,(*it)->label);
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
				case TCODK_ESCAPE: if (mode == PAUSE){
							return NO_CHOICE;
						    }
				default: break;
			}
		}
	}
	
	{
		while (!TCODConsole::isWindowClosed()) {
		
			int currentItem = 0;
			for (MenuItem **it = items.begin(); it != items.end(); it++) {
				if (currentItem == selectedItem) {
					TCODConsole::root->setDefaultForeground(TCODColor::orange);
				} else {
					TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
				}
				TCODConsole::root->print(menux,menuy+currentItem*3-4,(*it)->label);
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
				case TCODK_ESCAPE: if (mode == PAUSE){
							return NO_CHOICE;
						    }
				default: break;
			}
		}
	}
	
	
	
	
	{
		while (!TCODConsole::isWindowClosed()) {
		
			int currentItem = 0;
			for (MenuItem **it = items.begin(); it != items.end(); it++) {
				if (currentItem == selectedItem) {
					TCODConsole::root->setDefaultForeground(TCODColor::orange);
				} else {
					TCODConsole::root->setDefaultForeground(TCODColor::lightBlue);
				}
				TCODConsole::root->print(menux,menuy+currentItem*3-4,(*it)->label);
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
				case TCODK_ESCAPE: if (mode == PAUSE){
							return NO_CHOICE;
						    }
				default: break;
			}
		}
	}
	return NONE;
}
void Gui::classSidebar(){
			//Create Character Race/Class information sidebar
			TCODConsole classBar(20,engine.screenHeight);
			classBar.setDefaultBackground(TCODColor::black);
			classBar.clear();
			classBar.setDefaultForeground(TCODColor(200,180,50));
			classBar.printFrame(0,0,20,
			engine.screenHeight,true,TCOD_BKGND_ALPHA(50),"CHARACTER INFO");
			//renderBar(1,3,20, "HP", engine.player->destructible->hp,
			//engine.player->destructible->maxHp, TCODColor::lightRed, 
			//TCODColor::darkerRed);
			
			//Display Race/Class Info
			//classBar.setDefaultForeground(TCODColor::white);
			classBar.print(1,5,"RACE: ");
			switch(raceSelection){
				case 2:
					classBar.print(7,5,"ROBOT");
				break;
				case 3:
					classBar.print(7,5,"ALIEN");
				break;
				default:
					classBar.print(7,5,"HUMAN");
				break;
			}
			classBar.print(1,7,"CLASS: ");
			switch(roleSelection){
				case 2:
					classBar.print(8,7,"EXPLORER");
				break;
				case 3:
					classBar.print(8,7,"MERCENARY");
				break;
				default:
					classBar.print(8,7,"MARINE");
				break;
			}
			classBar.print(6,9,"SUBCLASS");
			switch(jobSelection){
				case 2:
					classBar.print(6,11,"MEDIC");
				break;
				case 3:
					classBar.print(3,11,"QUARTERMASTER");
				break;
				case 4:
					classBar.print(3,11,"SURVIVALIST");
				break;
				case 5:
					classBar.print(6,11,"PIRATE");
				break;
				case 6:
					classBar.print(6,11,"MERCHANT");
				break;
				case 7:
					classBar.print(6,11,"ASSASSIN");
				break;
				case 8:
					classBar.print(6,11,"BRUTE");
				break;
				case 9:
					classBar.print(6,11,"HACKER");
				break;
				default:
					classBar.print(6,11,"INFANTRY");
				break;
			}
			classBar.print(7,15,"STATS");
			classBar.print(1,17,"AVAIL. POINTS: %d",statPoints);
			classBar.print(1,19,"VITALITY: %d",vitValue);
			classBar.print(1,21,"STRENGTH: %d",strValue);
			classBar.print(1,23,"DEXTERITY: %d",dexValue);
			classBar.print(1,25,"INTELLIGENCE: %d",intelValue);
			
			TCODConsole::blit(&classBar, 0, 0, 20, engine.screenHeight, TCODConsole::root, 0, 0);
}
