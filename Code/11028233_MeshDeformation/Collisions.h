#include <iostream>

#ifndef COLLISIONS_H
#define COLLISIONS_H

#define BIT(x) (1<<(x))
enum collisiontypes {
	COL_BOX = 0, //<Collide with nothing
	COL_SHPERE = BIT(0), //<Collide with ships
	COL_GROUND = BIT(1) //<Collide with walls
};

int ShperesCollideWith = COL_BOX;
int boxCollidesWith = COL_GROUND | COL_SHPERE;
int terrainCollidesWith = COL_SHPERE | COL_BOX;
#endif