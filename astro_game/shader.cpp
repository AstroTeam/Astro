#include "libtcod.hpp"
//#include "rad_shader.hpp"

void StandardShader::compute() {
	// turn off all lights
	//memset(lightmap,0,sizeof(TCODColor)*map->getWidth()*map->getHeight());
	for ( Light *l=lights.begin(); l != lights.end(); l++) {
		// compute the potential visible set for this light
		int minx=l->x-l->radius;
		int miny=l->y-l->radius;
		int maxx=l->x+l->radius;
		int maxy=l->y+l->radius;
		minx=MAX(0,minx);
		miny=MAX(0,miny);
		maxx=MIN(maxx,map->getWidth()-1);
		maxy=MIN(maxy,map->getHeight()-1);
		float offset = 1.0f/(1.0f+(float)(l->radius*l->radius)/20);
		float factor = 1.0f/(1.0f-offset);
		// compute the light's fov
		TCODMap lmap(maxx-minx+1,maxy-miny+1);
		for (int x=minx; x <= maxx; x++) {
			for (int y=miny; y <= maxy; y++) {
				lmap.setProperties(x-minx,y-miny,map->isWalkable(x,y),map->isTransparent(x,y));
			}
		}
		lmap.computeFov(l->x-minx,l->y-miny,l->radius);
		// compute the light's contribution		
		//double invSquaredRadius=1.0 / (l->radius*l->radius);
		for (int x=minx; x <= maxx; x++) {
			for (int y=miny; y <= maxy; y++) {
				if ( lmap.isInFov(x-minx,y-miny)) {
					int squaredDist = (l->x-x)*(l->x-x)+(l->y-y)*(l->y-y);
					// basic
					//double coef = 1.0-squaredDist*invSquaredRadius;
					// invsqr1
					//double coef=(1.0f/(1.0f+(float)(squaredDist)));
					// invsqr2
					double coef=(1.0f/(1.0f+(float)(squaredDist)/20)- offset)*factor;
					TCODColor *col=&lightmap[x+y*map->getWidth()];
					*col = *col + l->col * coef;
				}					
			}
		}
	}
}