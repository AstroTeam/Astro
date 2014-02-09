#include "main.hpp"
#include "SDL/SDL.h"
#include <math.h>

/* Engine::Engine() : gameStatus(STARTUP), fovRadius(3)
{
	TCODConsole::initRoot(80,50,"Astro", false);
	player = new Actor(40,25,'@', "player",TCODColor::white);
	actors.push(player);
	map = new Map(80,45);

} */

Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP),
	player(NULL),map(NULL), fovRadius(3),
	screenWidth(screenWidth),screenHeight(screenHeight),level(1),turnCount(0) {
	mapWidth = 100;
	mapHeight = 100;
	TCODConsole::initRoot(screenWidth,screenHeight,"Astro", false,TCOD_RENDERER_SDL);
	//TCODSystem::registerSDLRenderer(new Renderer());
	//TCODSystem::registerSDLRenderer(new blengine());
	
	mapcon = new TCODConsole(mapWidth,mapHeight);
	//engine.mapcon->setDefaultBackground(TCODColor::blue);
	mapconCpy = new TCODConsole(mapWidth, mapHeight);
	gui = new Gui();
	mapx1 = 0;
	mapx2 = 0;
	mapy1 = 0;
	mapy2 = 0;
//	rend = new Renderer();
}

Engine::~Engine() {
	term();
	delete gui;
}

void Engine::term() {
	actors.clearAndDelete();
	if (map) delete map;
	gui->clear();
}

void Engine::init() {
	player = new Actor(40,25,'@', "player",TCODColor::white);
	player->destructible = new PlayerDestructible(100, 2, "your cadaver");
	player->attacker = new Attacker(5);
	player->ai = new PlayerAi();
	player->container = new Container(26);
	actors.push(player);
	stairs = new Actor(0,0,'>', "stairs", TCODColor::white);
	stairs->blocks = false;
	actors.push(stairs);
	map = new Map(mapWidth, mapHeight);
	map->init(true);
	gui->message(TCODColor::red, 

    	"Welcome stranger! Prepare to face a horde of Astrocephalytes and Spores Creatures!");
	gameStatus = STARTUP;
}

void Engine::save() {
	if (player->destructible->isDead()) {
		TCODSystem::deleteFile("game.sav");
	} else {
		TCODZip zip;
		zip.putInt(level);
		zip.putInt(turnCount);
		//save the map first
		zip.putInt(map->width);
		zip.putInt(map->height);
		map->save(zip);
		//then the player
		player->save(zip);
		//then the stairs
		stairs->save(zip);
		//then all the other actors
		zip.putInt(actors.size() - 2);
		for (Actor **it = actors.begin(); it!=actors.end(); it++) {
			if (*it != player && *it != stairs) {
				(*it)->save(zip);
			}
		}
		//finally the message log
		gui->save(zip);
		zip.saveToFile("game.sav");
	}
}

void Engine::load(bool pause) {
	engine.gui->menu.clear();
	if (pause) {
	engine.gui->menu.addItem(Menu::NO_CHOICE, "RESUME GAME");
	}
	if (!pause) {
	engine.gui->menu.addItem(Menu::NEW_GAME, "NEW GAME");
	} else {
		engine.gui->menu.addItem(Menu::MAIN_MENU, "MAIN MENU");
	}
	
	if (pause) {
		engine.gui->menu.addItem(Menu::SAVE, "SAVE");
	}
	if(TCODSystem::fileExists("game.sav")) {
		if (pause) {
		engine.gui->menu.addItem(Menu::CONTINUE, "LOAD");
		} else {
		engine.gui->menu.addItem(Menu::CONTINUE, "CONTINUE");
		}
	}
	engine.gui->menu.addItem(Menu::EXIT,"EXIT");
	
	Menu::MenuItemCode menuItem = engine.gui->menu.pick( 
		pause ? Menu::PAUSE : Menu::MAIN);
	
	if (menuItem == Menu::EXIT || menuItem == Menu::NONE) {
		//exit or window closed
		save();
		exit(0);
	} else if (menuItem == Menu::NEW_GAME) {
		//new game 
		engine.term();
		engine.init();
	} else if (menuItem == Menu::SAVE) {
		save();
	} else if (menuItem == Menu::NO_CHOICE) {
	} else if (menuItem == Menu::MAIN_MENU) {
		save();
		TCODConsole::root->clear();
		load(false);
	}else {
		TCODZip zip;
		//continue a saved game
		engine.term();
		zip.loadFromFile("game.sav");
		level = zip.getInt();
		turnCount = zip.getInt();
		//load the map
		int width = zip.getInt();
		int height = zip.getInt();
		map = new Map(width,height);
		map->load(zip);
		//then the player
		player = new Actor(0,0,0,NULL,TCODColor::white);
		player->load(zip);
		//the stairs
		stairs = new Actor(0,0,0,NULL,TCODColor::white);
		stairs->load(zip);
		actors.push(player);
		actors.push(stairs);
		//then all other actors
		int nbActors = zip.getInt();
		while (nbActors > 0) {
			Actor *actor = new Actor(0,0,0,NULL, TCODColor::white);
			actor->load(zip);
			actors.push(actor);
			nbActors--;
		}
		//finally, the message log
		gui->load(zip);
		gui->message(TCODColor::pink,"loaded");
		gameStatus = STARTUP;
	} 
}
	
