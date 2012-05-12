#include "Geom.h"


Geom::Geom(bool isStatic, Vec2 extends)
{
	this->isStatic = isStatic;
	this->extends = extends;
	this->extends.x -=1e-7f;
	this->extends.y -=1e-7f;
}


Geom::~Geom(void)
{
}
