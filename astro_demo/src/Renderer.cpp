#include "main.hpp"
//#include "engine.cpp"
#include "SDL/SDL.h"
//#include "SDL/SDL_image.h"
#include <string>




void Renderer::render(void *sdlSurface){
/////////////////////////////////////////////////////////////////////rendering doubled up walkable decor!
	SDL_Surface *screen=(SDL_Surface *)sdlSurface;
	//floors
	SDL_Surface *floorTiles = SDL_LoadBMP("tile_assets/tiles.bmp");
	static int alpha = 255*.5;
	static bool Down = true;
	if (alpha > 0 && Down)
	{
		alpha = alpha-5;
		if (alpha < 0)
			alpha = 0;
	}
	else if (alpha == 0 && Down)
	{
		Down = false;
	}
	else if (alpha < 255*.5 )
	{
		alpha = alpha+5;
	}
	else if (alpha > 255*.5)
	{
		Down = true;
	}
	//glows
	SDL_Surface *itemsGlow = SDL_LoadBMP("tile_assets/alphaGlow.bmp");
	SDL_SetAlpha( itemsGlow, SDL_SRCALPHA, alpha);
	SDL_SetColorKey(itemsGlow,SDL_SRCCOLORKEY,255);
	//shadows
	SDL_Surface *shadows = SDL_LoadBMP("tile_assets/itemShadows.bmp");
	SDL_SetAlpha(shadows, SDL_SRCALPHA, 255*.25);
	SDL_SetColorKey(shadows,SDL_SRCCOLORKEY,255);
	

	//EQUIPMENT
	SDL_Surface *equipment = SDL_LoadBMP("tile_assets/equipment.bmp");
	SDL_SetColorKey(equipment,SDL_SRCCOLORKEY,255);
	
	//DECORATIONS
	SDL_Surface *decor = SDL_LoadBMP("tile_assets/decorations.bmp");
	SDL_SetColorKey(decor,SDL_SRCCOLORKEY,255);
	
	
	//SDL_SetColorKey(humanShadow,SDL_SRCCOLORKEY,255);
	//background
	//SDL_Surface *map = SDL_LoadBMP("starmap.bmp");
	
	//SDL_Surface *floorMap = SDL_LoadBMP("starmap2_blank.bmp");
	SDL_Surface *floorMap = SDL_LoadBMP("starmap2.bmp");
	SDL_Surface *terminal = SDL_LoadBMP("tile_assets/terminal.bmp");
	SDL_SetColorKey(terminal,SDL_SRCCOLORKEY,255);

	
	
	static bool first=true;
	if ( first ) {
			first=false;
		// set blue(255) as transparent for the root console
		// so that we only blit characters
		SDL_SetColorKey(screen,SDL_SRCCOLORKEY,255);
			
	}
	
	
	///////////////if there is a tile with multiple items, we need to render them both, ask garret how to detect this
	//engine.gui->message(TCODColor::red, "x1 is %d",engine.mapx1);
	//engine.gui->message(TCODColor::red, "x2 is %d",engine.mapx2);
	//engine.gui->message(TCODColor::red, "y1 is %d",engine.mapy1);
	//engine.gui->message(TCODColor::red, "y2 is %d",engine.mapy2);
	//engine.gui->message(TCODColor::red, "playerx  %d",plyx);
	//engine.gui->message(TCODColor::red, "playery  %d",plyy);
	
	SDL_Rect srcRect={0,0,16,16};
	int x = 0, y = 0;
	int plyx = 0, plyy = 0;
	for (int xM = engine.mapx1; xM < engine.mapx2+16; xM++) {
		for (int yM = engine.mapy1; yM < engine.mapy2+16; yM++) {
			//if it needs to be rendered over, render it over
			SDL_Rect dstRect={x*16,y*16,16,16};
			if(engine.mapcon->getChar(xM,yM) == 64)
			{
				plyx = x;
				plyy = y;
				if (engine.mapcon->getChar(xM,yM) != 163){//|| engine.gameStatus != engine.MAIN_MENU){
					TCODConsole::root->clear();	
				}
				
			}
			//SDL_Rect dstRect2={0,0,0,0};
			//SDL_Rect srcRect = {};
			if(engine.mapconCpy->getChar(xM,yM) == 31) 
			{ //replace 'down arrow thing' (31 ascii) with basic floor tiles
				//SDL_UpdateRect(floorMap, x*16, y*16, 16, 16);
				//SDL_FillRect(floorMap, &dstRect, 258);
				srcRect.x = 0;
				srcRect.y = 0;
				SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
				//SDL_UpdateRect(floorMap, x*16, y*16, 16, 16);
				
				//everything bodies to render behind
				if (engine.mapcon->getChar(xM,yM) == 181 || engine.mapcon->getChar(xM,yM) == 182 || engine.mapcon->getChar(xM,yM) == 183 || 
				engine.mapcon->getChar(xM,yM) == 184 || engine.mapcon->getChar(xM,yM) == 64 || engine.mapcon->getChar(xM,yM) == 164 || 
				engine.mapcon->getChar(xM,yM) == 165 || engine.mapcon->getChar(xM,yM) == 148 || engine.mapcon->getChar(xM,yM) == 132) 
				{
					for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {//is walkable?
						Actor *actor = *it;
						if (actor->x == xM && actor->y == yM && actor->destructible && actor->destructible->isDead()) {
							//doubles += 1;
							SDL_Rect srcRect2={10*16,3*16,16,16};
							SDL_Rect dstRect={x*16,y*16,16,16};
							//10 width 3 height for standard bodies
							//if they are spore bodies
							if (actor->ch == 162){
								srcRect2.y = 2*16;
							}
							
							SDL_BlitSurface(terminal,&srcRect2,floorMap,&dstRect);
						}
					}
				}
				
			}
			//replace 'up arrow thing' with darker floor tiles
			else if(engine.mapconCpy->getChar(xM,yM) == 30)
			{
				srcRect.x = 0;
				srcRect.y = 16;
				SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
			}
			//replace infected tiles lit
			else if(engine.mapconCpy->getChar(xM,yM) == 29){
				//SDL_FillRect(floorMap, &dstRect, 258);
				srcRect.x = 16;
				srcRect.y = 0;
				SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
				//SDL_FillRect(floorMap, &dstRect, 258);
				if (engine.mapcon->getChar(xM,yM) == 181 || engine.mapcon->getChar(xM,yM) == 182 || engine.mapcon->getChar(xM,yM) == 183 || 
				engine.mapcon->getChar(xM,yM) == 184 || engine.mapcon->getChar(xM,yM) == 64 || engine.mapcon->getChar(xM,yM) == 164 || 
				engine.mapcon->getChar(xM,yM) == 165 || engine.mapcon->getChar(xM,yM) == 148 || engine.mapcon->getChar(xM,yM) == 132) 
				{
					for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {//is walkable?
						Actor *actor = *it;
						if (actor->x == xM && actor->y == yM && actor->destructible && actor->destructible->isDead()) {
							//doubles += 1;
							SDL_Rect srcRect2={10*16,3*16,16,16};
							SDL_Rect dstRect={x*16,y*16,16,16};
							//10 width 3 height for standard bodies
							//if they are spore bodies
							if (actor->ch == 162){
								srcRect2.y = 2*16;
							}
							
							SDL_BlitSurface(terminal,&srcRect2,floorMap,&dstRect);
						}
					}
				}
			}
			//replace unlit infected tiles
			else if(engine.mapconCpy->getChar(xM,yM) == 28){
				srcRect.x = 16;
				srcRect.y = 16;
				SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
			}
			
			//check for doubles
			
			
			
			
			
			//shadows, always after tiles
			//flashbang shadow and glow
			if (engine.mapcon->getChar(xM,yM) == 181)  
			{
				srcRect.x = 16;
				srcRect.y = 0;
				SDL_BlitSurface(itemsGlow,&srcRect,floorMap,&dstRect);
				srcRect.x = 0;
				srcRect.y = 0;				
				SDL_BlitSurface(shadows,&srcRect,floorMap,&dstRect);	
			}
			//firebomb shadow and glow
			else if (engine.mapcon->getChar(xM,yM) == 182)  
			{
				srcRect.x = 0;
				srcRect.y = 0;
				SDL_BlitSurface(itemsGlow,&srcRect,floorMap,&dstRect);		
				SDL_BlitSurface(shadows,&srcRect,floorMap,&dstRect);	
			}
			//EMP glow
			else if (engine.mapcon->getChar(xM,yM) == 183)  
			{
				srcRect.x = 32;
				srcRect.y = 0;
				SDL_BlitSurface(itemsGlow,&srcRect,floorMap,&dstRect);
			}
			//Medkit shadow
			else if (engine.mapcon->getChar(xM,yM) == 184)  
			{
				srcRect.x = 16;
				srcRect.y = 0;
				SDL_BlitSurface(shadows,&srcRect,floorMap,&dstRect);
			}
			//filing cabinet
			else if (engine.mapcon->getChar(xM,yM) == 240)
			{
				if (engine.mapcon->getCharForeground(xM,yM) == TCODColor::white)
				{
					srcRect.x = 0;
				}
				else
				{
					srcRect.x = 16;
				}
				if(engine.mapconCpy->getChar(xM,yM-1) == '^')//if wall above it is wall
				{
					srcRect.y = 0;
				}
				else if (engine.mapconCpy->getChar(xM-1,yM) == '^')//if there is a wall to the left
				{
					srcRect.y = 16;
				}
				else if (engine.mapconCpy->getChar(xM+1,yM) == '^')//if there si a wall to the right
				{
					srcRect.y = 32;
				}
				else if (engine.mapconCpy->getChar(xM,yM+1) == '^')//if wall below use the backwards filing cabinet
				{
					srcRect.y = 48;
				}
				else //if nothing else or an error just print out the standard one just in case
				{
					srcRect.y = 0;
				}
				SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
			}
			else if (engine.mapcon->getChar(xM,yM) == 241)
			{
				if (engine.mapcon->getCharForeground(xM,yM) == TCODColor::white)
				{
					srcRect.x = 0;
				}
				else
				{
					srcRect.x = 16;
				}
				srcRect.y = 64;
				SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
			}
			else if (engine.mapcon->getChar(xM,yM) == 242)
			{
				if (engine.mapcon->getCharForeground(xM,yM) == TCODColor::white)
				{
					srcRect.y = 0;
				}
				else
				{
					srcRect.y = 16;
				}
				srcRect.x = 32;
				SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
			}
			//SDL_Delay(100);
			y++;
		}
		y=0;
		x++;
	}
			
	int x1 = 0, y1 = 0;
	for (int xM = engine.mapx1; xM < engine.mapx2+16; xM++) {
		for (int yM = engine.mapy1; yM < engine.mapy2+16; yM++) {
			//Player Shadow
			int yN = yM - 1;
			if (engine.mapcon->getChar(xM,yN) == 64 || engine.mapcon->getChar(xM,yN) == 164 || engine.mapcon->getChar(xM,yN) == 148 || engine.mapcon->getChar(xM,yN) == 132)  
			{
				int y2 = y1;
				y2 = y2*16;
				y2 += 16;
				SDL_Rect dstRectOffset={x1*16,y1*16,16,16};
				srcRect.x = 32;
				srcRect.y = 0;
				SDL_BlitSurface(shadows,&srcRect,floorMap,&dstRectOffset);
				//SDL_BlitSurface(humanShadow,NULL,floorMap,&dstRectOffset);
				//SDL_BlitSurface(humanShadow,NULL,screen,&dstRectOffset);
				//TCODConsole::flush();
			}
			
			
			y1++;
		}
		y1=0;
		x1++;
	}
	
	
	
	/*
	//render non static-shadows
	//if player is not dead //////////////////////////////////////need to make that test
	int deltaX =0, deltaY = 0;
	for (int x = mapx1; x <= mapx2; x++) 
	{
		for (int y = mapy1; y <= mapy2; y++)
		{
			if (TCODConsole::root->getChar(x,y) == 64)  
			{
				//convert map's to real x's and y's
				deltaX = x + ((85-22)/2);
				deltaY = y + ((47-14)/2);
			}
			
		}
	}
	
	engine.gui->message(TCODColor::red, "player x is  %d",plyx);
	engine.gui->message(TCODColor::red, "player y is %d",plyy);
	engine.gui->message(TCODColor::red, "calc player x is  %d",plyx*16);
	engine.gui->message(TCODColor::red, "calc player y is  %d",plyy*16);
	
	SDL_Rect srcRect2={0,0,32,32};
	SDL_Rect dstRect11 ={plyx*16+8,plyy*16+8,32,32};
	//SDL_Rect dstRect2={x1*16-16,y1*16,32,32};                      //shadows do not work when the map stops scrolling?
	SDL_BlitSurface(humanShadow,&srcRect2,map,&dstRect11);
	SDL_BlitSurface(humanShadow,&srcRect2,screen,&srcRect2);
	*/
	
	
	//SDL_Rect srcRect1={mapx1*16,mapy1*16,(mapx2-mapx1+16)*16,(mapy2-mapy1+16)*16};
	SDL_Rect dstRect1={22*16,0,(engine.mapx2-engine.mapx1)*16+16,(engine.mapy2-engine.mapy1)*16+16};

	//SDL_BlitSurface(screen,&dstRect1,floorMap,&srcRect1);
	SDL_BlitSurface(screen,&dstRect1,floorMap,NULL);
	
	
	SDL_Rect dstRectEquip={plyx*16,plyy*16,16,16};
	if (engine.gameStatus == engine.IDLE || engine.gameStatus == engine.NEW_TURN){
		for (Actor **it = engine.player->container->inventory.begin();it != engine.player->container->inventory.end();it++)
		{
		
			Actor *a = *it;
			if (a->pickable->type == Pickable::EQUIPMENT && ((Equipment*)(a->pickable))->equipped )//add case to not blit if inventory is open
			{
				if (strcmp(a->name,"Mylar-Lined Boots") == 0)
				{
					srcRect.x = 0;
					srcRect.y = 0;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				
				if (strcmp(a->name,"Titan-mail") == 0)
				{
					srcRect.x = 32;
					srcRect.y = 0;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				
				if (strcmp(a->name, "MLR") == 0)
				{
					srcRect.x = 16;
					srcRect.y = 0;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				
			}
		}
	}
	SDL_UpdateRect(floorMap, plyx*16, plyy*16, 16, 16);
	//SDL_UpdateRect(screen, plyx*16, plyy*16, 16, 16);
	
	
	//if the game is running add if statement sometime							
	//SDL_BlitSurface(floorMap,&srcRect1,screen,&dstRect1);	
	//GOES BLUE ON DEATH, UPDATE ON DEATH status?
	if (engine.gameStatus == engine.IDLE || engine.gameStatus == engine.NEW_TURN){
		SDL_BlitSurface(floorMap,NULL,screen,&dstRect1);	
	}
	TCODConsole::root->setDirty(22*16,0,(engine.mapx2-engine.mapx1)*16+16,(engine.mapy2-engine.mapy1)*16+16);
	//engine.gui->render();
	//else
	//{
	//	SDL_BlitSurface(titleScreen,NULL,screen,NULL);	
	//}
	//SDL_Flip(floorMap);

	SDL_FreeSurface(floorMap);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(floorTiles);
	SDL_FreeSurface(itemsGlow);
	SDL_FreeSurface(shadows);
	SDL_FreeSurface(equipment);
	SDL_FreeSurface(terminal);
	SDL_FreeSurface(decor);
	
}
	
	