void Engine::update() {
	if (gameStatus == STARTUP) map->computeFov();
	gameStatus = IDLE;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &lastKey, NULL); //delete the mouse stuff to remove mouse look (change &mouse to NULL)
	if (lastKey.vk == TCODK_ESCAPE) {
		//save();  why save automatically every time escape is called?
		load(true);
	} 
	player->update();
	if (map->isInfected(player->x, player->y)) {
		player->susceptible = true;
	}
	else {
		player->susceptible = false;
	}
	if (gameStatus == NEW_TURN){
		engine.turnCount++;
		for (Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
			Actor *actor = *iterator;
			if ( actor != player) {
				actor->update();
			}
		}
	}
}

float Engine::distance(int x1, int x2, int y1, int y2) {
	int dx = x1 - x2;
	int dy = y1 - y2;
	return sqrt(dx*dx+dy*dy);
}

void Engine::render()
{
	TCODConsole::root->clear();
	mapcon->clear();

	//draw the map
	map->render();
	TCODConsole::blit(mapcon, 0, 0, 0, 0, mapconCpy, 0, 0);
	//^this sets a lot of chars down on mapcon
	//currently renders the floors as ' ' space chars
	
	//render the floors!
	//we need to take the chars off mapcon and render them as the png's from floortile.png
//	rend->renderMap();
	//floors now rendered
	//WE MAY NEED TO SET A LOT OF KEYCOLORS FOR THE BLITS OF ITEMS AND SUCH
	
	//draw the actors
	for (Actor **iterator=actors.begin(); iterator != actors.end(); iterator++) {
		Actor *actor = *iterator;
		if ( actor != player) {
			if (map->isInFov(actor->x,actor->y)) {
				actor->render();
			} else if (map->isExplored(actor->x,actor->y)) {
				TCODColor tmpcol = actor->col;
				actor->col = TCODColor::lightGrey;
				actor->render();
				actor->col = tmpcol;
			}
		}
	}
	//^this blits the actors onto the mapcon as chars
	
	player->render();
	//show the player's stats
	
	gui->render();
	//^blits the sidebars onto the console
	
	
	//int mapx1 = 0, mapy1 = 0, mapy2 = 0, mapx2 = 0;
	
	mapx1 = player->x - ((screenWidth -22)/2);
	mapy1 = player->y - ((screenHeight -12)/2);
	mapx2 = player->x + ((screenWidth -22)/2);
	mapy2 = player->y + ((screenHeight -12)/2);
	
	
	if (mapx1 <= 0) {// <= lets it catch the time when it needs to stop scrolling
		mapx2 += (0-mapx1);
		mapx1 = 0;
		mapx2 += 1; //allows it to render the whole screen
	}	
	if (mapy1 <= 0) { 
		mapy2 += (0-mapy1);
		mapy1 = 0;
		mapy2 += 1;
	}
	if (mapx2 >= 100) {
		mapx1 += (100-mapx2);
		gui->message(TCODColor::green, "******************************************");
		mapx2 = 100;
		mapx1 -= 1;
	}
	if (mapy2 >= 100) {
		mapy1 += (100-mapy2);
		mapy2 = 100;
		mapy1 -= 1;
	}
	 /*
	 //non hard coded values
	 //worked with 100 by 100, don't know if this would work besides that
	if (mapx2 > mapWidth) {
		mapx1 += (mapWidth-mapx2);
		mapx2 = mapWidth;
	}
	if (mapy2 > mapHeight) {
		mapy1 += (mapHeight-mapy2);
		mapy2 = mapHeight;
	}
	*/
	
	
	
	
	
	//stops the map from spilling into the console
	int mapy2a = mapy2;
	if (mapy2a > TCODConsole::root->getHeight() - 12) mapy2a = TCODConsole::root->getHeight() - 12;
	gui->message(TCODColor::red, "y2a is %d",mapy2a);
	//need to make a list of '.' under other chars, that there would be a difference between mapcon and mapconCpy
	//then need to make some sort of flag
	
	
	//blitting of the map onto the screen...maybe blit onto temp root copy, then render and blit back
	TCODConsole::blit(mapcon, mapx1, mapy1, mapx2, mapy2a, 
		TCODConsole::root, 22, 0);
	
	
	//the comment below is the old gui code
	/* TCODConsole::root->print(1, screenHeight-2, "HP: %d/%d", 
	(int)player->destructible->hp,(int)player->destructible->maxHp); */
}

void Engine::nextLevel() {
	level++;
	
	gui->message(TCODColor::lightViolet, "Sitting at the top of the stairs, you take a brief moment to rest...");
	player->destructible->heal(player->destructible->maxHp/2);
	gui->message(TCODColor::red,"Gathering your courage, you rush down the dungeon stairs, mindful that greater dangers may lurk below...");
	
	delete map;
	//delete all actors but player and stairs
	for(Actor **it = actors.begin(); it != actors.end(); it++) {
		if (*it != player && *it != stairs) {
			delete *it;
			it = actors.remove(it);
		}
	}
	//create a new map
	map = new Map(mapWidth,mapHeight);
	map->init(true);
	gameStatus = STARTUP;
	save();
}

