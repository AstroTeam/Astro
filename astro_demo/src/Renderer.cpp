#include "main.hpp"
//#include "engine.cpp"
#include "SDL/SDL.h"
//#include "SDL/SDL_image.h"
#include <string>




void Renderer::render(void *sdlSurface){
	SDL_Surface *screen=(SDL_Surface *)sdlSurface;
	//floors
	SDL_Surface *floor = SDL_LoadBMP("tile_assets/floorTile.bmp");
	SDL_Surface *darkFloor = SDL_LoadBMP("tile_assets/floorTileDark.bmp");
	//flashbang
	SDL_Surface *flashGlow = SDL_LoadBMP("tile_assets/flashbang_alpha_glow_50.bmp");
	/*static int alpha = 255*.5;
	static bool Down = true;
	if (alpha > 0 && Down)
	{
		alpha = alpha-10;
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
	engine.gui->message(TCODColor::red, "alpha is  %d",alpha);*/
	SDL_SetAlpha( flashGlow, SDL_SRCALPHA, 255*.5);
	//SDL_SetAlpha( flashGlow, SDL_SRCALPHA, alpha);
	SDL_SetColorKey(flashGlow,SDL_SRCCOLORKEY,255);
	SDL_Surface *flashShadow = SDL_LoadBMP("tile_assets/flashbang_alpha_shadow_25.bmp");
	SDL_SetAlpha( flashShadow, SDL_SRCALPHA, 255*.25);
	SDL_SetColorKey(flashShadow,SDL_SRCCOLORKEY,255);
	//EMP glow
	SDL_Surface *EMPGlow = SDL_LoadBMP("tile_assets/EMP_alpha_glow_33.bmp");
	SDL_SetAlpha( EMPGlow, SDL_SRCALPHA, 255*.33);
	SDL_SetColorKey(EMPGlow,SDL_SRCCOLORKEY,255);
	//medkit shadow
	SDL_Surface *medShadow = SDL_LoadBMP("tile_assets/medkit_alpha_shadow_25.bmp");
	SDL_SetAlpha( medShadow, SDL_SRCALPHA, 255*.25);
	SDL_SetColorKey(medShadow,SDL_SRCCOLORKEY,255);
	//human Shadow
	//SDL_Surface *humanShadow = SDL_LoadBMP("tile_assets/human_alpha_shadow_25.bmp");
	//SDL_SetAlpha( humanShadow, SDL_SRCALPHA, 255*.25);
	//SDL_SetColorKey(humanShadow,SDL_SRCCOLORKEY,255);
	//background
	//SDL_Surface *map = SDL_LoadBMP("starmap.bmp");
	SDL_Surface *floorMap = SDL_LoadBMP("starmap2.bmp");
	static bool first=true;
	if ( first ) {
			first=false;
		// set blue(255) as transparent for the root console
		// so that we only blit characters
		SDL_SetColorKey(screen,SDL_SRCCOLORKEY,255);
			
	}
	
	/////////if processor power becomes an issue, we could modify the loops to not start from 0 and end at width/height, instead just render the final box dimensions
	///////////////if there is a tile with multiple items, we need to render them both, ask garret how to detect this
	
	
	//engine.mapconCpy is the map copy to be rendered with SDL as a background
	//scan mapconCpy and blit on good tile images
	int plyx = 0, plyy = 0;
	for (int x = 0; x < engine.mapconCpy->getWidth(); x++) {
		for (int y = 0; y < engine.mapconCpy->getHeight(); y++) {
			if(engine.mapcon->getChar(x,y) == 64)
			{
				plyx = x;
				plyy = y;
			}
		}
	}
	
	//engine.gui->message(TCODColor::red, "x1 is %d",engine.mapx1);
	//engine.gui->message(TCODColor::red, "x2 is %d",engine.mapx2);
	//engine.gui->message(TCODColor::red, "y1 is %d",engine.mapy1);
	//engine.gui->message(TCODColor::red, "y2 is %d",engine.mapy2);
	//engine.gui->message(TCODColor::red, "playerx  %d",plyx);
	//engine.gui->message(TCODColor::red, "playery  %d",plyy);
	int x = 0, y = 2;
	
	for (int xM = engine.mapx1; xM < engine.mapx2+16; xM++) {
		for (int yM = engine.mapy1; yM < engine.mapy2+16; yM++) {
			//if it needs to be rendered over, render it over
			SDL_Rect dstRect={x*16,y*16,16,16};
			
			//SDL_Rect dstRect2={0,0,0,0};
			//SDL_Rect srcRect = {};
			if(engine.mapconCpy->getChar(xM,yM) == 31) 
			{ //replace 'down arrow thing' (31 ascii) with basic floor tiles
				//SDL_BlitSurface(floor,NULL,floorMap,&dstRect);	
				//engine.gui->message(TCODColor::red, "tile x %d", xM);
				//engine.gui->message(TCODColor::red, "tile y %d", yM);
				//if (engine.mapcon->getChar(x,y) == 64)  // set playerx playery, updated later if you are looking/using item
				//{
				//	plyx = x;
				//	plyy = y;
				//}
				SDL_BlitSurface(floor,NULL,floorMap,&dstRect);
				
				
			}
			if(engine.mapconCpy->getChar(xM,yM) == 30) //replace 'up arrow thing' with darker floor tiles
			{
				SDL_BlitSurface(darkFloor,NULL,floorMap,&dstRect);
			}
			y++;
		}
		y=0;
		x++;
	}
	
	
	
	
	
	/*bool AOE = false;
	
	//AOE scanning loop and the alpha items loop
	for (int x = 0; x < engine.mapconCpy->getWidth(); x++) {
		for (int y = 0; y < engine.mapconCpy->getHeight(); y++) {
		//ALPHA LAYERS for items should be rendered after floors & infection but before items are placed (so just in this loop)
		//add the flashbang's alpha layer!  along with the firebomb!
		if (engine.mapcon->getChar(x,y) == 181 || engine.mapcon->getChar(x,y) == 182)  
		{
			//SDL_Rect dstRect={x*16,y*16,16,16};
			//SDL_BlitSurface(flashGlow,NULL,map,&dstRect);	
			//SDL_BlitSurface(flashShadow,NULL,map,&dstRect);	
		}
		//EMP glow
		if (engine.mapcon->getChar(x,y) == 183)  
		{
			//SDL_Rect dstRect={x*16,y*16,16,16};
			//SDL_BlitSurface(EMPGlow,NULL,map,&dstRect);
		}
		//Medkit shadow
		if (engine.mapcon->getChar(x,y) == 184)  
		{
			//SDL_Rect dstRect={x*16,y*16,16,16};
			//SDL_BlitSurface(medShadow,NULL,map,&dstRect);
		}
		if (!AOE && (engine.mapcon->getCharBackground(x,y) == TCODColor::pink || engine.mapcon->getCharBackground(x,y) == TCODColor::desaturatedPink))
				//exception for looking and for targetting
		{
			plyx = x;
			plyy = y;
		}
		if (engine.mapcon->getCharBackground(x,y) == TCODColor::darkerPink)
		{	//exception for center of an AOE effect
			plyx = x;
			plyy = y;
			AOE = true;
		}		
		}
		
	}*/
	
	
	
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
	
	//if the game is running add if statement sometime							
	//SDL_BlitSurface(floorMap,&srcRect1,screen,&dstRect1);	
	SDL_BlitSurface(floorMap,NULL,screen,&dstRect1);	
	//SDL_Flip(floorMap);

	
	SDL_FreeSurface(floorMap);
	SDL_FreeSurface(floor);
	SDL_FreeSurface(darkFloor);
	SDL_FreeSurface(flashGlow);
	SDL_FreeSurface(flashShadow);
	//SDL_FreeSurface(humanShadow);
	
}
	
	
