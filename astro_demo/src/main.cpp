#include "main.hpp"

Engine engine(85,48);

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