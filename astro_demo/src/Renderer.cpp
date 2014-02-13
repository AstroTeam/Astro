#include "main.hpp"
//#include "engine.cpp"
#include "SDL/SDL.h"
//#include "SDL/SDL_image.h"
#include <string>




void Renderer::render(void *sdlSurface){
/////////////////////////////////////////////////////////////////////rendering doubled up objects
	SDL_Surface *screen=(SDL_Surface *)sdlSurface;
	//SDL_Surface *titleScreen = SDL_LoadBMP("titleScreenHiRes.bmp");
	//floors
	SDL_Surface *floor = SDL_LoadBMP("tile_assets/floorTile.bmp");
	SDL_Surface *infectedFloor = SDL_LoadBMP("tile_assets/floorTile_infected.bmp");
	SDL_Surface *infectedFloorDark = SDL_LoadBMP("tile_assets/floorTileDark_infected.bmp");
	SDL_Surface *darkFloor = SDL_LoadBMP("tile_assets/floorTileDark.bmp");
	//flashbang
	SDL_Surface *flashGlow = SDL_LoadBMP("tile_assets/flashbang_alpha_glow_50.bmp");
	static int alpha = 255*.5;
	static int alphaStars = 255;
	static bool Down = true;
	static bool StarsDown = true;
	if (alphaStars > 0 && StarsDown)
	{
		alphaStars -= 1;
		if (alphaStars < 0)
			alphaStars = 0;
	}
	else if (alphaStars == 0 && StarsDown)
	{
		StarsDown = false;
	}
	else if (alphaStars < 255)
	{
		alphaStars += 1;
	}
	else if (alphaStars >= 255)
	{
		StarsDown = true;
	}
	
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
		alpha = alpha+10;
	}
	else if (alpha > 255*.5)
	{
		Down = true;
	}
	//engine.gui->message(TCODColor::red, "alpha is  %d",alpha);
	//SDL_SetAlpha( flashGlow, SDL_SRCALPHA, alpha);
	SDL_SetAlpha( flashGlow, SDL_SRCALPHA, alpha);
	SDL_SetColorKey(flashGlow,SDL_SRCCOLORKEY,255);
	SDL_Surface *flashShadow = SDL_LoadBMP("tile_assets/flashbang_alpha_shadow_25.bmp");
	SDL_SetAlpha( flashShadow, SDL_SRCALPHA, 255*.25);
	SDL_SetColorKey(flashShadow,SDL_SRCCOLORKEY,255);
	//firebomb
	SDL_Surface *fireGlow = SDL_LoadBMP("tile_assets/firebomb_alpha_glow_50.bmp");
	SDL_SetAlpha( fireGlow, SDL_SRCALPHA, alpha*.5);
	SDL_SetColorKey(fireGlow,SDL_SRCCOLORKEY,255);
	//EMP glow
	SDL_Surface *EMPGlow = SDL_LoadBMP("tile_assets/EMP_alpha_glow_33.bmp");
	SDL_SetAlpha( EMPGlow, SDL_SRCALPHA, 255*.33);
	SDL_SetColorKey(EMPGlow,SDL_SRCCOLORKEY,255);
	//medkit shadow
	SDL_Surface *medShadow = SDL_LoadBMP("tile_assets/medkit_alpha_shadow_25.bmp");
	SDL_SetAlpha( medShadow, SDL_SRCALPHA, 255*.25);
	SDL_SetColorKey(medShadow,SDL_SRCCOLORKEY,255);
	//human Shadow
	SDL_Surface *humanShadow = SDL_LoadBMP("tile_assets/human_alpha_shadow_25.bmp");
	SDL_SetAlpha( humanShadow, SDL_SRCALPHA, 255*.25);
	SDL_SetColorKey(humanShadow,SDL_SRCCOLORKEY,255);
	//EQUIPMENT
	//Mylar Boots
	SDL_Surface *mylarBoots = SDL_LoadBMP("tile_assets/Mylar_Boots.bmp");
	SDL_SetColorKey(mylarBoots,SDL_SRCCOLORKEY,255);
	SDL_Surface *titanMail = SDL_LoadBMP("tile_assets/Titanium_nanoChainmail.bmp");
	SDL_SetColorKey(titanMail,SDL_SRCCOLORKEY,255);
	//SDL_SetColorKey(humanShadow,SDL_SRCCOLORKEY,255);
	//background
	//SDL_Surface *map = SDL_LoadBMP("starmap.bmp");
	
	SDL_Surface *floorMap = SDL_LoadBMP("starmap2_blank.bmp");
	SDL_Surface *floorMapStars = SDL_LoadBMP("starmap2.bmp");
	SDL_Surface *floorMapStarsAlt = SDL_LoadBMP("starmap2_alt.bmp");
	SDL_SetAlpha( floorMapStars, SDL_SRCALPHA, alphaStars);
	SDL_SetAlpha( floorMapStarsAlt, SDL_SRCALPHA, 255-alphaStars);
	SDL_BlitSurface(floorMapStars,NULL,floorMap,NULL);
	SDL_BlitSurface(floorMapStarsAlt,NULL,floorMap,NULL);
	SDL_Surface *terminal = SDL_LoadBMP("tile_assets/terminal.bmp");
	SDL_SetColorKey(terminal,SDL_SRCCOLORKEY,255);
	
	SDL_Surface *pink = SDL_LoadBMP("tile_assets/pink.bmp");
	
	
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
				SDL_BlitSurface(floor,NULL,floorMap,&dstRect);
				//SDL_UpdateRect(floorMap, x*16, y*16, 16, 16);
			}
			//replace 'up arrow thing' with darker floor tiles
			if(engine.mapconCpy->getChar(xM,yM) == 30)
			{
				SDL_BlitSurface(darkFloor,NULL,floorMap,&dstRect);
			}
			//replace infected tiles lit
			if(engine.mapconCpy->getChar(xM,yM) == 29){
				//SDL_FillRect(floorMap, &dstRect, 258);
				SDL_BlitSurface(infectedFloor,NULL,floorMap,&dstRect);
				//SDL_FillRect(floorMap, &dstRect, 258);
			}
			//replace unlit infected tiles
			if(engine.mapconCpy->getChar(xM,yM) == 28){
				SDL_BlitSurface(infectedFloorDark,NULL,floorMap,&dstRect);
			}
			
			//check for doubles
			
			for (Actor **it = engine.actors.begin(); it != engine.actors.end(); it++) {
				Actor *actor = *it;
				if (actor->x == xM && actor->y == yM && actor->destructible && actor->destructible->isDead()) {
					//doubles += 1;
					SDL_Rect srcRect={10*16,3*16,16,16};
					SDL_Rect dstRect={x*16,y*16,16,16};
					//10 width 3 height for standard bodies
					//if they are spore bodies
					if (actor->ch == 162){
						srcRect.y = 2*16;
					}
					
					SDL_BlitSurface(terminal,&srcRect,floorMap,&dstRect);
				}
			}
			
			
			
			//shadows, always after tiles
			//flashbang shadow and glow
			if (engine.mapcon->getChar(xM,yM) == 181)  
			{
				SDL_BlitSurface(flashGlow,NULL,floorMap,&dstRect);	
				SDL_BlitSurface(flashShadow,NULL,floorMap,&dstRect);	
			}
			//firebomb shadow and glow
			if (engine.mapcon->getChar(xM,yM) == 182)  
			{
				SDL_BlitSurface(fireGlow,NULL,floorMap,&dstRect);	
				SDL_BlitSurface(flashShadow,NULL,floorMap,&dstRect);	
			}
			//EMP glow
			if (engine.mapcon->getChar(xM,yM) == 183)  
			{
				//SDL_Rect dstRect={x*16,y*16,16,16};
				SDL_BlitSurface(EMPGlow,NULL,floorMap,&dstRect);
			}
			//Medkit shadow
			if (engine.mapcon->getChar(xM,yM) == 184)  
			{
				//SDL_Rect dstRect={x*16,y*16,16,16};
				SDL_BlitSurface(medShadow,NULL,floorMap,&dstRect);
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
				SDL_BlitSurface(humanShadow,NULL,floorMap,&dstRectOffset);
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
					SDL_BlitSurface(mylarBoots,NULL,floorMap,&dstRectEquip);
				}
				
				if (strcmp(a->name,"Titan-mail") == 0)
				{
					SDL_BlitSurface(titanMail,NULL,floorMap,&dstRectEquip);
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
	SDL_FreeSurface(floorMapStars);
	SDL_FreeSurface(floorMapStarsAlt);
	SDL_FreeSurface(floor);
	SDL_FreeSurface(darkFloor);
	SDL_FreeSurface(flashGlow);
	SDL_FreeSurface(flashShadow);
	SDL_FreeSurface(infectedFloor);
	SDL_FreeSurface(infectedFloorDark);
	SDL_FreeSurface(mylarBoots);
	SDL_FreeSurface(titanMail);
	SDL_FreeSurface(pink);
	SDL_FreeSurface(terminal);
	//SDL_FreeSurface(titleScreen);
	//SDL_FreeSurface(humanShadow);
	
}
	
	
