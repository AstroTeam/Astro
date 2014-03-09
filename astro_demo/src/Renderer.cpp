#include "main.hpp"
//#include "engine.cpp"
#include "SDL/SDL.h"
//#include "SDL/SDL_image.h"
#include <string>


void Renderer::render(void *sdlSurface){
/////////////////////////////////////////////////////////////////////rendering doubled up walkable decor!
	SDL_Surface *screen=(SDL_Surface *)sdlSurface;
	static bool first=true;
	if ( first ) {
			first=false;
		// set blue(255) as transparent for the root console
		// so that we only blit characters
	SDL_SetColorKey(screen,SDL_SRCCOLORKEY,255);
			
	}
	if (engine.invState == 0)
	{
	//set fps
	TCODSystem::setFps(60);
	//floors
	SDL_SetColorKey(screen,SDL_SRCCOLORKEY,255);
	SDL_Surface *floorTiles = SDL_LoadBMP("tile_assets/tiles.bmp");
	SDL_SetColorKey(floorTiles,SDL_SRCCOLORKEY,255);
	//glowing variable alpha set
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
	//Background
	SDL_Surface *floorMap = SDL_LoadBMP("starmap2.bmp");
	SDL_Surface *terminal = SDL_LoadBMP("tile_assets/terminal.bmp");
	SDL_SetColorKey(terminal,SDL_SRCCOLORKEY,255);

	
	
	
	
	
	//messages
	//engine.gui->message(TCODColor::red, "x1 is %d",engine.mapx1);
	//engine.gui->message(TCODColor::red, "x2 is %d",engine.mapx2);
	//engine.gui->message(TCODColor::red, "y1 is %d",engine.mapy1);
	//engine.gui->message(TCODColor::red, "y2 is %d",engine.mapy2);
	//engine.gui->message(TCODColor::red, "playerx  %d",plyx);
	//engine.gui->message(TCODColor::red, "playery  %d",plyy);
	
	SDL_Rect srcRect={0,0,16,16};
	SDL_Rect dstRect1={22*16,0,(engine.mapx2-engine.mapx1)*16+16,(engine.mapy2-engine.mapy1)*16+16};
	int x = 0, y = 0;
	int plyx = 0, plyy = 0;
	for (int xM = engine.mapx1; xM < engine.mapx2+16; xM++) {
		for (int yM = engine.mapy1; yM < engine.mapy2+16; yM++) {
			
			//refresh if looking
			SDL_Rect dstRect={x*16,y*16,16,16};
			if (TCODConsole::root->getCharBackground(22*16+xM,yM) == TCODColor::darkerPink || //refresh if looking
				TCODConsole::root->getCharBackground(22*16+xM,yM) == TCODColor::pink)
			{
				TCODConsole::root->clear();
			}
			//detects where the player is, if it is @, human, alien or robot base
			if(engine.mapcon->getChar(xM,yM) == 64 || engine.mapcon->getChar(xM,yM) == 143 ||
     		  engine.mapcon->getChar(xM,yM) == 159 || engine.mapcon->getChar(xM,yM) == 175)
			{
				plyx = x;
				plyy = y;
				//refresh if not deadz  (163 is corpse)
				if (engine.mapcon->getChar(xM,yM) != 163){//|| engine.gameStatus != engine.MAIN_MENU){ //refresh if player is alive
					TCODConsole::root->clear();	
				}
				
			}
			//if the tile is a floor
			if(engine.mapconCpy->getChar(xM,yM) == 31) 
			{ //replace 'down arrow thing' (31 ascii) with basic floor tiles
				
				//office floors, can test for any floors
				int r = engine.map->tileType(xM,yM);
				if (r == 2)//2 is office
				{
					srcRect.x = 48;
				}
				else if (r == 3)//3 is barracks
				{
					srcRect.x = 16;
				}
				else //else is regular floors
				{
					srcRect.x = 0;
				}
				srcRect.y = 0;
				SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
				
				//render infection over it
				if (engine.map->isInfected(xM,yM))
				{
					srcRect.x = 128;
					srcRect.y = 0;
					SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
				}
				//any decor to render just on top of floors
				if (engine.mapconDec->getChar(xM,yM) == ' ')
				{
					//do nothing if common case (i.e.) no decor
					//commmon case is fast
				}
				//if the decor is papers, render over
				else if (engine.mapconDec->getChar(xM,yM) == 5 || engine.mapconDec->getChar(xM,yM) == 6 || 
						 engine.mapconDec->getChar(xM,yM) == 7 || engine.mapconDec->getChar(xM,yM) == 8)
				{
					srcRect.y = 32;
					srcRect.x = 32+ (engine.mapconDec->getChar(xM,yM) - 5 ) * 16;
					SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
								
				}
						
					
				
				
				//OPTIMIZE ME -->  COMMON CASE IS SLOW
				
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
				//render office floors, same as lit tiles, just different y value
				int r = engine.map->tileType(xM,yM);
				if (r == 2) 
				{
					srcRect.x = 48;
				}
				else if (r == 3)//3 is barracks
				{
					srcRect.x = 16;
				}
				else
				{
					srcRect.x = 0;
				}
				srcRect.y = 16;
				SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
				
				//add infection, same a lit, except y
				if (engine.map->isInfected(xM,yM))
				{
					srcRect.x = 128;
					srcRect.y = 16;
					SDL_BlitSurface(floorTiles,&srcRect,floorMap,&dstRect);
				}
				
				//render decor *DARK*
				if (engine.mapconDec->getChar(xM,yM) == ' ')
				{
					//do nothing if common case (i.e.) no decor
					//common case is fast
				}
				else if (engine.mapconDec->getChar(xM,yM) == 5 || engine.mapconDec->getChar(xM,yM) == 6 ||  //dark papers
						 engine.mapconDec->getChar(xM,yM) == 7 || engine.mapconDec->getChar(xM,yM) == 8)
				{
					srcRect.y = 48;
					srcRect.x = 32+ (engine.mapconDec->getChar(xM,yM) - 5 ) * 16;
					SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
								
				}
			}
			
			
			
			
			//OPTIMIZE ME -->  COMMON CASE IS SLOW
			
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
			//Battery Glow
			else if (engine.mapcon->getChar(xM,yM) == 186)  
			{
				srcRect.x = 48;
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
					//light
					srcRect.x = 0;
				}
				else
				{
					//dark
					srcRect.x = 16;
				}
				if(engine.mapcon->getChar(xM,yM-1) == '^')//if wall above it is wall
				{
					srcRect.y = 0;
				}
				else if (engine.mapcon->getChar(xM-1,yM) == '^')//if there is a wall to the left
				{
					srcRect.y = 16;
				}
				else if (engine.mapcon->getChar(xM+1,yM) == '^')//if there si a wall to the right
				{
					srcRect.y = 32;
				}
				else if (engine.mapcon->getChar(xM,yM+1) == '^')//if wall below use the backwards filing cabinet
				{
					srcRect.y = 48;
				}
				else //if nothing else or an error just print out the standard one just in case
				{
					srcRect.y = 0;
				}
				SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
			}
			//smashed filing cabinet
			else if (engine.mapcon->getChar(xM,yM) == 241)
			{
				if (engine.mapcon->getCharForeground(xM,yM) == TCODColor::white)
				{
					//light
					srcRect.x = 0;
				}
				else
				{
					//dark
					srcRect.x = 16;
				}
				srcRect.y = 64;
				SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
				
			}
			//desks
			else if (engine.mapcon->getChar(xM,yM) == 242)
			{
				if (engine.mapcon->getCharForeground(xM,yM) == TCODColor::white)
				{
					//light
					srcRect.y = 0;
				}
				else
				{
					//dark
					srcRect.y = 16;
				}
				//the different versions to add based on mapconDec's value
				if(engine.mapconDec->getChar(xM,yM) == 1)
				{
					srcRect.x=32+48;
				} else if (engine.mapconDec->getChar(xM,yM) == 2)
				{
					srcRect.x=32+16;
				}else if (engine.mapconDec->getChar(xM,yM) == 3)
				{
					srcRect.x=32+32;
				}else
				{
					srcRect.x=32;
				}
				
				SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
			}
			//barracks decor
			else if (engine.mapcon->getChar(xM,yM) == 243)
			{
				if (engine.mapcon->getCharForeground(xM,yM) == TCODColor::white)
				{
					//light
					srcRect.y = 0;
				}
				else
				{
					//dark
					srcRect.y = 16;
				}
				if(engine.mapconDec->getChar(xM,yM) == 9)//headboard of bed
				{
					srcRect.x=96;
				}else if(engine.mapconDec->getChar(xM,yM) == 15)//foot of bed
				{
					srcRect.x=144;
				}else if(engine.mapconDec->getChar(xM,yM) == 12)//backward headboard
				{
					srcRect.x=96;
					srcRect.y += 32;
				}else if(engine.mapconDec->getChar(xM,yM) == 18)//backward foot
				{
					srcRect.x=144;
					srcRect.y += 32;
				}else if(engine.mapconDec->getChar(xM,yM) == 22)//locker
				{
					srcRect.x=192;
					//srcRect.y += 32;
				}
				
				SDL_BlitSurface(decor,&srcRect,floorMap,&dstRect);
			}
			
			y++;
		}
		y=0;
		x++;
	}
		
	//OPTIMIZE ME?  INCLUDE IN PREVIOUS LOOP?, CAN'T, MUST BE AFTER ALL TILES :(
	int x1 = 0, y1 = 0;
	for (int xM = engine.mapx1; xM < engine.mapx2+16; xM++) {
		for (int yM = engine.mapy1; yM < engine.mapy2+16; yM++) {
			//Human Shadows
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
				
			}
			
			
			y1++;
		}
		y1=0;
		x1++;
	}
	
	//flicker lights
	for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor = *it;
		//if (actor->x ==x && actor->y == y) {
		//	return actor;
		//}
		if (actor->ch == 224 && engine.distance(actor->x,engine.player->x,actor->y,engine.player->y) < 11){
			//LightAi *l = (LightAi*)actor->ai;
			//TCODRandom *myRandom = new TCODRandom();
			//float rng = myRandom->getFloat(0.0000f,1.0000f);
			//l->flicker(actor,rng);
		}
	}
	
	
