#include "Geom.h"


Geom::Geom(bool isStatic, bool isGoal, bool isBad, Vec2 extends, unsigned int colorR, unsigned int colorG, unsigned int colorB)
{
	this->isStatic = isStatic;
	this->isGoal = isGoal;
	this->isBad = isBad;
	this->extends = extends;
	this->extends.x -=1e-7f;
	this->extends.y -=1e-7f;
	this->colorR = colorR;
	this->colorG = colorG;
	this->colorB = colorB;
}


Geom::~Geom(void)
{
}
