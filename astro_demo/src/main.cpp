#include "main.hpp"

Engine engine(160/1.875,100/1.875);

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