void Engine::sendToBack(Actor *actor) {
	actors.remove(actor);
	actors.insertBefore(actor,0);
}

Actor *Engine::getClosestMonster(int x, int y, float range) const {
	Actor *closest = NULL;
	float bestDistance = 1E6f;
	
	for (Actor **iterator = actors.begin(); iterator != actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor != player && actor->destructible 
			&& !actor->destructible->isDead()) {
			float distance = actor->getDistance(x,y);
			if (distance < bestDistance && (distance <= range || range ==0.0f)) {
				bestDistance = distance;
				closest = actor;
			}
		}
	}
	return closest;
}

bool Engine::pickATile(int *x, int *y, float maxRange, float AOE) {   //need to make middle tile unique 
	while (!TCODConsole::isWindowClosed()) {
		int dx = 0, dy = 0;
		render();
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&lastKey,NULL);
		switch (lastKey.vk) {
		case TCODK_UP: dy = -1; break;
		case TCODK_DOWN: dy = 1; break;
		case TCODK_LEFT: dx =-1; break;
		case TCODK_RIGHT: dx = 1; break;
		case TCODK_ENTER: 
		{
			if ((player->getDistance(*x,*y) > maxRange && maxRange != 0)) {
				gui->message(TCODColor::pink,"this tile is out of range!");
			} else {
				return true;
			}
		}
		case TCODK_ESCAPE: return false;
		default: break;
		}
		*x += dx;
		*y += dy;
		
		if (*x > 99) *x = 99;
		if (*x < 0) *x = 0;
		if (*y > 99) *y = 99;
		if (*y < 0) *y = 0;
		
		//things with AOE > 1 need to have a more unique middle color so i can render them
		if (AOE > 1.0 )
		{
			//make center unique
			for (int i = 0; i < map->height; i++) {
				for (int j = 0; j < map->width; j++) {
					if ( distance(*x,j,*y,i) <= AOE ) {
						if (distance(*x,j,*y,i) < 1.0) {//middle
							mapcon->setCharBackground(j,i,TCODColor::darkerPink);
							
						}
						else if ( distance(*x,player->x,*y,player->y) >= maxRange && maxRange != 0) {
							mapcon->setCharBackground(j,i,TCODColor::desaturatedPink);
						
						} else {
							mapcon->setCharBackground(j,i,TCODColor::pink);
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < map->height; i++) {
				for (int j = 0; j < map->width; j++) {
					if ( distance(*x,j,*y,i) <= AOE ) {
						if ( distance(*x,player->x,*y,player->y) >= maxRange && maxRange != 0) {
							mapcon->setCharBackground(j,i,TCODColor::desaturatedPink);
						} else {
							mapcon->setCharBackground(j,i,TCODColor::pink);
						}
					}
				}
			}
		}
	//int mapx1 = 0, mapy1 = 0, mapy2 = 0, mapx2 = 0;
	
	mapx1 = *x - ((screenWidth -22)/2);
	mapy1 = *y - ((screenHeight -12)/2);
	mapx2 = *x + ((screenWidth -22)/2);
	mapy2 = *y + ((screenHeight -12)/2);
	
if (mapx1 < 0) {
		mapx2 += (0-mapx1);
		mapx1 = 0;
	}	
	if (mapy1 < 0) { 
		mapy2 += (0-mapy1);
		mapy1 = 0;
	}
	if (mapx2 > 100) {
		mapx1 += (100-mapx2);
		mapx2 = 100;
	}
	if (mapy2 > 100) {
		mapy1 += (100-mapy2);
		mapy2 = 100;
	}
	//if (mapx2 > TCODConsole::root->getWidth() - 22) mapx2 = TCODConsole::root->getWidth() - 14;
	if (mapy2 > TCODConsole::root->getHeight() - 12) mapy2 = TCODConsole::root->getHeight() - 12;
	 
	TCODConsole::blit(mapcon, mapx1, mapy1, mapx2, mapy2, 
		TCODConsole::root, 22, 0);
		
	TCODConsole::flush();
		
	} 
	return false;
}

Actor *Engine::getActor(int x, int y) const {
	for (Actor **it = actors.begin(); it != actors.end(); it++) {
		Actor *actor = *it;
		if (actor->x ==x && actor->y==y && actor->destructible &&
			!actor->destructible->isDead()) {
			return actor;
		}
	}
	return NULL;
}

Actor *Engine::getAnyActor(int x, int y) const {
	for (Actor **it = actors.begin(); it != actors.end(); it++) {
		Actor *actor = *it;
		if (actor->x ==x && actor->y == y) {
			return actor;
		}
	}
	return NULL;
}

void Engine::win() {
	gui->message(TCODColor::darkRed,"You win!");
	gameStatus=Engine::VICTORY;
}
