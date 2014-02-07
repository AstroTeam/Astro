#include "main.hpp"
//#include "engine.cpp"
#include "SDL/SDL.h"
//#include "SDL/SDL_image.h"
#include <string>




void Renderer::render(void *sdlSurface){
	SDL_Surface *screen=(SDL_Surface *)sdlSurface;
	SDL_Surface *floor = SDL_LoadBMP("floorTile.bmp");
	SDL_Surface *darkFloor = SDL_LoadBMP("floorTileDark.bmp");
	SDL_Surface *map = SDL_LoadBMP("starmap.bmp");
	static bool first=true;
	if ( first ) {
			first=false;
		// set blue(255) as transparent for the root console
		// so that we only blit characters
		SDL_SetColorKey(screen,SDL_SRCCOLORKEY,255);
			
	}
	
	//engine.mapconCpy is the map copy to be rendered with SDL as a background
	//scan mapconCpy and blit on good tile images
	int plyx = 0, plyy = 0;
	for (int x = 0; x < engine.mapconCpy->getWidth(); x++) {
		for (int y = 0; y < engine.mapconCpy->getHeight(); y++) {
			//if it needs to be rendered over, render it over
			SDL_Rect dstRect={x*16,y*16,16,16};
			if(engine.mapconCpy->getChar(x,y) == 31) 
			{ //replace 'down arrow thing' (31 ascii) with basic floor tiles
				SDL_BlitSurface(floor,NULL,map,&dstRect);				
				if (engine.mapcon->getChar(x,y) == 64)  // or if looking or item using
				{
					plyx = x;
					plyy = y;
				}
				
				
			}
			if(engine.mapconCpy->getChar(x,y) == 30) //replace 'up arrow thing' with darker floor tiles
			{
				SDL_BlitSurface(darkFloor,NULL,map,&dstRect);
			}
		}
	}
	bool AOE = false;
	for (int x = 0; x < engine.mapconCpy->getWidth(); x++) {
		for (int y = 0; y < engine.mapconCpy->getHeight(); y++) {
		if (!AOE && (engine.mapcon->getCharBackground(x,y) == TCODColor::pink || engine.mapcon->getCharBackground(x,y) == TCODColor::desaturatedPink))
				//exception for looking and for targetting
		{
			plyx = x;
			plyy = y;
		}
		if (engine.mapcon->getCharBackground(x,y) == TCODColor::darkerPink)
		{
			plyx = x;
			plyy = y;
			AOE = true;
		}		
		}
		
	}
	int mapx1 = 0, mapy1 = 0, mapy2 = 0, mapx2 = 0;
	
	mapx1 = plyx - ((85 -22)/2);
	mapy1 = plyy - ((47 -14)/2);
	mapx2 = plyx + ((85 -22)/2);
	mapy2 = plyy + ((47 -14)/2);
	
	if (mapx1 < 0) {
		mapx2 += (0-mapx1);
		mapx1 = 0;
	}	
	if (mapy1 < 0) { 
		mapy2 += (0-mapy1);
		mapy1 = 0;
	}
	if (mapx2 > 99) {
		mapx1 += (99-mapx2);
		mapx2 = 99;
	}
	if (mapy2 > 99) {
		mapy1 += (99-mapy2);
		mapy2 = 99;
	}

	SDL_Rect srcRect1={mapx1*16,mapy1*16,(mapx2-mapx1+16)*16,(mapy2-mapy1+16)*16};
	SDL_Rect dstRect1={22*16,0,(mapx2-mapx1+16)*16,(mapy2-mapy1+16)*16};

	SDL_BlitSurface(screen,&dstRect1,map,&srcRect1);
	
	
	//if the game is running add if statement sometime							//////redraw during item use!  because player hasn't moved!
	SDL_BlitSurface(map,&srcRect1,screen,&dstRect1);							///////////////bottom of screen = blue by one row?  not big enough screen?
	SDL_FreeSurface(map);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	//static const TCODColor darkWall(0,0,100);
		
	//we need to scan through the mapcon and replace all the ' ' characters with our floortile.png
	//SDL_Surface *map = (SDL_Surface*)engine.mapcon;
//	TCODConsole *tile = new TCODConsole(16,16);
	//SDL_Surface *tile = SDL_LoadBMP("floorTile.bmp");
	//tile->setDefaultForeground(TCODColor::white);
	/*TCODConsole *offscreenConsole = new TCODConsole(1,1);
	offscreenConsole->setDefaultBackground(TCODColor::red);
	
	offscreenConsole->clear();
	//offscreenConsole->setChar(0, 0, '#');
	//SDL_Surface *tiel = (SDL_Surface*)offscreenConsole;
	//tiel = SDL_LoadBMP("floorTile.bmp");
	SDL_Surface *tiel = SDL_LoadBMP("floorTile.bmp");
	SDL_Flip(tiel);
	//float ascii;
	//ascii = 32;
	//SDL_Surface *tilez = (SDL_Surface*)tile;
	//i might have mapHeight and mapWidth backwards, may cause problems if the map is rectangular
	for (int x = 0; x < TCODConsole::root->getWidth(); x++) {
		for (int y = 0; y < TCODConsole::root->getHeight(); y++) {
				//char *c = engine.mapcon->getChar(x,y);
				//engine.gui->message(TCODColor::white, c, ...);
				//engine.gui->message(TCODColor::red,"ascii code %d",engine.mapcon->getChar(engine.player->x,engine.player->y));
//				engine.gui->message(TCODColor::red, "ascii is  %d",TCODConsole::root->getChar(50,20));
				//TCODConsole::root->setCharBackground(50,20,TCODColor::desaturatedPink);
				//TCODConsole::root->setCharForeground(50,20,TCODColor::desaturatedPink);

				//static int i = engine.mapcon->getChar(engine.player->x,engine.player->y);
				if (engine.mapcon->getCharBackground(x,y) != TCODColor::grey){TCODConsole::root->getChar(x,y) == 46) {//if ascii code of cell == space "32" ascii code
				//TCODConsole::root->setCharForeground(x,y,TCODColor::desaturatedPink);
				
				//if (x%2 == 1){
				//	SDL_Surface *map = (SDL_Surface*)engine.mapcon;
						//map = SDL_SetVideoMode(engine.mapWidth, engine.mapHeight, 32, SDL_SWSURFACE);
						//SDL_SetColorKey(map,SDL_SRCCOLORKEY,0);
					//SDL_Rect srcRect={0,0,16,16};
					SDL_Rect dstRect={x*16,y*16,16,16};
				//	SDL_BlitSurface(tilez,NULL,map,&dstRect);
					//TCODConsole::blit(offscreenConsole, 0, 0, 16, 16, engine.mapcon, x, y);
					SDL_BlitSurface(tiel,NULL,screen,&dstRect);
					//SDL_BlitSurface();
					//SDL_Flip(map);
				//	SDL_UpdateRect(map, 0, 0, 0, 0);
					//engine.mapcon->setDirty( 0, 0, 100, 100);
					//TCODConsole::flush();
				}
			}
	}
		
	SDL_FreeSurface(tiel);*/
}
	
	
