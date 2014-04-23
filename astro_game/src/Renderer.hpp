class Renderer : public ITCODSDLRenderer 
{
public:
	
	//void render(void *sdlSurface);
	void render(void *sdlSurface);
private:
	
	
	
};

/*  HOW TO COPY MAPS
TCODMap * map = new TCODMap(50,50); // allocate the map
map->setProperties(10,10,true,true); // set a cell as 'empty'
TCODMap * map2 = new TCODMap(10,10); // allocate another map
map2->copy(map);
*/