/*
	legacy error messages
	engine.gui->message(TCODColor::red, "player x is  %d",plyx);
	engine.gui->message(TCODColor::red, "player y is %d",plyy);
	engine.gui->message(TCODColor::red, "calc player x is  %d",plyx*16);
	engine.gui->message(TCODColor::red, "calc player y is  %d",plyy*16);
*/
	
	
	//SDL_Rect srcRect1={mapx1*16,mapy1*16,(mapx2-mapx1+16)*16,(mapy2-mapy1+16)*16};
	//SDL_BlitSurface(screen,&dstRect1,floorMap,&srcRect1);
	SDL_BlitSurface(screen,&dstRect1,floorMap,NULL);
	
	
	//COULD OPTIMIZE --> CONTAINER/INVENTORY/PLAYER CONTAINS A LIST OF JUST EQUIPMENT TO RENDER, instead of scanning all items
	SDL_Rect dstRectEquip={plyx*16,plyy*16,16,16};
	if (engine.gameStatus == engine.IDLE || engine.gameStatus == engine.NEW_TURN){
		for (Actor **it = engine.player->container->inventory.begin();it != engine.player->container->inventory.end();it++)
		{
		
			Actor *a = *it;
			if (a->pickable->type == Pickable::EQUIPMENT && ((Equipment*)(a->pickable))->equipped )//add case to not blit if inventory is open
			{
				//equipment
				if (strcmp(a->name,"Mylar-Lined Boots") == 0)//legacy first thing!
				{
					srcRect.x = 0;
					srcRect.y = 0;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				//marine starter stuff
				else if (strcmp(a->name,"Marine Fatigue Pants") == 0)
				{
					srcRect.x = 16;
					srcRect.y = 16;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Marine Fatigue Jacket") == 0)
				{
					srcRect.x = 48;
					srcRect.y = 16;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Marine Medical Jacket") == 0)
				{
					srcRect.x = 64;
					srcRect.y = 16;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Marine Quarter-Master Jacket") == 0)
				{
					srcRect.x = 80;
					srcRect.y = 16;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Combat Boots") == 0)
				{
					srcRect.x = 0;
					srcRect.y = 16;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Marine Ballistic Helmet") == 0)
				{
					srcRect.x = 32;
					srcRect.y = 16;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				//explorer starter stuff
				else if (strcmp(a->name,"T-Shirt (red)") == 0)
				{
					srcRect.x = 0;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Boarding Vest") == 0)
				{
					srcRect.x = 16;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Boarding Boots") == 0)
				{
					srcRect.x = 32;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				//mercenary starter stuff
				else if (strcmp(a->name,"Skinsuit Leggings") == 0)
				{
					srcRect.x = 80;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Skinsuit Jacket") == 0)
				{
					srcRect.x = 64;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Balaclava") == 0)
				{
					srcRect.x = 48;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Bruiser Gloves") == 0)
				{
					srcRect.x = 96;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"Tech Helmet") == 0)
				{
					srcRect.x = 112;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name,"T-Shirt (grey)") == 0)
				{
					srcRect.x = 128;
					srcRect.y = 32;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				
				//rando
				else if (strcmp(a->name,"Titan-mail") == 0)
				{
					srcRect.x = 32;
					srcRect.y = 0;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				else if (strcmp(a->name, "MLR") == 0)
				{
					srcRect.x = 16;
					srcRect.y = 0;
					SDL_BlitSurface(equipment,&srcRect,floorMap,&dstRectEquip);
				}
				
			}
		}
	}
	SDL_UpdateRect(floorMap, plyx*16, plyy*16, 16, 16);
	
	
	//if the game is idle or new turn, blit onto screen
	if (engine.gameStatus == engine.IDLE || engine.gameStatus == engine.NEW_TURN){
		SDL_BlitSurface(floorMap,NULL,screen,&dstRect1);	
	}
	//set everything to dirty
	TCODConsole::root->setDirty(22*16,0,(engine.mapx2-engine.mapx1)*16+16,(engine.mapy2-engine.mapy1)*16+16);
	//free all surfaces
	SDL_FreeSurface(floorMap);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(floorTiles);
	SDL_FreeSurface(itemsGlow);
	SDL_FreeSurface(shadows);
	SDL_FreeSurface(equipment);
	SDL_FreeSurface(terminal);
	SDL_FreeSurface(decor);
	}
	//if inventory is open begin animation
	else if (engine.invState == 1)
	{
	TCODSystem::setFps(60);
	//TCODConsole::root->clear();
	engine.invFrames++;
	first=true;
		// set blue(255) as transparent for the root console
		// so that we only blit characters
	SDL_SetColorKey(screen,SDL_SRCCOLORKEY,0);
	//SDL_Surface *floorMap = SDL_LoadBMP("starmap2.bmp");
	SDL_Surface *backpack = SDL_LoadBMP("tile_assets/backpack.bmp");
	SDL_SetColorKey(backpack,SDL_SRCCOLORKEY,255);
	SDL_Surface *bg = SDL_LoadBMP("tile_assets/invBG.bmp");
	//SDL_Rect dstRect1={22*16,0,(engine.mapx2-engine.mapx1)*16+16,(engine.mapy2-engine.mapy1)*16+16};
	SDL_Rect dstBack={(engine.screenWidth*16)/2-375,engine.screenHeight*16-48,750,750};
	SDL_Rect dstBack1={0,0,750,750};
	//int second = TCODSystem::getFps();
	if (engine.invFrames > 30)
	{
		//SDL_BlitSurface(screen,NULL,bg,NULL);
		//TCODConsole::flush();
		//SDL_BlitSurface(bg,NULL,screen,NULL);
		engine.invState = 2;
		SDL_FreeSurface(backpack);
		SDL_FreeSurface(bg);
	}
	else
	{
		SDL_BlitSurface(bg,NULL,screen,NULL);
		//TCODConsole::root->clear();
		dstBack.y -= (engine.invFrames*16);
		//dstBack.y = 20;
		SDL_BlitSurface(backpack,&dstBack1,screen,&dstBack);
		SDL_FreeSurface(backpack);
	SDL_FreeSurface(bg);
	}
	}
	else if (engine.invState == 2)
	{
	TCODSystem::setFps(60);
	//TCODConsole::root->clear();
	engine.invFrames++;
	first=true;
		// set blue(255) as transparent for the root console
		// so that we only blit characters
	SDL_SetColorKey(screen,SDL_SRCCOLORKEY,0);
	//SDL_Surface *floorMap = SDL_LoadBMP("starmap2.bmp");
	SDL_Surface *tab = SDL_LoadBMP("tile_assets/tablet.bmp");
	SDL_SetColorKey(tab,SDL_SRCCOLORKEY,255);
	SDL_Surface *tabBig = SDL_LoadBMP("tile_assets/tablet-big.bmp");
	SDL_SetColorKey(tabBig,SDL_SRCCOLORKEY,255);
	SDL_Surface *backpack = SDL_LoadBMP("tile_assets/backpack.bmp");
	SDL_Surface *flap = SDL_LoadBMP("tile_assets/backpack-flap.bmp");
	SDL_SetColorKey(flap,SDL_SRCCOLORKEY,255);
	SDL_Surface *bg = SDL_LoadBMP("tile_assets/invBG.bmp");
	SDL_SetColorKey(backpack,SDL_SRCCOLORKEY,255);
	//SDL_Rect dstRect1={22*16,0,(engine.mapx2-engine.mapx1)*16+16,(engine.mapy2-engine.mapy1)*16+16};
	SDL_Rect dstBack={(engine.screenWidth*16)/2-375,engine.screenHeight*16-48,750,750};
	SDL_Rect dstBack1={0,0,750,750};
	SDL_Rect dstTab={0,0,250,250};
	SDL_Rect dstTabS={(engine.screenWidth*16)/2-125,250,250,250};
	//int second = TCODSystem::getFps();
	SDL_BlitSurface(bg,NULL,screen,NULL);
	dstBack.y -= (30*16);
	static int y = 0;
	if (engine.invFrames > 60+15)
	{
		//SDL_BlitSurface(screen,NULL,bg,NULL);
		//TCODConsole::flush();
		//SDL_BlitSurface(bg,NULL,screen,NULL);
		engine.invState = 4;
		y = 0;
		SDL_BlitSurface(backpack,&dstBack1,screen,&dstBack);
		SDL_Rect screenTab ={(engine.screenWidth*16)/2-(708/2),48,708,650};
		SDL_BlitSurface(tabBig,NULL,screen,&screenTab);
		SDL_FreeSurface(flap);
		SDL_FreeSurface(backpack);
		SDL_FreeSurface(tab);
		SDL_FreeSurface(tabBig);
		SDL_FreeSurface(bg);
	}
	else if (engine.invFrames > 45)
	{
		
		TCODSystem::setFps(45);
		SDL_BlitSurface(backpack,&dstBack1,screen,&dstBack);
		
		SDL_Rect bigTab    ={0,750-(engine.invFrames-45)*22,708,((engine.invFrames-45)*22)};//(16+((engine.invFrames*16)-45))};
		
		//SDL_Rect screenTab ={(engine.screenWidth*16)/2-(708/2),48,708,750};
		if (((engine.invFrames-45)*22) >= 650)
			{y += 28;}
			
		SDL_Rect screenTab ={(engine.screenWidth*16)/2-(708/2),y,708,650};
		
		SDL_BlitSurface(tabBig,&bigTab,screen,&screenTab);
		
		SDL_FreeSurface(flap);
		SDL_FreeSurface(backpack);
		SDL_FreeSurface(tab);
		SDL_FreeSurface(tabBig);
		SDL_FreeSurface(bg);
		
	}
	else
	{
		TCODSystem::setFps(45);
		//TCODConsole::root->clear();
		
		dstTabS.y -= (engine.invFrames-30)*25;
		//dstBack.y = 20;
		SDL_BlitSurface(backpack,&dstBack1,screen,&dstBack);
		SDL_BlitSurface(tab,&dstTab,screen,&dstTabS);
		SDL_BlitSurface(flap,&dstBack1,screen,&dstBack);
		SDL_FreeSurface(flap);
		SDL_FreeSurface(backpack);
		SDL_FreeSurface(tab);
		SDL_FreeSurface(tabBig);
		SDL_FreeSurface(bg);
		
	}
	}
	else if (engine.invState == 4)
	{
		TCODSystem::setFps(60);
		TCODConsole::root->clear();
		
		first=true;
		//static bool Rend = true;
		//SDL_Rect dstBack={(engine.screenWidth*16)/2-375,(engine.screenHeight*16-48)-(30*16),750,750};
		//SDL_Rect dstBack1={0,0,750,750};
		
		SDL_Surface *tabBig = SDL_LoadBMP("tile_assets/tablet-big.bmp");
		//SDL_Surface *pointer = SDL_LoadBMP("tile_assets/finger.bmp");
		SDL_SetColorKey(tabBig,SDL_SRCCOLORKEY,255);
		//SDL_SetColorKey(pointer,SDL_SRCCOLORKEY,255);
		//SDL_Surface *bg = SDL_LoadBMP("tile_assets/invBG.bmp");
		SDL_SetColorKey(screen,SDL_SRCCOLORKEY,0);
		SDL_Rect screenTab ={(engine.screenWidth*16)/2-(708/2),48,708,650};
		
		//SDL_Surface *backpack = SDL_LoadBMP("tile_assets/backpack.bmp");
		//SDL_SetColorKey(backpack,SDL_SRCCOLORKEY,255);
		
		
		
		
		//SDL_UpdateRect(bg, 0, 0, 0,0);
		
		/*static int oldX = 0;
		static int oldY = 0;
		int x = engine.selX*16;
		int y = engine.selY*16;
		
		if (oldX != x || oldY != y)
		{
			//TCODConosle *temp = new TCODConsole(85,44);
			TCODConsole::blit(TCODConsole::root, 0, 0, 85, 44, engine.temporary , 0,0);
			
			SDL_Surface *tempy =(SDL_Surface *)engine.temporary;
			SDL_SetColorKey(tempy,SDL_SRCCOLORKEY,0);
			SDL_BlitSurface(tempy,&screenTab,tabBig,NULL);
			SDL_BlitSurface(backpack,&dstBack1,bg,&dstBack);
			SDL_BlitSurface(bg,NULL,screen,NULL);
			
			SDL_BlitSurface(tabBig,NULL,screen,&screenTab);
			oldX = x;
			oldY = y;
		}*/
		//
		
		//
		//SDL_BlitSurface(tabBig,NULL,bg,&screenTab);
		SDL_BlitSurface(screen,&screenTab,tabBig,NULL);
		SDL_BlitSurface(tabBig,NULL,screen,&screenTab);
		//SDL_UpdateRect(screen, 0, 0, 85*16,35*16);
		//TCODConsole::root->setDefaultBackground(TCODColor::black);
		//TCODConsole::root->setDefaultBackground(TCODColor::blue);
		//TCODConsole::root->clear();
		//engine.gui->render();
		//SDL_BlitSurface(screen,NULL,bg,NULL);
		
		//SDL_Rect pointerBox ={x,y,375,800};
		//SDL_BlitSurface(pointer,NULL,screen,&pointerBox);
		
		
		//SDL_BlitSurface(bg,NULL,screen,NULL);
		//SDL_FreeSurface(bg);
		SDL_FreeSurface(tabBig);
		//SDL_FreeSurface(backpack);
		//SDL_FreeSurface(pointer);
	}
	
}
	

