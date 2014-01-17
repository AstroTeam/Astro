#include "main.hpp"

Engine engine(160/1.25,100/1.25);

int main() {
	engine.load(false);
	while (!TCODConsole::isWindowClosed()) {
		engine.update();
		engine.render();
		TCODConsole::flush();
	}
	engine.save();
	return 0;
}