#include "main.hpp"
#include "SDL/SDL.h"
//#include "SDL/SDL_image.h"
#include <string>
//g++ src/*.cpp -o tuto -lmingw32 -lSDLmain -lSDL -Iinclude -Llib -ltcod-mingw -static-libgcc -static-libstdc++ -Wall


<<<<<<< HEAD
=======
//Engine engine(120/1.875,75/1.875);
>>>>>>> origin/master
Engine engine(85,47);
//SDL_Init( SDL_INIT_EVERYTHING );



<<<<<<< HEAD

int main() {
=======
int main( int argc, char* args[] ) {
	SDL_Init( SDL_INIT_EVERYTHING );
	TCODSystem::registerSDLRenderer(new Renderer());

	//TCODSystem::registerSDLRenderer(new EngineRend());
>>>>>>> origin/master
	engine.load(false);
	while (!TCODConsole::isWindowClosed()) {
		engine.update();
		engine.render();
		//TCODConsole::root->setCharBackground(50,20,TCODColor::desaturatedPink);
		//TCODConsole::root->setCharForeground(50,20,TCODColor::desaturatedPink);
		//engine.gui->message(TCODColor::red, "ascii is  %d",TCODConsole::root->getChar(50,20));

		TCODConsole::flush();
	}
	engine.save();
	SDL_Quit();
	return 0;
}

