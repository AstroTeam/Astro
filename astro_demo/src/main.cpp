#include "main.hpp"

Engine engine(145/1.875,80/1.875);

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